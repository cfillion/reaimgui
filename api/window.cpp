/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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

#include <version.hpp>

#include "callback.hpp"
#include "flags.hpp"
#include "helper.hpp"

#include "../src/api_eel.hpp"

#include <imgui/imgui.h>

API_SECTION("Window",
R"(Functions for creating and manipulating windows.
Note that the bottom of the window stack always contains a window called "Debug".)");

using SizeCallback = Callback<ImGuiSizeCallbackData>;

class DecorationBehavior {
public:
  DecorationBehavior(Context *ctx, ImGuiWindowFlags *flags)
    : m_enabled {!ctx->IO().ConfigViewportsNoDecoration}
  {
    if(m_enabled) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
      *flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
    }
  }

  ~DecorationBehavior()
  {
    if(m_enabled)
      ImGui::PopStyleVar();
  }

private:
  bool m_enabled;
};

static bool nativeWindowBehavior(const char *name, bool *p_open)
{
  ImGuiWindowFlags flags {};
  DecorationBehavior dec {Context::current(), &flags};
  ImGui::Begin(name, openPtrBehavior(p_open), flags);
  auto &beginCount {ImGui::GetCurrentWindow()->BeginCount};
  ImGui::End();

  --beginCount;
  return beginCount == 0;
}

API_FUNC(0_5, bool, Begin, (Context*,ctx)
(const char*,name) (RWO<bool*>,p_open) (RO<int*>,flags,ImGuiWindowFlags_None),
R"(Push window to the stack and start appending to it.

- Passing true to 'p_open' shows a window-closing widget in the upper-right
  corner of the window, which clicking will set the boolean to false when returned.
- You may append multiple times to the same window during the same frame by
  calling Begin()/End() pairs multiple times. Some information such as 'flags'
  or 'p_open' will only be considered by the first call to Begin().
- Begin() return false to indicate the window is collapsed or fully clipped,
  so you may early out and omit submitting anything to the window.)")
{
  FRAME_GUARD;

  WindowFlags clean_flags {API_GET(flags)};
  DecorationBehavior dec {ctx, *clean_flags};
  const bool rv {ImGui::Begin(name, openPtrBehavior(p_open), clean_flags)};
  if(!rv)
    ImGui::End();
  return rv;
}

API_FUNC(0_8, void, End, (Context*,ctx),
R"(Pop window from the stack. See Begin.)")
{
  FRAME_GUARD;
  ImGui::End();
}

API_SECTION_DEF(childs, ROOT_SECTION, "Child Windows",
R"(Use child windows to begin into a self-contained independent
scrolling/clipping regions within a host window.
Child windows can embed their own child.)");

API_FUNC(0_9, bool, BeginChild, (Context*,ctx)
(const char*,str_id) (RO<double*>,size_w,0.0) (RO<double*>,size_h,0.0)
(RO<int*>,child_flags,ImGuiChildFlags_None)
(RO<int*>,window_flags,ImGuiWindowFlags_None),
R"(Manual sizing (each axis can use a different setting e.g. size_w=0 and size_h=400):
- = 0.0: use remaining parent window size for this axis
- \> 0.0: use specified size for this axis
- < 0.0: right/bottom-align to specified distance from available content boundaries

Specifying ChildFlags_AutoResizeX or ChildFlags_AutoResizeY makes the sizing
automatic based on child contents.

Combining both ChildFlags_AutoResizeX _and_ ChildFlags_AutoResizeY defeats
purpose of a scrolling region and is NOT recommended.

Returns false to indicate the window is collapsed or fully clipped.)")
{
  FRAME_GUARD;
  const bool rv {
    ImGui::BeginChild(str_id, ImVec2(API_GET(size_w), API_GET(size_h)),
      API_GET(child_flags), API_GET(window_flags))
  };
  if(!rv)
    ImGui::EndChild();
  return rv;
}

