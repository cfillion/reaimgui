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

#include "../src/color.hpp"

#include <variant>

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

template<typename... T>
using StyleFields = std::variant<T ImGuiStyle::*...>;

constexpr int baseStyleVar {__COUNTER__};
static constexpr StyleFields<float, ImVec2> g_styleVars[] {
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
  STYLEVAR(ImageBorderSize),
  STYLEVAR(TabRounding),
  STYLEVAR(TabBorderSize),
  STYLEVAR(TabBarBorderSize),
  STYLEVAR(TabBarOverlineSize),
  STYLEVAR(TableAngledHeadersAngle),
  STYLEVAR(TableAngledHeadersTextAlign),
  STYLEVAR(TreeLinesSize),
  STYLEVAR(TreeLinesRounding),
  STYLEVAR(ButtonTextAlign),
  STYLEVAR(SelectableTextAlign),
  STYLEVAR(SeparatorTextBorderSize),
  STYLEVAR(SeparatorTextAlign),
  STYLEVAR(SeparatorTextPadding),
  STYLEVAR(DockingSeparatorSize),
};

static_assert(std::size(g_styleVars) == ImGuiStyleVar_COUNT);

API_SUBSECTION("Variables");

static void assertStyleVar(const size_t var_idx)
{
  if(var_idx >= std::size(g_styleVars))
    throw reascript_error {"unknown style variable"};
}

API_FUNC(0_1, void, PushStyleVar, (Context*,ctx)
(int,idx) (double,val1) (RO<double*>,val2),
R"(Temporarily modify a style variable.
Call PopStyleVar to undo after use (before the end of the frame).
See StyleVar_* for possible values of 'idx'.)")
{
  FRAME_GUARD;
  assertStyleVar(idx);

  std::visit([idx, val1, val2](auto ImGuiStyle::*field) {
    if constexpr(std::is_same_v<ImVec2,
                            std::decay_t<decltype(ImGui::GetStyle().*field)>>) {
      if(!val2)
        throw reascript_error {"this variable requires two values (x, y)"};
      ImGui::PushStyleVar(idx, ImVec2(val1, *val2));
    }
    else {
      if(val2)
        throw reascript_error {"second value ignored for this variable"};
      ImGui::PushStyleVar(idx, val1);
    }
  }, g_styleVars[idx]);
}

API_FUNC(0_10, void, PushStyleVarX, (Context*,ctx)
(int,idx) (double,val_x),
"Modify the X component of a style variable. See PushStyleVar.")
{
  FRAME_GUARD;
  // not using the g_styleVars table so assertStyleVar is unnecessary here
  // ImGui already validates internally via GetStyleVarInfo
  ImGui::PushStyleVarX(idx, val_x);
}

API_FUNC(0_10, void, PushStyleVarY, (Context*,ctx)
(int,idx) (double,val_y),
"Modify the X component of a style variable. See PushStyleVar.")
{
  FRAME_GUARD;
  ImGui::PushStyleVarY(idx, val_y);
}

API_FUNC(0_1, void, PopStyleVar, (Context*,ctx)
(RO<int*>,count,1),
"Reset a style variable.")
{
  FRAME_GUARD;
  ImGui::PopStyleVar(API_GET(count));
}

API_FUNC(0_1, void, GetStyleVar, (Context*,ctx)
(int,var_idx) (W<double*>,val1) (W<double*>,val2),
"")
{
  FRAME_GUARD;
  assertStyleVar(var_idx);

  std::visit([val1, val2](auto ImGuiStyle::*field) {
    const ImGuiStyle &style {ImGui::GetStyle()};
    if constexpr(std::is_same_v<ImVec2, std::decay_t<decltype(style.*field)>>) {
      if(val1) *val1 = (style.*field).x;
      if(val2) *val2 = (style.*field).y;
    }
    else if(val1) *val1 = style.*field;
  }, g_styleVars[var_idx]);
}

API_ENUM(0_1, StyleVar_Alpha,
  "Global alpha applies to everything in Dear ImGui.");
API_ENUM(0_5_5, StyleVar_DisabledAlpha,
R"(Additional alpha multiplier applied by BeginDisabled.
  Multiply over current value of Alpha.)");
API_ENUM(0_1, StyleVar_WindowPadding,
  "Padding within a window.");
