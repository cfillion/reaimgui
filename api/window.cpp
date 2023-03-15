/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

#include "flags.hpp"
#include "helper.hpp"

#include <imgui/imgui.h>

API_SECTION("Window",
R"(Functions for creating and manipulating windows.
Note that the bottom of the window stack always contains a window called "Debug".)");

class DecorationBehavior {
public:
  DecorationBehavior(Context *ctx, ImGuiWindowFlags *flags)
    : m_enabled { !ctx->IO().ConfigViewportsNoDecoration }
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
  DecorationBehavior dec { Context::current(), &flags };
  ImGui::Begin(name, openPtrBehavior(p_open), flags);
  auto &beginCount { ImGui::GetCurrentWindow()->BeginCount };
  ImGui::End();

  --beginCount;
  return beginCount == 0;
}

DEFINE_API(bool, Begin, (ImGui_Context*,ctx)
(const char*,name)(bool*,API_RWO(p_open))
(int*,API_RO(flags),ImGuiWindowFlags_None),
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

  WindowFlags flags { API_RO_GET(flags) };
  DecorationBehavior dec { ctx, *flags };
  const bool rv { ImGui::Begin(name, openPtrBehavior(API_RWO(p_open)), flags) };
  if(!rv)
    ImGui::End();
  return rv;
}

DEFINE_API(void, End, (ImGui_Context*,ctx),
R"(Pop window from the stack. See Begin.)")
{
  FRAME_GUARD;
  ImGui::End();
}

API_SUBSECTION("Child Windows",
R"(Use child windows to begin into a self-contained independent
scrolling/clipping regions within a host window.
Child windows can embed their own child.)");

DEFINE_API(bool, BeginChild, (ImGui_Context*,ctx)
(const char*,str_id)(double*,API_RO(size_w),0.0)(double*,API_RO(size_h),0.0)
(bool*,API_RO(border),false)(int*,API_RO(flags),ImGuiWindowFlags_None),
R"(For each independent axis of 'size':
- \> 0.0: fixed size
- = 0.0: use remaining host window size
- < 0.0: use remaining window size minus abs(size)
(Each axis can use a different mode, e.g. size = 0x400.)

Returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window.)")
{
  FRAME_GUARD;
  const bool rv {
    ImGui::BeginChild(str_id,
      ImVec2(API_RO_GET(size_w), API_RO_GET(size_h)),
      API_RO_GET(border), API_RO_GET(flags))
  };
  if(!rv)
    ImGui::EndChild();
  return rv;
}

DEFINE_API(void, EndChild, (ImGui_Context*,ctx),
"See BeginChild.")
{
  FRAME_GUARD;
  ImGui::EndChild();
}

DEFINE_API(bool, BeginChildFrame, (ImGui_Context*,ctx)
(const char*,str_id)(double,size_w)(double,size_h)
(int*,API_RO(flags),ImGuiWindowFlags_None),
R"(Helper to create a child window / scrolling region that looks like a normal
widget frame. See BeginChild.)")
{
  FRAME_GUARD;
  const ImGuiID id { ImGui::GetID(str_id) };
  const bool rv {
    ImGui::BeginChildFrame(id, ImVec2(size_w, size_h), API_RO_GET(flags))
  };
  if(!rv)
    ImGui::EndChildFrame();
  return rv;
}

DEFINE_API(void, EndChildFrame, (ImGui_Context*,ctx),
"See BeginChildFrame.")
{
  FRAME_GUARD;
  ImGui::EndChildFrame();
}

DEFINE_SECTION(properties, ROOT_SECTION, "Properties",
R"(Prefer using SetNextWindow* functions (before Begin) rather that SetWindow* functions
(after Begin).

'Current window' = the window we are appending into while inside a Begin()/End()
block. 'Next window' = next window we will Begin() into.)");

DEFINE_API(bool, IsWindowAppearing, (ImGui_Context*,ctx),
"Use after Begin/BeginPopup/BeginPopupModal to tell if a window just opened.")
{
  FRAME_GUARD;
  return ImGui::IsWindowAppearing();
}

DEFINE_API(bool, IsWindowFocused, (ImGui_Context*,ctx)
(int*,API_RO(flags),ImGuiFocusedFlags_None),
R"(Is current window focused? or its root/child, depending on flags.
See flags for options.)")
{
  FRAME_GUARD;
  return ImGui::IsWindowFocused(API_RO_GET(flags));
}

