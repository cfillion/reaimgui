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

#include <windows.h>

class SelectFont {
public:
  SelectFont(LOGFONT *);
  ~SelectFont();

  operator bool() const { return !!m_font; }
  HDC dc() const { return m_dc; }

private:
  HDC m_dc;
  HFONT m_font;
};

SelectFont::SelectFont(LOGFONT *desc)
  : m_dc {CreateCompatibleDC(nullptr)}, m_font {CreateFontIndirect(desc)}
{
  SelectObject(m_dc, m_font);
}

SelectFont::~SelectFont()
{
  DeleteDC(m_dc);
  DeleteObject(m_font);
}

static bool addGenericAttrs(const char *family, LOGFONT *desc)
{
  constexpr std::pair<const char *, unsigned char> genericMap[] {
    {Font::CURSIVE,    FF_SCRIPT    },
    {Font::FANTASY,    FF_DECORATIVE},
    {Font::MONOSPACE,  FF_MODERN    },
    {Font::SANS_SERIF, FF_SWISS     },
    {Font::SERIF,      FF_ROMAN     },
  };

  for(const auto &generic : genericMap) {
    if(!_stricmp(family, generic.first)) {
      desc->lfPitchAndFamily = generic.second;
      return true;
    }
  }

  return false;
}

static int CALLBACK enumStyles(const LOGFONT *desc,
  const TEXTMETRIC *, DWORD, LPARAM lParam)
{
  int *missingStyles {reinterpret_cast<int *>(lParam)};
  if(desc->lfWeight > FW_NORMAL) *missingStyles &= ~ReaImGuiFontFlags_Bold;
  if(desc->lfItalic) *missingStyles &= ~ReaImGuiFontFlags_Italic;
  return 1;
}

bool Font::resolve(const char *family, const int style)
{
  LOGFONT desc {
    .lfWeight       = style & ReaImGuiFontFlags_Bold ? FW_BOLD : FW_NORMAL,
    .lfItalic       = !!(style & ReaImGuiFontFlags_Italic),
    .lfOutPrecision = OUT_TT_ONLY_PRECIS,
  };
  if(!addGenericAttrs(family, &desc))
    MultiByteToWideChar(CP_UTF8, 0, family, -1, desc.lfFaceName, LF_FACESIZE - 1);

  SelectFont sel {&desc};
  if(!sel)
    return false;

  const DWORD dataSize {GetFontData(sel.dc(), 0, 0, nullptr, 0)};
  if(dataSize == GDI_ERROR)
    return false;
  std::vector<unsigned char> fontData(dataSize);
  GetFontData(sel.dc(), 0, 0, fontData.data(), fontData.size());
  m_data = std::move(fontData);
  m_index = 0;

  m_missingStyles = style;
  GetTextFace(sel.dc(), LF_FACESIZE, desc.lfFaceName);
  EnumFontFamiliesEx(sel.dc(), &desc, &enumStyles,
    reinterpret_cast<LPARAM>(&m_missingStyles), 0);

  return true;
}
