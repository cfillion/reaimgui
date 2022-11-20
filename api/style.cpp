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

#include "color.hpp"
#include "variant.hpp"

API_SECTION("Style");

template<int Index, int EnumValue, typename T>
constexpr auto assertStyleVar(T ImGuiStyle::*var)
{
  static_assert(Index == EnumValue,
    "array indices do not match with enum values in imgui.h");
  return var;
}

#define STYLEVAR(name) \
  assertStyleVar<__COUNTER__ - baseStyleVar - 1, \
                 ImGuiStyleVar_##name>(&ImGuiStyle::name)

constexpr int baseStyleVar { __COUNTER__ };
static const std::variant<float ImGuiStyle::*, ImVec2 ImGuiStyle::*> g_styleVars[] {
  STYLEVAR(Alpha),
  STYLEVAR(DisabledAlpha),
  STYLEVAR(WindowPadding),
  STYLEVAR(WindowRounding),
  STYLEVAR(WindowBorderSize),
  STYLEVAR(WindowMinSize),
  STYLEVAR(WindowTitleAlign),
  STYLEVAR(ChildRounding),
  STYLEVAR(ChildBorderSize),
  STYLEVAR(PopupRounding),
  STYLEVAR(PopupBorderSize),
  STYLEVAR(FramePadding),
  STYLEVAR(FrameRounding),
  STYLEVAR(FrameBorderSize),
  STYLEVAR(ItemSpacing),
  STYLEVAR(ItemInnerSpacing),
  STYLEVAR(IndentSpacing),
  STYLEVAR(CellPadding),
  STYLEVAR(ScrollbarSize),
  STYLEVAR(ScrollbarRounding),
  STYLEVAR(GrabMinSize),
  STYLEVAR(GrabRounding),
  STYLEVAR(TabRounding),
  STYLEVAR(ButtonTextAlign),
  STYLEVAR(SelectableTextAlign),
};

static_assert(std::size(g_styleVars) == ImGuiStyleVar_COUNT);

API_SUBSECTION("Variables");

DEFINE_API(void, PushStyleVar, (ImGui_Context*,ctx)
(int,var_idx)(double,val1)(double*,API_RO(val2)),
R"(See ImGui_StyleVar_* for possible values of 'var_idx'.

Default values: val2 = nil)",
{
  FRAME_GUARD;
  if(static_cast<size_t>(var_idx) >= std::size(g_styleVars))
    throw reascript_error { "unknown style variable" };

  std::visit([var_idx, val1, API_RO(val2)](auto ImGuiStyle::*field) {
    if constexpr (std::is_same_v<ImVec2, std::decay_t<decltype(ImGui::GetStyle().*field)>>) {
      if(!API_RO(val2))
        throw reascript_error { "this variable requires two values (x, y)" };
      ImGui::PushStyleVar(var_idx, ImVec2(val1, *API_RO(val2)));
    }
    else {
      if(API_RO(val2))
        throw reascript_error { "second value ignored for this variable" };
      ImGui::PushStyleVar(var_idx, val1);
    }
  }, g_styleVars[var_idx]);
});

DEFINE_API(void, PopStyleVar, (ImGui_Context*,ctx)
(int*,API_RO(count)),
R"(Reset a style variable.

Default values: count = 1)",
{
  FRAME_GUARD;
  ImGui::PopStyleVar(valueOr(API_RO(count), 1));
});

DEFINE_API(void, GetStyleVar, (ImGui_Context*,ctx)
(int,var_idx)(double*,API_W(val1))(double*,API_W(val2)),
"",
{
  FRAME_GUARD;
  if(static_cast<size_t>(var_idx) >= std::size(g_styleVars))
    throw reascript_error { "unknown style variable" };

  std::visit([API_W(val1), API_W(val2)](auto ImGuiStyle::*field) {
    const ImGuiStyle &style { ImGui::GetStyle() };
    if constexpr (std::is_same_v<ImVec2, std::decay_t<decltype(style.*field)>>) {
      if(API_W(val1)) *API_W(val1) = (style.*field).x;
      if(API_W(val2)) *API_W(val2) = (style.*field).y;
    }
    else
      if(API_W(val1)) *API_W(val1) = style.*field;
  }, g_styleVars[var_idx]);
});

