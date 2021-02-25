#include "api_helper.hpp"

DEFINE_API(void, PushID, (ImGui_Context*,ctx)
(const char*,str_id),
R"(Push string into the ID stack. Read the FAQ for more details about how ID are handled in dear imgui.
If you are creating widgets in a loop you most likely want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.)",
{
  Context::check(ctx)->enterFrame();
  ImGui::PushID(str_id);
});

DEFINE_API(void, PopID, (ImGui_Context*,ctx),
"Pop from the ID stack.",
{
  Context::check(ctx)->enterFrame();
  ImGui::PopID();
});

DEFINE_API(bool, IsItemHovered, (ImGui_Context*,ctx)(int*,API_RO(flags)),
R"(Is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGui_HoveredFlags_* for more options.

Default values: flags = ImGui_HoveredFlags_None)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemHovered(valueOr(API_RO(flags), 0));
});

DEFINE_API(bool, IsItemActive, (ImGui_Context*,ctx),
"Is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemActive();
});

DEFINE_API(bool, IsItemFocused, (ImGui_Context*,ctx),
"Is the last item focused for keyboard/gamepad navigation?",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemFocused();
});

DEFINE_API(bool, IsItemClicked, (ImGui_Context*,ctx)(int*,API_RO(mouseButton)),
R"(Is the last item clicked? (e.g. button/node just clicked on) == IsMouseClicked(mouse_button) && IsItemHovered().

Default values: mousebutton = ImGui_MouseButton_Left)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemClicked(valueOr(API_RO(mouseButton), ImGuiMouseButton_Left));
});

DEFINE_API(bool, IsItemVisible, (ImGui_Context*,ctx),
"Is the last item visible? (items may be out of sight because of clipping/scrolling)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemVisible();
});

DEFINE_API(bool, IsItemEdited, (ImGui_Context*,ctx),
R"(Did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemEdited();
});

DEFINE_API(bool, IsItemActivated, (ImGui_Context*,ctx),
"Was the last item just made active (item was previously inactive).",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemActivated();
});

DEFINE_API(bool, IsItemDeactivated, (ImGui_Context*,ctx),
"Was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that requires continuous editing.",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemDeactivated();
});

DEFINE_API(bool, IsItemDeactivatedAfterEdit, (ImGui_Context*,ctx),
"Was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that requires continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemDeactivatedAfterEdit();
});

DEFINE_API(bool, IsItemToggledOpen, (ImGui_Context*,ctx),
"Was the last item open state toggled? Set by TreeNode().",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsItemToggledOpen();
});

DEFINE_API(bool, IsAnyItemHovered, (ImGui_Context*,ctx),
"is any item hovered?",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsAnyItemHovered();
});

DEFINE_API(bool, IsAnyItemActive, (ImGui_Context*,ctx),
"is any item active?",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsAnyItemActive();
});

DEFINE_API(bool, IsAnyItemFocused, (ImGui_Context*,ctx),
"is any item focused?",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsAnyItemFocused();
});

DEFINE_API(void, GetItemRectMin, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Get upper-left bounding rectangle of the last item (screen space)",
{
  Context::check(ctx)->enterFrame();
  const ImVec2 &rect { ImGui::GetItemRectMin() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectMax, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Get lower-right bounding rectangle of the last item (screen space)",
{
  Context::check(ctx)->enterFrame();
  const ImVec2 &rect { ImGui::GetItemRectMax() };
  if(API_W(x)) *API_W(x) = rect.x;
  if(API_W(y)) *API_W(y) = rect.y;
});

DEFINE_API(void, GetItemRectSize, (ImGui_Context*,ctx)
(double*,API_W(w))(double*,API_W(h)),
"Get size of last item",
{
  Context::check(ctx)->enterFrame();
  const ImVec2 &rect { ImGui::GetItemRectSize() };
  if(API_W(w)) *API_W(w) = rect.x;
  if(API_W(h)) *API_W(h) = rect.y;
});

DEFINE_API(void, SetItemAllowOverlap, (ImGui_Context*,ctx),
"Allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetItemAllowOverlap();
});

// Focus, Activation
DEFINE_API(void, SetItemDefaultFocus, (ImGui_Context*,ctx),
R"~(Make last item the default focused item of a window.

Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when applicable to signify "this is the default item")~",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetItemDefaultFocus();
});

DEFINE_API(void, SetKeyboardFocusHere, (ImGui_Context*,ctx)
(int*,API_RO(offset)),
R"(Focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

Default values: offset = 0)",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetKeyboardFocusHere(valueOr(API_RO(offset), 0));
});

DEFINE_API(void, PushAllowKeyboardFocus, (ImGui_Context*,ctx)
(bool,allowKeyboardFocus),
"Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets",
{
  Context::check(ctx)->enterFrame();
  ImGui::PushAllowKeyboardFocus(allowKeyboardFocus);
});

DEFINE_API(void, PopAllowKeyboardFocus, (ImGui_Context*,ctx),
"See ImGui_PushAllowKeyboardFocus",
{
  Context::check(ctx)->enterFrame();
  ImGui::PopAllowKeyboardFocus();
});

DEFINE_API(void, PushButtonRepeat, (ImGui_Context*,ctx)
(bool,repeat),
"In 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.",
{
  Context::check(ctx)->enterFrame();
  ImGui::PushButtonRepeat(repeat);
});


DEFINE_API(void, PopButtonRepeat, (ImGui_Context*,ctx),
"See ImGui_PushButtonRepeat",
{
  Context::check(ctx)->enterFrame();
  ImGui::PopButtonRepeat();
});

DEFINE_API(void, PushTextWrapPos, (ImGui_Context*,ctx)
(double*,API_RO(wrapLocalPosX)),
R"(Push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space.

Default values: wrapLocalPosX = 0.0)",
{
  Context::check(ctx)->enterFrame();
  ImGui::PushTextWrapPos(valueOr(API_RO(wrapLocalPosX), 0.0));
});

DEFINE_API(void, PopTextWrapPos, (ImGui_Context*,ctx),
"",
{
  Context::check(ctx)->enterFrame();
  ImGui::PopTextWrapPos();
});

// Parameters stacks (current window)
DEFINE_API(void, PushItemWidth, (ImGui_Context*,ctx)
(double,itemWidth),
R"(Push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side). 0.0f = default to ~2/3 of windows width,)",
{
  Context::check(ctx)->enterFrame();
  ImGui::PushItemWidth(itemWidth);
});

DEFINE_API(void, PopItemWidth, (ImGui_Context*,ctx),
"See ImGui_PushItemWidth",
{
  Context::check(ctx)->enterFrame();
  ImGui::PopItemWidth();
});

DEFINE_API(void, SetNextItemOpen, (ImGui_Context*,ctx)
(bool,isOpen)(int*,API_RO(cond)),
R"(Set next TreeNode/CollapsingHeader open state. Can also be done with the ImGui_TreeNodeFlags_DefaultOpen flag.

'cond' is ImGui_Cond_Always by default.)",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetNextItemOpen(isOpen, valueOr(API_RO(cond), ImGuiCond_Always));
});

DEFINE_API(void, SetNextItemWidth, (ImGui_Context*,ctx)
(double,itemWidth),
R"(Set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side))",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetNextItemWidth(itemWidth);
});
// IMGUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