DEFINE_API(bool, IsWindowHovered, (ImGui_Context*,ctx)
(int*,API_RO(flags),ImGuiHoveredFlags_None),
R"(Is current window hovered (and typically: not blocked by a popup/modal)?
See flags for options.)")
{
  FRAME_GUARD;
  return ImGui::IsWindowHovered(API_RO_GET(flags));
}

DEFINE_API(void, GetWindowPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
R"(Get current window position in screen space (useful if you want to do your own
drawing via the DrawList API).)")
{
  FRAME_GUARD;
  const ImVec2 &vec { ImGui::GetWindowPos() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
}

DEFINE_API(void, GetWindowSize, (ImGui_Context*,ctx)
(double*,API_W(w))(double*,API_W(h)),
"Get current window size")
{
  FRAME_GUARD;
  const ImVec2 &vec { ImGui::GetWindowSize() };
  if(API_W(w)) *API_W(w) = vec.x;
  if(API_W(h)) *API_W(h) = vec.y;
}

DEFINE_API(double, GetWindowWidth, (ImGui_Context*,ctx),
"Get current window width (shortcut for (select(1, GetWindowSize())).")
{
  FRAME_GUARD;
  return ImGui::GetWindowWidth();
}

DEFINE_API(double, GetWindowHeight, (ImGui_Context*,ctx),
"Get current window height (shortcut for (select(2, GetWindowSize())).")
{
  FRAME_GUARD;
  return ImGui::GetWindowHeight();
}

DEFINE_API(double, GetWindowDpiScale, (ImGui_Context*,ctx),
R"(Get DPI scale currently associated to the current window's viewport
(1.0 = 96 DPI).)")
{
  FRAME_GUARD;
  return ImGui::GetWindowDpiScale();
}

DEFINE_API(void, SetNextWindowPos, (ImGui_Context*,ctx)
(double,pos_x)(double,pos_y)(int*,API_RO(cond),ImGuiCond_Always)
(double*,API_RO(pivot_x),0.0)(double*,API_RO(pivot_y),0.0),
"Set next window position. Use pivot=(0.5,0.5) to center on given point, etc.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), API_RO_GET(cond),
    ImVec2(API_RO_GET(pivot_x), API_RO_GET(pivot_y)));
}

DEFINE_API(void, SetNextWindowSize, (ImGui_Context*,ctx)
(double,size_w)(double,size_h)(int*,API_RO(cond),ImGuiCond_Always),
"Set next window size. set axis to 0.0 to force an auto-fit on this axis.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowSize(ImVec2(size_w, size_h), API_RO_GET(cond));
}

DEFINE_API(void, SetNextWindowSizeConstraints, (ImGui_Context*,ctx)
(double,size_min_w)(double,size_min_h)(double,size_max_w)(double,size_max_h),
R"(Set next window size limits. use -1,-1 on either X/Y axis to preserve the
current size. Sizes will be rounded down.)")
{
  FRAME_GUARD;
  ImGui::SetNextWindowSizeConstraints(
    ImVec2(size_min_w, size_min_h), ImVec2(size_max_w, size_max_h));
}

DEFINE_API(void, SetNextWindowContentSize, (ImGui_Context*,ctx)
(double,size_w)(double,size_h),
R"(Set next window content size (~ scrollable client area, which enforce the
range of scrollbars). Not including window decorations (title bar, menu bar,
etc.) nor StyleVar_WindowPadding. set an axis to 0.0 to leave it automatic.)")
{
  FRAME_GUARD;
  ImGui::SetNextWindowContentSize(ImVec2(size_w, size_h));
}

DEFINE_API(void, SetNextWindowCollapsed, (ImGui_Context*,ctx)
(bool,collapsed)(int*,API_RO(cond),ImGuiCond_Always),
"Set next window collapsed state.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowCollapsed(collapsed, API_RO_GET(cond));
}

DEFINE_API(void, SetNextWindowFocus, (ImGui_Context*,ctx),
"Set next window to be focused / top-most.")
{
  FRAME_GUARD;
  ImGui::SetNextWindowFocus();
}

