/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
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

#include "api_helper.hpp"

#include <algorithm>

enum class StyleVarType { Unknown, Float, ImVec2 };

static StyleVarType styleVarType(const ImGuiStyleVar var)
{
  constexpr ImGuiStyleVar floatVars[] {
    ImGuiStyleVar_Alpha,
    ImGuiStyleVar_ChildBorderSize,
    ImGuiStyleVar_ChildRounding,
    ImGuiStyleVar_FrameBorderSize,
    ImGuiStyleVar_FrameRounding,
    ImGuiStyleVar_GrabMinSize,
    ImGuiStyleVar_GrabRounding,
    ImGuiStyleVar_IndentSpacing,
    ImGuiStyleVar_PopupBorderSize,
    ImGuiStyleVar_PopupRounding,
    ImGuiStyleVar_ScrollbarRounding,
    ImGuiStyleVar_ScrollbarSize,
    ImGuiStyleVar_TabRounding,
    ImGuiStyleVar_WindowBorderSize,
    ImGuiStyleVar_WindowRounding,
  };

  constexpr ImGuiStyleVar vec2Vars[] {
    ImGuiStyleVar_ButtonTextAlign,
    ImGuiStyleVar_SelectableTextAlign,
    ImGuiStyleVar_CellPadding,
    ImGuiStyleVar_ItemSpacing,
    ImGuiStyleVar_ItemInnerSpacing,
    ImGuiStyleVar_FramePadding,
    ImGuiStyleVar_WindowPadding,
    ImGuiStyleVar_WindowMinSize,
    ImGuiStyleVar_WindowTitleAlign,
  };

  if(std::find(std::begin(floatVars), std::end(floatVars), var) != std::end(floatVars))
    return StyleVarType::Float;
  else if(std::find(std::begin(vec2Vars), std::end(vec2Vars), var) != std::end(vec2Vars))
    return StyleVarType::ImVec2;
  else
    return StyleVarType::Unknown;
}

DEFINE_API(double, GetFontSize, (ImGui_Context*,ctx),
"Get current font size (= height in pixels) of current font with current scale applied",
{
  FRAME_GUARD;
  return ImGui::GetFontSize();
});

DEFINE_API(void, CalcTextSize, (ImGui_Context*,ctx)
(const char*,text)(double*,API_W(w))(double*,API_W(h))
(bool*,API_RO(hide_text_after_double_hash))(double*,API_RO(wrap_width)),
"Default values: hide_text_after_double_hash = false, wrap_width = -1.0",
{
  FRAME_GUARD;
  const ImVec2 &size {
    ImGui::CalcTextSize(text, nullptr,
      valueOr(API_RO(hide_text_after_double_hash), false),
      valueOr(API_RO(wrap_width), -1.0))
  };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
});

DEFINE_API(void, PushStyleVar, (ImGui_Context*,ctx)
(int,var_idx)(double,val1)(double*,API_RO(val2)),
R"(See ImGui_StyleVar_* for possible values of 'var_idx'.

Default values: val2 = nil)",
{
  FRAME_GUARD;

  switch(styleVarType(var_idx)) {
  case StyleVarType::Unknown:
    throw reascript_error { "unknown style variable" };
  case StyleVarType::Float:
    if(API_RO(val2))
      throw reascript_error { "second value ignored for this variable" };
    ImGui::PushStyleVar(var_idx, val1);
    break;
  case StyleVarType::ImVec2:
    if(!API_RO(val2))
      throw reascript_error { "this variable requires two values" };
    ImGui::PushStyleVar(var_idx, ImVec2(val1, *API_RO(val2)));
    break;
  }
});

DEFINE_API(void, PopStyleVar, (ImGui_Context*,ctx)
(int*,API_RO(count)),
R"(Reset a style variable.

Default values: count = 1)",
{
  FRAME_GUARD;
  ImGui::PopStyleVar(valueOr(API_RO(count), 1));
});

#define CASE_FLOAT_VAR(var)                   \
  case ImGuiStyleVar_##var:                   \
    if(API_W(val1)) *API_W(val1) = style.var; \
    break;

#define CASE_IMVEC2_VAR(var)                    \
  case ImGuiStyleVar_##var:                     \
    if(API_W(val1)) *API_W(val1) = style.var.x; \
    if(API_W(val2)) *API_W(val2) = style.var.y; \
    break;

