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

#include "helper.hpp"

#include "../src/font.hpp"

API_SECTION("Font",
R"(Supports loading fonts from the system by family name or from a file.
Glyphs may contain colors in COLR/CPAL format.)");

API_FUNC(0_10, Font*, CreateFont,
(const char*,family) (RO<int*>,flags,ReaImGuiFontFlags_None),
R"(Load a font matching a font family name.

The family name can be an installed font or one of the generic fonts:
sans-serif, serif, monospace, cursive, fantasy.

See CreateFontFromFile.)")
{
  return new SysFont {family, API_GET(flags)};
}

API_FUNC(0_10, Font*, CreateFontFromFile,
(const char*,file) (RO<int*>,index,0) (RO<int*>,flags,ReaImGuiFontFlags_None),
R"(Load a font from a file. Available characters are limited to those
contained in the file.

Bits 0-15 of 'index' are the the index of the face in the font file (starting
from 0). Set to 0 if the font file contains only one font face.
Bits 16-30 (for TrueType GX and OpenType Font Variations only) specify the
named instance index for the current face index (starting from 1).
0 ignores named instances.

The font styles in 'flags' are simulated by the rasterizer.
See also CreateFontFromMem.)")
{
  return new Font {file,
    static_cast<unsigned int>(API_GET(index)), API_GET(flags)};
}

API_FUNC(0_10, Font*, CreateFontFromMem,
(const char*,data) (int,data_sz)
(RO<int*>,index,0) (RO<int*>,flags,ReaImGuiFontFlags_None),
R"(Requires REAPER v6.44 or newer for EEL and Lua. Use CreateFont or
explicitely specify data_sz to support older versions.

See CreateFontFromFile for the meaning of 'index' and 'flags'.)")
{
  std::vector<unsigned char> buffer;
  buffer.reserve(data_sz);
  std::copy(data, data + data_sz, std::back_inserter(buffer));
  return new Font {std::move(buffer),
    static_cast<unsigned int>(API_GET(index)), API_GET(flags)};
}

API_FUNC(0_4, Font*, GetFont, (Context*,ctx),
"Get the current font")
{
  FRAME_GUARD;
  return static_cast<Font *>(ImGui::GetFont()->Sources.front()->UserData);
}

API_FUNC(0_10, void, PushFont, (Context*,ctx)
(Font*,font) (double,font_size_base_unscaled),
R"(Change the current font. Pass font=nil to only change the size. See PopFont.)")
{
  FRAME_GUARD;
  ImFont *imfont = nullptr;
  if(font) {
    assertValid(font);
    imfont = font->instance(ctx);
  }
  ImGui::PushFont(imfont, font_size_base_unscaled);
}

API_FUNC(0_4, void, PopFont, (Context*,ctx),
"See PushFont.")
{
  FRAME_GUARD;
  ImGui::PopFont();
}

API_FUNC(0_1, double, GetFontSize, (Context*,ctx),
R"(Get current font size (= height in pixels) of current font with current scale
applied. See also GetTextLineHeight and GetFrameHeight.)")
{
  FRAME_GUARD;
  return ImGui::GetFontSize(); // ctx->style().FontSizeBase?
}

API_ENUM_NS(0_4, ReaImGui, FontFlags_None,   "");
API_ENUM_NS(0_4, ReaImGui, FontFlags_Bold,   "");
API_ENUM_NS(0_4, ReaImGui, FontFlags_Italic, "");
