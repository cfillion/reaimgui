/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "font.hpp"

#include <AppKit/AppKit.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

// NSFontWeight* constants are only available on macOS 10.11+
constexpr float FONT_WEIGHT_BOLD {0.4f};

struct SysFontMatch {
  NSFontDescriptor *desc;
  NSURL *url;
  operator bool() const { return desc && url; }
};

static NSString *sysUIFontFamily()
{
  // [font familyName] returns ".AppleSystemUIFont" on macOS 10.14 which
  // doesn't resolve when using matchingFontDescriptorWithMandatoryKeys.
  //
  // NSFontFamilyAttribute gives the true name ".SF NS Text" except on macOS 15
  // (& likely others) where it behaves the same as [font familyName].
  //
  // The extra roundtrip to the font file is required to avoid this.

  NSFont *font {[NSFont systemFontOfSize:9]};
  NSFontDescriptor *desc
    {[NSFontDescriptor fontDescriptorWithName:[font fontName]
                                         size:[font pointSize]]};
  NSURL *url {(__bridge_transfer NSURL *)CTFontDescriptorCopyAttribute
    ((__bridge CTFontDescriptorRef)desc, kCTFontURLAttribute)};
  NSArray *collection {(__bridge_transfer NSArray *)
    CTFontManagerCreateFontDescriptorsFromURL((__bridge CFURLRef)url)};
  if(!(desc = [collection firstObject]))
    return @"Helvetica Neue";
  return [desc objectForKey:NSFontFamilyAttribute];
}

static NSString *translateGenericFont(const char *family)
{
  static const std::pair<const char *, NSString *> genericMap[] {
    {SysFont::SANS_SERIF, sysUIFontFamily()},
    {SysFont::SERIF,      @"Times"         },
    {SysFont::MONOSPACE,  @"Courier"       },
    {SysFont::CURSIVE,    @"Apple Chancery"},
    {SysFont::FANTASY,    @"Papyrus"       },
  };

  for(const auto &generic : genericMap) {
    if(!strcasecmp(generic.first, family))
      return generic.second;
  }

  return [NSString stringWithUTF8String:family];
}

static NSNumber *styleToSymTraits(const int style)
{
  int traits {};
  if(style & ReaImGuiFontFlags_Bold)   traits |= NSFontBoldTrait;
  if(style & ReaImGuiFontFlags_Italic) traits |= NSFontItalicTrait;
  return [NSNumber numberWithUnsignedInt:traits];
}

static std::pair<unsigned int, int> indexForMatch(unsigned int index,
  NSFontDescriptor *desc, int missingStyles)
{
  NSDictionary<NSFontDescriptorVariationKey, id> *variation
    {[desc objectForKey:NSFontVariationAttribute]};
  if(variation)
    ++index, index <<= 16;

  // Getting the traits through NSFont becomes less expensive than via
  // [NSFontDescriptor objectForKey:NSFontTraitsAttribute] after the first call.
  // On macOS 13+ with SFNS.ttf, reading the traits parses JSON metadata!
  using NSFontTraitsDictionary = NSDictionary<NSFontDescriptorAttributeName, id>;
  NSFont *font {[NSFont fontWithDescriptor:desc size:9]};
  auto traits {(__bridge_transfer NSFontTraitsDictionary *)
    CTFontCopyTraits((__bridge CTFontRef)font)};

  if([traits[NSFontWeightTrait] floatValue] >= FONT_WEIGHT_BOLD)
    missingStyles &= ~ReaImGuiFontFlags_Bold;
  if([traits[NSFontSlantTrait] floatValue] > 0.f)
    missingStyles &= ~ReaImGuiFontFlags_Italic;

  return {index, missingStyles};
}

static std::pair<unsigned int, int> findIndex(
  const SysFontMatch target, const int styles)
{
  NSArray *collection {(__bridge_transfer NSArray *)
    CTFontManagerCreateFontDescriptorsFromURL((__bridge CFURLRef)target.url)};
  const auto count {std::min<unsigned long>([collection count], 0xFFFF)};

  NSString *targetName {[target.desc objectForKey:NSFontNameAttribute]};
  for(unsigned short i {}; i < count; ++i) {
    NSFontDescriptor *desc {collection[i]};
    if([[desc objectForKey:NSFontNameAttribute] isEqualToString:targetName])
      return indexForMatch(i, desc, styles);
  }

  return {0, styles};
}

static SysFontMatch findMatchingFile(NSDictionary *attrs)
{
  NSFontDescriptor *font
    {[NSFontDescriptor fontDescriptorWithFontAttributes:attrs]};
  font = [font matchingFontDescriptorWithMandatoryKeys:[NSSet set]];
  return {font, (__bridge_transfer NSURL *)CTFontDescriptorCopyAttribute
    ((__bridge CTFontDescriptorRef)font, kCTFontURLAttribute)};
}

void SysFont::initPlatform()
{
}

std::optional<FontSource> SysFont::resolve(const unsigned int codepoint) const
{
  NSMutableDictionary *traits {[NSMutableDictionary dictionaryWithCapacity:3]};
  traits[NSFontSymbolicTrait] = styleToSymTraits(m_styles);
  if(m_styles & ReaImGuiFontFlags_Bold)
    traits[NSFontWeightTrait] = [NSNumber numberWithFloat:FONT_WEIGHT_BOLD];
  if(!(m_styles & ReaImGuiFontFlags_Italic))
    traits[NSFontSlantTrait] = @0.f;

  NSMutableDictionary *attrs {[NSMutableDictionary dictionaryWithCapacity:2]};
  if(codepoint) {
    auto cs {[NSCharacterSet characterSetWithRange:NSMakeRange(codepoint, 1)]};
    attrs[NSFontCharacterSetAttribute] = cs;
  }
  else
    attrs[NSFontFamilyAttribute] = translateGenericFont(m_family.c_str());
  attrs[NSFontTraitsAttribute] = traits;

  SysFontMatch match {findMatchingFile(attrs)};

  if(!match) {
    [attrs removeObjectForKey:NSFontTraitsAttribute];
    match = findMatchingFile(attrs);
    attrs[NSFontTraitsAttribute] = traits;
  }

  if(!match) {
    if(codepoint)
      [attrs removeObjectForKey:NSFontCharacterSetAttribute];
    attrs[NSFontFamilyAttribute] = translateGenericFont(SysFont::SANS_SERIF);
    if(!(match = findMatchingFile(attrs)))
      return std::nullopt;
  }

  FontSource src {[[match.url path] UTF8String]};
  std::tie(src.m_index, src.m_styles) = findIndex(match, m_styles);
  return src;
}