API_FUNC(0_8, void, EndChild, (Context*,ctx),
"See BeginChild.")
{
  FRAME_GUARD;
  ImGui::EndChild();
}

API_SECTION_P(childs, "Child Flags",
R"(About using AutoResizeX/AutoResizeY flags:
- May be combined with SetNextWindowSizeConstraints to set a min/max size for
  each axis (see Demo > Child > Auto-resize with Constraints).
- Size measurement for a given axis is only performed when the child window is
  within visible boundaries, or is just appearing.
  - This allows BeginChild to return false when not within boundaries
    (e.g. when scrolling), which is more optimal. BUT it won't update its
    auto-size while clipped. While not perfect, it is a better default behavior
    as the always-on performance gain is more valuable than the occasional
    "resizing after becoming visible again" glitch.
  - You may also use ChildFlags_AlwaysAutoResize to force an update even when
    child window is not in view. HOWEVER PLEASE UNDERSTAND THAT DOING SO WILL
    PREVENT BeginChild FROM EVER RETURNING FALSE, disabling benefits of coarse
    clipping.)");

API_ENUM(0_9, ImGui, ChildFlags_None, "");
API_ENUM(0_9, ImGui, ChildFlags_Border, "Show an outer border and enable WindowPadding.");
API_ENUM(0_9, ImGui, ChildFlags_AlwaysUseWindowPadding,
R"(Pad with StyleVar_WindowPadding even if no border are drawn (no padding by
default for non-bordered child windows because it makes more sense).)");
API_ENUM(0_9, ImGui, ChildFlags_ResizeX,
R"(Allow resize from right border (layout direction).
Enables .ini saving (unless WindowFlags_NoSavedSettings passed to window flags).)");
API_ENUM(0_9, ImGui, ChildFlags_ResizeY,
R"(Allow resize from bottom border (layout direction).
Enables .ini saving (unless WindowFlags_NoSavedSettings passed to window flags).)");
API_ENUM(0_9, ImGui, ChildFlags_AutoResizeX, "Enable auto-resizing width. Read notes above.");
API_ENUM(0_9, ImGui, ChildFlags_AutoResizeY, "Enable auto-resizing height. Read notes above.");
API_ENUM(0_9, ImGui, ChildFlags_AlwaysAutoResize,
R"(Combined with AutoResizeX/AutoResizeY. Always measure size even when child
is hidden, always return true, always disable clipping optimization! NOT RECOMMENDED.)");
API_ENUM(0_9, ImGui, ChildFlags_FrameStyle,
R"(Style the child window like a framed item: use Col_FrameBg,
   StyleVar_FrameRounding, StyleVar_FrameBorderSize, StyleVar_FramePadding
   instead of Col_ChildBg, StyleVar_ChildRounding, StyleVar_ChildBorderSize,
   StyleVar_WindowPadding.)");
API_ENUM(0_9_2, ImGui, ChildFlags_NavFlattened,
R"(Share focus scope, allow gamepad/keyboard navigation to cross over parent
   border to this child or between sibling child windows.)");

API_SECTION_DEF(properties, ROOT_SECTION, "Properties",
R"(Prefer using SetNextWindow* functions (before Begin) rather that SetWindow* functions
(after Begin).

'Current window' = the window we are appending into while inside a Begin()/End()
block. 'Next window' = next window we will Begin() into.)");

API_FUNC(0_1, bool, IsWindowAppearing, (Context*,ctx),
"Use after Begin/BeginPopup/BeginPopupModal to tell if a window just opened.")
{
  FRAME_GUARD;
  return ImGui::IsWindowAppearing();
}

API_FUNC(0_1, bool, IsWindowFocused, (Context*,ctx)
(RO<int*>,flags,ImGuiFocusedFlags_None),
R"(Is current window focused? or its root/child, depending on flags.
See flags for options.)")
{
  FRAME_GUARD;
  return ImGui::IsWindowFocused(API_GET(flags));
}

