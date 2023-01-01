/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

DEFINE_API(bool, BeginPopup, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags),ImGuiWindowFlags_None),
R"(Query popup state, if open start appending into the window. Call EndPopup
afterwards. WindowFlags* are forwarded to the window.

Return true if the popup is open, and you can start outputting to it.)",
{
  FRAME_GUARD;
  return ImGui::BeginPopup(str_id, WindowFlags { API_RO_GET(flags) });
});

DEFINE_API(bool, BeginPopupModal, (ImGui_Context*,ctx)
(const char*,name)(bool*,API_RWO(p_open))
(int*,API_RO(flags),ImGuiWindowFlags_None),
R"(Block every interaction behind the window, cannot be closed by user, add a
dimming background, has a title bar. Return true if the modal is open, and you
can start outputting to it. See BeginPopup.)",
{
  FRAME_GUARD;
  WindowFlags flags { API_RO_GET(flags) };
  return ImGui::BeginPopupModal(name, openPtrBehavior(API_RWO(p_open)), flags);
});

DEFINE_API(void, EndPopup, (ImGui_Context*,ctx),
"Only call EndPopup() if BeginPopupXXX() returns true!",
{
  FRAME_GUARD;
  ImGui::EndPopup();
});

DEFINE_API(void, OpenPopup, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(popup_flags),ImGuiPopupFlags_None),
R"(Set popup state to open (don't call every frame!).
ImGuiPopupFlags are available for opening options.

If not modal: they can be closed by clicking anywhere outside them, or by
pressing ESCAPE.

Use PopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's
already one at the same level.)",
{
  FRAME_GUARD;
  ImGui::OpenPopup(str_id, API_RO_GET(popup_flags));
});

DEFINE_API(void, OpenPopupOnItemClick, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))
(int*,API_RO(popup_flags),ImGuiPopupFlags_MouseButtonRight),
R"(Helper to open popup when clicked on last item. return true when just opened.
(Note: actually triggers on the mouse _released_ event to be consistent with
popup behaviors.))",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(str_id));

  ImGui::OpenPopupOnItemClick(API_RO(str_id), API_RO_GET(popup_flags));
});

DEFINE_API(void, CloseCurrentPopup, (ImGui_Context*,ctx),
R"(Manually close the popup we have begin-ed into.
Use inside the BeginPopup/EndPopup scope to close manually.

CloseCurrentPopup() is called by default by Selectable/MenuItem when activated.)",
{
  FRAME_GUARD;
  ImGui::CloseCurrentPopup();
});

DEFINE_API(bool, IsPopupOpen, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags),ImGuiPopupFlags_None),
R"(Return true if the popup is open at the current BeginPopup level of the
popup stack.

- With PopupFlags_AnyPopupId: return true if any popup is open at the current
  BeginPopup() level of the popup stack.
- With PopupFlags_AnyPopupId + PopupFlags_AnyPopupLevel: return true if any
  popup is open.)",
{
  FRAME_GUARD;
  return ImGui::IsPopupOpen(str_id, API_RO_GET(flags));
});

DEFINE_SECTION(flags, ROOT_SECTION, "Flags",
  "For OpenPopup*(), BeginPopupContext*() and IsPopupOpen.");

DEFINE_ENUM(ImGui, PopupFlags_None, "");
DEFINE_ENUM(ImGui, PopupFlags_NoOpenOverExistingPopup,
R"(For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup
   at the same level of the popup stack.)");
API_SECTION_P(flags, "BeginPopupContext*");
DEFINE_ENUM(ImGui, PopupFlags_NoOpenOverItems,
R"(For BeginPopupContextWindow: don't return true when hovering items,
   only when hovering empty space.)");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonLeft,
R"(For BeginPopupContext*(): open on Left Mouse release.
   Guaranteed to always be == 0 (same as MouseButton_Left).)");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonRight,
R"(For BeginPopupContext*(): open on Right Mouse release.
   Guaranteed to always be == 1 (same as MouseButton_Right).)");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonMiddle,
R"(For BeginPopupContext*(): open on Middle Mouse release.
   Guaranteed to always be == 2 (same as MouseButton_Middle).)");
API_SECTION_P(flags, "IsPopupOpen");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopupId,
  "For IsPopupOpen: ignore the str_id parameter and test for any popup.");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopupLevel,
R"(For IsPopupOpen: search/test at any level of the popup stack
  (default test in the current level).)");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopup,
  "PopupFlags_AnyPopupId | PopupFlags_AnyPopupLevel");

API_SUBSECTION("Open+begin combined helpers", R"(
Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g.
hovering an item and right-clicking. They are convenient to easily create
context menus, hence the name.

Notice that BeginPopupContextXXX takes PopupFlags just like OpenPopup and
unlike BeginPopup.

We exceptionally default their flags to 1 (== PopupFlags_MouseButtonRight) for
backward compatibility with older API taking 'int mouse_button = 1' parameter,
so if you add other flags remember to re-add the PopupFlags_MouseButtonRight.)");

DEFINE_API(bool, BeginPopupContextItem, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))
(int*,API_RO(popup_flags),ImGuiPopupFlags_MouseButtonRight),
R"(This is a helper to handle the simplest case of associating one named popup
to one given widget. You can pass a nil str_id to use the identifier of the last
item. This is essentially the same as calling OpenPopupOnItemClick + BeginPopup
but written to avoid computing the ID twice because BeginPopupContextXXX
functions may be called very frequently.

If you want to use that on a non-interactive item such as Text you need to pass
in an explicit ID here.)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(str_id));

  return ImGui::BeginPopupContextItem(API_RO(str_id), API_RO_GET(popup_flags));
});

DEFINE_API(bool, BeginPopupContextWindow, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))
(int*,API_RO(popup_flags),ImGuiPopupFlags_MouseButtonRight),
"Open+begin popup when clicked on current window.",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(str_id));

  return ImGui::BeginPopupContextWindow(API_RO(str_id), API_RO_GET(popup_flags));
});

API_SUBSECTION("Tooltips",
  "Tooltip are windows following the mouse. They do not take focus away.");

DEFINE_API(void, BeginTooltip, (ImGui_Context*,ctx),
R"(Begin/append a tooltip window.
To create full-featured tooltip (with any kind of items).)",
{
  FRAME_GUARD;
  ImGui::BeginTooltip();
});

DEFINE_API(void, EndTooltip, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  ImGui::EndTooltip();
});

DEFINE_API(void, SetTooltip, (ImGui_Context*,ctx)(const char*,text),
R"(Set a text-only tooltip, typically use with IsItemHovered. override any
previous call to SetTooltip.)",
{
  FRAME_GUARD;
  ImGui::SetTooltip("%s", text);
});