DEFINE_ENUM(ImGui, StyleVar_Alpha,               "Global alpha applies to everything in Dear ImGui.");
DEFINE_ENUM(ImGui, StyleVar_DisabledAlpha,       "Additional alpha multiplier applied by ImGui_BeginDisabled. Multiply over current value of Alpha.");
DEFINE_ENUM(ImGui, StyleVar_WindowPadding,       "Padding within a window.");
DEFINE_ENUM(ImGui, StyleVar_WindowRounding,      "Radius of window corners rounding. Set to 0.0 to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.");
DEFINE_ENUM(ImGui, StyleVar_WindowBorderSize,    "Thickness of border around windows. Generally set to 0.0 or 1.0. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_WindowMinSize,       "Minimum window size. This is a global setting. If you want to constrain individual windows, use ImGui_SetNextWindowSizeConstraints.");
DEFINE_ENUM(ImGui, StyleVar_WindowTitleAlign,    "Alignment for title bar text. Defaults to (0.0,0.5) for left-aligned,vertically centered.");
DEFINE_ENUM(ImGui, StyleVar_ChildRounding,       "Radius of child window corners rounding. Set to 0.0 to have rectangular windows.");
DEFINE_ENUM(ImGui, StyleVar_ChildBorderSize,     "Thickness of border around child windows. Generally set to 0.0 or 1.0. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_PopupRounding,       "Radius of popup window corners rounding. (Note that tooltip windows use ImGui_StyleVar_WindowRounding.)");
DEFINE_ENUM(ImGui, StyleVar_PopupBorderSize,     "Thickness of border around popup/tooltip windows. Generally set to 0.0 or 1.0. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_FramePadding,        "Padding within a framed rectangle (used by most widgets).");
DEFINE_ENUM(ImGui, StyleVar_FrameRounding,       "Radius of frame corners rounding. Set to 0.0 to have rectangular frame (used by most widgets).");
DEFINE_ENUM(ImGui, StyleVar_FrameBorderSize,     "Thickness of border around frames. Generally set to 0.0 or 1.0. (Other values are not well tested and more CPU/GPU costly).");
DEFINE_ENUM(ImGui, StyleVar_ItemSpacing,         "Horizontal and vertical spacing between widgets/lines.");
DEFINE_ENUM(ImGui, StyleVar_ItemInnerSpacing,    "Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).");
DEFINE_ENUM(ImGui, StyleVar_IndentSpacing,       "Horizontal indentation when e.g. entering a tree node. Generally == (ImGui_GetFontSize + ImGui_StyleVar_FramePadding.x*2).");
DEFINE_ENUM(ImGui, StyleVar_CellPadding,         "Padding within a table cell.");
DEFINE_ENUM(ImGui, StyleVar_ScrollbarSize,       "Width of the vertical scrollbar, Height of the horizontal scrollbar.");
DEFINE_ENUM(ImGui, StyleVar_ScrollbarRounding,   "Radius of grab corners for scrollbar.");
DEFINE_ENUM(ImGui, StyleVar_GrabMinSize,         "Minimum width/height of a grab box for slider/scrollbar.");
DEFINE_ENUM(ImGui, StyleVar_GrabRounding,        "Radius of grabs corners rounding. Set to 0.0 to have rectangular slider grabs.");
DEFINE_ENUM(ImGui, StyleVar_TabRounding,         "Radius of upper corners of a tab. Set to 0.0 to have rectangular tabs.");
DEFINE_ENUM(ImGui, StyleVar_ButtonTextAlign,     "Alignment of button text when button is larger than text. Defaults to (0.5, 0.5) (centered).");
DEFINE_ENUM(ImGui, StyleVar_SelectableTextAlign, "Alignment of selectable text. Defaults to (0.0, 0.0) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.");

API_SUBSECTION("Colors");

DEFINE_API(int, GetColor, (ImGui_Context*,ctx)
(int,idx)(double*,API_RO(alpha_mul)),
R"(Retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value (RGBA). See ImGui_Col_* for available style colors.

Default values: alpha_mul = 1.0)",
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  return Color::toBigEndian(ImGui::GetColorU32(idx, valueOr(API_RO(alpha_mul), 1.0)));
});

DEFINE_API(int, GetColorEx, (ImGui_Context*,ctx)
(int,col_rgba),
"Retrieve given color with style alpha applied, packed as a 32-bit value (RGBA).",
{
  FRAME_GUARD;
  return Color::toBigEndian(ImGui::GetColorU32(Color::fromBigEndian(col_rgba)));
});

DEFINE_API(int, GetStyleColor, (ImGui_Context*,ctx)
(int,idx),
"Retrieve style color as stored in ImGuiStyle structure. Use to feed back into ImGui_PushStyleColor, Otherwise use ImGui_GetColor to get style color with style alpha baked in. See ImGui_Col_* for available style colors.",
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  return Color { ImGui::GetStyleColorVec4(idx) }.pack(true);
});

DEFINE_API(void, PushStyleColor, (ImGui_Context*,ctx)
(int,idx)(int,col_rgba),
"Modify a style color. Call ImGui_PopStyleColor to undo after use (before the end of the frame). See ImGui_Col_* for available style colors.",
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  ImGui::PushStyleColor(idx, Color(col_rgba));
});

DEFINE_API(void, PopStyleColor, (ImGui_Context*,ctx)
(int*,API_RO(count)),
"Default values: count = 1",
{
  FRAME_GUARD;
  ImGui::PopStyleColor(valueOr(API_RO(count), 1));
});