API_FUNC(0_9, bool, IsWindowHovered, (Context*,ctx)
(RO<int*>,flags,ImGuiHoveredFlags_None),
R"(Is current window hovered and hoverable (e.g. not blocked by a popup/modal)?
See HoveredFlags_* for options.)")
{
  FRAME_GUARD;
  return ImGui::IsWindowHovered(API_GET(flags));
}

API_FUNC(0_1, void, GetWindowPos, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
R"(Get current window position in screen space (note: it is unlikely you need to
use this. Consider using current layout pos instead, GetCursorScreenPos()).)")
{
  FRAME_GUARD;
  const ImVec2 &vec {ImGui::GetWindowPos()};
  if(x) *x = vec.x;
  if(y) *y = vec.y;
}

API_FUNC(0_1, void, GetWindowSize, (Context*,ctx)
(W<double*>,w) (W<double*>,h),
R"(Get current window size (note: it is unlikely you need to use this.
Consider using GetCursorScreenPos() and e.g. GetContentRegionAvail() instead))")
{
  FRAME_GUARD;
  const ImVec2 &vec {ImGui::GetWindowSize()};
  if(w) *w = vec.x;
  if(h) *h = vec.y;
}

API_FUNC(0_1, double, GetWindowWidth, (Context*,ctx),
"Get current window width (shortcut for (GetWindowSize().w).")
{
  FRAME_GUARD;
  return ImGui::GetWindowWidth();
}

API_FUNC(0_1, double, GetWindowHeight, (Context*,ctx),
"Get current window height (shortcut for (GetWindowSize().h).")
{
  FRAME_GUARD;
  return ImGui::GetWindowHeight();
}

API_FUNC(0_7_2, double, GetWindowDpiScale, (Context*,ctx),
R"(Get DPI scale currently associated to the current window's viewport
(1.0 = 96 DPI).)")
{
  FRAME_GUARD;
  return ImGui::GetWindowDpiScale();
}

API_FUNC(0_1, void, SetNextWindowPos, (Context*,ctx)
(double,pos_x) (double,pos_y) (RO<int*>,cond,ImGuiCond_Always)
(RO<double*>,pivot_x,0.0) (RO<double*>,pivot_y,0.0),
"Set next window position. Use pivot=(0.5,0.5) to center on given point, etc.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), API_GET(cond),
    ImVec2(API_GET(pivot_x), API_GET(pivot_y)));
}

API_FUNC(0_1, void, SetNextWindowSize, (Context*,ctx)
(double,size_w) (double,size_h) (RO<int*>,cond,ImGuiCond_Always),
"Set next window size. set axis to 0.0 to force an auto-fit on this axis.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowSize(ImVec2(size_w, size_h), API_GET(cond));
}

API_FUNC(0_8_5, void, SetNextWindowSizeConstraints, (Context*,ctx)
(double,size_min_w) (double,size_min_h) (double,size_max_w) (double,size_max_h)
(RO<Function*>,custom_callback),
R"(Set next window size limits. Use 0.0 or FLT_MAX (second return value of
NumericLimits_Float) if you don't want limits.

Use -1 for both min and max of same axis to preserve current size (which itself
is a constraint).

Use callback to apply non-trivial programmatic constraints.)")
{
  FRAME_GUARD;
  ImGui::SetNextWindowSizeConstraints(
    ImVec2(size_min_w, size_min_h), ImVec2(size_max_w, size_max_h),
    SizeCallback::use(custom_callback), custom_callback);
}

API_FUNC(0_1, void, SetNextWindowContentSize, (Context*,ctx)
(double,size_w) (double,size_h),
R"(Set next window content size (~ scrollable client area, which enforce the
range of scrollbars). Not including window decorations (title bar, menu bar,
etc.) nor StyleVar_WindowPadding. set an axis to 0.0 to leave it automatic.)")
{
  FRAME_GUARD;
  ImGui::SetNextWindowContentSize(ImVec2(size_w, size_h));
}

