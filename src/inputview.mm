/* ReaImGui: ReaScript binding for Dear ImGui
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

#include "inputview.hpp"

#include "context.hpp"

#include <swell/swell-types.h>

#include <Carbon/Carbon.h> // key code constants

constexpr NSRange kEmptyRange { NSNotFound, 0 };

@implementation InputView
- (instancetype)initWithContext:(Context *)context
                         parent:(NSView *)parent
{
  self = [super initWithFrame:parent.frame];
  m_context = context;

  // Fill the window to receive mouse click events
  self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  parent.autoresizesSubviews = YES;
  [parent addSubview:self];
  [[parent window] makeFirstResponder:self];

  return self;
}

- (BOOL)resignFirstResponder
{
  // Always retain focus.
  // For some reason pressing the Enter key leads to REAPER invoking SetFocus
  // on something else (the window perhaps?).
  //
  // This breaks receiving mouse input from SWELL.
  return NO;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
  return YES;
}

- (void)mouseDown:(NSEvent *)event
{
  m_context->mouseDown(WM_LBUTTONDOWN);
}

- (void)mouseUp:(NSEvent *)event
{
  m_context->mouseUp(WM_LBUTTONUP);
}

- (void)rightMouseDown:(NSEvent *)event
{
  m_context->mouseDown(WM_RBUTTONDOWN);
}

- (void)rightMouseUp:(NSEvent *)event
{
  m_context->mouseUp(WM_RBUTTONUP);
}

- (void)otherMouseDown:(NSEvent *)event
{
  m_context->mouseDown(WM_MBUTTONDOWN);
}

- (void)otherMouseUp:(NSEvent *)event
{
  m_context->mouseUp(WM_MBUTTONUP);
}

- (void)keyDown:(NSEvent *)event
{
  // Send key to the system input manager. It will reply by sending insertText.
  [self interpretKeyEvents:@[event]];

  // SWELL keyboard events report different key codes depending on the
  // modifiers. This is undesirable because it would lead to stuck keys:
  // Shift down, 4 down (keycode for $), Shift up, 4 up (oops, keycode for 4!).
  m_context->keyInput([event keyCode], true);
}

- (void)keyUp:(NSEvent *)event
{
  m_context->keyInput([event keyCode], false);
}

// Implement NSTextInputClient for IME-aware text input
// Extracted from GLFW (zlib license) with minimal changes
- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
  const NSEvent *event { [NSApp currentEvent] };

  if([event modifierFlags] & NSEventModifierFlagCommand)
    return;

  const NSString *characters;
  if([string isKindOfClass:[NSAttributedString class]])
    characters = [string string];
  else
    characters = string;

  NSRange range { NSMakeRange(0, [characters length]) };

  while(range.length) {
    uint32_t codepoint {};

    if(![characters getBytes:&codepoint
                   maxLength:sizeof(codepoint)
                  usedLength:nullptr
                    encoding:NSUTF32StringEncoding
                     options:0
                       range:range
              remainingRange:&range])
      continue;
    else if(codepoint >= 0xf700 && codepoint <= 0xf7ff)
      continue; // unicode private range

    m_context->charInput(codepoint);
  }
}

- (BOOL)hasMarkedText
{
  return [m_markedText length] > 0;
}

- (NSRange)markedRange
{
  if([m_markedText length] > 0)
    return NSMakeRange(0, [m_markedText length] - 1);
  else
    return kEmptyRange;
}

- (NSRange)selectedRange
{
  return kEmptyRange;
}

- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange
{
#if !__has_feature(objc_arc)
  [m_markedText release];
#endif
  if([string isKindOfClass:[NSAttributedString class]])
    m_markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
  else
    m_markedText = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText
{
  [[m_markedText mutableString] setString:@""];
}

- (NSArray *)validAttributesForMarkedText
{
  return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange
{
  return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
  return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange
{
  // const NSRect frame { [[self superview] frame] };
  // return NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0);
  return NSMakeRect(0, 0, 0, 0); // this doesn't seem to be used
}

- (void)doCommandBySelector:(SEL)selector
{
}
@end

void setupMacOSKeyMap(ImGuiIO &io)
{
  io.KeyMap[ImGuiKey_Tab]         = kVK_Tab;
  io.KeyMap[ImGuiKey_LeftArrow]   = kVK_LeftArrow;
  io.KeyMap[ImGuiKey_RightArrow]  = kVK_RightArrow;
  io.KeyMap[ImGuiKey_UpArrow]     = kVK_UpArrow;
  io.KeyMap[ImGuiKey_DownArrow]   = kVK_DownArrow;
  io.KeyMap[ImGuiKey_PageUp]      = kVK_PageUp;
  io.KeyMap[ImGuiKey_PageDown]    = kVK_PageDown;
  io.KeyMap[ImGuiKey_Home]        = kVK_Home;
  io.KeyMap[ImGuiKey_End]         = kVK_End;
  io.KeyMap[ImGuiKey_Insert]      = kVK_Help;
  io.KeyMap[ImGuiKey_Delete]      = kVK_ForwardDelete;
  io.KeyMap[ImGuiKey_Backspace]   = kVK_Delete;
  io.KeyMap[ImGuiKey_Space]       = kVK_Space;
  io.KeyMap[ImGuiKey_Enter]       = kVK_Return;
  io.KeyMap[ImGuiKey_Escape]      = kVK_Escape;
  io.KeyMap[ImGuiKey_KeyPadEnter] = kVK_Return;
  io.KeyMap[ImGuiKey_A]           = kVK_ANSI_A;
  io.KeyMap[ImGuiKey_C]           = kVK_ANSI_C;
  io.KeyMap[ImGuiKey_V]           = kVK_ANSI_V;
  io.KeyMap[ImGuiKey_X]           = kVK_ANSI_X;
  io.KeyMap[ImGuiKey_Y]           = kVK_ANSI_Y;
  io.KeyMap[ImGuiKey_Z]           = kVK_ANSI_Z;
}
