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

API_SECTION("Tab Bar");

API_FUNC(0_1, bool, BeginTabBar, (Context*,ctx)
(const char*,str_id) (RO<int*>,flags,ImGuiTabBarFlags_None),
"Create and append into a TabBar.")
{
  FRAME_GUARD;
  return ImGui::BeginTabBar(str_id, API_GET(flags));
}

API_FUNC(0_1, void, EndTabBar, (Context*,ctx),
"Only call EndTabBar() if BeginTabBar() returns true!")
{
  FRAME_GUARD;
  ImGui::EndTabBar();
}

API_ENUM(0_1, ImGui, TabBarFlags_None, "");
API_ENUM(0_1, ImGui, TabBarFlags_Reorderable,
R"(Allow manually dragging tabs to re-order them + New tabs are appended at
   the end of list.)");
API_ENUM(0_1, ImGui, TabBarFlags_AutoSelectNewTabs,
  "Automatically select new tabs when they appear.");
API_ENUM(0_1, ImGui, TabBarFlags_TabListPopupButton,
  "Disable buttons to open the tab list popup.");
API_ENUM(0_1, ImGui, TabBarFlags_NoCloseWithMiddleMouseButton,
R"(Disable behavior of closing tabs (that are submitted with p_open != nil)
   with middle mouse button. You may handle this behavior manually on user's
   side with if(IsItemHovered() && IsMouseClicked(2)) p_open = false.)");
API_ENUM(0_1, ImGui, TabBarFlags_NoTabListScrollingButtons,
R"(Disable scrolling buttons (apply when fitting policy is
   TabBarFlags_FittingPolicyScroll).)");
API_ENUM(0_1, ImGui, TabBarFlags_NoTooltip,
  "Disable tooltips when hovering a tab.");
API_ENUM(0_9_2, ImGui, TabBarFlags_DrawSelectedOverline,
  "Draw selected overline markers over selected tab");
API_ENUM(0_1, ImGui, TabBarFlags_FittingPolicyResizeDown,
  "Resize tabs when they don't fit.");
API_ENUM(0_1, ImGui, TabBarFlags_FittingPolicyScroll,
  "Add scroll buttons when tabs don't fit.");

API_SUBSECTION("Tab Item");

API_FUNC(0_1, bool, BeginTabItem, (Context*,ctx)
(const char*,label) (RWO<bool*>,p_open) (RO<int*>,flags,ImGuiTabItemFlags_None),
R"(Create a Tab. Returns true if the Tab is selected.
Set 'p_open' to true to enable the close button.)")
{
  FRAME_GUARD;
  return ImGui::BeginTabItem(label, openPtrBehavior(p_open), API_GET(flags));
}

API_FUNC(0_1, void, EndTabItem, (Context*,ctx),
"Only call EndTabItem() if BeginTabItem() returns true!")
{
  FRAME_GUARD;
  ImGui::EndTabItem();
}

API_FUNC(0_1, bool, TabItemButton, (Context*,ctx)
(const char*,label) (RO<int*>,flags,ImGuiTabItemFlags_None),
R"(Create a Tab behaving like a button. Return true when clicked.
Cannot be selected in the tab bar.)")
{
  FRAME_GUARD;
  return ImGui::TabItemButton(label, API_GET(flags));
}

API_FUNC(0_1, void, SetTabItemClosed, (Context*,ctx)
(const char*,tab_or_docked_window_label),
R"(Notify TabBar or Docking system of a closed tab/window ahead
(useful to reduce visual flicker on reorderable tab bars).
For tab-bar: call after BeginTabBar and before Tab submissions.
Otherwise call with a window name.)")
{
  FRAME_GUARD;
  ImGui::SetTabItemClosed(tab_or_docked_window_label);
}

API_ENUM(0_1, ImGui, TabItemFlags_None, "");
API_ENUM(0_1, ImGui, TabItemFlags_UnsavedDocument,
  "Display a dot next to the title + set TabItemFlags_NoAssumedClosure.");
API_ENUM(0_1, ImGui, TabItemFlags_SetSelected,
  "Trigger flag to programmatically make the tab selected when calling BeginTabItem.");
API_ENUM(0_1, ImGui, TabItemFlags_NoCloseWithMiddleMouseButton,
R"(Disable behavior of closing tabs (that are submitted with p_open != nil) with
   middle mouse button. You can still repro this behavior on user's side with
   if(IsItemHovered() && IsMouseClicked(2)) p_open = false.)");
API_ENUM(0_1, ImGui, TabItemFlags_NoPushId,
  "Don't call PushID()/PopID() on BeginTabItem/EndTabItem.");
API_ENUM(0_1, ImGui, TabItemFlags_NoTooltip,
  "Disable tooltip for the given tab.");
API_ENUM(0_1, ImGui, TabItemFlags_NoReorder,
  "Disable reordering this tab or having another tab cross over this tab.");
API_ENUM(0_1, ImGui, TabItemFlags_Leading,
  "Enforce the tab position to the left of the tab bar (after the tab list popup button).");
API_ENUM(0_1, ImGui, TabItemFlags_Trailing,
  "Enforce the tab position to the right of the tab bar (before the scrolling buttons).");
API_ENUM(0_9, ImGui, TabItemFlags_NoAssumedClosure,
R"(Tab is selected when trying to close + closure is not immediately assumed
   (will wait for user to stop submitting the tab).
   Otherwise closure is assumed when pressing the X, so if you keep submitting
   the tab may reappear at end of tab bar.)");
