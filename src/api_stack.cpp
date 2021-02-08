#include "api_helper.hpp"

DEFINE_API(void, PushID, ((Window*,window))
((const char*,str_id)),
R"(Push string into the ID stack. Read the FAQ for more details about how ID are handled in dear imgui.
If you are creating widgets in a loop you most likely want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.)",
{
  USE_WINDOW(window);
  ImGui::PushID(str_id);
});

DEFINE_API(void, PopID, ((Window*,window)),
"Pop from the ID stack.",
{
  USE_WINDOW(window);
  ImGui::PopID();
});

// Parameters stacks (shared)
// IMGUI_API void          PushFont(ImFont* font);                                         // use NULL as a shortcut to push default font
// IMGUI_API void          PopFont();
// IMGUI_API void          PushStyleColor(ImGuiCol idx, ImU32 col);                        // modify a style color. always use this if you modify the style after NewFrame().
// IMGUI_API void          PushStyleColor(ImGuiCol idx, const ImVec4& col);
// IMGUI_API void          PopStyleColor(int count = 1);

enum class StyleVarType { Unknown, Float, ImVec2 };

static StyleVarType styleVarType(const ImGuiStyleVar var)
{
  constexpr ImGuiStyleVar floatVars[] {
    ImGuiStyleVar_Alpha,
    ImGuiStyleVar_ChildBorderSize,
    ImGuiStyleVar_ChildRounding,
    ImGuiStyleVar_FrameBorderSize,
    ImGuiStyleVar_FrameRounding,
    ImGuiStyleVar_GrabMinSize,
    ImGuiStyleVar_GrabRounding,
    ImGuiStyleVar_IndentSpacing,
    ImGuiStyleVar_PopupBorderSize,
    ImGuiStyleVar_PopupRounding,
    ImGuiStyleVar_ScrollbarRounding,
    ImGuiStyleVar_ScrollbarSize,
    ImGuiStyleVar_TabRounding,
    ImGuiStyleVar_WindowBorderSize,
    ImGuiStyleVar_WindowRounding,
  };

  constexpr ImGuiStyleVar vec2Vars[] {
    ImGuiStyleVar_ButtonTextAlign,
    ImGuiStyleVar_SelectableTextAlign,
    ImGuiStyleVar_CellPadding,
    ImGuiStyleVar_ItemSpacing,
    ImGuiStyleVar_ItemInnerSpacing,
    ImGuiStyleVar_FramePadding,
    ImGuiStyleVar_WindowPadding,
    ImGuiStyleVar_WindowMinSize,
    ImGuiStyleVar_WindowTitleAlign,
  };

  if(std::find(std::begin(floatVars), std::end(floatVars), var) != std::end(floatVars))
    return StyleVarType::Float;
  else if(std::find(std::begin(vec2Vars), std::end(vec2Vars), var) != std::end(vec2Vars))
    return StyleVarType::ImVec2;
  else
    return StyleVarType::Unknown;
}

DEFINE_API(bool, PushStyleVar, ((Window*,window))
((int,varIdx))((double,val1))((double*,val2InOptional)),
"See ImGui_StyleVar_* for possible values of 'varIdx'.",
{
  USE_WINDOW(window, false);

  switch(styleVarType(varIdx)) {
  case StyleVarType::Unknown:
    return false;
  case StyleVarType::Float:
    ImGui::PushStyleVar(varIdx, val1);
    return true;
  case StyleVarType::ImVec2:
    if(!val2InOptional)
      return false;
    ImGui::PushStyleVar(varIdx, ImVec2(val1, *val2InOptional));
    return true;
  }
});

DEFINE_API(void, PopStyleVar, ((Window*,window))
((int*,countInOptional)),
R"(Reset a style variable.

Default values: count = 1)",
{
  USE_WINDOW(window);

  ImGui::PopStyleVar(valueOr(countInOptional, 1));
});

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
R"(Push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space.

'wrapLocalPosX' defaults to 0.0.)",
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
