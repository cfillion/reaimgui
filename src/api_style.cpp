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

DEFINE_API(double, GetFontSize, ((ImGui_Context*,ctx)),
"Get current font size (= height in pixels) of current font with current scale applied",
{
  Context::check(ctx)->enterFrame();
  return ImGui::GetFontSize();
});

DEFINE_API(bool, PushStyleVar, ((ImGui_Context*,ctx))
((int,varIdx))((double,val1))((double*,API_RO(val2))),
"See ImGui_StyleVar_* for possible values of 'varIdx'.",
{
  Context::check(ctx)->enterFrame();

  switch(styleVarType(varIdx)) {
  case StyleVarType::Unknown:
    return false;
  case StyleVarType::Float:
    ImGui::PushStyleVar(varIdx, val1);
    return true;
  case StyleVarType::ImVec2:
    if(!API_RO(val2))
      return false;
    ImGui::PushStyleVar(varIdx, ImVec2(val1, *API_RO(val2)));
    return true;
  }

  return false;
});

DEFINE_API(void, PopStyleVar, ((ImGui_Context*,ctx))
((int*,API_RO(count))),
R"(Reset a style variable.

Default values: count = 1)",
{
  Context::check(ctx)->enterFrame();

  ImGui::PopStyleVar(valueOr(API_RO(count), 1));
});

#define CASE_FLOAT_VAR(var)   \
  case ImGuiStyleVar_##var:   \
    if(!API_W(val1))          \
      return false;           \
    *API_W(val1) = style.var; \
    return true;

#define CASE_IMVEC2_VAR(var)         \
  case ImGuiStyleVar_##var:          \
    if(!API_W(val1) || !API_W(val2)) \
      return false;                  \
    *API_W(val1) = style.var.x;      \
    *API_W(val2) = style.var.y;      \
    return true;

DEFINE_API(bool, GetStyleVar, ((ImGui_Context*,ctx))
((int,varIdx))((double*,API_W(val1)))((double*,API_W(val2))),
"",
{
  Context::check(ctx)->enterFrame(); // TODO: don't start a frame

  const ImGuiStyle &style { ImGui::GetStyle() };

  switch(varIdx) {
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
    return false;
  }
});

#undef CASE_FLOAT_VAR
#undef CASE_IMVEC2_VAR

// IMGUI_API ImVec2        GetFontTexUvWhitePixel();                                       // get UV coordinate for a while pixel, useful to draw custom shapes via the ImDrawList API
// IMGUI_API ImU32         GetColorU32(ImGuiCol idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for ImDrawList
// IMGUI_API ImU32         GetColorU32(const ImVec4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
// IMGUI_API ImU32         GetColorU32(ImU32 col);                                         // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
// IMGUI_API const ImVec4& GetStyleColorVec4(ImGuiCol idx);                                // retrieve style color as stored in ImGuiStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

DEFINE_API(bool, PushStyleColor, ((ImGui_Context*,ctx))
((int,idx))((int,rgba)),
"Modify a style color. always use this if you modify the style after NewFrame().",
{
  Context::check(ctx)->enterFrame();
  if(idx < 0 || idx >= ImGuiCol_COUNT)
    return false; // out of range!
  ImGui::PushStyleColor(idx, ImVec4{Color(rgba)});
  return true;
});

DEFINE_API(void, PopStyleColor, ((ImGui_Context*,ctx))
((int*,API_RO(count))),
"Default values: count = 1",
{
  Context::check(ctx)->enterFrame();
  // TODO harden
  ImGui::PopStyleColor(valueOr(API_RO(count), 1));
});

DEFINE_API(int, ColorConvertHSVtoRGB,
((double,h))((double,s))((double,v))((double*,API_RO(alpha))),
"Return 0x00RRGGBB or, if alpha is provided, 0xRRGGBBAA.",
{
  const bool alpha { API_RO(alpha) != nullptr };
  ImVec4 rgba;
  if(alpha)
    rgba.w = *API_RO(alpha);
  ImGui::ColorConvertHSVtoRGB(h, s, v, rgba.x, rgba.y, rgba.z);
  return Color{rgba}.pack(alpha);
});
