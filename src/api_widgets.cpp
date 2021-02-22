#include "api_helper.hpp"

DEFINE_API(bool, Button, ((ImGui_Context*,ctx))
((const char*,label))((double*,API_RO(width)))((double*,API_RO(height))),
R"(Most widgets return true when the value has been changed or when pressed/selected
You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.)",
{
  Context::check(ctx)->enterFrame();

  return ImGui::Button(label,
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)));
});

DEFINE_API(bool, SmallButton, ((ImGui_Context*,ctx))((const char*,label)),
"button with FramePadding=(0,0) to easily embed within text",
{
  Context::check(ctx)->enterFrame();
  return ImGui::SmallButton(label);
});
// IMGUI_API bool          InvisibleButton(const char* str_id, const ImVec2& size, ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)

DEFINE_API(bool, ArrowButton, ((ImGui_Context*,ctx))
((const char*,str_id))((int,dir)),
"Square button with an arrow shape",
{
  Context::check(ctx)->enterFrame();
  return ImGui::ArrowButton(str_id, dir);
});

// IMGUI_API void          Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
// IMGUI_API bool          ImageButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding

DEFINE_API(bool, Checkbox, ((ImGui_Context*,ctx))
((const char*, label))((bool*, API_RW(value))),
"",
{
  Context::check(ctx)->enterFrame();
  if(!API_RW(value))
    return false;
  return ImGui::Checkbox(label, API_RW(value));
});

DEFINE_API(bool, CheckboxFlags, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(flags)))((int,flagsValue)), // unsigned int* is broken in REAPER
"",
{
  Context::check(ctx)->enterFrame();
  return ImGui::CheckboxFlags(label, API_RW(flags), flagsValue);
});

DEFINE_API(bool, RadioButton, ((ImGui_Context*,ctx))
((const char*,label))((bool,active)),
R"(Use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; })",
{
  Context::check(ctx)->enterFrame();
  return ImGui::RadioButton(label, active);
});

DEFINE_API(bool, RadioButtonEx, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value)))((int,valueButton)),
"Shortcut to handle RadioButton's example pattern when value is an integer",
{
  Context::check(ctx)->enterFrame();
  return ImGui::RadioButton(label, API_RW(value), valueButton);
});

DEFINE_API(void, ProgressBar, ((ImGui_Context*,ctx))
((double,fraction))
((double*,API_RO(width)))((double*,API_RO(height)))
((const char*,API_RO(overlay))),
"Default values: width = -FLT_MIN, height = 0.0, overlay = nil",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(overlay));

  ImGui::ProgressBar(fraction,
    ImVec2(valueOr(API_RO(width), -FLT_MIN), valueOr(API_RO(height), 0.0)),
    API_RO(overlay));
});

DEFINE_API(void, Bullet, ((ImGui_Context*,ctx)),
"Draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses.",
{
  Context::check(ctx)->enterFrame();
  ImGui::Bullet();
});

DEFINE_API(bool, Selectable, ((ImGui_Context*,ctx))
((const char*,label))((bool*,API_RWO(selected)))
((int*,API_RO(flags)))((double*,API_RO(width)))((double*,API_RO(height))),
R"(A selectable highlights when hovered, and can display another color when selected.
Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.

Default values: flags = ImGui_SelectableFlags_None, width = 0.0, height = 0.0)",
{
  Context::check(ctx)->enterFrame();

  bool selectedOmitted {};
  bool *selected { API_RWO(selected) ? API_RWO(selected) : &selectedOmitted };
  return ImGui::Selectable(label, selected,
    valueOr(API_RO(flags), ImGuiSelectableFlags_None),
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)));
});
