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
R"(Create a Tab. Returns true if the Tab is selected. Set 'p_open' to true to enable the close button.

Default values: p_open = nil, flags = ImGui_TabItemFlags_None)",
{
  FRAME_GUARD;
  return ImGui::BeginTabItem(label, openPtrBehavior(API_RWO(p_open)),
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
"Notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after ImGui_BeginTabBar and before Tab submissions. Otherwise call with a window name.",
{
  FRAME_GUARD;
  ImGui::SetTabItemClosed(tab_or_docked_window_label);
});

// ImGuiTabBarFlags
DEFINE_ENUM(ImGui, TabBarFlags_None,                         "Flags for ImGui_BeginTabBar.");
DEFINE_ENUM(ImGui, TabBarFlags_Reorderable,                  "Allow manually dragging tabs to re-order them + New tabs are appended at the end of list.");
DEFINE_ENUM(ImGui, TabBarFlags_AutoSelectNewTabs,            "Automatically select new tabs when they appear.");
DEFINE_ENUM(ImGui, TabBarFlags_TabListPopupButton,           "Disable buttons to open the tab list popup.");
DEFINE_ENUM(ImGui, TabBarFlags_NoCloseWithMiddleMouseButton, "Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if(ImGui_IsItemHovered() && ImGui_IsMouseClicked(2)) *p_open = false.");
DEFINE_ENUM(ImGui, TabBarFlags_NoTabListScrollingButtons,    "Disable scrolling buttons (apply when fitting policy is ImGui_TabBarFlags_FittingPolicyScroll).");
DEFINE_ENUM(ImGui, TabBarFlags_NoTooltip,                    "Disable tooltips when hovering a tab.");
DEFINE_ENUM(ImGui, TabBarFlags_FittingPolicyResizeDown,      "Resize tabs when they don't fit.");
DEFINE_ENUM(ImGui, TabBarFlags_FittingPolicyScroll,          "Add scroll buttons when tabs don't fit.");

// ImGuiTabItemFlags
DEFINE_ENUM(ImGui, TabItemFlags_None,                         "Flags for ImGui_BeginTabItem.");
DEFINE_ENUM(ImGui, TabItemFlags_UnsavedDocument,              "Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. Also: tab is selected on closure and closure is deferred by one frame to allow code to undo it without flicker.");
DEFINE_ENUM(ImGui, TabItemFlags_SetSelected,                  "Trigger flag to programmatically make the tab selected when calling ImGui_BeginTabItem.");
DEFINE_ENUM(ImGui, TabItemFlags_NoCloseWithMiddleMouseButton, "Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (ImGui_IsItemHovered() && ImGui_IsMouseClicked(2)) *p_open = false.");
DEFINE_ENUM(ImGui, TabItemFlags_NoPushId,                     "Don't call ImGui_PushID(tab->ID)/ImGui_PopID() on ImGui_BeginTabItem/ImGui_EndTabItem.");
DEFINE_ENUM(ImGui, TabItemFlags_NoTooltip,                    "Disable tooltip for the given tab.");
DEFINE_ENUM(ImGui, TabItemFlags_NoReorder,                    "Disable reordering this tab or having another tab cross over this tab.");
DEFINE_ENUM(ImGui, TabItemFlags_Leading,                      "Enforce the tab position to the left of the tab bar (after the tab list popup button).");
DEFINE_ENUM(ImGui, TabItemFlags_Trailing,                     "Enforce the tab position to the right of the tab bar (before the scrolling buttons).");
