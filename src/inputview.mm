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

#include <Carbon/Carbon.h>

// prevent name conflicts between Carbon.h and swell-functions.h
#define CreateEvent     SWELL_CreateEvent
#define GetMenu         SWELL_GetMenu
#define DrawMenuBar     SWELL_DrawMenuBar
#define DeleteMenu      SWELL_DeleteMenu
#define CheckMenuItem   SWELL_CheckMenuItem
#define EnableMenuItem  SWELL_EnableMenuItem
#define InsertMenuItem  SWELL_InsertMenuItem
#define SetMenuItemText SWELL_SetMenuItemText
#define ShowWindow      SWELL_ShowWindow
#define IsWindowVisible SWELL_IsWindowVisible
#include <swell/swell.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

constexpr NSRange kEmptyRange { NSNotFound, 0 };

static NSEvent *eventCharsIgnoringMods(NSEvent *event)
{
  // NSEvent's charactersIgnoringModifiers does not ignore Shift.
  //
  // This is undesirable because it would lead to stuck keys:
  // Shift down, 4 down (value for '$'), Shift up, 4 up (oops, value for '4'!).

  const auto inputSource { TISCopyCurrentKeyboardLayoutInputSource() };
  const auto layoutData { static_cast<CFDataRef>(
    TISGetInputSourceProperty(inputSource, kTISPropertyUnicodeKeyLayoutData)) };
  const auto layout
    { reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layoutData)) };

  UInt32 deadKeyState {};
  UniChar str[4];
  UniCharCount strLen {};
  UCKeyTranslate(layout, [event keyCode], kUCKeyActionDown, 0, LMGetKbdType(),
    kUCKeyTranslateNoDeadKeysMask, &deadKeyState, std::size(str), &strLen, str);

  if(!strLen)
    return event;

  NSString *chars { [NSString stringWithCharacters:str length:strLen] };

  return [NSEvent keyEventWithType:[event type]
                          location:[event locationInWindow]
                     modifierFlags:[event modifierFlags]
                         timestamp:[event timestamp]
                      windowNumber:[event windowNumber]
                           context:nil
                        characters:chars
       charactersIgnoringModifiers:chars
                         isARepeat:[event isARepeat]
                           keyCode:[event keyCode]];
}

static uint8_t virtualKeyCode(NSEvent *event)
{
  if([event modifierFlags] & NSEventModifierFlagShift)
    event = eventCharsIgnoringMods(event);

  int flags;
  return SWELL_MacKeyToWindowsKey((__bridge void *)event, &flags) & 0xFF;
}

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

- (BOOL)acceptsFirstResponder
{
  // Make ourselves first responder again when regaining focus while docked
  return YES;
}

- (BOOL)resignFirstResponder
{
  m_context->clearFocus();
  return YES;
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

  m_context->keyInput(virtualKeyCode(event), true);
}

- (void)keyUp:(NSEvent *)event
{
  m_context->keyInput(virtualKeyCode(event), false);
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
