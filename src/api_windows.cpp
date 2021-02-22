#include "api_helper.hpp"

#include <imgui/imgui.h>

// Cannot name 'open' as 'openInOutOptional' (and have it listed in the docs as
// a return value) because REAPER would always make it non-null.
DEFINE_API(bool, Begin, ((ImGui_Context*,ctx))
((const char*, name))((bool*, API_RWO(open)))((int*, API_RO(windowFlags))),
R"(Push window to the stack and start appending to it. See ImGui_End.

- Passing 'open' shows a window-closing widget in the upper-right corner of the window, which clicking will set the boolean to false when clicked.
- You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times. Some information such as 'flags' or 'open' will only be considered by the first call to Begin().
- Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
  [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
- Note that the bottom of window stack always contains a window called "Debug".)",
{
  Context::check(ctx)->enterFrame();
  ImGuiWindowFlags flags { valueOr(API_RO(windowFlags), 0) };
  flags |= ImGuiWindowFlags_NoSavedSettings;
  return ImGui::Begin(name, API_RWO(open), flags);
});

DEFINE_API(void, End, ((ImGui_Context*,ctx)),
R"(Pop window from the stack. See ImGui_Begin.)",
{
  Context::check(ctx)->enterFrame();
  ImGui::End();
});

DEFINE_API(bool, BeginChild, ((ImGui_Context*,ctx))
((const char*,str_id))((double*,API_RO(width)))((double*,API_RO(height)))
((bool*,API_RO(border)))((int*,API_RO(flags))),
R"(Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
- For each independent axis of 'size': ==0.0f: use remaining host window size / >0.0f: fixed size / <0.0f: use remaining window size minus abs(size) / Each axis can use a different mode, e.g. ImVec2(0,400).
- BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window.
  Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
  [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
   BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
   returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]

Default values: width = 0.0, height = 0.0, border = false, flags = ImGui_WindowFlags_None)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::BeginChild(str_id,
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)),
    valueOr(API_RO(border), false), valueOr(API_RO(flags), ImGuiWindowFlags_None));
});

DEFINE_API(void, EndChild, ((ImGui_Context*,ctx)),
"See ImGui_BeginChild.",
{
  Context::check(ctx)->enterFrame();
  ImGui::EndChild();
});

DEFINE_API(bool, IsWindowAppearing, ((ImGui_Context*,ctx)),
"",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsWindowAppearing();
});

DEFINE_API(bool, IsWindowCollapsed, ((ImGui_Context*,ctx)),
"",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsWindowCollapsed();
});

DEFINE_API(bool, IsWindowFocused, ((ImGui_Context*,ctx))
((int*,API_RO(flags))),
R"(Is current window focused? or its root/child, depending on flags. see flags for options.

Default values: flags = ImGui_FocusedFlags_None)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsWindowFocused(valueOr(API_RO(flags), ImGuiFocusedFlags_None));
});

DEFINE_API(bool, IsWindowHovered, ((ImGui_Context*,ctx))
((int*,API_RO(flags))),
R"(Is current window hovered (and typically: not blocked by a popup/modal)? see flags for options.

Default values: flags = ImGui_HoveredFlags_None)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsWindowHovered(valueOr(API_RO(flags), ImGuiHoveredFlags_None));
});

DEFINE_API(void, SetNextWindowPos, ((ImGui_Context*,ctx))
((double,x))((double,y))((int*,API_RO(cond)))
((double*,API_RO(pivotX)))((double*,API_RO(pivotY))),
R"(Set next window position. Call before Begin(). Use pivot=(0.5f,0.5f) to center on given point, etc.

Default values: cond = ImGui_Cond_Always, pivotX = 0.0, pivotY = 0.0)",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetNextWindowPos(ImVec2(x, y), valueOr(API_RO(cond), ImGuiCond_Always),
    ImVec2(valueOr(API_RO(pivotX), 0.0), valueOr(API_RO(pivotY), 0.0)));
});

DEFINE_API(void, SetNextWindowSize, ((ImGui_Context*,ctx))
((double,x))((double,y))((int*,API_RO(cond))),
R"(Set next window size. set axis to 0.0f to force an auto-fit on this axis. Call before Begin().

Default values: cond = ImGui_Cond_Always)",
{
  Context::check(ctx)->enterFrame();
  ImGui::SetNextWindowSize(ImVec2(x, y), valueOr(API_RO(cond), ImGuiCond_Always));
});

// DEFINE_API(void, SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be rounded down. Use callback to apply non-trivial programmatic constraints.
// DEFINE_API(void, SetNextWindowContentSize(const ImVec2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
// DEFINE_API(void, SetNextWindowCollapsed(bool collapsed, ImGuiCond cond = 0);                 // set next window collapsed state. call before Begin()
// DEFINE_API(void, SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
// DEFINE_API(void, SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGuiWindowFlags_NoBackground.


    IMGUI_API ImVec2        GetContentRegionAvail();                                        // == GetContentRegionMax() - GetCursorPos()
    IMGUI_API ImVec2        GetContentRegionMax();                                          // current content boundaries (typically window boundaries including scrolling, or current column boundaries), in windows coordinates
    IMGUI_API ImVec2        GetWindowContentRegionMin();                                    // content boundaries min (roughly (0,0)-Scroll), in window coordinates
    IMGUI_API ImVec2        GetWindowContentRegionMax();                                    // content boundaries max (roughly (0,0)+Size-Scroll) where Size can be override with SetNextWindowContentSize(), in window coordinates
DEFINE_API(double, GetWindowContentRegionWidth, ((ImGui_Context*,ctx)),
"",
{
  Context::check(ctx)->enterFrame();
  return ImGui::GetWindowContentRegionWidth();
});