API_FUNC(0_1, void, SetNextWindowCollapsed, (Context*,ctx)
(bool,collapsed) (RO<int*>,cond,ImGuiCond_Always),
"Set next window collapsed state.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowCollapsed(collapsed, API_GET(cond));
}

API_FUNC(0_1, void, SetNextWindowFocus, (Context*,ctx),
"Set next window to be focused / top-most.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowFocus();
}

API_FUNC(0_1, void, SetNextWindowScroll, (Context*,ctx)
(double,scroll_x) (double,scroll_y),
"Set next window scrolling value (use < 0.0 to not affect a given axis).")
{
  FRAME_GUARD;
  ImGui::SetNextWindowScroll(ImVec2(scroll_x, scroll_y));
}

API_FUNC(0_1, void, SetNextWindowBgAlpha, (Context*,ctx)
(double,alpha),
R"(Set next window background color alpha. Helper to easily override the Alpha
component of Col_WindowBg/Col_ChildBg/Col_PopupBg.
You may also use WindowFlags_NoBackground for a fully transparent window.)")
{
  FRAME_GUARD;
  ImGui::SetNextWindowBgAlpha(alpha);
}

API_FUNC(0_5, void, SetWindowPos, (Context*,ctx)
(double,pos_x) (double,pos_y) (RO<int*>,cond,ImGuiCond_Always),
R"((Not recommended) Set current window position - call within Begin/End.
Prefer using SetNextWindowPos, as this may incur tearing and minor side-effects.)")
{
  FRAME_GUARD;
  ImGui::SetWindowPos(ImVec2(pos_x, pos_y), API_GET(cond));
}

API_FUNC(0_5, void, SetWindowSize, (Context*,ctx)
(double,size_w) (double,size_h) (RO<int*>,cond,ImGuiCond_Always),
R"((Not recommended) Set current window size - call within Begin/End.
Set size_w and size_h to 0 to force an auto-fit.
Prefer using SetNextWindowSize, as this may incur tearing and minor side-effects.)")
{
  FRAME_GUARD;
  ImGui::SetWindowSize(ImVec2(size_w, size_h), API_GET(cond));
}

API_FUNC(0_5, void, SetWindowCollapsed, (Context*,ctx)
(bool,collapsed) (RO<int*>,cond,ImGuiCond_Always),
R"((Not recommended) Set current window collapsed state.
Prefer using SetNextWindowCollapsed.)")
{
  FRAME_GUARD;
  ImGui::SetWindowCollapsed(collapsed, API_GET(cond));
}

API_FUNC(0_5, void, SetWindowFocus, (Context*,ctx),
R"((Not recommended) Set current window to be focused / top-most.
Prefer using SetNextWindowFocus.)")
{
  FRAME_GUARD;
  ImGui::SetWindowFocus();
}

API_FUNC(0_5, void, SetWindowPosEx, (Context*,ctx)
(const char*,name) (double,pos_x) (double,pos_y) (RO<int*>,cond,ImGuiCond_Always),
"Set named window position.")
{
  FRAME_GUARD;
  ImGui::SetWindowPos(name, ImVec2(pos_x, pos_y), API_GET(cond));
}

API_FUNC(0_5, void, SetWindowSizeEx, (Context*,ctx)
(const char*,name) (double,size_w) (double,size_h)
(RO<int*>,cond,ImGuiCond_Always),
"Set named window size. Set axis to 0.0 to force an auto-fit on this axis.")
{
  FRAME_GUARD;
  ImGui::SetWindowSize(name, ImVec2(size_w, size_h), API_GET(cond));
}

API_FUNC(0_5, void, SetWindowCollapsedEx, (Context*,ctx)
(const char*,name) (bool,collapsed) (RO<int*>,cond,ImGuiCond_Always),
"Set named window collapsed state.")
{
  FRAME_GUARD;
  ImGui::SetWindowCollapsed(name, collapsed, API_GET(cond));
}

