/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "api_helper.hpp"

#include <imgui/imgui.h>

DEFINE_API(bool, Begin, (ImGui_Context*,ctx)
(const char*,name)(bool*,API_W(p_open))(int*,API_RO(flags)),
R"(Push window to the stack and start appending to it. See ImGui_End.

- Passing true to 'open' shows a window-closing widget in the upper-right corner of the window, which clicking will set the boolean to false when returned.
- You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times. Some information such as 'flags' or 'open' will only be considered by the first call to Begin().
- Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window.
- Note that the bottom of window stack always contains a window called "Debug".

Default values: p_open = nil, flags = ImGui_WindowFlags_None)",
{
  FRAME_GUARD;

  ImGuiWindowFlags flags { valueOr(API_RO(flags), ImGuiWindowFlags_None) };

  if(!ctx->IO().ConfigViewportsNoDecoration) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    flags |= ImGuiWindowFlags_NoTitleBar |
             ImGuiWindowFlags_NoResize   ;
  }

  const bool rv { ImGui::Begin(name, openPtrBehavior(API_W(p_open)), flags) };

  if(!ctx->IO().ConfigViewportsNoDecoration)
    ImGui::PopStyleVar();

  if(!rv)
    ImGui::End();

  return rv;
});

DEFINE_API(void, End, (ImGui_Context*,ctx),
R"(Pop window from the stack. See ImGui_Begin.)",
{
  FRAME_GUARD;
  ImGui::End();
});

DEFINE_API(bool, BeginChild, (ImGui_Context*,ctx)
(const char*,str_id)(double*,API_RO(size_w))(double*,API_RO(size_h))
(bool*,API_RO(border))(int*,API_RO(flags)),
R"(Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
- For each independent axis of 'size': ==0.0f: use remaining host window size / >0.0f: fixed size / <0.0f: use remaining window size minus abs(size) / Each axis can use a different mode, e.g. ImVec2(0,400).
- BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window.

See ImGui_EndChild.

Default values: size_w = 0.0, size_h = 0.0, border = false, flags = ImGui_WindowFlags_None)",
{
  FRAME_GUARD;
  const ImGuiWindowFlags flags { valueOr(API_RO(flags), ImGuiWindowFlags_None) };
  const ImVec2 size { valueOr(API_RO(size_w), 0.f),
                      valueOr(API_RO(size_h), 0.f) };
  const bool border { valueOr(API_RO(border), false) };
  const bool rv { ImGui::BeginChild(str_id, size, border, flags) };
  if(!rv)
    ImGui::EndChild();
  return rv;
});

DEFINE_API(void, EndChild, (ImGui_Context*,ctx),
"See ImGui_BeginChild.",
{
  FRAME_GUARD;
  ImGui::EndChild();
});

DEFINE_API(bool, BeginChildFrame, (ImGui_Context*,ctx)
(const char*,str_id)(double,size_w)(double,size_h)(int*,API_RO(flags)),
R"(Helper to create a child window / scrolling region that looks like a normal widget frame. See ImGui_EndChildFrame.

Default values: flags = ImGui_WindowFlags_None)",
{
  FRAME_GUARD;
  const ImGuiID id { ImGui::GetID(str_id) };
  ImGuiWindowFlags flags { valueOr(API_RO(flags), ImGuiWindowFlags_None) };
  const bool rv { ImGui::BeginChildFrame(id, ImVec2(size_w, size_h), flags) };
  if(!rv)
    ImGui::EndChildFrame();
  return rv;
});

DEFINE_API(void, EndChildFrame, (ImGui_Context*,ctx),
"See ImGui_BeginChildFrame.",
{
  FRAME_GUARD;
  ImGui::EndChildFrame();
});

DEFINE_API(void, BeginTooltip, (ImGui_Context*,ctx),
"Begin/append a tooltip window. to create full-featured tooltip (with any kind of items).",
{
  FRAME_GUARD;
  ImGui::BeginTooltip();
});

DEFINE_API(void, EndTooltip, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  ImGui::EndTooltip();
});

DEFINE_API(void, SetTooltip, (ImGui_Context*,ctx)(const char*,text),
"Set a text-only tooltip, typically use with ImGui_IsItemHovered(). override any previous call to SetTooltip().",
{
  FRAME_GUARD;
  ImGui::SetTooltip("%s", text);
});

DEFINE_API(bool, IsWindowAppearing, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  return ImGui::IsWindowAppearing();
});

DEFINE_API(bool, IsWindowCollapsed, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  return ImGui::IsWindowCollapsed();
});

DEFINE_API(bool, IsWindowFocused, (ImGui_Context*,ctx)
(int*,API_RO(flags)),
R"(Is current window focused? or its root/child, depending on flags. see flags for options.

Default values: flags = ImGui_FocusedFlags_None)",
{
  FRAME_GUARD;
  return ImGui::IsWindowFocused(valueOr(API_RO(flags), ImGuiFocusedFlags_None));
});