API_ENUM(0_1, StyleVar_WindowRounding,
R"(Radius of window corners rounding. Set to 0.0 to have rectangular windows.
  Large values tend to lead to variety of artifacts and are not recommended.)");
API_ENUM(0_1, StyleVar_WindowBorderSize,
R"(Thickness of border around windows. Generally set to 0.0 or 1.0.
  (Other values are not well tested and more CPU/GPU costly).)");
API_ENUM(0_1, StyleVar_WindowMinSize,
R"(Minimum window size. This is a global setting.
  If you want to constrain individual windows, use SetNextWindowSizeConstraints.)");
API_ENUM(0_1, StyleVar_WindowTitleAlign,
R"(Alignment for title bar text.
   Defaults to (0.0,0.5) for left-aligned,vertically centered.)");
API_ENUM(0_1, StyleVar_ChildRounding,
  "Radius of child window corners rounding. Set to 0.0 to have rectangular windows.");
API_ENUM(0_1, StyleVar_ChildBorderSize,
R"(Thickness of border around child windows. Generally set to 0.0 or 1.0.
   (Other values are not well tested and more CPU/GPU costly).)");
API_ENUM(0_1, StyleVar_PopupRounding,
R"(Radius of popup window corners rounding.
   (Note that tooltip windows use StyleVar_WindowRounding.))");
API_ENUM(0_1, StyleVar_PopupBorderSize,
R"(Thickness of border around popup/tooltip windows. Generally set to 0.0 or 1.0.
   (Other values are not well tested and more CPU/GPU costly).)");
API_ENUM(0_1, StyleVar_FramePadding,
  "Padding within a framed rectangle (used by most widgets).");
API_ENUM(0_1, StyleVar_FrameRounding,
R"(Radius of frame corners rounding.
   Set to 0.0 to have rectangular frame (used by most widgets).)");
API_ENUM(0_1, StyleVar_FrameBorderSize,
R"(Thickness of border around frames. Generally set to 0.0 or 1.0.
   (Other values are not well tested and more CPU/GPU costly).)");
API_ENUM(0_1, StyleVar_ItemSpacing,
  "Horizontal and vertical spacing between widgets/lines.");
API_ENUM(0_1, StyleVar_ItemInnerSpacing,
R"(Horizontal and vertical spacing between within elements of a composed widget
   (e.g. a slider and its label).)");
API_ENUM(0_1, StyleVar_IndentSpacing,
R"(Horizontal indentation when e.g. entering a tree node.
   Generally == (GetFontSize + StyleVar_FramePadding.x*2).)");
API_ENUM(0_1, StyleVar_CellPadding,
R"(Padding within a table cell.
   CellPadding.x is locked for entire table.
   CellPadding.y may be altered between different rows.)");
API_ENUM(0_1, StyleVar_ScrollbarSize,
  "Width of the vertical scrollbar, Height of the horizontal scrollbar.");
API_ENUM(0_1, StyleVar_ScrollbarRounding,
  "Radius of grab corners for scrollbar.");
API_ENUM(0_1, StyleVar_GrabMinSize,
  "Minimum width/height of a grab box for slider/scrollbar.");
API_ENUM(0_1, StyleVar_GrabRounding,
  "Radius of grabs corners rounding. Set to 0.0 to have rectangular slider grabs.");
API_ENUM(0_10, StyleVar_ImageBorderSize, "Thickness of border around Image calls.");
API_ENUM(0_1, StyleVar_TabRounding,
  "Radius of upper corners of a tab. Set to 0.0 to have rectangular tabs.");
API_ENUM(0_9, StyleVar_TabBorderSize, "Thickness of border around tabs.");
API_ENUM(0_9, StyleVar_TabBarBorderSize,
  "Thickness of tab-bar separator, which takes on the tab active color to denote focus.");
API_ENUM(0_10, StyleVar_TabBarOverlineSize,
  "Thickness of tab-bar overline, which highlights the selected tab-bar.");
API_ENUM(0_9, StyleVar_TableAngledHeadersAngle,
  "Angle of angled headers (supported values range from -50.0 degrees to +50.0 degrees).");
API_ENUM(0_9_1, StyleVar_TableAngledHeadersTextAlign,
  "Alignment of angled headers within the cell");
API_ENUM(0_10, StyleVar_TreeLinesSize,
  "Thickness of outlines when using TreeNodeFlags_DrawLines.");
