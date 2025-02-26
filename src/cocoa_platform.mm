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

#include "platform.hpp"

#include "cocoa_window.hpp"

#include <AppKit/AppKit.h>
#include <imgui/imgui.h>

static HWND g_fakeCapture;

void Platform::install()
{
  // Temprarily enable repeat character input
  // WARNING: this is application-wide!
  NSUserDefaults *defaults {[NSUserDefaults standardUserDefaults]};
  [defaults registerDefaults:@{@"ApplePressAndHoldEnabled":@NO}];

  ImGuiIO &io {ImGui::GetIO()};
  io.BackendPlatformName = "reaper_imgui_cocoa";
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new CocoaWindow {viewport, dockerHost};
}

void Platform::updateMonitors()
{
  // TODO
  // if(!g_monitorsChanged)
  //   return;

  ImGuiPlatformIO &pio {ImGui::GetPlatformIO()};
  pio.Monitors.resize(0); // recycle allocated memory (don't use clear here!)

  NSArray<NSScreen *> *screens {[NSScreen screens]};
  const CGFloat mainHeight {screens[0].frame.size.height};

  for(NSScreen *screen in screens) {
    const NSRect frame {[screen frame]}, workFrame {[screen visibleFrame]};
    ImGuiPlatformMonitor monitor;
    monitor.MainPos.x  = frame.origin.x;
    monitor.MainPos.y  = mainHeight - frame.origin.y - frame.size.height;
    monitor.MainSize.x = frame.size.width;
    monitor.MainSize.y = frame.size.height;
    monitor.WorkPos.x  = workFrame.origin.x;
    monitor.WorkPos.y  = mainHeight - workFrame.origin.y - workFrame.size.height;
    monitor.WorkSize.x = workFrame.size.width;
    monitor.WorkSize.y = workFrame.size.height;
    monitor.DpiScale   = [screen backingScaleFactor];

    pio.Monitors.push_back(monitor);
  }
}

ImVec2 Platform::getCursorPos()
{
  // SWELL's GetCursorPos returns Y from 0-1080 instead of 0-1079 on a 1080p
  // monitor. Doing ceil(Y) - 1 here to workaround that.
  const NSPoint loc {[NSEvent mouseLocation]};
  return {floorf(loc.x), ceilf(loc.y) - 1};
}

void Platform::scalePosition(ImVec2 *pos, bool, const ImGuiViewport *)
{
  pos->y = ImGui::GetPlatformIO().Monitors[0].MainSize.y - 1 - pos->y;
}

HWND Platform::windowFromPoint(const ImVec2 nativePoint)
{
  NSPoint point {NSMakePoint(nativePoint.x, nativePoint.y)};
  const NSInteger windowNumber
    {[NSWindow windowNumberAtPoint:point belowWindowWithWindowNumber:0]};
  NSWindow *window {[NSApp windowWithWindowNumber:windowNumber]};
  if(!window)
    return nullptr;

  NSView *view {[window contentView]};
  if(!view || ![view respondsToSelector:@selector(onSwellMessage:p1:p2:)])
    return nullptr;

  point = [window convertScreenToBase:point];
  if(NSView *childView {[view hitTest:point]})
    view = childView;

  return (__bridge HWND)view;
}

float Platform::scaleForWindow(HWND hwnd)
{
  return [[(__bridge NSView *)hwnd window] backingScaleFactor];
}

@interface NSCursor()
+ (NSCursor *)_windowResizeNorthWestSouthEastCursor;
+ (NSCursor *)_windowResizeNorthEastSouthWestCursor;
+ (NSCursor *)_windowResizeNorthSouthCursor;
+ (NSCursor *)_windowResizeEastWestCursor;
@end

HCURSOR Platform::getCursor(const ImGuiMouseCursor cur)
{
  switch(cur) {
  case ImGuiMouseCursor_None:
    static NSCursor *blank {
      [[NSCursor alloc] initWithImage:[[NSImage alloc]initWithSize:NSMakeSize(1, 1)]
                              hotSpot:NSMakePoint(0, 0)]
    };
    return (__bridge HCURSOR)blank;
  case ImGuiMouseCursor_ResizeAll:
    static HCURSOR bm {LoadCursor(nullptr, IDC_SIZEALL)};
    return bm;
  }

  static NSCursor * const cursors[ImGuiMouseCursor_COUNT] {
    [NSCursor arrowCursor],
    [NSCursor IBeamCursor],
    nullptr, // ResizeAll
    [NSCursor _windowResizeNorthSouthCursor],
    [NSCursor _windowResizeEastWestCursor],
    [NSCursor _windowResizeNorthEastSouthWestCursor],
    [NSCursor _windowResizeNorthWestSouthEastCursor],
    [NSCursor pointingHandCursor],
    [NSCursor operationNotAllowedCursor],
  };

  return (__bridge HCURSOR)cursors[cur];
}

// Not using SWELL capture to fix keyboard input when the REAPER setting
// "Allow keyboard commands when mouse-editing" is disabled in
// Preferences > General > Advanced UI tweaks. Otherwise REAPER would skip
// invoking accelerator callbacks (and not send WM_KEY* either).
//
// Capture behavior is implemented by [EventHandler appMouseEvent].
HWND Platform::getCapture()
{
  return g_fakeCapture;
}

void Platform::setCapture(HWND hwnd)
{
  g_fakeCapture = hwnd;
}

void Platform::releaseCapture()
{
  g_fakeCapture = nullptr;
}