DEFINE_API(bool, IsWindowHovered, (ImGui_Context*,ctx)
(int*,API_RO(flags)),
R"(Is current window hovered (and typically: not blocked by a popup/modal)? see flags for options.

Default values: flags = ImGui_HoveredFlags_None)",
{
  FRAME_GUARD;
  return ImGui::IsWindowHovered(valueOr(API_RO(flags), ImGuiHoveredFlags_None));
});

DEFINE_API(void, GetWindowPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Get current window position in screen space (useful if you want to do your own drawing via the DrawList API)",
{
  FRAME_GUARD;
  const ImVec2 &vec { ImGui::GetWindowPos() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
});

DEFINE_API(void, GetWindowSize, (ImGui_Context*,ctx)
(double*,API_W(w))(double*,API_W(h)),
"Get current window size",
{
  FRAME_GUARD;
  const ImVec2 &vec { ImGui::GetWindowSize() };
  if(API_W(w)) *API_W(w) = vec.x;
  if(API_W(h)) *API_W(h) = vec.y;
});

DEFINE_API(double, GetWindowWidth, (ImGui_Context*,ctx),
"Get current window width (shortcut for ({ImGui_GetWindowSize()})[1])",
{
  FRAME_GUARD;
  return ImGui::GetWindowWidth();
});

DEFINE_API(double, GetWindowHeight, (ImGui_Context*,ctx),
"Get current window height (shortcut for ({ImGui_GetWindowSize()})[2])",
{
  FRAME_GUARD;
  return ImGui::GetWindowHeight();
});

DEFINE_API(void, SetNextWindowPos, (ImGui_Context*,ctx)
(double,pos_x)(double,pos_y)(int*,API_RO(cond))
(double*,API_RO(pivot_x))(double*,API_RO(pivot_y)),
R"(Set next window position. Call before Begin(). Use pivot=(0.5,0.5) to center on given point, etc.

Default values: cond = ImGui_Cond_Always, pivot_x = 0.0, pivot_y = 0.0)",
{
  FRAME_GUARD;
  const ImGuiCond cond { valueOr(API_RO(cond), ImGuiCond_Always) };
  const ImVec2 pivot { valueOr(API_RO(pivot_x), 0.f),
                       valueOr(API_RO(pivot_y), 0.f) };
  ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), cond, pivot);
});

DEFINE_API(void, SetNextWindowSize, (ImGui_Context*,ctx)
(double,size_w)(double,size_h)(int*,API_RO(cond)),
R"(Set next window size. set axis to 0.0f to force an auto-fit on this axis. Call before Begin().

Default values: cond = ImGui_Cond_Always)",
{
  FRAME_GUARD;
  const ImGuiCond cond { valueOr(API_RO(cond), ImGuiCond_Always) };
  ImGui::SetNextWindowSize(ImVec2(size_w, size_h), cond);
});

DEFINE_API(void, SetNextWindowSizeConstraints, (ImGui_Context*,ctx)
(double,size_min_w)(double,size_min_h)(double,size_max_w)(double,size_max_h),
"Set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be rounded down.",
{
  FRAME_GUARD;
  ImGui::SetNextWindowSizeConstraints(
    ImVec2(size_min_w, size_min_h), ImVec2(size_max_w, size_max_h));
});

DEFINE_API(void, SetNextWindowContentSize, (ImGui_Context*,ctx)
(double,size_w)(double,size_h),
"Set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. Call before Begin().",
{
  FRAME_GUARD;
  ImGui::SetNextWindowContentSize(ImVec2(size_w, size_h));
});

DEFINE_API(void, SetNextWindowCollapsed, (ImGui_Context*,ctx)
(bool,collapsed)(int*,API_RO(cond)),
R"(Set next window collapsed state. Call before Begin().

Default values: cond = ImGui_Cond_Always)",
{
  FRAME_GUARD;
  const ImGuiCond cond { valueOr(API_RO(cond), ImGuiCond_Always) };
  ImGui::SetNextWindowCollapsed(collapsed, cond);
});

DEFINE_API(void, SetNextWindowFocus, (ImGui_Context*,ctx),
"Set next window to be focused / top-most. Call before Begin().",
{
  FRAME_GUARD;
  ImGui::SetNextWindowFocus();
});

DEFINE_API(void, SetNextWindowBgAlpha, (ImGui_Context*,ctx)
(double,alpha),
"Set next window background color alpha. helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGui_WindowFlags_NoBackground.",
{
  FRAME_GUARD;
  ImGui::SetNextWindowBgAlpha(alpha);
});

DEFINE_API(void, SetWindowPos, (ImGui_Context*,ctx)
(const char*,name)(double,pos_x)(double,pos_y)(int*,API_RO(cond)),
R"(Set named window position. See ImGui_SetNextWindowPos.

Default values: cond = ImGui_Cond_Always)",
{
  FRAME_GUARD;
  const ImGuiCond cond { valueOr(API_RO(cond), ImGuiCond_Always) };
  ImGui::SetWindowPos(name, ImVec2(pos_x, pos_y), cond);
});