API_ENUM(0_10, StyleVar_TreeLinesRounding,
  "Radius of lines connecting child nodes to the vertical line.");
API_ENUM(0_1, StyleVar_ButtonTextAlign,
R"(Alignment of button text when button is larger than text.
   Defaults to (0.5, 0.5) (centered).)");
API_ENUM(0_1, StyleVar_SelectableTextAlign,
R"(Alignment of selectable text. Defaults to (0.0, 0.0) (top-left aligned).
   It's generally important to keep this left-aligned if you want to lay
   multiple items on a same line.)");
API_ENUM(0_8_4, StyleVar_SeparatorTextBorderSize,
  "Thickness of border in SeparatorText()");
API_ENUM(0_8_4, StyleVar_SeparatorTextAlign,
R"(Alignment of text within the separator.
Defaults to (0.0, 0.5) (left aligned, center).)");
API_ENUM(0_8_4, StyleVar_SeparatorTextPadding,
R"(Horizontal offset of text from each edge of the separator + spacing on other
axis. Generally small values. .y is recommended to be == StyleVar_FramePadding.y.)");

API_SUBSECTION("Colors");

API_FUNC(0_1, int, GetColor, (Context*,ctx)
(int,idx) (RO<double*>,alpha_mul,1.0),
R"(Retrieve given style color with style alpha applied and optional extra alpha
multiplier, packed as a 32-bit value (RGBA). See Col_* for available style colors.)")
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  const ImGuiCol col {idx};
  return Color::toBigEndian(ImGui::GetColorU32(col, API_GET(alpha_mul)));
}

API_FUNC(0_1, int, GetColorEx, (Context*,ctx)
(int,col_rgba) (RO<double*>,alpha_mul,1.0),
"Retrieve given color with style alpha applied, packed as a 32-bit value (RGBA).")
{
  FRAME_GUARD;
  col_rgba = Color::fromBigEndian(col_rgba);
  col_rgba = ImGui::GetColorU32(static_cast<ImU32>(col_rgba), API_GET(alpha_mul));
  col_rgba = Color::toBigEndian(col_rgba);
  return col_rgba;
}

API_FUNC(0_1, int, GetStyleColor, (Context*,ctx)
(int,idx),
R"(Retrieve style color as stored in ImGuiStyle structure.
Use to feed back into PushStyleColor, Otherwise use GetColor to get style color
with style alpha baked in. See Col_* for available style colors.)")
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  return Color {ImGui::GetStyleColorVec4(idx)}.pack(true);
}

API_FUNC(0_1, void, PushStyleColor, (Context*,ctx)
(int,idx) (int,col_rgba),
R"(Temporarily modify a style color.
Call PopStyleColor to undo after use (before the end of the frame).
See Col_* for available style colors.)")
{
  FRAME_GUARD;
  IM_ASSERT(idx >= 0 && idx < ImGuiCol_COUNT);
  ImGui::PushStyleColor(idx, Color(col_rgba));
}

API_FUNC(0_1, void, PopStyleColor, (Context*,ctx)
(RO<int*>,count,1),
"")
{
  FRAME_GUARD;
  ImGui::PopStyleColor(API_GET(count));
}

API_FUNC(0_9, void, DebugFlashStyleColor, (Context*,ctx)
(int,idx),
"")
{
  FRAME_GUARD;
  ImGui::DebugFlashStyleColor(idx);
}

API_ENUM(0_1, Col_Text,                  "");
API_ENUM(0_1, Col_TextDisabled,          "");
API_ENUM(0_1, Col_WindowBg,
  "Background of normal windows. See also WindowFlags_NoBackground.");
API_ENUM(0_1, Col_ChildBg,
  "Background of child windows.");
API_ENUM(0_1, Col_PopupBg,
  "Background of popups, menus, tooltips windows.");
API_ENUM(0_1, Col_Border,                "");
API_ENUM(0_1, Col_BorderShadow,          "");
API_ENUM(0_1, Col_FrameBg,
  "Background of checkbox, radio button, plot, slider, text input.");
