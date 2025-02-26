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

#include <fontconfig/fontconfig.h>

class FontConfig {
public:
  FontConfig() : m_fc {FcInitLoadConfigAndFonts()} {}
  FontConfig(const FontConfig &) = delete;
  ~FontConfig() { FcConfigDestroy(m_fc); }

  operator FcConfig *() { return m_fc; }

private:
  FcConfig *m_fc;
};

class FontPattern {
public:
  FontPattern(FcPattern *p = FcPatternCreate()) : m_pattern {p} {}
  FontPattern(const FontPattern &) = delete;
  ~FontPattern() { FcPatternDestroy(m_pattern); }

  operator bool() const { return m_pattern; }

  template<typename T>
  T get(const char *object, int n = 0) const;

  template<typename T>
  bool add(const char *object, T val);

  FontPattern bestMatch(FcConfig * = nullptr);

private:
  FcPattern *m_pattern;
};

template<>
const char *FontPattern::get(const char *object, const int n) const
{
  FcChar8 *value;
  if(FcPatternGetString(m_pattern, object, n, &value) == FcResultMatch)
    return reinterpret_cast<const char *>(value);
  return nullptr;
}

template<>
int FontPattern::get(const char *object, const int n) const
{
  int value;
  if(FcPatternGetInteger(m_pattern, object, n, &value) == FcResultMatch)
    return value;
  return 0;
}

template<>
bool FontPattern::get(const char *object, const int n) const
{
  int value;
  if(FcPatternGetBool(m_pattern, object, n, &value) == FcResultMatch)
    return value;
  return false;
}

template<>
bool FontPattern::add(const char *object, const char *val)
{
  return FcPatternAddString(m_pattern, object,
    reinterpret_cast<const FcChar8 *>(val));
}

template<>
bool FontPattern::add(const char *object, const int val)
{
  return FcPatternAddInteger(m_pattern, object, val);
}

FontPattern FontPattern::bestMatch(FcConfig *fc)
{
  FcConfigSubstitute(fc, m_pattern, FcMatchPattern);
  FcDefaultSubstitute(m_pattern);

  FcResult result;
  return {FcFontMatch(fc, m_pattern, &result)};
}

bool Font::resolve(const char *family, const int style)
{
  FontConfig fc;

  FontPattern query;
  query.add(FC_FAMILY, family);
  query.add(FC_WEIGHT,
    style & ReaImGuiFontFlags_Bold ? FC_WEIGHT_BOLD : FC_WEIGHT_NORMAL);
  query.add(FC_SLANT,
    style & ReaImGuiFontFlags_Italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);

  const FontPattern &font {query.bestMatch(fc)};
  if(!font)
    return false;

  m_data = font.get<const char *>(FC_FILE);
  m_index = font.get<int>(FC_INDEX);

  // FC_WEIGHT is bold if requested in the query even if the chosen font doesn't
  // support that style. FC_EMBOLDEN is true in those cases.
  m_missingStyles = style;
  if(font.get<int>(FC_WEIGHT) > FC_WEIGHT_NORMAL && !font.get<bool>(FC_EMBOLDEN))
    m_missingStyles &= ~ReaImGuiFontFlags_Bold;
  if(font.get<int>(FC_SLANT) == FC_SLANT_ITALIC)
    m_missingStyles &= ~ReaImGuiFontFlags_Italic;

  return true;
}
