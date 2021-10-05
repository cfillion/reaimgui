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

#include "flags.hpp"

DEFINE_API(bool, BeginPopup, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags)),
R"(Popups, Modals

- They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
- If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
- Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
- The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
- You can bypass the hovering restriction by using ImGui_HoveredFlags_AllowWhenBlockedByPopup when calling ImGui_IsItemHovered or ImGui_IsWindowHovered.
- IMPORTANT: Popup identifiers are relative to the current ID stack, so ImGui_OpenPopup and BeginPopup generally needs to be at the same level of the stack.

Query popup state, if open start appending into the window. Call ImGui_EndPopup afterwards. ImGui_WindowFlags* are forwarded to the window.

Return true if the popup is open, and you can start outputting to it.

Default values: flags = ImGui_WindowFlags_None)",
{
  FRAME_GUARD;
  WindowFlags flags { API_RO(flags) };
  return ImGui::BeginPopup(str_id, flags);
});

DEFINE_API(bool, BeginPopupModal, (ImGui_Context*,ctx)
(const char*,name)(bool*,API_RWO(p_open))(int*,API_RO(flags)),
R"(Block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar. Return true if the modal is open, and you can start outputting to it. See ImGui_BeginPopup.

Default values: p_open = nil, flags = ImGui_WindowFlags_None)",
{
  FRAME_GUARD;
  WindowFlags flags { API_RO(flags) };
  return ImGui::BeginPopupModal(name, openPtrBehavior(API_RWO(p_open)), flags);
});

DEFINE_API(void, EndPopup, (ImGui_Context*,ctx),
"Only call EndPopup() if BeginPopupXXX() returns true!",
{
  FRAME_GUARD;
  ImGui::EndPopup();
});

DEFINE_API(void, OpenPopup, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(popup_flags)),
R"(Set popup state to open (don't call every frame!). ImGuiPopupFlags are available for opening options.

If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
Use ImGui_PopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level.

Default values: popup_flags = ImGui_PopupFlags_None)",
{
  FRAME_GUARD;
  ImGui::OpenPopup(str_id, valueOr(API_RO(popup_flags), ImGuiPopupFlags_None));
});

DEFINE_API(void, OpenPopupOnItemClick, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(Helper to open popup when clicked on last item. return true when just opened. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)

Default values: str_id = nil, popup_flags = ImGui_PopupFlags_MouseButtonRight)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(str_id));

  ImGui::OpenPopupOnItemClick(API_RO(str_id),
    valueOr(API_RO(popup_flags), ImGuiPopupFlags_MouseButtonRight));
});

DEFINE_API(void, CloseCurrentPopup, (ImGui_Context*,ctx),
R"(Manually close the popup we have begin-ed into. Use inside the ImGUi_BeginPopup/ImGui_EndPopup scope to close manually.

CloseCurrentPopup() is called by default by ImGui_Selectable/ImGui_MenuItem when activated.)",
{
  FRAME_GUARD;
  ImGui::CloseCurrentPopup();
});

DEFINE_API(bool, BeginPopupContextItem, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(This is a helper to handle the simplest case of associating one named popup to one given widget. You can pass a NULL str_id to use the identifier of the last item. This is essentially the same as calling ImGui_OpenPopupOnItemClick + ImGui_BeginPopup but written to avoid computing the ID twice because BeginPopupContextXXX functions may be called very frequently.

Open+begin popup when clicked on last item. if you can pass a NULL str_id only if the previous item had an id. If you want to use that on a non-interactive item such as ImGui_Text you need to pass in an explicit ID here.

- IMPORTANT: Notice that BeginPopupContextXXX takes ImGui_PopupFlags just like ImGui_OpenPopup and unlike ImGui_BeginPopup.
- IMPORTANT: We exceptionally default their flags to 1 (== ImGui_PopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add the ImGui_PopupFlags_MouseButtonRight.

Default values: str_id = nil, popup_flags = ImGui_PopupFlags_MouseButtonRight)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(str_id));

  return ImGui::BeginPopupContextItem(API_RO(str_id),
    valueOr(API_RO(popup_flags), ImGuiPopupFlags_MouseButtonRight));
});

DEFINE_API(bool, BeginPopupContextWindow, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(Open+begin popup when clicked on current window.

Default values: str_id = nil, popup_flags = ImGui_PopupFlags_MouseButtonRight)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(str_id));

  return ImGui::BeginPopupContextWindow(API_RO(str_id),
    valueOr(API_RO(popup_flags), ImGuiPopupFlags_MouseButtonRight));
});

DEFINE_API(bool, IsPopupOpen, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags)),
R"(Return true if the popup is open at the current ImGui_BeginPopup level of the popup stack.

With ImGui_PopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
With ImGui_PopupFlags_AnyPopupId + ImGui_PopupFlags_AnyPopupLevel: return true if any popup is open.

Default values: flags = ImGui_PopupFlags_None)",
{
  FRAME_GUARD;
  const ImGuiPopupFlags flags { valueOr(API_RO(flags), ImGuiPopupFlags_None) };
  return ImGui::IsPopupOpen(str_id, flags);
});

DEFINE_API(void, BeginTooltip, (ImGui_Context*,ctx),
"Begin/append a tooltip window. To create full-featured tooltip (with any kind of items).",
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
"Set a text-only tooltip, typically use with ImGui_IsItemHovered. override any previous call to ImGui_SetTooltip.",
{
  FRAME_GUARD;
  ImGui::SetTooltip("%s", text);
});

// ImGuiPopupFlags
DEFINE_ENUM(ImGui, PopupFlags_None,                    "Flags for OpenPopup*(), BeginPopupContext*(), ImGui_IsPopupOpen.");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonLeft,         "For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as ImGui_MouseButton_Left).");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonRight,        "For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as ImGui_MouseButton_Right).");
DEFINE_ENUM(ImGui, PopupFlags_MouseButtonMiddle,       "For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as ImGui_MouseButton_Middle).");
DEFINE_ENUM(ImGui, PopupFlags_NoOpenOverExistingPopup, "For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack.");
DEFINE_ENUM(ImGui, PopupFlags_NoOpenOverItems,         "For ImGui_BeginPopupContextWindow: don't return true when hovering items, only when hovering empty space.");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopupId,              "For ImGui_IsPopupOpen: ignore the str_id parameter and test for any popup.");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopupLevel,           "For ImGui_IsPopupOpen: search/test at any level of the popup stack (default test in the current level).");
DEFINE_ENUM(ImGui, PopupFlags_AnyPopup,                "ImGui_PopupFlags_AnyPopupId | ImGui_PopupFlags_AnyPopupLevel");
