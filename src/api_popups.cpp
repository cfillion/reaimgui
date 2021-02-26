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
  Context::check(ctx)->enterFrame();
  return ImGui::BeginPopup(str_id, valueOr(API_RO(flags), ImGuiWindowFlags_None));
});

DEFINE_API(bool, BeginPopupModal, (ImGui_Context*,ctx)
(const char*,name)(bool*,API_RWO(p_open))(int*,API_RO(flags)),
R"(Block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar. Return true if the modal is open, and you can start outputting to it. See ImGui_BeginPopup.

Default values: flags = ImGui_WindowFlags_None)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::BeginPopupModal(name, API_RWO(p_open),
    valueOr(API_RO(flags), ImGuiWindowFlags_None));
});

DEFINE_API(void, EndPopup, (ImGui_Context*,ctx),
"only call EndPopup() if BeginPopupXXX() returns true!",
{
  Context::check(ctx)->enterFrame();
  ImGui::EndPopup();
});

DEFINE_API(void, OpenPopup, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags)),
R"(Set popup state to open (don't call every frame!). ImGuiPopupFlags are available for opening options.

If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
Use ImGuiPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().

Default values: flags = ImGui_PopupFlags_None)",
{
  Context::check(ctx)->enterFrame();
  ImGui::OpenPopup(str_id, valueOr(API_RO(flags), ImGuiPopupFlags_None));
});

DEFINE_API(void, OpenPopupOnItemClick, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(Helper to open popup when clicked on last item. return true when just opened. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)

Default values: str_id = nil, popup_plags = ImGui_PopupFlags_MouseButtonRight)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(str_id));

  ImGui::OpenPopupOnItemClick(API_RO(str_id),
    valueOr(API_RO(popup_flags), ImGuiPopupFlags_MouseButtonRight));
});

DEFINE_API(void, CloseCurrentPopup, (ImGui_Context*,ctx),
R"(Manually close the popup we have begin-ed into. Use inside the BeginPopup()/EndPopup() scope to close manually.

CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activateda)",
{
  Context::check(ctx)->enterFrame();
  ImGui::CloseCurrentPopup();
});

DEFINE_API(bool, BeginPopupContextItem, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(This is a helper to handle the simplest case of associating one named popup to one given widget. You can pass a NULL str_id to use the identifier of the last item. This is essentially the same as calling OpenPopupOnItemClick() + BeginPopup() but written to avoid computing the ID twice because BeginPopupContextXXX functions may be called very frequently.

Open+begin popup when clicked on last item. if you can pass a NULL str_id only if the previous item had an id. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here.

- IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup().
- IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(str_id));

  return ImGui::BeginPopupContextItem(API_RO(str_id),
    valueOr(API_RO(popup_flags), ImGuiPopupFlags_MouseButtonRight));
});

DEFINE_API(bool, BeginPopupContextWindow, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(Open+begin popup when clicked on current window.

Default values: str_id = nil, popup_flags = ImGui_PopupFlags_MouseButtonRight)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(str_id));

  return ImGui::BeginPopupContextWindow(API_RO(str_id),
    valueOr(API_RO(popup_flags), ImGuiPopupFlags_MouseButtonRight));
});

DEFINE_API(bool, BeginPopupContextVoid, (ImGui_Context*,ctx)
(const char*,API_RO(str_id))(int*,API_RO(popup_flags)),
R"(Open+begin popup when clicked in void (where there are no windows).

Default values: str_id = nil, popup_flags = ImGui_PopupFlags_MouseButtonRight)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(str_id));

  return ImGui::BeginPopupContextVoid(API_RO(str_id),
    valueOr(API_RO(popup_flags), ImGuiPopupFlags_MouseButtonRight));
});

// Popups: test function
//  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
//  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
//  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId + ImGuiPopupFlags_AnyPopupLevel: return true if any popup is open.
IMGUI_API bool          IsPopupOpen(const char* str_id, ImGuiPopupFlags flags = 0);                         // return true if the popup is open.
