#include "api_helper.hpp"

DEFINE_API(void, PushID, ((ImGui_Context*,ctx))
((const char*,str_id)),
R"(Push string into the ID stack. Read the FAQ for more details about how ID are handled in dear imgui.
If you are creating widgets in a loop you most likely want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.)",
{
  ENTER_CONTEXT(ctx);
  ImGui::PushID(str_id);
});

DEFINE_API(void, PopID, ((ImGui_Context*,ctx)),
"Pop from the ID stack.",
{
  ENTER_CONTEXT(ctx);
  ImGui::PopID();
});

// Parameters stacks (shared)
// IMGUI_API void          PushFont(ImFont* font);                                         // use NULL as a shortcut to push default font
// IMGUI_API void          PopFont();
// IMGUI_API void          PushStyleColor(ImGuiCol idx, ImU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
// IMGUI_API void          PushStyleColor(ImGuiCol idx, const ImVec4& col);
// IMGUI_API void          PopStyleColor(int count = 1);

DEFINE_API(void, PushAllowKeyboardFocus, ((ImGui_Context*,ctx))((bool,allowKeyboardFocus)),
"Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets",
{
  ENTER_CONTEXT(ctx);
  ImGui::PushAllowKeyboardFocus(allowKeyboardFocus);
});

DEFINE_API(void, PopAllowKeyboardFocus, ((ImGui_Context*,ctx)),
"See ImGui_PushAllowKeyboardFocus",
{
  ENTER_CONTEXT(ctx);
  ImGui::PopAllowKeyboardFocus();
});

DEFINE_API(void, PushButtonRepeat, ((ImGui_Context*,ctx))((bool,repeat)),
"In 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.",
{
  ENTER_CONTEXT(ctx);
  ImGui::PushButtonRepeat(repeat);
});


DEFINE_API(void, PopButtonRepeat, ((ImGui_Context*,ctx)),
"See ImGui_PushButtonRepeat",
{
  ENTER_CONTEXT(ctx);
  ImGui::PopButtonRepeat();
});

// Parameters stacks (current window)
// IMGUI_API void          PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side). 0.0f = default to ~2/3 of windows width,
// IMGUI_API void          PopItemWidth();
DEFINE_API(void, SetNextItemWidth, ((ImGui_Context*,ctx))
((double,itemWidth)),
R"(Set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side))",
{
  ENTER_CONTEXT(ctx);
  ImGui::SetNextItemWidth(itemWidth);
});
// IMGUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
