/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#include "font.hpp"

API_SECTION("Font");

DEFINE_API(ImGui_Font*, CreateFont,
(const char*,family_or_file)(int,size)(int*,API_RO(flags)),
R"(Load a font matching a font family name or from a font file. The font will remain valid while it's attached to a context. See ImGui_AttachFont.

The family name can be an installed font or one of the generic fonts: sans-serif, serif, monospace, cursive, fantasy.

If 'family_or_file' specifies a path to a font file (contains a / or \):
- The first byte of 'flags' is used as the font index within the file
- The font styles in 'flags' are simulated by the font renderer

Default values: flags = ImGui_FontFlags_None)",
{
  const int flags { valueOr(API_RO(flags), ReaImGuiFontFlags_None) };
  return new Font { family_or_file, size, flags };
});

static void outOfFrameCheck(Context *ctx)
{
  if(ctx->inFrame())
    throw reascript_error { "cannot modify font texture: a frame has already begun" };
}

DEFINE_API(void, AttachFont, (ImGui_Context*,ctx)
(ImGui_Font*,font),
"Enable a font for use in the given context. Fonts must be attached as soon as possible after creating the context or on a new defer cycle.",
{
  assertValid(ctx);
  assertValid(font);
  outOfFrameCheck(ctx);

  ctx->fonts().add(font);
});

DEFINE_API(void, DetachFont, (ImGui_Context*,ctx)
(ImGui_Font*,font),
"Unload a font from the given context. The font will be destroyed if is not attached to any context.",
{
  assertValid(ctx);
  assertValid(font);
  outOfFrameCheck(ctx);

  ctx->fonts().remove(font);
});

DEFINE_API(ImGui_Font*, GetFont, (ImGui_Context*,ctx),
"Get the current font",
{
  FRAME_GUARD;
  return ctx->fonts().get(ImGui::GetFont());
});

DEFINE_API(void, PushFont, (ImGui_Context*,ctx)
(ImGui_Font*,font),
"Change the current font. Use nil to push the default font. See ImGui_PopFont.",
{
  FRAME_GUARD;
  ImGui::PushFont(ctx->fonts().instanceOf(font));
});

DEFINE_API(void, PopFont, (ImGui_Context*,ctx),
"See ImGui_PushFont.",
{
  FRAME_GUARD;
  ImGui::PopFont();
});

DEFINE_API(double, GetFontSize, (ImGui_Context*,ctx),
"Get current font size (= height in pixels) of current font with current scale applied",
{
  FRAME_GUARD;
  return ImGui::GetFontSize();
});

DEFINE_ENUM(ReaImGui, FontFlags_None, "");
DEFINE_ENUM(ReaImGui, FontFlags_Bold, "");
DEFINE_ENUM(ReaImGui, FontFlags_Italic, "");