DEFINE_API(void, SetWindowSize, (ImGui_Context*,ctx)
(const char*,name)(double,size_w)(double,size_h)(int*,API_RO(cond)),
R"(Set named window size. set axis to 0.0f to force an auto-fit on this axis. See ImGui_SetNextWindowSize.

Default values: cond = ImGui_Cond_Always)",
{
  FRAME_GUARD;
  const ImGuiCond cond { valueOr(API_RO(cond), ImGuiCond_Always) };
  ImGui::SetWindowSize(name, ImVec2(size_w, size_h), cond);
});

DEFINE_API(void, SetWindowCollapsed, (ImGui_Context*,ctx)
(const char*,name)(bool,collapsed)(int*,API_RO(cond)),
R"(Set named window collapsed state. See ImGui_SetNextWindowCollapsed.

Default values: cond = ImGui_Cond_Always)",
{
  FRAME_GUARD;
  const ImGuiCond cond { valueOr(API_RO(cond), ImGuiCond_Always) };
  ImGui::SetWindowCollapsed(name, collapsed, cond);
});

DEFINE_API(void, SetWindowFocus, (ImGui_Context*,ctx)
(const char*,name),
"Set named window to be focused / top-most. Use NULL to remove focus. See ImGui_SetNextWindowFocus.",
{
  FRAME_GUARD;
  ImGui::SetWindowFocus(name);
});

DEFINE_API(void, GetContentRegionAvail, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"== GetContentRegionMax() - GetCursorPos()",
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetContentRegionAvail() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
});

DEFINE_API(void, GetContentRegionMax, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Current content boundaries (typically window boundaries including scrolling, or current column boundaries), in windows coordinates",
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetContentRegionMax() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
});

DEFINE_API(void, GetWindowContentRegionMin, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Content boundaries min (roughly (0,0)-Scroll), in window coordinates",
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetWindowContentRegionMin() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
});

DEFINE_API(void, GetWindowContentRegionMax, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Content boundaries max (roughly (0,0)+Size-Scroll) where Size can be override with SetNextWindowContentSize(), in window coordinates",
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetWindowContentRegionMax() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
});

DEFINE_API(double, GetWindowContentRegionWidth, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  return ImGui::GetWindowContentRegionWidth();
});

// Windows Scrolling
DEFINE_API(double, GetScrollX, (ImGui_Context*,ctx),
"Get scrolling amount [0 .. GetScrollMaxX()]",
{
  FRAME_GUARD;
  return ImGui::GetScrollX();
});

DEFINE_API(double, GetScrollY, (ImGui_Context*,ctx),
"Get scrolling amount [0 .. GetScrollMaxY()]",
{
  FRAME_GUARD;
  return ImGui::GetScrollY();
});

DEFINE_API(void, SetScrollX, (ImGui_Context*,ctx)
(double,scroll_x),
"Set scrolling amount [0 .. GetScrollMaxX()]",
{
  FRAME_GUARD;
  ImGui::SetScrollX(scroll_x);
});

DEFINE_API(void, SetScrollY, (ImGui_Context*,ctx)
(double,scroll_y),
"Set scrolling amount [0 .. GetScrollMaxY()]",
{
  FRAME_GUARD;
  ImGui::SetScrollY(scroll_y);
});

DEFINE_API(double, GetScrollMaxX, (ImGui_Context*,ctx),
"Get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x",
{
  FRAME_GUARD;
  return ImGui::GetScrollMaxX();
});

DEFINE_API(double, GetScrollMaxY, (ImGui_Context*,ctx),
"Get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y",
{
  FRAME_GUARD;
  return ImGui::GetScrollMaxY();
});

DEFINE_API(void, SetScrollHereX, (ImGui_Context*,ctx)
(double*,API_RO(center_x_ratio)),
R"(Adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.

Default values: center_x_ratio = 0.5)",
{
  FRAME_GUARD;
  ImGui::SetScrollHereX(valueOr(API_RO(center_x_ratio), 0.5));
});

DEFINE_API(void, SetScrollHereY, (ImGui_Context*,ctx)
(double*,API_RO(center_y_ratio)),
R"(Adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.

Default values: center_y_ratio = 0.5)",
{
  FRAME_GUARD;
  ImGui::SetScrollHereY(valueOr(API_RO(center_y_ratio), 0.5));
});

DEFINE_API(void, SetScrollFromPosX, (ImGui_Context*,ctx)
(double,local_x)(double*,API_RO(center_x_ratio)),
R"(Adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

Default values: center_x_ratio = 0.5)",
{
  FRAME_GUARD;
  ImGui::SetScrollFromPosX(local_x, valueOr(API_RO(center_x_ratio), 0.5));
});

DEFINE_API(void, SetScrollFromPosY, (ImGui_Context*,ctx)
(double,local_y)(double*,API_RO(center_y_ratio)),
R"(Adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

Default values: center_y_ratio = 0.5)",
{
  FRAME_GUARD;
  ImGui::SetScrollFromPosY(local_y, valueOr(API_RO(center_y_ratio), 0.5));
});