DEFINE_API(void, SetNextWindowScroll, (ImGui_Context*,ctx)
(double,scroll_x)(double,scroll_y),
"Set next window scrolling value (use < 0.0 to not affect a given axis).")
{
  FRAME_GUARD;
  ImGui::SetNextWindowScroll(ImVec2(scroll_x, scroll_y));
}

DEFINE_API(void, SetNextWindowBgAlpha, (ImGui_Context*,ctx)
(double,alpha),
R"(Set next window background color alpha. Helper to easily override the Alpha
component of Col_WindowBg/Col_ChildBg/Col_PopupBg.
You may also use WindowFlags_NoBackground for a fully transparent window.)")
{
  FRAME_GUARD;
  ImGui::SetNextWindowBgAlpha(alpha);
}

DEFINE_API(void, SetWindowPos, (ImGui_Context*,ctx)
(double,pos_x)(double,pos_y)(int*,API_RO(cond),ImGuiCond_Always),
R"((Not recommended) Set current window position - call within Begin/End.
Prefer using SetNextWindowPos, as this may incur tearing and minor side-effects.)")
{
  FRAME_GUARD;
  ImGui::SetWindowPos(ImVec2(pos_x, pos_y), API_RO_GET(cond));
}

DEFINE_API(void, SetWindowSize, (ImGui_Context*,ctx)
(double,size_w)(double,size_h)(int*,API_RO(cond),ImGuiCond_Always),
R"((Not recommended) Set current window size - call within Begin/End.
Set size_w and size_h to 0 to force an auto-fit.
Prefer using SetNextWindowSize, as this may incur tearing and minor side-effects.)")
{
  FRAME_GUARD;
  ImGui::SetWindowSize(ImVec2(size_w, size_h), API_RO_GET(cond));
}

DEFINE_API(void, SetWindowCollapsed, (ImGui_Context*,ctx)
(bool,collapsed)(int*,API_RO(cond),ImGuiCond_Always),
R"((Not recommended) Set current window collapsed state.
Prefer using SetNextWindowCollapsed.)")
{
  FRAME_GUARD;
  ImGui::SetWindowCollapsed(collapsed, API_RO_GET(cond));
}

DEFINE_API(void, SetWindowFocus, (ImGui_Context*,ctx),
R"((Not recommended) Set current window to be focused / top-most.
Prefer using SetNextWindowFocus.)")
{
  FRAME_GUARD;
  ImGui::SetWindowFocus();
}

DEFINE_API(void, SetWindowPosEx, (ImGui_Context*,ctx)
(const char*,name)(double,pos_x)(double,pos_y)
(int*,API_RO(cond),ImGuiCond_Always),
"Set named window position.")
{
  FRAME_GUARD;
  ImGui::SetWindowPos(name, ImVec2(pos_x, pos_y), API_RO_GET(cond));
}

DEFINE_API(void, SetWindowSizeEx, (ImGui_Context*,ctx)
(const char*,name)(double,size_w)(double,size_h)
(int*,API_RO(cond),ImGuiCond_Always),
"Set named window size. Set axis to 0.0 to force an auto-fit on this axis.")
{
  FRAME_GUARD;
  ImGui::SetWindowSize(name, ImVec2(size_w, size_h), API_RO_GET(cond));
}

DEFINE_API(void, SetWindowCollapsedEx, (ImGui_Context*,ctx)
(const char*,name)(bool,collapsed)(int*,API_RO(cond),ImGuiCond_Always),
"Set named window collapsed state.")
{
  FRAME_GUARD;
  ImGui::SetWindowCollapsed(name, collapsed, API_RO_GET(cond));
}

DEFINE_API(void, SetWindowFocusEx, (ImGui_Context*,ctx)
(const char*,name),
"Set named window to be focused / top-most. Use an empty name to remove focus.")
{
  FRAME_GUARD;
  nullIfEmpty(name);
  ImGui::SetWindowFocus(name);
}

API_SECTION_P(properties, "Focused Flags", "For IsWindowFocused.");

DEFINE_ENUM(ImGui, FocusedFlags_None, "");
DEFINE_ENUM(ImGui, FocusedFlags_ChildWindows,
  "Return true if any children of the window is focused.");
DEFINE_ENUM(ImGui, FocusedFlags_RootWindow,
  "Test from root window (top most parent of the current hierarchy).");
DEFINE_ENUM(ImGui, FocusedFlags_AnyWindow,
  "Return true if any window is focused.");
