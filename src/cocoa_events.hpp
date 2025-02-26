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

#ifndef REAIMGUI_COCOA_EVENTS_HPP
#define REAIMGUI_COCOA_EVENTS_HPP

#include <AppKit/AppKit.h>

class Context;

@interface EventHandler : NSObject {
@private
  id m_mouseMonitor;
}

- (instancetype)init;
- (void)dealloc;

- (void)watchView:(NSView *)view context:(Context *)ctx;

- (void)screenChanged:(NSNotification *)notification;
- (void)menuDidBeginTracking:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;
- (NSEvent *)appMouseEvent:(NSEvent *)event;
@end

#endif