API_ENUM(0_1, Col_FrameBgHovered,        "");
API_ENUM(0_1, Col_FrameBgActive,         "");
API_ENUM(0_1, Col_TitleBg,               "Title bar");
API_ENUM(0_1, Col_TitleBgActive,         "Title bar when focused");
API_ENUM(0_1, Col_TitleBgCollapsed,      "Title bar when collapsed");
API_ENUM(0_1, Col_MenuBarBg,             "");
API_ENUM(0_1, Col_ScrollbarBg,           "");
API_ENUM(0_1, Col_ScrollbarGrab,         "");
API_ENUM(0_1, Col_ScrollbarGrabHovered,  "");
API_ENUM(0_1, Col_ScrollbarGrabActive,   "");
API_ENUM(0_1, Col_CheckMark,             "Checkbox tick and RadioButton circle");
API_ENUM(0_1, Col_SliderGrab,            "");
API_ENUM(0_1, Col_SliderGrabActive,      "");
API_ENUM(0_1, Col_Button,                "");
API_ENUM(0_1, Col_ButtonHovered,         "");
API_ENUM(0_1, Col_ButtonActive,          "");
API_ENUM(0_1, Col_Header,
  "Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem.");
API_ENUM(0_1, Col_HeaderHovered,         "");
API_ENUM(0_1, Col_HeaderActive,          "");
API_ENUM(0_1, Col_Separator,             "");
API_ENUM(0_1, Col_SeparatorHovered,      "");
API_ENUM(0_1, Col_SeparatorActive,       "");
API_ENUM(0_1, Col_ResizeGrip,
  "Resize grip in lower-right and lower-left corners of windows.");
API_ENUM(0_1, Col_ResizeGripHovered,     "");
API_ENUM(0_1, Col_ResizeGripActive,      "");
API_ENUM(0_10, Col_InputTextCursor,      "InputText cursor/caret");
API_ENUM(0_1, Col_TabHovered,            "Tab background, when hovered");
API_ENUM(0_1, Col_Tab,
  "Tab background, when tab-bar is focused & tab is unselected");
API_ENUM(0_9_2, Col_TabSelected,
  "Tab background, when tab-bar is focused & tab is selected");
API_ENUM(0_9_2, Col_TabSelectedOverline,
  "Tab horizontal overline, when tab-bar is focused & tab is selected");
API_ENUM(0_9_2, Col_TabDimmed,
  "Tab background, when tab-bar is unfocused & tab is unselected");
API_ENUM(0_9_2, Col_TabDimmedSelected,
  "Tab background, when tab-bar is unfocused & tab is selected");
API_ENUM(0_9_2, Col_TabDimmedSelectedOverline,
  "Horizontal overline, when tab-bar is unfocused & tab is selected");
API_ENUM(0_5, Col_DockingPreview,
  "Preview overlay color when about to docking something.");
API_ENUM(0_5, Col_DockingEmptyBg,
  "Background color for empty node (e.g. CentralNode with no window docked into it).");
API_ENUM(0_1, Col_PlotLines,             "");
API_ENUM(0_1, Col_PlotLinesHovered,      "");
API_ENUM(0_1, Col_PlotHistogram,         "");
API_ENUM(0_1, Col_PlotHistogramHovered,  "");
API_ENUM(0_1, Col_TableHeaderBg, "Table header background.");
API_ENUM(0_1, Col_TableBorderStrong,
  "Table outer and header borders (prefer using Alpha=1.0 here).");
API_ENUM(0_1, Col_TableBorderLight,
  "Table inner borders (prefer using Alpha=1.0 here).");
API_ENUM(0_1, Col_TableRowBg, "Table row background (even rows).");
API_ENUM(0_1, Col_TableRowBgAlt, "Table row background (odd rows).");
API_ENUM(0_10, Col_TextLink, "Hyperlink color");
API_ENUM(0_1, Col_TextSelectedBg, "Selected text inside an InputText");
API_ENUM(0_10, Col_TreeLines,
  "Tree node hierarchy outlines when using TreeNodeFlags_DrawLines");
API_ENUM(0_1, Col_DragDropTarget, "Rectangle highlighting a drop target");
API_ENUM(0_10, Col_NavCursor,
  "Color of keyboard/gamepad navigation cursor/rectangle, when visible");
API_ENUM(0_1, Col_NavWindowingHighlight, "Highlight window when using CTRL+TAB.");
API_ENUM(0_1, Col_NavWindowingDimBg,
  "Darken/colorize entire screen behind the CTRL+TAB window list, when active.");
API_ENUM(0_1, Col_ModalWindowDimBg,
  "Darken/colorize entire screen behind a modal window, when one is active.");
