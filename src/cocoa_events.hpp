#ifndef REAIMGUI_COCOA_EVENTS_HPP
#define REAIMGUI_COCOA_EVENTS_HPP

#include <AppKit/AppKit.h>

@interface EventHandler : NSObject
- (instancetype)init;
- (void)dealloc;

- (void)watchView:(NSView *)view;
- (void)windowDidResignKey:(NSNotification *)notification;
@end

#endif
