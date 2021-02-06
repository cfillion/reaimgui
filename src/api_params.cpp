#include "api_helper.hpp"

// Parameters stacks (shared)
// IMGUI_API void          PushFont(ImFont* font);                                         // use NULL as a shortcut to push default font
// IMGUI_API void          PopFont();
// IMGUI_API void          PushStyleColor(ImGuiCol idx, ImU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
// IMGUI_API void          PushStyleColor(ImGuiCol idx, const ImVec4& col);
// IMGUI_API void          PopStyleColor(int count = 1);
// IMGUI_API void          PushStyleVar(ImGuiStyleVar idx, float val);                     // modify a style float variable. always use this if you modify the style after NewFrame().
// IMGUI_API void          PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);             // modify a style ImVec2 variable. always use this if you modify the style after NewFrame().
// IMGUI_API void          PopStyleVar(int count = 1);

DEFINE_API(void, PushAllowKeyboardFocus, ((Window*,window))((bool,allowKeyboardFocus)),
"Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets",
{
  USE_WINDOW(window);
  ImGui::PushAllowKeyboardFocus(allowKeyboardFocus);
});

DEFINE_API(void, PopAllowKeyboardFocus, ((Window*,window)),
"See ImGui_PushAllowKeyboardFocus",
{
  USE_WINDOW(window);
  ImGui::PopAllowKeyboardFocus();
});

DEFINE_API(void, PushButtonRepeat, ((Window*,window))((bool,repeat)),
"In 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.",
{
  USE_WINDOW(window);
  ImGui::PushButtonRepeat(repeat);
});


DEFINE_API(void, PopButtonRepeat, ((Window*,window)),
"See ImGui_PushButtonRepeat",
{
  USE_WINDOW(window);
  ImGui::PopButtonRepeat();
});

// Parameters stacks (current window)
// IMGUI_API void          PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side). 0.0f = default to ~2/3 of windows width,
// IMGUI_API void          PopItemWidth();
// IMGUI_API void          SetNextItemWidth(float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
// IMGUI_API float         CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.

DEFINE_API(void, PushTextWrapPos, ((Window*,window))
((double*,wrapLocalPosXInOptional)),
"Push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space",
{
  USE_WINDOW(window);
  ImGui::PushTextWrapPos(valueOr(wrapLocalPosXInOptional, 0.0));
});

DEFINE_API(void, PopTextWrapPos, ((Window*,window)),
"",
{
  USE_WINDOW(window);
  ImGui::PopTextWrapPos();
});