API_FUNC(0_5, void, SetWindowFocusEx, (Context*,ctx)
(const char*,name),
"Set named window to be focused / top-most. Use an empty name to remove focus.")
{
  FRAME_GUARD;
  nullIfEmpty(name);
  ImGui::SetWindowFocus(name);
}

API_SECTION_P(properties, "Focused Flags", "For IsWindowFocused.");

API_ENUM(0_1, ImGui, FocusedFlags_None, "");
API_ENUM(0_1, ImGui, FocusedFlags_ChildWindows,
  "Return true if any children of the window is focused.");
API_ENUM(0_1, ImGui, FocusedFlags_RootWindow,
  "Test from root window (top most parent of the current hierarchy).");
API_ENUM(0_1, ImGui, FocusedFlags_AnyWindow,
  "Return true if any window is focused.");
API_ENUM(0_5_10, ImGui, FocusedFlags_NoPopupHierarchy,
R"(Do not consider popup hierarchy (do not treat popup emitter as parent of
   popup) (when used with _ChildWindows or _RootWindow).)");
API_ENUM(0_5_10, ImGui, FocusedFlags_DockHierarchy,
R"(Consider docking hierarchy (treat dockspace host as parent of docked window)
   (when used with _ChildWindows or _RootWindow).)");
API_ENUM(0_1, ImGui, FocusedFlags_RootAndChildWindows,
  "FocusedFlags_RootWindow | FocusedFlags_ChildWindows");

API_SECTION_P(properties, "Size Callback", "For SetNextWindowSizeConstraints.");

API_EELVAR(0_8_5, ImVec2, Pos, "Read-only. Window position, for reference.");
API_EELVAR(0_8_5, ImVec2, CurrentSize, "Read-only. Current window size.");
API_EELVAR(0_8_5, ImVec2, DesiredSize,
R"(Read-write. Desired size, based on user's mouse position.
Write to this field to restrain resizing.)");

template<>
void SizeCallback::storeVars(Function *func)
{
  func->setDouble("Pos.x",         s_data->Pos.x);
  func->setDouble("Pos.y",         s_data->Pos.y);
  func->setDouble("CurrentSize.x", s_data->CurrentSize.x);
  func->setDouble("CurrentSize.y", s_data->CurrentSize.y);
  func->setDouble("DesiredSize.x", s_data->DesiredSize.x);
  func->setDouble("DesiredSize.y", s_data->DesiredSize.y);
}

template<>
void SizeCallback::loadVars(const Function *func)
{
  s_data->DesiredSize.x = *func->getDouble("DesiredSize.x");
  s_data->DesiredSize.y = *func->getDouble("DesiredSize.y");
}

API_SUBSECTION("Docking",
R"(Dock windows into other windows or in REAPER dockers.

Dock IDs are:
- 0 = undocked
- -1 to -16 = REAPER docker index
- \> 0 = Dear ImGui dockspace ID (when the user docked the window into another one).

Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking.
Drag from window menu button (upper-left button) to undock an entire node (all windows).
DockingWithShift == true, you instead need to hold SHIFT to _enable_ docking.)");

API_FUNC(0_5, bool, IsWindowDocked, (Context*,ctx),
"Is current window docked into another window or a REAPER docker?")
{
  FRAME_GUARD;
  return ImGui::IsWindowDocked();
}

API_FUNC(0_5, int, GetWindowDockID, (Context*,ctx),
"")
{
  FRAME_GUARD;
  return ImGui::GetWindowDockID();
}

API_FUNC(0_5, void, SetNextWindowDockID, (Context*,ctx)
(int,dock_id) (RO<int*>,cond,ImGuiCond_Always),
"")
{
  FRAME_GUARD;
  ImGui::SetNextWindowDockID(dock_id, API_GET(cond));
}

