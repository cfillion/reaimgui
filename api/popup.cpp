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

#include "flags.hpp"

API_SECTION("Popup & Modal",
R"(- They block normal mouse hovering detection (and therefore most mouse
  interactions) behind them.
- If not modal: they can be closed by clicking anywhere outside them, or by
  pressing ESCAPE.
- Their visibility state (~bool) is held internally instead of being held by the
  programmer as we are used to with regular Begin*() calls.

The 3 properties above are related: we need to retain popup visibility state in
the library because popups may be closed as any time.

You can bypass the hovering restriction by using
HoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered or IsWindowHovered.

IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup
and BeginPopup generally needs to be at the same level of the stack.)");

API_FUNC(0_1, bool, BeginPopup, (Context*,ctx)
(const char*,str_id) (RO<int*>,flags,ImGuiWindowFlags_None),
R"(Query popup state, if open start appending into the window. Call EndPopup
afterwards if returned true. WindowFlags* are forwarded to the window.

Return true if the popup is open, and you can start outputting to it.)")
{
  FRAME_GUARD;
  return ImGui::BeginPopup(str_id, WindowFlags {API_GET(flags)});
}

API_FUNC(0_1, bool, BeginPopupModal, (Context*,ctx)
(const char*,name) (RWO<bool*>,p_open) (RO<int*>,flags,ImGuiWindowFlags_None),
R"(Block every interaction behind the window, cannot be closed by user, add a
dimming background, has a title bar. Return true if the modal is open, and you
can start outputting to it. See BeginPopup.)")
{
  FRAME_GUARD;
  WindowFlags clean_flags {API_GET(flags)};
  return ImGui::BeginPopupModal(name, openPtrBehavior(p_open), clean_flags);
}

API_FUNC(0_8, void, EndPopup, (Context*,ctx),
"Only call EndPopup() if BeginPopup*() returns true!")
{
  FRAME_GUARD;
  ImGui::EndPopup();
}

API_FUNC(0_1, void, OpenPopup, (Context*,ctx)
(const char*,str_id) (RO<int*>,popup_flags,ImGuiPopupFlags_None),
R"(Set popup state to open (don't call every frame!).
ImGuiPopupFlags are available for opening options.

If not modal: they can be closed by clicking anywhere outside them, or by
pressing ESCAPE.

Use PopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's
already one at the same level.)")
{
  FRAME_GUARD;
  ImGui::OpenPopup(str_id, API_GET(popup_flags));
}

API_FUNC(0_1, void, OpenPopupOnItemClick, (Context*,ctx)
(RO<const char*>,str_id) (RO<int*>,popup_flags,ImGuiPopupFlags_MouseButtonRight),
R"(Helper to open popup when clicked on last item. return true when just opened.
(Note: actually triggers on the mouse _released_ event to be consistent with
popup behaviors.))")
{
  FRAME_GUARD;
  nullIfEmpty(str_id);
  ImGui::OpenPopupOnItemClick(str_id, API_GET(popup_flags));
}

API_FUNC(0_1, void, CloseCurrentPopup, (Context*,ctx),
R"(Manually close the popup we have begin-ed into.
Use inside the BeginPopup/EndPopup scope to close manually.

CloseCurrentPopup() is called by default by Selectable/MenuItem when activated.)")
{
  FRAME_GUARD;
  ImGui::CloseCurrentPopup();
}

API_FUNC(0_1, bool, IsPopupOpen, (Context*,ctx)
(const char*,str_id) (RO<int*>,flags,ImGuiPopupFlags_None),
R"(Return true if the popup is open at the current BeginPopup level of the
popup stack.

- With PopupFlags_AnyPopupId: return true if any popup is open at the current
  BeginPopup() level of the popup stack.
- With PopupFlags_AnyPopupId + PopupFlags_AnyPopupLevel: return true if any
  popup is open.)")
{
  FRAME_GUARD;
  return ImGui::IsPopupOpen(str_id, API_GET(flags));
}

API_SECTION_DEF(flags, ROOT_SECTION, "Flags");
API_ENUM(0_1, ImGui, PopupFlags_None, "");
API_SECTION_P(flags, "For OpenPopup* and BeginPopupContext*");
API_ENUM(0_1, ImGui, PopupFlags_NoOpenOverExistingPopup,
  "Don't open if there's already a popup at the same level of the popup stack.");
API_ENUM(0_9, ImGui, PopupFlags_NoReopen,
R"(Don't reopen same popup if already open
   (won't reposition, won't reinitialize navigation).)");
API_SECTION_P(flags, "For BeginPopupContext*");
API_ENUM(0_1, ImGui, PopupFlags_NoOpenOverItems,
R"(For BeginPopupContextWindow: don't return true when hovering items,
   only when hovering empty space.)");
