#include "api_helper.hpp"

// - Most of the functions are referring to the last/previous item we submitted.
// - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
DEFINE_API(bool, IsItemHovered, ((ImGui_Context*,ctx))((int*,API_RO(flags))),
"Is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGuiHoveredFlags for more options.",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::IsItemHovered(valueOr(API_RO(flags), 0));
});
// IMGUI_API bool          IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
// IMGUI_API bool          IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?

DEFINE_API(bool, IsItemClicked, ((ImGui_Context*,ctx))((int*,API_RO(mouseButton))),
R"(Is the last item clicked? (e.g. button/node just clicked on) == IsMouseClicked(mouse_button) && IsItemHovered().

Default values: mousebutton = ImGui_MouseButton_Left)",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::IsItemClicked(valueOr(API_RO(mouseButton), ImGuiMouseButton_Left));
});

// IMGUI_API bool          IsItemVisible();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
// IMGUI_API bool          IsItemEdited();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
// IMGUI_API bool          IsItemActivated();                                                  // was the last item just made active (item was previously inactive).
// IMGUI_API bool          IsItemDeactivated();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that requires continuous editing.
// IMGUI_API bool          IsItemDeactivatedAfterEdit();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that requires continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
// IMGUI_API bool          IsItemToggledOpen();                                                // was the last item open state toggled? set by TreeNode().
// IMGUI_API bool          IsAnyItemHovered();                                                 // is any item hovered?
// IMGUI_API bool          IsAnyItemActive();                                                  // is any item active?
// IMGUI_API bool          IsAnyItemFocused();                                                 // is any item focused?

DEFINE_API(void, GetItemRectMin, ((ImGui_Context*,ctx))
((double*,API_W(x)))((double*,API_W(y))),
"Get upper-left bounding rectangle of the last item (screen space)",
{
  ENTER_CONTEXT(ctx);
  const ImVec2 &rect { ImGui::GetItemRectMin() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectMax, ((ImGui_Context*,ctx))
((double*,API_W(x)))((double*,API_W(y))),
"Get lower-right bounding rectangle of the last item (screen space)",
{
  ENTER_CONTEXT(ctx);
  const ImVec2 &rect { ImGui::GetItemRectMax() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectSize, ((ImGui_Context*,ctx))
((double*,API_W(w)))((double*,API_W(h))),
"Get size of last item",
{
  ENTER_CONTEXT(ctx);
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
  ENTER_CONTEXT(ctx);
  ImGui::SetItemDefaultFocus();
});

DEFINE_API(void, SetKeyboardFocusHere, ((ImGui_Context*,ctx))
((int*,API_RO(offset))),
R"(Focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

Default values: offset = 0)",
{
  ENTER_CONTEXT(ctx);
  ImGui::SetKeyboardFocusHere(valueOr(API_RO(offset), 0));
});