API_SUBSECTION("Content Region",
R"(Retrieve available space from a given point.
GetContentRegionAvail() is frequently useful.)");

API_FUNC(0_1, void, GetContentRegionAvail, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
"== GetContentRegionMax() - GetCursorPos()")
{
  FRAME_GUARD;

  const ImVec2 &vec {ImGui::GetContentRegionAvail()};
  if(x) *x = vec.x;
  if(y) *y = vec.y;
}

API_FUNC(0_1, void, GetContentRegionMax, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
R"(Current content boundaries (typically window boundaries including scrolling,
or current column boundaries), in windows coordinates.)")
{
  FRAME_GUARD;

  const ImVec2 &vec {ImGui::GetContentRegionMax()};
  if(x) *x = vec.x;
  if(y) *y = vec.y;
}

API_FUNC(0_1, void, GetWindowContentRegionMin, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
"Content boundaries min (roughly (0,0)-Scroll), in window coordinates.")
{
  FRAME_GUARD;

  const ImVec2 &vec {ImGui::GetWindowContentRegionMin()};
  if(x) *x = vec.x;
  if(y) *y = vec.y;
}

API_FUNC(0_1, void, GetWindowContentRegionMax, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
R"(Content boundaries max (roughly (0,0)+Size-Scroll) where Size can be
overridden with SetNextWindowContentSize, in window coordinates.)")
{
  FRAME_GUARD;

  const ImVec2 &vec {ImGui::GetWindowContentRegionMax()};
  if(x) *x = vec.x;
  if(y) *y = vec.y;
}

API_SUBSECTION("Scrolling",
R"(Any change of Scroll will be applied at the beginning of next frame in the
first call to Begin().

You may instead use SetNextWindowScroll() prior to calling Begin() to avoid this
delay, as an alternative to using SetScrollX()/SetScrollY().)");

API_FUNC(0_1, double, GetScrollX, (Context*,ctx),
"Get scrolling amount [0 .. GetScrollMaxX()]")
{
  FRAME_GUARD;
  return ImGui::GetScrollX();
}

API_FUNC(0_1, double, GetScrollY, (Context*,ctx),
"Get scrolling amount [0 .. GetScrollMaxY()]")
{
  FRAME_GUARD;
  return ImGui::GetScrollY();
}

API_FUNC(0_1, void, SetScrollX, (Context*,ctx)
(double,scroll_x),
"Set scrolling amount [0 .. GetScrollMaxX()]")
{
  FRAME_GUARD;
  ImGui::SetScrollX(scroll_x);
}

API_FUNC(0_1, void, SetScrollY, (Context*,ctx)
(double,scroll_y),
"Set scrolling amount [0 .. GetScrollMaxY()]")
{
  FRAME_GUARD;
  ImGui::SetScrollY(scroll_y);
}

API_FUNC(0_1, double, GetScrollMaxX, (Context*,ctx),
"Get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x")
{
  FRAME_GUARD;
  return ImGui::GetScrollMaxX();
}

API_FUNC(0_1, double, GetScrollMaxY, (Context*,ctx),
"Get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y")
{
  FRAME_GUARD;
  return ImGui::GetScrollMaxY();
}

API_FUNC(0_1, void, SetScrollHereX, (Context*,ctx)
(RO<double*>,center_x_ratio,0.5),
R"(Adjust scrolling amount to make current cursor position visible.
center_x_ratio=0.0: left, 0.5: center, 1.0: right.
When using to make a "default/current item" visible,
consider using SetItemDefaultFocus instead.)")
{
  FRAME_GUARD;
  ImGui::SetScrollHereX(API_GET(center_x_ratio));
}

API_FUNC(0_1, void, SetScrollHereY, (Context*,ctx)
(RO<double*>,center_y_ratio,0.5),
R"(Adjust scrolling amount to make current cursor position visible.
center_y_ratio=0.0: top, 0.5: center, 1.0: bottom.
When using to make a "default/current item" visible,
consider using SetItemDefaultFocus instead.)")
{
  FRAME_GUARD;
  ImGui::SetScrollHereY(API_GET(center_y_ratio));
}

