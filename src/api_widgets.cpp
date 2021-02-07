#include "api_helper.hpp"

DEFINE_API(bool, Button, ((Window*,window))
((const char*,label))((double*,widthInOptional))((double*,heightInOptional)),
R"(Most widgets return true when the value has been changed or when pressed/selected
You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.)",
{
  USE_WINDOW(window, false);

  return ImGui::Button(label,
    ImVec2(valueOr(widthInOptional, 0.0), valueOr(heightInOptional, 0.0)));
});

DEFINE_API(bool, SmallButton, ((Window*,window))((const char*,label)),
"button with FramePadding=(0,0) to easily embed within text",
{
  USE_WINDOW(window, false);
  return ImGui::SmallButton(label);
});
// IMGUI_API bool          InvisibleButton(const char* str_id, const ImVec2& size, ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)

DEFINE_API(bool, ArrowButton, ((Window*,window))
((const char*,str_id))((int,dir)),
"Square button with an arrow shape",
{
  USE_WINDOW(window, false);
  return ImGui::ArrowButton(str_id, dir);
});

// IMGUI_API void          Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
// IMGUI_API bool          ImageButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding

DEFINE_API(bool, Checkbox, ((Window*, window))
((const char*, label))((bool*, valueInOut)),
"",
{
  USE_WINDOW(window, false);
  if(!valueInOut)
    return false;
  return ImGui::Checkbox(label, valueInOut);
});

DEFINE_API(bool, CheckboxFlags, ((Window*,window))
((const char*,label))((int*,flagsInOut))((int,flagsValue)), // unsigned int* is broken in REAPER
"",
{
  USE_WINDOW(window, false);
  return ImGui::CheckboxFlags(label, flagsInOut, flagsValue);
});

DEFINE_API(bool, RadioButton, ((Window*,window))
((const char*,label))((bool active)),
R"(Use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; })",
{
  USE_WINDOW(window, false);
  return ImGui::RadioButton(label, active);
});

DEFINE_API(bool, RadioButtonEx, ((Window*,window))
((const char*,label))((int*,valueInOut))((int,valueButton)),
"Shortcut to handle RadioButton's example pattern when value is an integer",
{
  USE_WINDOW(window, false);
  return ImGui::RadioButton(label, valueInOut, valueButton);
});

// IMGUI_API void          ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0), const char* overlay = NULL);

DEFINE_API(void, Bullet, ((Window*,window)),
"Draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses.",
{
  USE_WINDOW(window);
  ImGui::Bullet();
});
