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

#include "cocoa_events.hpp"

#include "context.hpp"
#include "platform.hpp"
#include "window.hpp"

#include <objc/runtime.h>
#include <functional>

#ifndef _WIN32
#  include <swell/swell.h>
#  include <WDL/wdltypes.h>
#endif

constexpr char GET_WND_CTX {};

static Context *getWindowContext(NSWindow *window)
{
  if(Context *(^getContext)() {objc_getAssociatedObject(window, &GET_WND_CTX)}) {
    Context *ctx {getContext()};
    if(Resource::isValid(ctx))
      return ctx;
    else
      objc_setAssociatedObject(window, &GET_WND_CTX, nil, OBJC_ASSOCIATION_ASSIGN);
  }

  return nullptr;
}

@implementation EventHandler
- (EventHandler *)init
{
  NSNotificationCenter *nc {[NSNotificationCenter defaultCenter]};
  [nc addObserver:self
         selector:@selector(screenChanged:)
             name:NSApplicationDidChangeScreenParametersNotification
           object:nil];
  [nc addObserver:self
         selector:@selector(menuDidBeginTracking:)
             name:NSMenuDidBeginTrackingNotification
           object:nil];
  [nc addObserver:self
         selector:@selector(windowDidResignKey:)
             name:NSWindowDidResignKeyNotification
           object:nil];

  constexpr NSEventMask mouseEventMask {
    NSEventMaskLeftMouseDown  | NSEventMaskLeftMouseUp  |
    NSEventMaskRightMouseDown | NSEventMaskRightMouseUp |
    NSEventMaskOtherMouseDown | NSEventMaskOtherMouseUp
  };
  __weak EventHandler *weakSelf {self}; // don't prevent dealloc
  auto onMouseEvent
    {^NSEvent *(NSEvent *event) { return [weakSelf appMouseEvent:event]; }};
  m_mouseMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:mouseEventMask
                                                         handler:onMouseEvent];

  return self;
}

- (void)dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [NSEvent removeMonitor:m_mouseMonitor];
}

- (void)watchView:(NSView *)view context:(Context *)ctx
{
  Context *(^getContext)() {^{ return ctx; }};
  objc_setAssociatedObject([view window], &GET_WND_CTX,
                           getContext, OBJC_ASSOCIATION_COPY);
}

- (void)screenChanged:(NSNotification *)notification
{
  // some version of macOS after 10.14 does not call
  // NSWindowDelegate::windowDidMove when changing display resolution
  using namespace std::placeholders;
  Resource::foreach<Context>(std::bind(&Context::invalidateViewportsPos, _1));
}

- (void)menuDidBeginTracking:(NSNotification *)notification
{
  // Not using Context::updateFocus: the window under the menu still has focus
  if(Context *ctx {Window::contextFromHwnd(GetForegroundWindow())})
    ctx->IO().ClearInputKeys();

  if(HWND capture {Platform::getCapture()}) {
    Window *window
      {reinterpret_cast<Window *>(GetWindowLongPtr(capture, GWLP_USERDATA))};
    window->releaseMouse();
  }
}

- (void)windowDidResignKey:(NSNotification *)notification
{
  // The next window has not yet been promoted to key window yet at this time.
  // It may or may not belong to the same context, so updateFocus() must wait.
  NSWindow *window {[notification object]};
  dispatch_async(dispatch_get_main_queue(), ^{
    if(Context *context {getWindowContext(window)})
      context->updateFocus();
  });
}

- (NSEvent *)appMouseEvent:(NSEvent *)event
{
  // Redirect mouse events to the currently captured window.
  // This is for receiving mouseUp events after the window that got the
  // mouseDown was destroyed. See also Window::transferCapture.
  //
  // Cannot use [NSEvent mouseEventWithType] to recreate the event with
  // a different windowNumber, because there is no way to specify the
  // NSEvent buttonNumber. CGEventCreateMouseEvent can set the buttonNumber
  // but not the target window.

  HWND capture {Platform::getCapture()}; // only returns our windows
  if(!capture)
    return event;

  const auto button {[event buttonNumber]}; // 0-32
  if(button >= ImGuiMouseButton_COUNT)
    return nil;

  Window *window
    {reinterpret_cast<Window *>(GetWindowLongPtr(capture, GWLP_USERDATA))};

  switch(event.type) {
  case NSEventTypeLeftMouseDown:
  case NSEventTypeRightMouseDown:
  case NSEventTypeOtherMouseDown:
    window->mouseDown(button);
    return nil;
  case NSEventTypeLeftMouseUp:
  case NSEventTypeRightMouseUp:
  case NSEventTypeOtherMouseUp:
    window->mouseUp(button);
    return nil;
  default:
    return event;
  }
}
@end
