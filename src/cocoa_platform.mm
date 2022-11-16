/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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
#include "opengl_renderer.hpp"

#include <AppKit/AppKit.h>
#include <imgui/imgui.h>

static HWND g_fakeCapture;

void Platform::install()
{
  // Temprarily enable repeat character input
  // WARNING: this is application-wide!
  NSUserDefaults *defaults { [NSUserDefaults standardUserDefaults] };
  [defaults registerDefaults:@{@"ApplePressAndHoldEnabled":@NO}];

  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_cocoa";

  OpenGLRenderer::install();
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new CocoaWindow { viewport, dockerHost };
}

void Platform::updateMonitors()
{
  // TODO
  // if(!g_monitorsChanged)
  //   return;

  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  pio.Monitors.resize(0); // recycle allocated memory (don't use clear here!)

  NSArray<NSScreen *> *screens { [NSScreen screens] };
  const CGFloat mainHeight { screens[0].frame.size.height };

  for(NSScreen *screen in screens) {
    const NSRect frame { [screen frame] }, workFrame { [screen visibleFrame] };
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

void Platform::scalePosition(ImVec2 *pos, bool, float)
{
  pos->y = ImGui::GetPlatformIO().Monitors[0].MainSize.y - pos->y;
}

static ImGuiViewport *nextViewportUnder
  (const NSPoint pos, const NSInteger windowNumber)
{
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };

  for(int i { 1 }; i < pio.Viewports.Size; ++i) { // skip the main viewport
    ImGuiViewport *viewport { pio.Viewports[i] };
    NSView *superView { (__bridge NSView *)viewport->PlatformHandle };

    // PlatformHandle is NULL for inactive DockerHosts
    if(!superView || [[superView window] windowNumber] != windowNumber)
      continue;

    // NSView's hitTest takes a point in the coordinate system of the view's
    // superview, not of the view itself.
    NSPoint clientPos { [[superView window] convertScreenToBase:pos] };
    clientPos = [superView convertPoint:clientPos fromView:nil];

    NSView *inputView { [superView subviews][0] };
    if([inputView hitTest:clientPos])
     return viewport;
  }

  return nullptr;
}

ImGuiViewport *Platform::viewportUnder(const ImVec2 pos)
{
  const NSPoint point { NSMakePoint(pos.x, pos.y) };

  NSInteger number { 0 };
  ImGuiViewport *viewport;

  do {
    number = [NSWindow windowNumberAtPoint:point belowWindowWithWindowNumber:number];
    viewport = nextViewportUnder(point, number);
  } while(viewport && !!(viewport->Flags & ImGuiViewportFlags_NoInputs));

  return viewport;
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
  if(cur == ImGuiMouseCursor_ResizeAll) {
    static HCURSOR bm { LoadCursor(nullptr, IDC_SIZEALL) };
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
// Real capturing is not necessary to get mouse release events outside of the
// window on macOS anyway.
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