DEFINE_ENUM(ImGui, FocusedFlags_NoPopupHierarchy,
R"(Do not consider popup hierarchy (do not treat popup emitter as parent of
   popup) (when used with _ChildWindows or _RootWindow).)");
DEFINE_ENUM(ImGui, FocusedFlags_DockHierarchy,
R"(Consider docking hierarchy (treat dockspace host as parent of docked window)
   (when used with _ChildWindows or _RootWindow).)");
DEFINE_ENUM(ImGui, FocusedFlags_RootAndChildWindows,
  "FocusedFlags_RootWindow | FocusedFlags_ChildWindows");

API_SUBSECTION("Docking",
R"(Dock windows into other windows or in REAPER dockers.

Dock IDs are:
- 0 = undocked
- -1 to -16 = REAPER docker index
- \> 0 = Dear ImGui dockspace ID (when the user docked the window into another one).

Set ConfigFlags_DockingEnable when creating your context or with
SetConfigVar(ConfigVar_Flags) to enable docking (disabled by default).)");

DEFINE_API(bool, IsWindowDocked, (ImGui_Context*,ctx),
"Is current window docked into another window or a REAPER docker?")
{
  FRAME_GUARD;
  return ImGui::IsWindowDocked();
}

DEFINE_API(int, GetWindowDockID, (ImGui_Context*,ctx),
"")
{
  FRAME_GUARD;
  return ImGui::GetWindowDockID();
}

DEFINE_API(void, SetNextWindowDockID, (ImGui_Context*,ctx)
(int,dock_id)(int*,API_RO(cond),ImGuiCond_Always),
"")
{
  FRAME_GUARD;
  ImGui::SetNextWindowDockID(dock_id, API_RO_GET(cond));
}

API_SUBSECTION("Content Region",
R"(Retrieve available space from a given point.
GetContentRegionAvail() is frequently useful.)");

DEFINE_API(void, GetContentRegionAvail, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"== GetContentRegionMax() - GetCursorPos()")
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetContentRegionAvail() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
}

DEFINE_API(void, GetContentRegionMax, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
R"(Current content boundaries (typically window boundaries including scrolling,
or current column boundaries), in windows coordinates.)")
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetContentRegionMax() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
}

DEFINE_API(void, GetWindowContentRegionMin, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Content boundaries min (roughly (0,0)-Scroll), in window coordinates.")
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetWindowContentRegionMin() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
}

DEFINE_API(void, GetWindowContentRegionMax, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
R"(Content boundaries max (roughly (0,0)+Size-Scroll) where Size can be
overridden with SetNextWindowContentSize, in window coordinates.)")
{
  FRAME_GUARD;

  const ImVec2 &vec { ImGui::GetWindowContentRegionMax() };
  if(API_W(x)) *API_W(x) = vec.x;
  if(API_W(y)) *API_W(y) = vec.y;
}

API_SUBSECTION("Scrolling",
R"(Any change of Scroll will be applied at the beginning of next frame in the
first call to Begin().

You may instead use SetNextWindowScroll() prior to calling Begin() to avoid this
delay, as an alternative to using SetScrollX()/SetScrollY().)");

DEFINE_API(double, GetScrollX, (ImGui_Context*,ctx),
"Get scrolling amount [0 .. GetScrollMaxX()]")
{
  FRAME_GUARD;
  return ImGui::GetScrollX();
}

DEFINE_API(double, GetScrollY, (ImGui_Context*,ctx),
"Get scrolling amount [0 .. GetScrollMaxY()]")
{
  FRAME_GUARD;
  return ImGui::GetScrollY();
}

DEFINE_API(void, SetScrollX, (ImGui_Context*,ctx)
(double,scroll_x),
"Set scrolling amount [0 .. GetScrollMaxX()]")
{
  FRAME_GUARD;
  ImGui::SetScrollX(scroll_x);
}

DEFINE_API(void, SetScrollY, (ImGui_Context*,ctx)
(double,scroll_y),
"Set scrolling amount [0 .. GetScrollMaxY()]")
{
  FRAME_GUARD;
  ImGui::SetScrollY(scroll_y);
}

DEFINE_API(double, GetScrollMaxX, (ImGui_Context*,ctx),
"Get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x")
{
  FRAME_GUARD;
  return ImGui::GetScrollMaxX();
}

