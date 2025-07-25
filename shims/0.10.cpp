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

#include "shims.hpp"

#include "../src/font.hpp"
#include "../src/image.hpp"

#include <imgui/imgui_internal.h>

SHIM("0.10",
  (int, ItemFlags_ButtonRepeat)
  (int, ItemFlags_NoTabStop)
  (void, PushItemFlag, Context*, ImGuiItemFlags, bool)
  (void, PopItemFlag, Context*)

  (void, GetContentRegionAvail, Context*, W<double*>, W<double*>)
  (void, GetCursorScreenPos, Context*, W<double *>, W<double *>)
  (void, GetWindowPos, Context*, W<double *>, W<double *>)

  (int, SelectableFlags_NoAutoClosePopups)
  (int, ChildFlags_Borders)

  (Context*, CreateContext, const char*, RO<int*>)
  (double, GetConfigVar, Context*, int)
  (void, SetConfigVar, Context*, int, double)
  (int, ConfigFlags_NavEnableKeyboard)
  (int, ConfigVar_Flags)
  (int, ConfigVar_DebugHighlightIdConflicts)

  (int, Col_NavCursor)
  (int, SliderFlags_ClampOnInput)
  (int, TreeNodeFlags_SpanLabelWidth)

  (void, GetStyleVar, Context*, int, W<double*>, W<double*>)
  (void, PushStyleVar, Context*, int, double, RO<double*>)
  (void, PushStyleColor, Context*, int, int)
  (void, PopStyleColor, Context*, RO<int*>)
  (void, PopStyleVar, Context*, RO<int*>)
  (int, Col_Border)
  (int, StyleVar_ImageBorderSize)
  (void, ImageWithBg, Context*, Image*, double, double,
    RO<double*>, RO<double*>, RO<double*>, RO<double*>, RO<int*>, RO<int*>)
);

// dear imgui v1.91
SHIM_FUNC(0_1, void, PushButtonRepeat, (Context*,ctx) (bool,repeat))
{
  api.PushItemFlag(ctx, api.ItemFlags_ButtonRepeat(), repeat);
}

SHIM_FUNC(0_8_5, void, PushTabStop, (Context*,ctx) (bool,tab_stop))
{
  api.PushItemFlag(ctx, api.ItemFlags_NoTabStop(), !tab_stop);
}

SHIM_ALIAS(0_1,   PopButtonRepeat, PopItemFlag);
SHIM_ALIAS(0_8_5, PopTabStop, PopItemFlag);

SHIM_FUNC(0_1, void, GetContentRegionMax,
  (Context*,ctx) (W<double*>,x) (W<double*>,y))
{
  double dx, dy;
  api.GetContentRegionAvail(ctx, x, y);
  api.GetCursorScreenPos(ctx, &dx, &dy);
  if(x) *x += dx;
  if(y) *y += dy;
  api.GetWindowPos(ctx, &dx, &dy);
  if(x) *x -= dx;
  if(y) *y -= dy;
}

SHIM_FUNC(0_1, void, GetWindowContentRegionMin,
  (Context*,ctx) (W<double*>,x) (W<double*>,y))
{
  FRAME_GUARD;

  ImGuiWindow *window {ctx->imgui()->CurrentWindow};
  if(x) *x = window->ContentRegionRect.Min.x - window->Pos.x;
  if(y) *y = window->ContentRegionRect.Min.y - window->Pos.y;
}

SHIM_FUNC(0_1, void, GetWindowContentRegionMax,
  (Context*,ctx) (W<double*>,x) (W<double*>,y))
{
  FRAME_GUARD;

  ImGuiWindow *window {ctx->imgui()->CurrentWindow};
  if(x) *x = window->ContentRegionRect.Max.x - window->Pos.x;
  if(y) *y = window->ContentRegionRect.Max.y - window->Pos.y;
}

SHIM_ALIAS(0_1, SelectableFlags_DontClosePopups, SelectableFlags_NoAutoClosePopups);

// dear imgui v1.91.1
SHIM_ALIAS(0_9, ChildFlags_Border, ChildFlags_Borders);

