#include "api_helper.hpp"

DEFINE_API(bool, Button, ((ImGui_Context*,ctx))
((const char*,label))((double*,widthInOptional))((double*,heightInOptional)),
R"(Most widgets return true when the value has been changed or when pressed/selected
You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.)",
{
  ENTER_CONTEXT(ctx, false);

  return ImGui::Button(label,
    ImVec2(valueOr(widthInOptional, 0.0), valueOr(heightInOptional, 0.0)));
});

DEFINE_API(bool, SmallButton, ((ImGui_Context*,ctx))((const char*,label)),
"button with FramePadding=(0,0) to easily embed within text",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::SmallButton(label);
});
// IMGUI_API bool          InvisibleButton(const char* str_id, const ImVec2& size, ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)

DEFINE_API(bool, ArrowButton, ((ImGui_Context*,ctx))
((const char*,str_id))((int,dir)),
"Square button with an arrow shape",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::ArrowButton(str_id, dir);
});

// IMGUI_API void          Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
// IMGUI_API bool          ImageButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding

DEFINE_API(bool, Checkbox, ((ImGui_Context*,ctx))
((const char*, label))((bool*, valueInOut)),
"",
{
  ENTER_CONTEXT(ctx, false);
  if(!valueInOut)
    return false;
  return ImGui::Checkbox(label, valueInOut);
});

DEFINE_API(bool, CheckboxFlags, ((ImGui_Context*,ctx))
((const char*,label))((int*,flagsInOut))((int,flagsValue)), // unsigned int* is broken in REAPER
"",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::CheckboxFlags(label, flagsInOut, flagsValue);
});

DEFINE_API(bool, RadioButton, ((ImGui_Context*,ctx))
((const char*,label))((bool active)),
R"(Use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; })",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::RadioButton(label, active);
});

DEFINE_API(bool, RadioButtonEx, ((ImGui_Context*,ctx))
((const char*,label))((int*,valueInOut))((int,valueButton)),
"Shortcut to handle RadioButton's example pattern when value is an integer",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::RadioButton(label, valueInOut, valueButton);
});

// IMGUI_API void          ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0), const char* overlay = NULL);

DEFINE_API(void, Bullet, ((ImGui_Context*,ctx)),
"Draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses.",
{
  ENTER_CONTEXT(ctx);
  ImGui::Bullet();
});

DEFINE_API(bool, Selectable, ((ImGui_Context*,ctx))
((const char*,label))((bool*,selectedInOutOptional))
((int*,flagsInOptional))((double*,widthInOptional))((double*,heightInOptional)),
R"(A selectable highlights when hovered, and can display another color when selected.
Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.

Default values: flags = ImGui_SelectableFlags_None, width = 0.0, height = 0.0)",
{
  ENTER_CONTEXT(ctx, false);

  bool selectedOmitted {};
  bool *selected { selectedInOutOptional ? selectedInOutOptional : &selectedOmitted };
  return ImGui::Selectable(label, selected,
    valueOr(flagsInOptional, ImGuiSelectableFlags_None),
    ImVec2(valueOr(widthInOptional, 0.0), valueOr(heightInOptional, 0.0)));
});
