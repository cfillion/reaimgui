#include "api_helper.hpp"

#include <imgui/imgui.h>

// Cannot name 'open' as 'openInOutOptional' (and have it listed in the docs as
// a return value) because REAPER would always make it non-null.
DEFINE_API(bool, Begin, ((ImGui_Context*,ctx))
((const char*, name))((bool*, openInOptional))((int*, windowFlagsInOptional)),
R"(Push window to the stack and start appending to it. See ImGui_End.

- Passing 'open' shows a window-closing widget in the upper-right corner of the window, which clicking will set the boolean to false when clicked.
- You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times. Some information such as 'flags' or 'open' will only be considered by the first call to Begin().
- Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
  [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
- Note that the bottom of window stack always contains a window called "Debug".)",
{
  ENTER_CONTEXT(ctx, false);
  ImGuiWindowFlags flags { valueOr(windowFlagsInOptional, 0) };
  flags |= ImGuiWindowFlags_NoSavedSettings;
  return ImGui::Begin(name, openInOptional, flags);
});

DEFINE_API(void, End, ((ImGui_Context*,ctx)),
R"(Pop window from the stack. See ImGui_Begin.)",
{
  ENTER_CONTEXT(ctx);
  ImGui::End();
});
DEFINE_API(void, SetNextWindowPos, ((ImGui_Context*,ctx))
((double,x))((double,y))((int*,condInOptional))
((double*,pivotXInOptional))((double*,pivotYInOptional)),
R"(Set next window position. Call before Begin(). Use pivot=(0.5f,0.5f) to center on given point, etc.

Default values: cond = ImGui_Cond_Always, pivotX = 0.0, pivotY = 0.0)",
{
  ENTER_CONTEXT(ctx);
  ImGui::SetNextWindowPos(ImVec2(x, y), valueOr(condInOptional, ImGuiCond_Always),
    ImVec2(valueOr(pivotXInOptional, 0.0), valueOr(pivotYInOptional, 0.0)));
});

DEFINE_API(void, SetNextWindowSize, ((ImGui_Context*,ctx))
((double,x))((double,y))((int*,condInOptional)),
R"(Set next window size. set axis to 0.0f to force an auto-fit on this axis. Call before Begin().

Default values: cond = ImGui_Cond_Always)",
{
  ENTER_CONTEXT(ctx);
  ImGui::SetNextWindowSize(ImVec2(x, y), valueOr(condInOptional, ImGuiCond_Always));
});

// DEFINE_API(void, SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be rounded down. Use callback to apply non-trivial programmatic constraints.
// DEFINE_API(void, SetNextWindowContentSize(const ImVec2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
// DEFINE_API(void, SetNextWindowCollapsed(bool collapsed, ImGuiCond cond = 0);                 // set next window collapsed state. call before Begin()
// DEFINE_API(void, SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
// DEFINE_API(void, SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGuiWindowFlags_NoBackground.
