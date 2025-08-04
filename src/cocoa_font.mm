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

// Mode of operation:
// 1. Apply generic font name substitution
// 2. Find the font file for the family and style (no exact match on style)
// 3.   -> Find the index of the font exactly matching the style in the file
// 4.      -> If none found, search for a font with a similar style
//            and tell the rasterizer which style(s) were missing.

static NSString *sysUIFontFamily()
{
  // [font fontName] returns ".AppleSystemUIFont" on macOS 10.14 which
  // doesn't resolve when using matchingFontDescriptorWithMandatoryKeys.
  //
  // kCTFontFamilyNameAttribute gives the true name ".SF NS Text" except
  // on 15.3 (+ likely others) where it behaves the same as [font fontName].
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
  return (__bridge_transfer NSString *)CTFontDescriptorCopyAttribute
    ((__bridge CTFontDescriptorRef)desc, kCTFontFamilyNameAttribute);
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

static unsigned int styleToTraits(const int style)
{
  int traits {};
  if(style & ReaImGuiFontFlags_Bold)   traits |= NSFontBoldTrait;
  if(style & ReaImGuiFontFlags_Italic) traits |= NSFontItalicTrait;
  return traits;
}

static std::optional<unsigned int> findExactMatch(
  NSArray *collection, const unsigned int wantTraits)
{
  unsigned short index {};

  for(NSFontDescriptor __strong *font in collection) {
    auto symTraits {[font symbolicTraits]};
    symTraits &= NSFontBoldTrait | NSFontItalicTrait;

    NSDictionary<NSFontDescriptorAttributeName, id> *fontTraits
      {[font objectForKey:NSFontTraitsAttribute]};
    auto weight {[static_cast<NSNumber *>(fontTraits[NSFontWeightTrait]) floatValue]};
    if(weight < NSFontWeightBold)
      symTraits &= ~NSFontBoldTrait;

    if(wantTraits == symTraits) {
      NSDictionary<NSFontDescriptorVariationKey, id> *variation
        {[font objectForKey:NSFontVariationAttribute]};
      return variation ? (index + 1) << 16 : index;
    }

    if(index++ == 0xFFFF)
      break;
  }

  return std::nullopt;
}

static std::pair<unsigned int, int> findClosestMatch(NSURL *url, const int style)
{
  NSArray *collection {(__bridge_transfer NSArray *)
    CTFontManagerCreateFontDescriptorsFromURL((__bridge CFURLRef)url)};

  if(auto index {findExactMatch(collection, styleToTraits(style))})
    return {*index, ReaImGuiFontFlags_None};

  static int fallbacks[] {ReaImGuiFontFlags_Bold, ReaImGuiFontFlags_Italic};

  for(const int fallback : fallbacks) {
    if(auto index {findExactMatch(collection, styleToTraits(fallback))})
      return {*index, style & ~fallback};
  }

  return {0, style};
}

static NSURL *findMatchingFile(NSDictionary *attrs)
{
  NSFontDescriptor *match
    {[NSFontDescriptor fontDescriptorWithFontAttributes:attrs]};
  match = [match matchingFontDescriptorWithMandatoryKeys:[NSSet set]];
  return (__bridge_transfer NSURL *)CTFontDescriptorCopyAttribute
    ((__bridge CTFontDescriptorRef)match, kCTFontURLAttribute);
}

std::optional<FontSource> SysFont::resolve(const unsigned int codepoint) const
{
  NSString *family {translateGenericFont(m_family.c_str())};

  const auto traits {@{
    NSFontSymbolicTrait: [NSNumber numberWithInteger:styleToTraits(m_styles)]
  }};

  NSMutableDictionary *attrs {[NSMutableDictionary dictionaryWithCapacity:2]};
  if(codepoint) {
    auto cs {[NSCharacterSet characterSetWithRange:NSMakeRange(codepoint, 1)]};
    attrs[NSFontCharacterSetAttribute] = cs;
  }
  else
    attrs[NSFontFamilyAttribute] = family;
  attrs[NSFontTraitsAttribute] = traits;

  NSURL *url {findMatchingFile(attrs)};

  if(!url) {
    [attrs removeObjectForKey:NSFontTraitsAttribute];
    url = findMatchingFile(attrs);
    attrs[NSFontTraitsAttribute] = traits;
  }

  if(!url) {
    family = translateGenericFont(SysFont::SANS_SERIF);
    if(codepoint)
      [attrs removeObjectForKey:NSFontCharacterSetAttribute];
    attrs[NSFontFamilyAttribute] = family;
    if(!(url = findMatchingFile(attrs)))
      return std::nullopt;
  }

  FontSource src {[[url path] UTF8String]};
  std::tie(src.m_index, src.m_styles) = findClosestMatch(url, m_styles);
  return src;
}
