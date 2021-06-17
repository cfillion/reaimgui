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

#include "color.hpp"

DEFINE_API(void, Text, (ImGui_Context*,ctx)
(const char*,text),
"",
{
  FRAME_GUARD;
  ImGui::TextUnformatted(text);
});

DEFINE_API(void, TextColored, (ImGui_Context*,ctx)
(int,col_rgba)(const char*,text),
"Shortcut for PushStyleColor(ImGuiCol_Text, color); Text(text); PopStyleColor();",
{
  FRAME_GUARD;

  ImVec4 color { Color(col_rgba) };
  ImGui::PushStyleColor(ImGuiCol_Text, color);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextDisabled, (ImGui_Context*,ctx)
(const char*,text),
"",
{
  FRAME_GUARD;
  const ImGuiStyle &style { ImGui::GetStyle() };
  ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextWrapped, (ImGui_Context*,ctx)
(const char*,text),
"Shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().",
{
  FRAME_GUARD;
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextUnformatted(text);
  ImGui::PopTextWrapPos();
});

DEFINE_API(void, LabelText, (ImGui_Context*,ctx)
(const char*,label)(const char*,text),
"Display text+label aligned the same way as value+label widgets",
{
  FRAME_GUARD;
  ImGui::LabelText(label, "%s", text);
});

DEFINE_API(void, BulletText, (ImGui_Context*,ctx)
(const char*,text),
"Shortcut for Bullet()+Text()",
{
  FRAME_GUARD;
  ImGui::Bullet();
  ImGui::TextUnformatted(text);
});

DEFINE_API(bool, Button, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RO(size_w))(double*,API_RO(size_h)),
R"(Most widgets return true when the value has been changed or when pressed/selected
You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.

Default values: size_w = 0.0, size_h = 0.0)",
{
  FRAME_GUARD;

  return ImGui::Button(label,
    ImVec2(valueOr(API_RO(size_w), 0.0), valueOr(API_RO(size_h), 0.0)));
});

DEFINE_API(bool, SmallButton, (ImGui_Context*,ctx)
(const char*,label),
"Button with FramePadding=(0,0) to easily embed within text",
{
  FRAME_GUARD;
  return ImGui::SmallButton(label);
});

DEFINE_API(bool, InvisibleButton, (ImGui_Context*,ctx)
(const char*,str_id)(double,size_w)(double,size_h)(int*,API_RO(flags)),
R"(Flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.).

Default values: flags = ImGui_ButtonFlags_None)",
{
  FRAME_GUARD;
  return ImGui::InvisibleButton(str_id, ImVec2(size_w, size_h),
    valueOr(API_RO(flags), ImGuiButtonFlags_None));
});

DEFINE_API(bool, ArrowButton, (ImGui_Context*,ctx)
(const char*,str_id)(int,dir),
"Square button with an arrow shape",
{
  FRAME_GUARD;
  return ImGui::ArrowButton(str_id, dir);
});

DEFINE_API(bool, Checkbox, (ImGui_Context*,ctx)
(const char*, label)(bool*, API_RW(v)),
"",
{
  FRAME_GUARD;
  if(!API_RW(v))
    return false;
  return ImGui::Checkbox(label, API_RW(v));
});

DEFINE_API(bool, CheckboxFlags, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(flags))(int,flags_value),
"",
{
  FRAME_GUARD;
  return ImGui::CheckboxFlags(label, API_RW(flags), flags_value);
});

DEFINE_API(bool, RadioButton, (ImGui_Context*,ctx)
(const char*,label)(bool,active),
R"(Use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; })",
{
  FRAME_GUARD;
  return ImGui::RadioButton(label, active);
});

DEFINE_API(bool, RadioButtonEx, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v))(int,v_button),
"Shortcut to handle RadioButton's example pattern when value is an integer",
{
  FRAME_GUARD;
  return ImGui::RadioButton(label, API_RW(v), v_button);
});