DEFINE_API(void, GetStyleVar, (ImGui_Context*,ctx)
(int,var_idx)(double*,API_W(val1))(double*,API_W(val2)),
"",
{
  FRAME_GUARD;

  const ImGuiStyle &style { ImGui::GetStyle() };

  switch(var_idx) {
  CASE_FLOAT_VAR(Alpha)
  CASE_FLOAT_VAR(ChildBorderSize)
  CASE_FLOAT_VAR(ChildRounding)
  CASE_FLOAT_VAR(FrameBorderSize)
  CASE_FLOAT_VAR(FrameRounding)
  CASE_FLOAT_VAR(GrabMinSize)
  CASE_FLOAT_VAR(GrabRounding)
  CASE_FLOAT_VAR(IndentSpacing)
  CASE_FLOAT_VAR(PopupBorderSize)
  CASE_FLOAT_VAR(PopupRounding)
  CASE_FLOAT_VAR(ScrollbarRounding)
  CASE_FLOAT_VAR(ScrollbarSize)
  CASE_FLOAT_VAR(TabRounding)
  CASE_FLOAT_VAR(WindowBorderSize)
  CASE_FLOAT_VAR(WindowRounding)

  CASE_IMVEC2_VAR(ButtonTextAlign)
  CASE_IMVEC2_VAR(SelectableTextAlign)
  CASE_IMVEC2_VAR(CellPadding)
  CASE_IMVEC2_VAR(ItemSpacing)
  CASE_IMVEC2_VAR(ItemInnerSpacing)
  CASE_IMVEC2_VAR(FramePadding)
  CASE_IMVEC2_VAR(WindowPadding)
  CASE_IMVEC2_VAR(WindowMinSize)
  CASE_IMVEC2_VAR(WindowTitleAlign)
  default:
    throw reascript_error { "unknown style variable" };
  }
});

#undef CASE_FLOAT_VAR
#undef CASE_IMVEC2_VAR

DEFINE_API(int, GetColor, (ImGui_Context*,ctx)
(int,idx)(double*,API_RO(alpha_mul)),
R"(Retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value (RGBA). See ImGui_Col_* for available style colors.

Default values: alpha_mul = 1.0)",
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  return Color::abgr2rgba(ImGui::GetColorU32(idx, valueOr(API_RO(alpha_mul), 1.0)));
});

DEFINE_API(int, GetColorEx, (ImGui_Context*,ctx)
(int,col_rgba),
"Retrieve given color with style alpha applied, packed as a 32-bit value (RGBA).",
{
  FRAME_GUARD;
  return Color::abgr2rgba(ImGui::GetColorU32(Color::rgba2abgr(col_rgba)));
});

DEFINE_API(int, GetStyleColor, (ImGui_Context*,ctx)
(int,idx),
"Retrieve style color as stored in ImGuiStyle structure. Use to feed back into PushStyleColor(), Otherwise use ImGui_GetColor() to get style color with style alpha baked in. See ImGui_Col_* for available style colors.",
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  const ImVec4 &col { ImGui::GetStyleColorVec4(idx) };
  return Color{col}.pack();
});

DEFINE_API(const char*, GetStyleColorName, (int,idx),
"Get a string corresponding to the enum value (for display, saving, etc.).",
{
  return ImGui::GetStyleColorName(idx);
});

DEFINE_API(void, PushStyleColor, (ImGui_Context*,ctx)
(int,idx)(int,col_rgba),
"Modify a style color. Call ImGui_PopStyleColor to undo after use (before the end of the frame). See ImGui_Col_* for available style colors.",
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  ImGui::PushStyleColor(idx, ImVec4{Color(col_rgba)});
});

DEFINE_API(void, PopStyleColor, (ImGui_Context*,ctx)
(int*,API_RO(count)),
"Default values: count = 1",
{
  FRAME_GUARD;
  ImGui::PopStyleColor(valueOr(API_RO(count), 1));
});

DEFINE_API(int, ColorConvertHSVtoRGB,
(double,h)(double,s)(double,v)(double*,API_RO(alpha))
(double*,API_W(r))(double*,API_W(g))(double*,API_W(b)),
R"(Return 0x00RRGGBB or, if alpha is provided, 0xRRGGBBAA.

Default values: alpha = nil)",
{
  const bool alpha { API_RO(alpha) != nullptr };
  ImVec4 color;
  if(alpha)
    color.w = *API_RO(alpha);
  ImGui::ColorConvertHSVtoRGB(h, s, v, color.x, color.y, color.z);
  if(API_W(r)) *API_W(r) = color.x;
  if(API_W(g)) *API_W(g) = color.y;
  if(API_W(b)) *API_W(b) = color.z;
  return Color{color}.pack(alpha);
});

DEFINE_API(int, ColorConvertRGBtoHSV,
(double,r)(double,g)(double,b)(double*,API_RO(alpha))
(double*,API_W(h))(double*,API_W(s))(double*,API_W(v)),
R"(Return 0x00HHSSVV or, if alpha is provided, 0xHHSSVVAA.

Default values: alpha = nil)",
{
  const bool alpha { API_RO(alpha) != nullptr };
  ImVec4 color;
  if(alpha)
    color.w = *API_RO(alpha);
  ImGui::ColorConvertRGBtoHSV(r, g, b, color.x, color.y, color.z);
  if(API_W(h)) *API_W(h) = color.x;
  if(API_W(s)) *API_W(s) = color.y;
  if(API_W(v)) *API_W(v) = color.z;
  return Color{color}.pack(alpha);
});

DEFINE_API(int, ColorConvertNative,
(int,rgb),
"Convert native colors coming from REAPER. This swaps the red and blue channels of the specified 0xRRGGBB color on Windows.",
{
  return Color::convertNative(rgb);
});
