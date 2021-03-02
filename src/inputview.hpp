/* ReaImGui: ReaScript binding for dear imgui
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

#ifndef REAIMGUI_INPUTVIEW_HPP
#define REAIMGUI_INPUTVIEW_HPP

#include <AppKit/AppKit.h>

class Context;
struct ImGuiIO;

void setupMacOSKeyMap(ImGuiIO &);

@interface InputView : NSView<NSTextInputClient>
{
  @private
  NSMutableAttributedString *m_markedText;
  Context *m_context;
}

- (instancetype)initWithContext:(Context *)context
                         parent:(NSView *)parent;
- (BOOL)resignFirstResponder;

- (void)mouseDown:(NSEvent *)event;
- (void)mouseUp:(NSEvent *)event;
- (void)rightMouseDown:(NSEvent *)event;
- (void)rightMouseUp:(NSEvent *)event;
- (void)otherMouseDown:(NSEvent *)event;
- (void)otherMouseUp:(NSEvent *)event;
- (void)keyDown:(NSEvent *)event;
- (void)keyUp:(NSEvent *)event;

// NSTextInputClient
- (void)insertText:(id)string replacementRange:(NSRange)replacementRange;
- (BOOL)hasMarkedText;
- (NSRange)markedRange;
- (NSRange)selectedRange;
- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange;
- (void)unmarkText;
- (NSArray *)validAttributesForMarkedText;
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange;
- (NSUInteger)characterIndexForPoint:(NSPoint)point;
- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange;
- (void)doCommandBySelector:(SEL)selector;
@end

#endif

// vi: ft=objcpp
