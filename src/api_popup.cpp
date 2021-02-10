#include "api_helper.hpp"

DEFINE_API(bool, BeginPopup, ((ImGui_Context*,ctx))
((const char*,str_id))((int*,flagsInOptional)),
R"(Popups, Modals
 - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
 - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
 - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
 - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
 - You can bypass the hovering restriction by using ImGuiHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
 - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.
   This is sometimes leading to confusing mistakes. May rework this in the future.
Popups: begin/end functions
 - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards. ImGuiWindowFlags are forwarded to the window.
 - BeginPopupModal(): block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar.

Return true if the popup is open, and you can start outputting to it.

Default values: flags = ImGui_WindowFlags_None)",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::BeginPopup(str_id, valueOr(flagsInOptional, ImGuiWindowFlags_None));
});

// IMGUI_API bool          BeginPopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
DEFINE_API(void, EndPopup, ((ImGui_Context*,ctx)),
"only call EndPopup() if BeginPopupXXX() returns true!",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndPopup();
});

// Popups: open/close functions
//  - OpenPopup(): set popup state to open. ImGuiPopupFlags are available for opening options.
//  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
//  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
//  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME: need some options).
//  - Use ImGuiPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().
DEFINE_API(void, OpenPopup, ((ImGui_Context*,ctx))
((const char*,str_id))((int*,flagsInOptional)),
R"(Set popup state to open (don't call every frame!).

If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.

Default values: flags = ImGui_PopupFlags_None)",
{
  ENTER_CONTEXT(ctx);
  ImGui::OpenPopup(str_id, valueOr(flagsInOptional, ImGuiPopupFlags_None));
});

IMGUI_API void          OpenPopupOnItemClick(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);   // helper to open popup when clicked on last item. return true when just opened. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
IMGUI_API void          CloseCurrentPopup();                                                                // manually close the popup we have begin-ed into.
// Popups: open+begin combined functions helpers
//  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
//  - They are convenient to easily create context menus, hence the name.
//  - IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add ImGuiWindowFlags to the BeginPopupContextXXX functions in the future.
//  - IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.
IMGUI_API bool          BeginPopupContextItem(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked on last item. if you can pass a NULL str_id only if the previous item had an id. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
IMGUI_API bool          BeginPopupContextWindow(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);// open+begin popup when clicked on current window.
IMGUI_API bool          BeginPopupContextVoid(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked in void (where there are no windows).
// Popups: test function
//  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
//  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
//  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId + ImGuiPopupFlags_AnyPopupLevel: return true if any popup is open.
IMGUI_API bool          IsPopupOpen(const char* str_id, ImGuiPopupFlags flags = 0);                         // return true if the popup is open.
