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

#include "helper.hpp"

DEFINE_API(void, GetItemRectMin, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Get upper-left bounding rectangle of the last item (screen space)",
{
  FRAME_GUARD;
  const ImVec2 &rect { ImGui::GetItemRectMin() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectMax, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Get lower-right bounding rectangle of the last item (screen space)",
{
  FRAME_GUARD;
  const ImVec2 &rect { ImGui::GetItemRectMax() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectSize, (ImGui_Context*,ctx)
(double*,API_W(w))(double*,API_W(h)),
"Get size of last item",
{
  FRAME_GUARD;
  const ImVec2 &rect { ImGui::GetItemRectSize() };
  if(API_W(w)) *API_W(w) = rect.x;
  if(API_W(h)) *API_W(h) = rect.y;
});

DEFINE_API(void, SetItemAllowOverlap, (ImGui_Context*,ctx),
"Allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.",
{
  FRAME_GUARD;
  ImGui::SetItemAllowOverlap();
});

DEFINE_API(void, BeginDisabled, (ImGui_Context*,ctx)
(bool*,API_RO(disabled)),
R"(
Disable all user interactions and dim items visuals (applying style.DisabledAlpha over current colors).

ImGui_BeginDisabled(false) essentially does nothing useful but is provided to facilitate use of boolean expressions. If you can avoid calling ImGui_BeginDisabled(False)/ImGui_EndDisabled() best to avoid it.

Defalt values: disabled = true)",
{
  FRAME_GUARD;
  ImGui::BeginDisabled(valueOr(API_RO(disabled), true));
});

DEFINE_API(void, EndDisabled, (ImGui_Context*,ctx),
"See ImGui_BeginDisabled.",
{
  FRAME_GUARD;
  ImGui::EndDisabled();
});

// Focus, Activation
DEFINE_API(void, SetItemDefaultFocus, (ImGui_Context*,ctx),
R"~(Make last item the default focused item of a window.

Prefer using "SetItemDefaultFocus()" over "if (ImGui_IsWindowAppearing()) ImGui_SetScrollHereY()" when applicable to signify "this is the default item")~",
{
  FRAME_GUARD;
  ImGui::SetItemDefaultFocus();
});

DEFINE_API(void, SetKeyboardFocusHere, (ImGui_Context*,ctx)
(int*,API_RO(offset)),
R"(Focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

Default values: offset = 0)",
{
  FRAME_GUARD;
  ImGui::SetKeyboardFocusHere(valueOr(API_RO(offset), 0));
});

DEFINE_API(void, PushAllowKeyboardFocus, (ImGui_Context*,ctx)
(bool,allow_keyboard_focus),
"Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets",
{
  FRAME_GUARD;
  ImGui::PushAllowKeyboardFocus(allow_keyboard_focus);
});

DEFINE_API(void, PopAllowKeyboardFocus, (ImGui_Context*,ctx),
"See ImGui_PushAllowKeyboardFocus",
{
  FRAME_GUARD;
  ImGui::PopAllowKeyboardFocus();
});

DEFINE_API(void, PushItemWidth, (ImGui_Context*,ctx)
(double,item_width),
R"(Push width of items for common large "item+label" widgets. >0.0: width in pixels, <0.0 align xx pixels to the right of window (so -FLT_MIN always align width to the right side). 0.0 = default to ~2/3 of windows width,)",
{
  FRAME_GUARD;
  ImGui::PushItemWidth(item_width);
});

DEFINE_API(void, PopItemWidth, (ImGui_Context*,ctx),
"See ImGui_PushItemWidth",
{
  FRAME_GUARD;
  ImGui::PopItemWidth();
});

DEFINE_API(void, SetNextItemWidth, (ImGui_Context*,ctx)
(double,item_width),
R"(Set width of the _next_ common large "item+label" widget. >0.0: width in pixels, <0.0 align xx pixels to the right of window (so -FLT_MIN always align width to the right side))",
{
  FRAME_GUARD;
  ImGui::SetNextItemWidth(item_width);
});

DEFINE_API(double, CalcItemWidth, (ImGui_Context*,ctx),
"Width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.",
{
  FRAME_GUARD;
  return ImGui::CalcItemWidth();
});

DEFINE_API(bool, IsItemHovered, (ImGui_Context*,ctx)(int*,API_RO(flags)),
R"(Is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGui_HoveredFlags_* for more options.

Default values: flags = ImGui_HoveredFlags_None)",
{
  FRAME_GUARD;
  return ImGui::IsItemHovered(valueOr(API_RO(flags), ImGuiHoveredFlags_None));
});