// dear imgui v1.91.2
SHIM_FUNC(0_5, Context*, CreateContext,
(const char*,label) (RO<int*>,config_flags,0))
{
  Context *ctx {api.CreateContext(label, config_flags)};
  api.SetConfigVar(ctx, api.ConfigVar_DebugHighlightIdConflicts(), false);

  if(!(API_GET(config_flags) & api.ConfigFlags_NavEnableKeyboard())) {
    int flags {static_cast<int>(api.GetConfigVar(ctx, api.ConfigVar_Flags()))};
    flags &= ~api.ConfigFlags_NavEnableKeyboard();
    api.SetConfigVar(ctx, api.ConfigVar_Flags(), flags);
  }

  return ctx;
}

// dear imgui v1.91.3
SHIM_ALIAS(0_1, SliderFlags_AlwaysClamp, SliderFlags_ClampOnInput);

// dear imgui v1.91.4
SHIM_ALIAS(0_1, Col_NavHighlight, Col_NavCursor);
// no known usage in public search results
SHIM_CONST(0_1, ConfigFlags_NavEnableSetMousePos, 0);
SHIM_CONST(0_8, ConfigFlags_NavNoCaptureKeyboard, 0);

// dear imgui v1.91.7
SHIM_ALIAS(0_9_1, TreeNodeFlags_SpanTextWidth, TreeNodeFlags_SpanLabelWidth);

// dear imgui v1.91.8
SHIM_CONST(0_1, ColorEditFlags_AlphaPreview, 0); // no replacement

// dear imgui v1.91.9
SHIM_FUNC(0_8, void, Image, (Context*,ctx) (class Image*,image)
  (double,image_size_w) (double,image_size_h)
  (RO<double*>,uv0_x) (RO<double*>,uv0_y)
  (RO<double*>,uv1_x) (RO<double*>,uv1_y)
  (RO<int*>,tint_col_rgba/*,0xFFFFFFFF*/) (RO<int*>,border_col_rgba,0x00000000))
{
  // Copied from upstream's own temporary shim
  // Preserve behavior where border is always visible when border_col's Alpha is >0.0f
  double image_border_size;
  api.GetStyleVar(ctx, api.StyleVar_ImageBorderSize(), &image_border_size, nullptr);

  const double border_size
    {API_GET(border_col_rgba) & 0xFF ? std::max(1.0, image_border_size) : 0.0f};
  api.PushStyleVar(ctx, api.StyleVar_ImageBorderSize(), border_size, nullptr);
  api.PushStyleColor(ctx, api.Col_Border(), API_GET(border_col_rgba));
  api.ImageWithBg(ctx, image, image_size_w, image_size_h,
    uv0_x, uv0_y, uv1_x, uv1_y, 0, tint_col_rgba);
  api.PopStyleColor(ctx, nullptr);
  api.PopStyleVar(ctx, nullptr);
}

// dear imgui v1.92.0
enum { FONT_INDEX_MASK = 0xFF, FONT_STYLE_MASK = ~0xFF };

SHIM_FUNC(0_9, Font*, CreateFont,
(const char*,family_or_file) (int,size) (RO<int*>,flags,ReaImGuiFontFlags_None))
{
  const int index {API_GET(flags) & FONT_INDEX_MASK};
  const int style {API_GET(flags) & FONT_STYLE_MASK};

  Font *font;
  if(strpbrk(family_or_file, "/\\"))
    font = new Font {family_or_file, index, style};
  else
    font = new Font {family_or_file, style};
  font->setLegacySize(size);
  return font;
}

SHIM_FUNC(0_9_3, Font*, CreateFontFromMem,
(const char*,data) (int,data_sz) (int,size) (RO<int*>,flags,ReaImGuiFontFlags_None))
{
  std::vector<unsigned char> buffer;
  buffer.reserve(data_sz);
  std::copy(data, data + data_sz, std::back_inserter(buffer));
  const int index {API_GET(flags) & FONT_INDEX_MASK};
  const int style {API_GET(flags) & FONT_STYLE_MASK};
  Font *font {new Font {std::move(buffer), index, style}};
  font->setLegacySize(size);
  return font;
}

SHIM_FUNC(0_4, void, PushFont, (Context*,ctx) (Font*,font))
{
  FRAME_GUARD;

  ImFont *imfont;
  if(font) {
    assertValid(font);
    imfont = font->instance(ctx);
  }
  else
    imfont = ImGui::GetDefaultFont();

  // TODO: use api.GetDefaultFont once implemented (and api.PushFont)
  ImGui::PushFont(imfont, imfont->LegacySize);
}
