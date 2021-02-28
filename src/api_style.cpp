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

// IMGUI_API ImFont*       GetFont();                                                      // get current font

DEFINE_API(double, GetFontSize, (ImGui_Context*,ctx),
"Get current font size (= height in pixels) of current font with current scale applied",
{
  Context::check(ctx)->enterFrame();
  return ImGui::GetFontSize();
});

DEFINE_API(void, CalcTextSize, (ImGui_Context*,ctx)
(const char*,text)(double*,API_W(w))(double*,API_W(h))
(bool*,API_RO(hide_text_after_double_hash))(double*,API_RO(wrap_width)),
"Default values: hide_text_after_double_hash = false, wrap_width = -1.0",
{
  Context::check(ctx)->enterFrame();
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
  Context::check(ctx)->enterFrame();

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
  Context::check(ctx)->enterFrame();
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
  Context::check(ctx)->enterFrame();

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

// IMGUI_API ImVec2        GetFontTexUvWhitePixel();                                       // get UV coordinate for a while pixel, useful to draw custom shapes via the ImDrawList API

DEFINE_API(int, GetColor, (ImGui_Context*,ctx)
(int,idx)(double*,API_RO(alpha_mul)),
R"(Retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value (RGBA).

Default values: alpha_mul = 1.0)",
{
  Context::check(ctx)->enterFrame();
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  return Color::abgr2rgba(ImGui::GetColorU32(idx, valueOr(API_RO(alpha_mul), 1.0)));
});

// IMGUI_API ImU32         GetColorU32(ImGuiCol idx, float alpha_mul = 1.0f);              // 
// IMGUI_API ImU32         GetColorU32(const ImVec4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
// IMGUI_API ImU32         GetColorU32(ImU32 col);                                         // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
// IMGUI_API const ImVec4& GetStyleColorVec4(ImGuiCol idx);                                // retrieve style color as stored in ImGuiStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

DEFINE_API(const char*, GetStyleColorName, (int,col_idx),
"Get a string corresponding to the enum value (for display, saving, etc.).",
{
  return ImGui::GetStyleColorName(col_idx);
});

DEFINE_API(void, PushStyleColor, (ImGui_Context*,ctx)
(int,idx)(int,col_rgba),
"Modify a style color. always use this if you modify the style after NewFrame().",
{
  Context::check(ctx)->enterFrame();
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  ImGui::PushStyleColor(idx, ImVec4{Color(col_rgba)});
});

DEFINE_API(void, PopStyleColor, (ImGui_Context*,ctx)
(int*,API_RO(count)),
"Default values: count = 1",
{
  Context::check(ctx)->enterFrame();
  ImGui::PopStyleColor(valueOr(API_RO(count), 1));
});

DEFINE_API(int, ColorConvertHSVtoRGB,
(double,h)(double,s)(double,v)(double*,API_RO(alpha)),
R"(Return 0x00RRGGBB or, if alpha is provided, 0xRRGGBBAA.

Default values: alpha = nil)",
{
  const bool alpha { API_RO(alpha) != nullptr };
  ImVec4 rgba;
  if(alpha)
    rgba.w = *API_RO(alpha);
  ImGui::ColorConvertHSVtoRGB(h, s, v, rgba.x, rgba.y, rgba.z);
  return Color{rgba}.pack(alpha);
});