DEFINE_API(bool, IsItemActive, (ImGui_Context*,ctx),
"Is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)",
{
  FRAME_GUARD;
  return ImGui::IsItemActive();
});

DEFINE_API(bool, IsItemFocused, (ImGui_Context*,ctx),
"Is the last item focused for keyboard/gamepad navigation?",
{
  FRAME_GUARD;
  return ImGui::IsItemFocused();
});

DEFINE_API(bool, IsItemClicked, (ImGui_Context*,ctx)(int*,API_RO(mouse_button)),
R"(Is the last item clicked? (e.g. button/node just clicked on) == ImGui_IsMouseClicked(mouse_button) && ImGui_IsItemHovered().

This is NOT equivalent to the behavior of e.g. ImGui_Button. Most widgets have specific reactions based on mouse-up/down state, mouse position etc.

Default values: mouse_button = ImGui_MouseButton_Left)",
{
  FRAME_GUARD;
  return ImGui::IsItemClicked(valueOr(API_RO(mouse_button), ImGuiMouseButton_Left));
});

DEFINE_API(bool, IsItemVisible, (ImGui_Context*,ctx),
"Is the last item visible? (items may be out of sight because of clipping/scrolling)",
{
  FRAME_GUARD;
  return ImGui::IsItemVisible();
});

DEFINE_API(bool, IsItemEdited, (ImGui_Context*,ctx),
R"(Did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.)",
{
  FRAME_GUARD;
  return ImGui::IsItemEdited();
});

DEFINE_API(bool, IsItemActivated, (ImGui_Context*,ctx),
"Was the last item just made active (item was previously inactive).",
{
  FRAME_GUARD;
  return ImGui::IsItemActivated();
});

DEFINE_API(bool, IsItemDeactivated, (ImGui_Context*,ctx),
"Was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that requires continuous editing.",
{
  FRAME_GUARD;
  return ImGui::IsItemDeactivated();
});

DEFINE_API(bool, IsItemDeactivatedAfterEdit, (ImGui_Context*,ctx),
"Was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that requires continuous editing. Note that you may get false positives (some widgets such as ImGui_Combo/ImGui_ListBox/ImGui_Selectable will return true even when clicking an already selected item).",
{
  FRAME_GUARD;
  return ImGui::IsItemDeactivatedAfterEdit();
});

DEFINE_API(bool, IsAnyItemHovered, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  return ImGui::IsAnyItemHovered();
});

DEFINE_API(bool, IsAnyItemActive, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  return ImGui::IsAnyItemActive();
});

DEFINE_API(bool, IsAnyItemFocused, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  return ImGui::IsAnyItemFocused();
});

// ImGuiHoveredFlags
// Flags: for IsItemHovered(), IsWindowHovered() etc.
DEFINE_ENUM(ImGui, HoveredFlags_None,                         "Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.");
DEFINE_ENUM(ImGui, HoveredFlags_ChildWindows,                 "ImGui_IsWindowHovered only: Return true if any children of the window is hovered.");
DEFINE_ENUM(ImGui, HoveredFlags_RootWindow,                   "ImGui_IsWindowHovered only: Test from root window (top most parent of the current hierarchy).");
DEFINE_ENUM(ImGui, HoveredFlags_AnyWindow,                    "ImGui_IsWindowHovered only: Return true if any window is hovered.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenBlockedByPopup,      "Return true even if a popup window is normally blocking access to this item/window.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenBlockedByActiveItem, "Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenOverlapped,          "Return true even if the position is obstructed or overlapped by another window.");
DEFINE_ENUM(ImGui, HoveredFlags_AllowWhenDisabled,            "Return true even if the item is disabled.");
DEFINE_ENUM(ImGui, HoveredFlags_RectOnly,                     "ImGui_HoveredFlags_AllowWhenBlockedByPopup | ImGui_HoveredFlags_AllowWhenBlockedByActiveItem | ImGui_HoveredFlags_AllowWhenOverlapped");
DEFINE_ENUM(ImGui, HoveredFlags_RootAndChildWindows,          "ImGui_HoveredFlags_RootWindow | ImGui_HoveredFlags_ChildWindows");