DEFINE_API(void, ProgressBar, (ImGui_Context*,ctx)
(double,fraction)
(double*,API_RO(size_arg_w))(double*,API_RO(size_arg_h))
(const char*,API_RO(overlay)),
"Default values: size_arg_w = -FLT_MIN, size_arg_h = 0.0, overlay = nil",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(overlay));

  ImGui::ProgressBar(fraction,
    ImVec2(valueOr(API_RO(size_arg_w), -FLT_MIN), valueOr(API_RO(size_arg_h), 0.0)),
    API_RO(overlay));
});

DEFINE_API(void, Bullet, (ImGui_Context*,ctx),
"Draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses.",
{
  FRAME_GUARD;
  ImGui::Bullet();
});

DEFINE_API(bool, Selectable, (ImGui_Context*,ctx)
(const char*,label)(bool*,API_RWO(p_selected))
(int*,API_RO(flags))(double*,API_RO(size_w))(double*,API_RO(size_h)),
R"(A selectable highlights when hovered, and can display another color when selected.
Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.

Default values: flags = ImGui_SelectableFlags_None, size_w = 0.0, size_h = 0.0)",
{
  FRAME_GUARD;

  bool selectedOmitted {};
  bool *selected { API_RWO(p_selected) ? API_RWO(p_selected) : &selectedOmitted };
  return ImGui::Selectable(label, selected,
    valueOr(API_RO(flags), ImGuiSelectableFlags_None),
    ImVec2(valueOr(API_RO(size_w), 0.0), valueOr(API_RO(size_h), 0.0)));
});

DEFINE_API(bool, TreeNode, (ImGui_Context*,ctx)
(const char*, label)(int*,API_RO(flags)),
R"(TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.

Default values: flags = ImGui_TreeNodeFlags_None)",
{
  FRAME_GUARD;
  return ImGui::TreeNodeEx(label, valueOr(API_RO(flags), ImGuiTreeNodeFlags_None));
});

DEFINE_API(bool, TreeNodeEx, (ImGui_Context*,ctx)
(const char*, str_id)(const char*, label)(int*,API_RO(flags)),
R"(Helper variation to easily decorelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet(). See ImGui_TreeNode.

Default values: flags = ImGui_TreeNodeFlags_None)",
{
  FRAME_GUARD;
  const ImGuiTreeNodeFlags flags { valueOr(API_RO(flags), ImGuiTreeNodeFlags_None) };
  return ImGui::TreeNodeEx(str_id, flags, "%s", label);
});

DEFINE_API(void, TreePush, (ImGui_Context*,ctx)
(const char*,str_id),
"~ Indent()+PushId(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.",
{
  FRAME_GUARD;
  ImGui::TreePush(str_id);
});

DEFINE_API(void, TreePop, (ImGui_Context*,ctx),
"Unindent()+PopId()",
{
  FRAME_GUARD;
  ImGui::TreePop();
});

DEFINE_API(double, GetTreeNodeToLabelSpacing, (ImGui_Context*,ctx),
"Horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode",
{
  FRAME_GUARD;
  return ImGui::GetTreeNodeToLabelSpacing();
});

DEFINE_API(bool, CollapsingHeader, (ImGui_Context*,ctx)
(const char*, label)(bool*,API_RWO(p_visible))(int*,API_RO(flags)),
R"(CollapsingHeader returns true when opened but do not indent nor push into the ID stack (because of the ImGui_TreeNodeFlags_NoTreePushOnOpen flag).

This is basically the same as calling TreeNode(label, ImGui_TreeNodeFlags_CollapsingHeader). You can remove the _NoTreePushOnOpen flag if you want behavior closer to normal TreeNode().

When 'visible' is provided: if 'true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if 'false' don't display the header.

Default values: flags = ImGui_TreeNodeFlags_None)",
{
  FRAME_GUARD;
  const ImGuiTreeNodeFlags flags { valueOr(API_RO(flags), ImGuiTreeNodeFlags_None) };
  return ImGui::CollapsingHeader(label, API_RWO(p_visible), flags);
});