API_FUNC(0_1, void, SetScrollFromPosX, (Context*,ctx)
(double,local_x) (RO<double*>,center_x_ratio,0.5),
R"(Adjust scrolling amount to make given position visible.
Generally GetCursorStartPos() + offset to compute a valid position.)")
{
  FRAME_GUARD;
  ImGui::SetScrollFromPosX(local_x, API_GET(center_x_ratio));
}

API_FUNC(0_1, void, SetScrollFromPosY, (Context*,ctx)
(double,local_y) (RO<double*>,center_y_ratio,0.5),
R"(Adjust scrolling amount to make given position visible.
Generally GetCursorStartPos() + offset to compute a valid position.)")
{
  FRAME_GUARD;
  ImGui::SetScrollFromPosY(local_y, API_GET(center_y_ratio));
}

API_SUBSECTION("Debug Windows");

API_FUNC(0_5_4, void, ShowAboutWindow, (Context*,ctx)
(RWO<bool*>,p_open),
R"(Create About window.
Display ReaImGui version, Dear ImGui version, credits and build/system information.)")
{
  FRAME_GUARD;

  ImGuiWindowFlags flags {ImGuiWindowFlags_AlwaysAutoResize};
  DecorationBehavior dec {ctx, &flags};
  if(ImGui::Begin("About Dear ImGui", openPtrBehavior(p_open), flags)) {
    ImGui::Separator();
    ImGui::Text("reaper_imgui %s (API %s)",
      REAIMGUI_VERSION, API::version().toString().c_str());
    ImGui::Separator();
    ImGui::TextUnformatted("By Christian Fillion and contributors.");
    ImGui::TextUnformatted("ReaImGui is licensed under the LGPL.");
    ImGui::Spacing();
    ImGui::Separator();
  }
  ImGui::End();

  ImGui::ShowAboutWindow(); // appends to the same window by title
}

API_FUNC(0_1, void, ShowMetricsWindow, (Context*,ctx)
(RWO<bool*>,p_open),
R"(Create Metrics/Debugger window.
Display Dear ImGui internals: windows, draw commands, various internal state, etc.)")
{
  FRAME_GUARD;

  if(nativeWindowBehavior("Dear ImGui Metrics/Debugger", p_open))
    ImGui::ShowMetricsWindow();
}

API_FUNC(0_7, void, ShowDebugLogWindow, (Context*,ctx)
(RWO<bool*>,p_open),
"Create Debug Log window. display a simplified log of important dear imgui events.")
{
  FRAME_GUARD;

  if(nativeWindowBehavior("Dear ImGui Debug Log", p_open))
    ImGui::ShowDebugLogWindow();
}

API_FUNC(0_9, void, ShowIDStackToolWindow, (Context*,ctx)
(RWO<bool*>,p_open),
R"(Create Stack Tool window. Hover items with mouse to query information about
the source of their unique ID.)")
{
  FRAME_GUARD;

  if(nativeWindowBehavior("Dear ImGui ID Stack Tool", p_open))
    ImGui::ShowIDStackToolWindow();
}

API_SUBSECTION("Flags",
R"(For Begin and BeginChild.

(Those are per-window flags. There are shared flags in SetConfigVar:
ConfigVar_WindowsResizeFromEdges and ConfigVar_WindowsMoveFromTitleBarOnly))");

API_ENUM(0_1, ImGui, WindowFlags_None,       "Default flag.");
API_ENUM(0_1, ImGui, WindowFlags_NoTitleBar, "Disable title-bar.");
API_ENUM(0_1, ImGui, WindowFlags_NoResize,   "Disable user resizing with the lower-right grip.");
API_ENUM(0_1, ImGui, WindowFlags_NoMove,     "Disable user moving the window.");
API_ENUM(0_1, ImGui, WindowFlags_NoScrollbar,
  "Disable scrollbars (window can still scroll with mouse or programmatically).");