DEFINE_API(double, GetScrollMaxY, (ImGui_Context*,ctx),
"Get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y")
{
  FRAME_GUARD;
  return ImGui::GetScrollMaxY();
}

DEFINE_API(void, SetScrollHereX, (ImGui_Context*,ctx)
(double*,API_RO(center_x_ratio),0.5),
R"(Adjust scrolling amount to make current cursor position visible.
center_x_ratio=0.0: left, 0.5: center, 1.0: right.
When using to make a "default/current item" visible,
consider using SetItemDefaultFocus instead.)")
{
  FRAME_GUARD;
  ImGui::SetScrollHereX(API_RO_GET(center_x_ratio));
}

DEFINE_API(void, SetScrollHereY, (ImGui_Context*,ctx)
(double*,API_RO(center_y_ratio),0.5),
R"(Adjust scrolling amount to make current cursor position visible.
center_y_ratio=0.0: top, 0.5: center, 1.0: bottom.
When using to make a "default/current item" visible,
consider using SetItemDefaultFocus instead.)")
{
  FRAME_GUARD;
  ImGui::SetScrollHereY(API_RO_GET(center_y_ratio));
}

DEFINE_API(void, SetScrollFromPosX, (ImGui_Context*,ctx)
(double,local_x)(double*,API_RO(center_x_ratio),0.5),
R"(Adjust scrolling amount to make given position visible.
Generally GetCursorStartPos() + offset to compute a valid position.)")
{
  FRAME_GUARD;
  ImGui::SetScrollFromPosX(local_x, API_RO_GET(center_x_ratio));
}

DEFINE_API(void, SetScrollFromPosY, (ImGui_Context*,ctx)
(double,local_y)(double*,API_RO(center_y_ratio),0.5),
R"(Adjust scrolling amount to make given position visible.
Generally GetCursorStartPos() + offset to compute a valid position.)")
{
  FRAME_GUARD;
  ImGui::SetScrollFromPosY(local_y, API_RO_GET(center_y_ratio));
}

API_SUBSECTION("Debug Windows");

DEFINE_API(void, ShowAboutWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(p_open)),
R"(Create About window.
Display ReaImGui version, Dear ImGui version, credits and build/system information.)")
{
  FRAME_GUARD;

  ImGuiWindowFlags flags { ImGuiWindowFlags_AlwaysAutoResize };
  DecorationBehavior dec { ctx, &flags };
  if(ImGui::Begin("About Dear ImGui", openPtrBehavior(API_RWO(p_open)), flags)) {
    ImGui::Separator();
    ImGui::Text("reaper_imgui %s", REAIMGUI_VERSION);
    ImGui::Separator();
    ImGui::TextUnformatted("By Christian Fillion and contributors.");
    ImGui::TextUnformatted("ReaImGui is licensed under the LGPL.");
    ImGui::Spacing();
    ImGui::Separator();
  }
  ImGui::End();

  ImGui::ShowAboutWindow(); // appends to the same window by title
}

DEFINE_API(void, ShowMetricsWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(p_open)),
R"(Create Metrics/Debugger window.
Display Dear ImGui internals: windows, draw commands, various internal state, etc.)")
{
  FRAME_GUARD;

  if(nativeWindowBehavior("Dear ImGui Metrics/Debugger", API_RWO(p_open)))
    ImGui::ShowMetricsWindow();
}

DEFINE_API(void, ShowDebugLogWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(p_open)),
"Create Debug Log window. display a simplified log of important dear imgui events.")
{
  FRAME_GUARD;

  if(nativeWindowBehavior("Dear ImGui Debug Log", API_RWO(p_open)))
    ImGui::ShowDebugLogWindow();
}

DEFINE_API(void, ShowStackToolWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(p_open)),
R"(Create Stack Tool window. Hover items with mouse to query information about
the source of their unique ID.)")
{
  FRAME_GUARD;

  if(nativeWindowBehavior("Dear ImGui Stack Tool", API_RWO(p_open)))
    ImGui::ShowStackToolWindow();
}

API_SUBSECTION("Flags",
R"(For Begin and BeginChild.

(Those are per-window flags. There are shared flags in SetConfigVar:
ConfigVar_WindowsResizeFromEdges and ConfigVar_WindowsMoveFromTitleBarOnly))");

