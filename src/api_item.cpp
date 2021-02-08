#include "api_helper.hpp"

// - Most of the functions are referring to the last/previous item we submitted.
// - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
DEFINE_API(bool, IsItemHovered, ((Window*,window))((int*,flagsInOptional)),
"Is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGuiHoveredFlags for more options.",
{
  USE_WINDOW(window, false);
  return ImGui::IsItemHovered(valueOr(flagsInOptional, 0));
});
// IMGUI_API bool          IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
// IMGUI_API bool          IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?

DEFINE_API(bool, IsItemClicked, ((Window*,window))((int*,mouseButtonInOptional)),
R"(Is the last item clicked? (e.g. button/node just clicked on) == IsMouseClicked(mouse_button) && IsItemHovered().

'mouseButton' is ImGui_MouseButton_Left by default.)",
{
  USE_WINDOW(window, false);
  return ImGui::IsItemClicked(valueOr(mouseButtonInOptional, ImGuiMouseButton_Left));
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

DEFINE_API(void, GetItemRectMin, ((Window*,window))
((double*,xOut))((double*,yOut)),
"Get upper-left bounding rectangle of the last item (screen space)",
{
  USE_WINDOW(window);
  const ImVec2 &rect { ImGui::GetItemRectMin() };
  if(xOut) *xOut = rect.x;
  if(yOut) *yOut = rect.y;
});

DEFINE_API(void, GetItemRectMax, ((Window*,window))
((double*,xOut))((double*,yOut)),
"Get lower-right bounding rectangle of the last item (screen space)",
{
  USE_WINDOW(window);
  const ImVec2 &rect { ImGui::GetItemRectMax() };
  if(xOut) *xOut = rect.x;
  if(yOut) *yOut = rect.y;
});

DEFINE_API(void, GetItemRectSize, ((Window*,window))
((double*,wOut))((double*,hOut)),
"Get size of last item",
{
  USE_WINDOW(window);
  const ImVec2 &rect { ImGui::GetItemRectSize() };
  if(wOut) *wOut = rect.x;
  if(hOut) *hOut = rect.y;
});

// IMGUI_API void          SetItemAllowOverlap(); // allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.

// Focus, Activation
DEFINE_API(void, SetItemDefaultFocus, ((Window*,window)),
R"~(Make last item the default focused item of a window.

Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when applicable to signify "this is the default item")~",
{
  USE_WINDOW(window);
  ImGui::SetItemDefaultFocus();
});

DEFINE_API(void, SetKeyboardFocusHere, ((Window*,window))
((int*,offsetInOptional)),
R"(Focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

Default values: offset = 0)",
{
  USE_WINDOW(window);
  ImGui::SetKeyboardFocusHere(valueOr(offsetInOptional, 0));
});
