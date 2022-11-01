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

#include "cocoa_events.hpp"

#include "context.hpp"
#include "window.hpp"

#include <objc/runtime.h>

#ifndef _WIN32
#  include <swell/swell.h>
#  include <WDL/wdltypes.h>
#endif

constexpr char GET_HWND_WEAK {};

static Window *getViewportWindow(NSWindow *window)
{
  if(HWND (^getHwndWeak)() { objc_getAssociatedObject(window, &GET_HWND_WEAK) }) {
    if(HWND hwnd { getHwndWeak() })
      return reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    else
      objc_setAssociatedObject(window, &GET_HWND_WEAK, nil, OBJC_ASSOCIATION_ASSIGN);
  }

  return nullptr;
}

@implementation EventHandler
- (EventHandler *)init
{
  NSNotificationCenter *nc { [NSNotificationCenter defaultCenter] };
  [nc addObserver:self
         selector:@selector(windowDidResignKey:)
             name:NSWindowDidResignKeyNotification
           object:nil];
  return self;
}

- (void)dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)watchView:(NSView *)view
{
  // attach a weak reference to the view on its current OS window
  __weak NSView *ptr { view };
  HWND (^getHwndWeak)() { ^{ return (__bridge HWND)ptr; } };
  objc_setAssociatedObject([view window], &GET_HWND_WEAK,
                           getHwndWeak, OBJC_ASSOCIATION_COPY);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
  // The next window has not yet been promoted to key window yet at this time.
  // It may or may not belong to the same context, so updateFocus() must wait.
  //
  // getViewportWindow checks whether the NSView still exists so it must be
  // called synchronously with updateFocus.
  NSWindow *window { [notification object] };
  dispatch_async(dispatch_get_main_queue(), ^{
    if(Window *viewportWindow { getViewportWindow(window) })
      viewportWindow->context()->updateFocus();
  });
}
@end