DEFINE_ENUM(ImGui, WindowFlags_None,       "Default flag.");
DEFINE_ENUM(ImGui, WindowFlags_NoTitleBar, "Disable title-bar.");
DEFINE_ENUM(ImGui, WindowFlags_NoResize,   "Disable user resizing with the lower-right grip.");
DEFINE_ENUM(ImGui, WindowFlags_NoMove,     "Disable user moving the window.");
DEFINE_ENUM(ImGui, WindowFlags_NoScrollbar,
  "Disable scrollbars (window can still scroll with mouse or programmatically).");
DEFINE_ENUM(ImGui, WindowFlags_NoScrollWithMouse,
R"(Disable user vertically scrolling with mouse wheel.
   On child window, mouse wheel will be forwarded to the parent unless
   NoScrollbar is also set.)");
DEFINE_ENUM(ImGui, WindowFlags_NoCollapse,
R"(Disable user collapsing window by double-clicking on it.
   Also referred to as Window Menu Button (e.g. within a docking node).)");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysAutoResize,
  "Resize every window to its content every frame.");
DEFINE_ENUM(ImGui, WindowFlags_NoBackground,
R"(Disable drawing background color (WindowBg, etc.) and outside border.
   Similar as using SetNextWindowBgAlpha(0.0).)");
DEFINE_ENUM(ImGui, WindowFlags_NoSavedSettings,
  "Never load/save settings in .ini file.");
DEFINE_ENUM(ImGui, WindowFlags_NoMouseInputs,
  "Disable catching mouse, hovering test with pass through.");
DEFINE_ENUM(ImGui, WindowFlags_MenuBar, "Has a menu-bar.");
DEFINE_ENUM(ImGui, WindowFlags_HorizontalScrollbar,
R"(Allow horizontal scrollbar to appear (off by default).
   You may use SetNextWindowContentSize(width, 0.0) prior to calling Begin() to
   specify width. Read code in the demo's "Horizontal Scrolling" section.)");
DEFINE_ENUM(ImGui, WindowFlags_NoFocusOnAppearing,
  "Disable taking focus when transitioning from hidden to visible state.");
// DEFINE_ENUM(ImGui, WindowFlags_NoBringToFrontOnFocus,
// R"(Disable bringing window to front when taking focus
//    (e.g. clicking on it or programmatically giving it focus).)");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysVerticalScrollbar,
  "Always show vertical scrollbar (even if ContentSize.y < Size.y).");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysHorizontalScrollbar,
  "Always show horizontal scrollbar (even if ContentSize.x < Size.x).");
DEFINE_ENUM(ImGui, WindowFlags_AlwaysUseWindowPadding,
R"(Ensure child windows without border uses StyleVar_WindowPadding
   (ignored by default for non-bordered child windows, because more convenient).)");
DEFINE_ENUM(ImGui, WindowFlags_NoNavInputs,
  "No gamepad/keyboard navigation within the window.");
DEFINE_ENUM(ImGui, WindowFlags_NoNavFocus,
R"(No focusing toward this window with gamepad/keyboard navigation
   (e.g. skipped by CTRL+TAB).)");
DEFINE_ENUM(ImGui, WindowFlags_UnsavedDocument,
R"(Display a dot next to the title. When used in a tab/docking context,
   tab is selected when clicking the X + closure is not assumed
   (will wait for user to stop submitting the tab).
   Otherwise closure is assumed when pressing the X,
   so if you keep submitting the tab may reappear at end of tab bar.)");
DEFINE_ENUM(ImGui, WindowFlags_NoDocking, "Disable docking of this window.");
DEFINE_ENUM(ImGui, WindowFlags_NoNav,
  "WindowFlags_NoNavInputs | WindowFlags_NoNavFocus");
DEFINE_ENUM(ImGui, WindowFlags_NoDecoration,
R"(WindowFlags_NoTitleBar | WindowFlags_NoResize | WindowFlags_NoScrollbar |
   WindowFlags_NoCollapse)");
DEFINE_ENUM(ImGui, WindowFlags_NoInputs,
  "WindowFlags_NoMouseInputs | WindowFlags_NoNavInputs | WindowFlags_NoNavFocus");

DEFINE_ENUM(ReaImGui, WindowFlags_TopMost,
  "Show the window above all non-topmost windows.");
