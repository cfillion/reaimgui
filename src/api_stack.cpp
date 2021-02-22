#include "api_helper.hpp"

DEFINE_API(void, PushID, ((ImGui_Context*,ctx))
((const char*,str_id)),
R"(Push string into the ID stack. Read the FAQ for more details about how ID are handled in dear imgui.
If you are creating widgets in a loop you most likely want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.)",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PushID(str_id);
});

DEFINE_API(void, PopID, ((ImGui_Context*,ctx)),
"Pop from the ID stack.",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PopID();
});

// Parameters stacks (shared)

DEFINE_API(void, PushAllowKeyboardFocus, ((ImGui_Context*,ctx))
((bool,allowKeyboardFocus)),
"Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PushAllowKeyboardFocus(allowKeyboardFocus);
});

DEFINE_API(void, PopAllowKeyboardFocus, ((ImGui_Context*,ctx)),
"See ImGui_PushAllowKeyboardFocus",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PopAllowKeyboardFocus();
});

DEFINE_API(void, PushButtonRepeat, ((ImGui_Context*,ctx))
((bool,repeat)),
"In 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PushButtonRepeat(repeat);
});


DEFINE_API(void, PopButtonRepeat, ((ImGui_Context*,ctx)),
"See ImGui_PushButtonRepeat",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PopButtonRepeat();
});

// Parameters stacks (current window)
DEFINE_API(void, PushItemWidth, ((ImGui_Context*,ctx))
((double,itemWidth)),
R"(Push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side). 0.0f = default to ~2/3 of windows width,)",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PushItemWidth(itemWidth);
});

DEFINE_API(void, PopItemWidth, ((ImGui_Context*,ctx)),
"See ImGui_PushItemWidth",
{
  ensureContext(ctx)->enterFrame();
  ImGui::PopItemWidth();
});

DEFINE_API(void, SetNextItemWidth, ((ImGui_Context*,ctx))
((double,itemWidth)),
R"(Set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side))",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SetNextItemWidth(itemWidth);
});
// IMGUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