DEFINE_API(bool, BeginMenuBar, (ImGui_Context*,ctx),
R"(Append to menu-bar of current window (requires ImGui_WindowFlags_MenuBar flag set on parent window). See ImGui_EndMenuBar.)",
{
  FRAME_GUARD;
  return ImGui::BeginMenuBar();
});

DEFINE_API(void, EndMenuBar, (ImGui_Context*,ctx),
R"(Only call EndMenuBar() if BeginMenuBar() returns true! See ImGui_BeginMenuBar.)",
{
  FRAME_GUARD;
  ImGui::EndMenuBar();
});

DEFINE_API(bool, BeginMenu, (ImGui_Context*,ctx)
(const char*, label)(bool*, API_RO(enabled)),
R"(Create a sub-menu entry. only call EndMenu() if this returns true! See ImGui_EndMenu.

Default values: enabled = true)",
{
  FRAME_GUARD;
  return ImGui::BeginMenu(label, valueOr(API_RO(enabled), true));
});

DEFINE_API(void, EndMenu, (ImGui_Context*,ctx),
R"(Only call EndMenu() if BeginMenu() returns true! See ImGui_BeginMenu.)",
{
  FRAME_GUARD;
  ImGui::EndMenu();
});

DEFINE_API(bool, MenuItem, (ImGui_Context*,ctx)
(const char*, label)(const char*, API_RO(shortcut))
(bool*, API_RWO(p_selected))(bool*, API_RO(enabled)),
R"(Return true when activated. Shortcuts are displayed for convenience but not processed by ImGui at the moment. Toggle state is written to 'selected' when provided.

Default values: enabled = true)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(shortcut));

  return ImGui::MenuItem(label, API_RO(shortcut), API_RWO(p_selected),
    valueOr(API_RO(enabled), true));
});

DEFINE_API(bool, BeginTabBar, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags)),
R"(Create and append into a TabBar.

Default values: flags = ImGui_TabBarFlags_None)",
{
  FRAME_GUARD;
  return ImGui::BeginTabBar(str_id, valueOr(API_RO(flags), ImGuiTabBarFlags_None));
});

DEFINE_API(void, EndTabBar, (ImGui_Context*,ctx),
"Only call EndTabBar() if BeginTabBar() returns true!",
{
  FRAME_GUARD;
  ImGui::EndTabBar();
});

DEFINE_API(bool, BeginTabItem, (ImGui_Context*,ctx)
(const char*,label)(bool*,API_RWO(p_open))(int*,API_RO(flags)),
R"(Create a Tab. Returns true if the Tab is selected.

Default values: p_open = nil, flags = ImGui_TabItemFlags_None
'open' is read/write.)",
{
  FRAME_GUARD;
  return ImGui::BeginTabItem(label, API_RWO(p_open),
    valueOr(API_RO(flags), ImGuiTabItemFlags_None));
});

DEFINE_API(void, EndTabItem, (ImGui_Context*,ctx),
"Only call EndTabItem() if BeginTabItem() returns true!",
{
  FRAME_GUARD;
  ImGui::EndTabItem();
});

DEFINE_API(bool, TabItemButton, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RO(flags)),
R"(Create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.

Default values: flags = ImGui_TabItemFlags_None)",
{
  FRAME_GUARD;
  return ImGui::TabItemButton(label,
    valueOr(API_RO(flags), ImGuiTabItemFlags_None));
});

DEFINE_API(void, SetTabItemClosed, (ImGui_Context*,ctx)
(const char*,tab_or_docked_window_label),
"Notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.",
{
  FRAME_GUARD;
  ImGui::SetTabItemClosed(tab_or_docked_window_label);
});