API_ENUM(0_1, ImGui, WindowFlags_NoScrollWithMouse,
R"(Disable user vertically scrolling with mouse wheel.
   On child window, mouse wheel will be forwarded to the parent unless
   NoScrollbar is also set.)");
API_ENUM(0_1, ImGui, WindowFlags_NoCollapse,
R"(Disable user collapsing window by double-clicking on it.
   Also referred to as Window Menu Button (e.g. within a docking node).)");
API_ENUM(0_1, ImGui, WindowFlags_AlwaysAutoResize,
  "Resize every window to its content every frame.");
API_ENUM(0_1, ImGui, WindowFlags_NoBackground,
R"(Disable drawing background color (WindowBg, etc.) and outside border.
   Similar as using SetNextWindowBgAlpha(0.0).)");
API_ENUM(0_4, ImGui, WindowFlags_NoSavedSettings,
  "Never load/save settings in .ini file.");
API_ENUM(0_1, ImGui, WindowFlags_NoMouseInputs,
  "Disable catching mouse, hovering test with pass through.");
API_ENUM(0_1, ImGui, WindowFlags_MenuBar, "Has a menu-bar.");
API_ENUM(0_1, ImGui, WindowFlags_HorizontalScrollbar,
R"(Allow horizontal scrollbar to appear (off by default).
   You may use SetNextWindowContentSize(width, 0.0) prior to calling Begin() to
   specify width. Read code in the demo's "Horizontal Scrolling" section.)");
API_ENUM(0_1, ImGui, WindowFlags_NoFocusOnAppearing,
  "Disable taking focus when transitioning from hidden to visible state.");
// API_ENUM(ImGui, WindowFlags_NoBringToFrontOnFocus,
// R"(Disable bringing window to front when taking focus
//    (e.g. clicking on it or programmatically giving it focus).)");
API_ENUM(0_1, ImGui, WindowFlags_AlwaysVerticalScrollbar,
  "Always show vertical scrollbar (even if ContentSize.y < Size.y).");
API_ENUM(0_1, ImGui, WindowFlags_AlwaysHorizontalScrollbar,
  "Always show horizontal scrollbar (even if ContentSize.x < Size.x).");
API_ENUM(0_1, ImGui, WindowFlags_NoNavInputs,
  "No gamepad/keyboard navigation within the window.");
API_ENUM(0_1, ImGui, WindowFlags_NoNavFocus,
R"(No focusing toward this window with gamepad/keyboard navigation
   (e.g. skipped by CTRL+TAB).)");
API_ENUM(0_1, ImGui, WindowFlags_UnsavedDocument,
R"(Display a dot next to the title. When used in a tab/docking context,
   tab is selected when clicking the X + closure is not assumed
   (will wait for user to stop submitting the tab).
   Otherwise closure is assumed when pressing the X,
   so if you keep submitting the tab may reappear at end of tab bar.)");
API_ENUM(0_5, ImGui, WindowFlags_NoDocking, "Disable docking of this window.");
API_ENUM(0_1, ImGui, WindowFlags_NoNav,
  "WindowFlags_NoNavInputs | WindowFlags_NoNavFocus");
API_ENUM(0_1, ImGui, WindowFlags_NoDecoration,
R"(WindowFlags_NoTitleBar | WindowFlags_NoResize | WindowFlags_NoScrollbar |
   WindowFlags_NoCollapse)");
API_ENUM(0_1, ImGui, WindowFlags_NoInputs,
  "WindowFlags_NoMouseInputs | WindowFlags_NoNavInputs | WindowFlags_NoNavFocus");

API_ENUM(0_5_5, ReaImGui, WindowFlags_TopMost,
  "Show the window above all non-topmost windows.");