DEFINE_ENUM(ImGui, Col_Text,                  "");
DEFINE_ENUM(ImGui, Col_TextDisabled,          "");
DEFINE_ENUM(ImGui, Col_WindowBg,              "Background of normal windows. See also ImGui_WindowFlags_NoBackground.");
DEFINE_ENUM(ImGui, Col_ChildBg,               "Background of child windows.");
DEFINE_ENUM(ImGui, Col_PopupBg,               "Background of popups, menus, tooltips windows.");
DEFINE_ENUM(ImGui, Col_Border,                "");
DEFINE_ENUM(ImGui, Col_BorderShadow,          "");
DEFINE_ENUM(ImGui, Col_FrameBg,               "Background of checkbox, radio button, plot, slider, text input.");
DEFINE_ENUM(ImGui, Col_FrameBgHovered,        "");
DEFINE_ENUM(ImGui, Col_FrameBgActive,         "");
DEFINE_ENUM(ImGui, Col_TitleBg,               "");
DEFINE_ENUM(ImGui, Col_TitleBgActive,         "");
DEFINE_ENUM(ImGui, Col_TitleBgCollapsed,      "");
DEFINE_ENUM(ImGui, Col_MenuBarBg,             "");
DEFINE_ENUM(ImGui, Col_ScrollbarBg,           "");
DEFINE_ENUM(ImGui, Col_ScrollbarGrab,         "");
DEFINE_ENUM(ImGui, Col_ScrollbarGrabHovered,  "");
DEFINE_ENUM(ImGui, Col_ScrollbarGrabActive,   "");
DEFINE_ENUM(ImGui, Col_CheckMark,             "");
DEFINE_ENUM(ImGui, Col_SliderGrab,            "");
DEFINE_ENUM(ImGui, Col_SliderGrabActive,      "");
DEFINE_ENUM(ImGui, Col_Button,                "");
DEFINE_ENUM(ImGui, Col_ButtonHovered,         "");
DEFINE_ENUM(ImGui, Col_ButtonActive,          "");
DEFINE_ENUM(ImGui, Col_Header,                "Header* colors are used for ImGui_CollapsingHeader, ImGui_TreeNode, ImGui_Selectable, ImGui_MenuItem.");
DEFINE_ENUM(ImGui, Col_HeaderHovered,         "");
DEFINE_ENUM(ImGui, Col_HeaderActive,          "");
DEFINE_ENUM(ImGui, Col_Separator,             "");
DEFINE_ENUM(ImGui, Col_SeparatorHovered,      "");
DEFINE_ENUM(ImGui, Col_SeparatorActive,       "");
DEFINE_ENUM(ImGui, Col_ResizeGrip,            "Resize grip in lower-right and lower-left corners of windows.");
DEFINE_ENUM(ImGui, Col_ResizeGripHovered,     "");
DEFINE_ENUM(ImGui, Col_ResizeGripActive,      "");
DEFINE_ENUM(ImGui, Col_Tab,                   "TabItem in a TabBar");
DEFINE_ENUM(ImGui, Col_TabHovered,            "");
DEFINE_ENUM(ImGui, Col_TabActive,             "");
DEFINE_ENUM(ImGui, Col_TabUnfocused,          "");
DEFINE_ENUM(ImGui, Col_TabUnfocusedActive,    "");
DEFINE_ENUM(ImGui, Col_DockingPreview,        "Preview overlay color when about to docking something.");
DEFINE_ENUM(ImGui, Col_DockingEmptyBg,        "Background color for empty node (e.g. CentralNode with no window docked into it).");
DEFINE_ENUM(ImGui, Col_PlotLines,             "");
DEFINE_ENUM(ImGui, Col_PlotLinesHovered,      "");
DEFINE_ENUM(ImGui, Col_PlotHistogram,         "");
DEFINE_ENUM(ImGui, Col_PlotHistogramHovered,  "");
DEFINE_ENUM(ImGui, Col_TableHeaderBg,         "Table header background.");
DEFINE_ENUM(ImGui, Col_TableBorderStrong,     "Table outer and header borders (prefer using Alpha=1.0 here).");
DEFINE_ENUM(ImGui, Col_TableBorderLight,      "Table inner borders (prefer using Alpha=1.0 here).");
DEFINE_ENUM(ImGui, Col_TableRowBg,            "Table row background (even rows).");
DEFINE_ENUM(ImGui, Col_TableRowBgAlt,         "Table row background (odd rows).");
DEFINE_ENUM(ImGui, Col_TextSelectedBg,        "");
DEFINE_ENUM(ImGui, Col_DragDropTarget,        "Rectangle highlighting a drop target");
DEFINE_ENUM(ImGui, Col_NavHighlight,          "Gamepad/keyboard: current highlighted item.");
DEFINE_ENUM(ImGui, Col_NavWindowingHighlight, "Highlight window when using CTRL+TAB.");
DEFINE_ENUM(ImGui, Col_NavWindowingDimBg,     "Darken/colorize entire screen behind the CTRL+TAB window list, when active.");
DEFINE_ENUM(ImGui, Col_ModalWindowDimBg,      "Darken/colorize entire screen behind a modal window, when one is active.");
