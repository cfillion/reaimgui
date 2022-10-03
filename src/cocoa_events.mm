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
