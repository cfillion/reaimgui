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
Glyphs may contain colors in COLR/CPAL format.

This API currently has multiple limitations (v1.0 blockers):
- ReaImGui rasterizes glyphs only from the Basic Latin and Latin Supplement
  Unicode blocks (U+0020 to U+00FF). UTF-8 is fully supported internally,
  however characters outside those blocks are displayed as '?'.
  See [issue #5](https://github.com/cfillion/reaimgui/issues/5).
- Dear ImGui does not support using new fonts in the middle of a frame.
  Because of this, fonts must first be registered using Attach before any
  other context functions are used in the same defer cycle.
  (Attaching a font is a heavy operation and should ideally be done outside
  of the defer loop.))");

API_FUNC(0_9, Font*, CreateFont,
(const char*,family_or_file) (int,size) (RO<int*>,flags,ReaImGuiFontFlags_None),
R"(Load a font matching a font family name or from a font file.
The font will remain valid while it's attached to a context. See Attach.

The family name can be an installed font or one of the generic fonts:
sans-serif, serif, monospace, cursive, fantasy.

If 'family_or_file' specifies a path to a font file (contains a / or \\):
- The first byte of 'flags' is used as the font index within the file
- The font styles in 'flags' are simulated by the font renderer)")
{
  return new Font {family_or_file, size, API_GET(flags)};
}

API_FUNC(0_9_3, Font*, CreateFontFromMem,
(const char*,data) (int,data_sz) (int,size) (RO<int*>,flags,ReaImGuiFontFlags_None),
R"(Requires REAPER v6.44 or newer for EEL and Lua. Use CreateFont or
explicitely specify data_sz to support older versions.

- The first byte of 'flags' is used as the font index within the file
- The font styles in 'flags' are simulated by the font renderer)")
{
  std::vector<unsigned char> buffer;
  buffer.reserve(data_sz);
  std::copy(data, data + data_sz, std::back_inserter(buffer));
  return new Font {std::move(buffer), size, API_GET(flags)};
}

API_FUNC(0_4, Font*, GetFont, (Context*,ctx),
"Get the current font")
{
  FRAME_GUARD;
  return ctx->fonts().get(ImGui::GetFont());
}

API_FUNC(0_4, void, PushFont, (Context*,ctx)
(Font*,font),
R"(Change the current font. Use nil to push the default font.
The font object must have been registered using Attach. See PopFont.)")
{
  FRAME_GUARD;
  ImGui::PushFont(ctx->fonts().instanceOf(font));
}

API_FUNC(0_4, void, PopFont, (Context*,ctx),
"See PushFont.")
{
  FRAME_GUARD;
  ImGui::PopFont();
}

API_FUNC(0_1, double, GetFontSize, (Context*,ctx),
R"(Get current font size (= height in pixels) of current font with current scale
applied.)")
{
  FRAME_GUARD;
  return ImGui::GetFontSize();
}

API_ENUM(0_4, ReaImGui, FontFlags_None,   "");
API_ENUM(0_4, ReaImGui, FontFlags_Bold,   "");
API_ENUM(0_4, ReaImGui, FontFlags_Italic, "");
