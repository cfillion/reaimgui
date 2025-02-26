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
// 1. Apply default font name subtitution
// 2. Find the font file for the family and style (no exact match on style)
// 3.   -> Find the index of the font exactly matching the style in the file
// 4.      -> If none found, search for a font with a similar style
//            and tell the rasterizer which style(s) were missing.

static const char *translateGenericFont(const char *family)
{
  constexpr std::pair<const char *, const char *> genericMap[] {
    {Font::CURSIVE,    "Apple Chancery"},
    {Font::FANTASY,    "Papyrus"       },
    {Font::MONOSPACE,  "Courier"       },
    {Font::SANS_SERIF, "Helvetica"     },
    {Font::SERIF,      "Times"         },
  };

  for(const auto &generic : genericMap) {
    if(!strcasecmp(generic.first, family))
      return generic.second;
  }

  return family;
}

static int styleToTraits(const int style)
{
  int traits {};
  if(style & ReaImGuiFontFlags_Bold)   traits |= NSFontBoldTrait;
  if(style & ReaImGuiFontFlags_Italic) traits |= NSFontItalicTrait;
  return traits;
}

static bool findExactMatch(NSArray *collection, const int style, int *index)
{
  const int traits {styleToTraits(style)};
  NSDictionary<NSFontDescriptorAttributeName, id> *attrs {@{
    NSFontTraitsAttribute: @{
      NSFontSymbolicTrait: [NSNumber numberWithInteger:traits]
    }
  }};

  NSSet *keys
    {[NSSet setWithArray:@[NSFontNameAttribute, NSFontTraitsAttribute]]};

  *index = 0;

  for(NSFontDescriptor __strong *font in collection) {
    font = [font fontDescriptorByAddingAttributes:attrs];
    font = [font matchingFontDescriptorWithMandatoryKeys:keys];

    if(font)
      return true;
    else
      ++*index;
  }

  return false;
}

static std::pair<int, int> findClosestMatch(NSURL *url, const int style)
{
  NSArray *collection {(__bridge_transfer NSArray *)
    CTFontManagerCreateFontDescriptorsFromURL((__bridge CFURLRef)url)};

  int index;
  if(findExactMatch(collection, style, &index))
    return {index, ReaImGuiFontFlags_None};

  static int fallbacks[] {ReaImGuiFontFlags_Bold, ReaImGuiFontFlags_Italic};

  for(const int fallback : fallbacks) {
    if(findExactMatch(collection, fallback, &index))
      return {index, style & ~fallback};
  }

  return {0, style};
}

static NSURL *findMatchingFile(NSDictionary *attrs)
{
  NSFontDescriptor *match
    {[NSFontDescriptor fontDescriptorWithFontAttributes:attrs]};
  return (__bridge_transfer NSURL *)CTFontDescriptorCopyAttribute
    ((__bridge CTFontDescriptorRef)match, kCTFontURLAttribute);
}

bool Font::resolve(const char *family, const int style)
{
  const int traits {styleToTraits(style)};
  family = translateGenericFont(family);

  NSMutableDictionary *attrs {[NSMutableDictionary dictionaryWithCapacity:2]};
  attrs[NSFontFamilyAttribute] = [NSString stringWithUTF8String:family];
  attrs[NSFontTraitsAttribute] = @{
    NSFontSymbolicTrait: [NSNumber numberWithInteger:traits]
  };

  NSURL *url {findMatchingFile(attrs)};

  if(!url) {
    family = translateGenericFont("sans-serif");
    attrs[NSFontFamilyAttribute] = [NSString stringWithUTF8String:family];
    if(!(url = findMatchingFile(attrs)))
      return false;
  }

  m_data = [[url path] UTF8String];
  std::tie(m_index, m_missingStyles) = findClosestMatch(url, style);
  return true;
}
