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

DEFINE_API(bool, BeginPopup, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags)),
R"(Popups, Modals
- They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
- If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
- Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
- The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
- You can bypass the hovering restriction by using ImGuiHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
- IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.

Query popup state, if open start appending into the window. Call EndPopup() afterwards. ImGuiWindowFlags are forwarded to the window.

Return true if the popup is open, and you can start outputting to it.

Default values: flags = ImGui_WindowFlags_None)",
{
  FRAME_GUARD;
  return ImGui::BeginPopup(str_id, valueOr(API_RO(flags), ImGuiWindowFlags_None));
});

DEFINE_API(bool, BeginPopupModal, (ImGui_Context*,ctx)
(const char*,name)(bool*,API_W(p_open))(int*,API_RO(flags)),
R"(Block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar. Return true if the modal is open, and you can start outputting to it. See ImGui_BeginPopup.

Default values: flags = ImGui_WindowFlags_None)",
{
  FRAME_GUARD;
  return ImGui::BeginPopupModal(name, openPtrBehavior(API_W(p_open)),
    valueOr(API_RO(flags), ImGuiWindowFlags_None));
});

DEFINE_API(void, EndPopup, (ImGui_Context*,ctx),
"only call EndPopup() if BeginPopupXXX() returns true!",
{
  FRAME_GUARD;
  ImGui::EndPopup();
});

DEFINE_API(void, OpenPopup, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(popup_flags)),
R"(Set popup state to open (don't call every frame!). ImGuiPopupFlags are available for opening options.

If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
Use ImGuiPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().

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
R"(Manually close the popup we have begin-ed into. Use inside the BeginPopup()/EndPopup() scope to close manually.

CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activateda)",
{
  FRAME_GUARD;
  ImGui::CloseCurrentPopup();
});

DEFINE_API(bool, BeginPopupContextItem, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(This is a helper to handle the simplest case of associating one named popup to one given widget. You can pass a NULL str_id to use the identifier of the last item. This is essentially the same as calling OpenPopupOnItemClick() + BeginPopup() but written to avoid computing the ID twice because BeginPopupContextXXX functions may be called very frequently.

Open+begin popup when clicked on last item. if you can pass a NULL str_id only if the previous item had an id. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here.

- IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup().
- IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.

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
R"(Return true if the popup is open at the current BeginPopup() level of the popup stack.

With ImGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
With ImGuiPopupFlags_AnyPopupId + ImGuiPopupFlags_AnyPopupLevel: return true if any popup is open.

Default values: flags = ImGui_PopupFlags_None)",
{
  FRAME_GUARD;
  const ImGuiPopupFlags flags { valueOr(API_RO(flags), ImGuiPopupFlags_None) };
  return ImGui::IsPopupOpen(str_id, flags);
});
