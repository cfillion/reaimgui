#include "api_helper.hpp"

// - Most of the functions are referring to the last/previous item we submitted.
// - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
DEFINE_API(bool, IsItemHovered, ((ImGui_Context*,ctx))((int*,API_RO(flags))),
R"(Is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGui_HoveredFlags_* for more options.

Default values: flags = ImGui_HoveredFlags_None)",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemHovered(valueOr(API_RO(flags), 0));
});

DEFINE_API(bool, IsItemActive, ((ImGui_Context*,ctx)),
"Is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemActive();
});

DEFINE_API(bool, IsItemFocused, ((ImGui_Context*,ctx)),
"Is the last item focused for keyboard/gamepad navigation?",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemFocused();
});

DEFINE_API(bool, IsItemClicked, ((ImGui_Context*,ctx))((int*,API_RO(mouseButton))),
R"(Is the last item clicked? (e.g. button/node just clicked on) == IsMouseClicked(mouse_button) && IsItemHovered().

Default values: mousebutton = ImGui_MouseButton_Left)",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemClicked(valueOr(API_RO(mouseButton), ImGuiMouseButton_Left));
});

DEFINE_API(bool, IsItemVisible, ((ImGui_Context*,ctx)),
"Is the last item visible? (items may be out of sight because of clipping/scrolling)",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemVisible();
});

DEFINE_API(bool, IsItemEdited, ((ImGui_Context*,ctx)),
R"(Did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.)",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemEdited();
});

DEFINE_API(bool, IsItemActivated, ((ImGui_Context*,ctx)),
"Was the last item just made active (item was previously inactive).",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemActivated();
});

DEFINE_API(bool, IsItemDeactivated, ((ImGui_Context*,ctx)),
"Was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that requires continuous editing.",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemDeactivated();
});

DEFINE_API(bool, IsItemDeactivatedAfterEdit, ((ImGui_Context*,ctx)),
"Was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that requires continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemDeactivatedAfterEdit();
});

DEFINE_API(bool, IsItemToggledOpen, ((ImGui_Context*,ctx)),
"Was the last item open state toggled? Set by TreeNode().",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::IsItemToggledOpen();
});

// IMGUI_API bool          IsAnyItemHovered();                                                 // is any item hovered?
// IMGUI_API bool          IsAnyItemActive();                                                  // is any item active?
// IMGUI_API bool          IsAnyItemFocused();                                                 // is any item focused?

DEFINE_API(void, GetItemRectMin, ((ImGui_Context*,ctx))
((double*,API_W(x)))((double*,API_W(y))),
"Get upper-left bounding rectangle of the last item (screen space)",
{
  ensureContext(ctx)->enterFrame();
  const ImVec2 &rect { ImGui::GetItemRectMin() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectMax, ((ImGui_Context*,ctx))
((double*,API_W(x)))((double*,API_W(y))),
"Get lower-right bounding rectangle of the last item (screen space)",
{
  ensureContext(ctx)->enterFrame();
  const ImVec2 &rect { ImGui::GetItemRectMax() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectSize, ((ImGui_Context*,ctx))
((double*,API_W(w)))((double*,API_W(h))),
"Get size of last item",
{
  ensureContext(ctx)->enterFrame();
  const ImVec2 &rect { ImGui::GetItemRectSize() };
  if(API_W(w)) *API_W(w) = rect.x;
  if(API_W(h)) *API_W(h) = rect.y;
});

// IMGUI_API void          SetItemAllowOverlap(); // allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.

// Focus, Activation
DEFINE_API(void, SetItemDefaultFocus, ((ImGui_Context*,ctx)),
R"~(Make last item the default focused item of a window.

Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when applicable to signify "this is the default item")~",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SetItemDefaultFocus();
});

DEFINE_API(void, SetKeyboardFocusHere, ((ImGui_Context*,ctx))
((int*,API_RO(offset))),
R"(Focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

Default values: offset = 0)",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SetKeyboardFocusHere(valueOr(API_RO(offset), 0));
});