API_ENUM(0_1, ImGui, PopupFlags_MouseButtonLeft,
R"(Open on Left Mouse release.
   Guaranteed to always be == 0 (same as MouseButton_Left).)");
API_ENUM(0_1, ImGui, PopupFlags_MouseButtonRight,
R"(Open on Right Mouse release.
   Guaranteed to always be == 1 (same as MouseButton_Right).)");
API_ENUM(0_1, ImGui, PopupFlags_MouseButtonMiddle,
R"(Open on Middle Mouse release.
   Guaranteed to always be == 2 (same as MouseButton_Middle).)");
API_SECTION_P(flags, "For IsPopupOpen");
API_ENUM(0_1, ImGui, PopupFlags_AnyPopupId,
  "Ignore the str_id parameter and test for any popup.");
API_ENUM(0_1, ImGui, PopupFlags_AnyPopupLevel,
  "Search/test at any level of the popup stack (default test in the current level).");
API_ENUM(0_1, ImGui, PopupFlags_AnyPopup,
  "PopupFlags_AnyPopupId | PopupFlags_AnyPopupLevel");

API_SUBSECTION("Open+Begin Combined Helpers", R"(
Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g.
hovering an item and right-clicking. They are convenient to easily create
context menus, hence the name.

Notice that BeginPopupContext* takes PopupFlags_* just like OpenPopup and
unlike BeginPopup.

We exceptionally default their flags to 1 (== PopupFlags_MouseButtonRight) for
backward compatibility with older API taking 'int mouse_button = 1' parameter,
so if you add other flags remember to re-add the PopupFlags_MouseButtonRight.)");

API_FUNC(0_1, bool, BeginPopupContextItem, (Context*,ctx)
(RO<const char*>,str_id) (RO<int*>,popup_flags,ImGuiPopupFlags_MouseButtonRight),
R"(This is a helper to handle the simplest case of associating one named popup
to one given widget. You can pass a nil str_id to use the identifier of the last
item. This is essentially the same as calling OpenPopupOnItemClick + BeginPopup
but written to avoid computing the ID twice because BeginPopupContext*
functions may be called very frequently.

If you want to use that on a non-interactive item such as Text you need to pass
in an explicit ID here.)")
{
  FRAME_GUARD;
  nullIfEmpty(str_id);
  return ImGui::BeginPopupContextItem(str_id, API_GET(popup_flags));
}

API_FUNC(0_1, bool, BeginPopupContextWindow, (Context*,ctx)
(RO<const char*>,str_id) (RO<int*>,popup_flags,ImGuiPopupFlags_MouseButtonRight),
"Open+begin popup when clicked on current window.")
{
  FRAME_GUARD;
  nullIfEmpty(str_id);
  return ImGui::BeginPopupContextWindow(str_id, API_GET(popup_flags));
}

API_SUBSECTION("Tooltips",
R"(Tooltips are windows following the mouse. They do not take focus away.
A tooltip window can contain items of any type.)");

API_FUNC(0_1, bool, BeginTooltip, (Context*,ctx),
"Begin/append a tooltip window.")
{
  FRAME_GUARD;
  return ImGui::BeginTooltip();
}

API_FUNC(0_8, void, EndTooltip, (Context*,ctx),
"Only call EndTooltip() if BeginTooltip()/BeginItemTooltip() returns true.")
{
  FRAME_GUARD;
  ImGui::EndTooltip();
}

API_FUNC(0_1, void, SetTooltip, (Context*,ctx) (const char*,text),
R"(Set a text-only tooltip. Often used after a IsItemHovered() check.
Override any previous call to SetTooltip.

Shortcut for `if (BeginTooltip()) { Text(...); EndTooltip(); }`.)")
{
  FRAME_GUARD;
  ImGui::SetTooltip("%s", text);
}

API_FUNC(0_9, bool, BeginItemTooltip, (Context*,ctx),
R"(Begin/append a tooltip window if preceding item was hovered. Shortcut for
`IsItemHovered(HoveredFlags_ForTooltip) && BeginTooltip()`.)")
{
  FRAME_GUARD;
  return ImGui::BeginItemTooltip();
}

API_FUNC(0_9, void, SetItemTooltip, (Context*,ctx) (const char*,text),
R"(Set a text-only tooltip if preceding item was hovered.
Override any previous call to SetTooltip(). Shortcut for
`if (IsItemHovered(HoveredFlags_ForTooltip)) { SetTooltip(...); }`.)")
{
  FRAME_GUARD;
  ImGui::SetItemTooltip("%s", text);
}
