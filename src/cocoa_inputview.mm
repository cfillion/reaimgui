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

#include "cocoa_inputview.hpp"

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
#include "window.hpp"

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

constexpr NSRange kEmptyRange { NSNotFound, 0 };

constexpr uint8_t
  VK_APPS       { 0x5D },
  VK_OEM_PLUS   { 0xBB },
  VK_OEM_COMMA  { 0xBC },
  VK_OEM_MINUS  { 0xBD },
  VK_OEM_PERIOD { 0xBE },
  VK_OEM_1      { 0xBA }, // ;:
  VK_OEM_2      { 0xBF }, // /?
  VK_OEM_3      { 0xC0 }, // `~
  VK_OEM_4      { 0xDB }, // [(
  VK_OEM_5      { 0xDC }, // \|
  VK_OEM_6      { 0xDD }, // ])
  VK_OEM_7      { 0xDE }, // '"
  VK_OEM_102    { 0xE2 };

static NSString *eventCharsIgnoringMods(NSEvent *event)
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

  if(strLen)
    return [NSString stringWithCharacters:str length:strLen];
  else
    return [event charactersIgnoringModifiers];
}

static uint8_t virtualKeyCode(NSEvent *event)
{
  const uint16_t keyCode { [event keyCode] };

  // hard-code these keys based on their physical location on the keyboard
  // https://developer.apple.com/library/archive/documentation/mac/pdf/MacintoshToolboxEssentials.pdf (figure 2-10)
  switch(keyCode) {
  case 0x33: return VK_BACK;
  case 0x72: return VK_INSERT;
  case 0x75: return VK_DELETE;
  case 0x73: return VK_HOME;
  case 0x77: return VK_END;
  case 0x74: return VK_PRIOR;
  case 0x79: return VK_NEXT;
  case 0x6e: return VK_APPS;
  case 0x7b: return VK_LEFT;
  case 0x7c: return VK_RIGHT;
  case 0x7e: return VK_UP;
  case 0x7d: return VK_DOWN;

  case 0x0a: return VK_OEM_102;
  case 0x2a: return VK_OEM_5;

  // number row
  case 0x12: return '1';
  case 0x13: return '2';
  case 0x14: return '3';
  case 0x15: return '4';
  case 0x17: return '5';
  case 0x16: return '6';
  case 0x1a: return '7';
  case 0x1c: return '8';
  case 0x19: return '9';
  case 0x1d: return '0';

  // numpad
  case 0x47: return VK_NUMLOCK;
  case 0x51: return VK_SEPARATOR;
  case 0x4b: return VK_DIVIDE;
  case 0x43: return VK_MULTIPLY;
  case 0x4e: return VK_SUBTRACT;
  case 0x45: return VK_ADD;
  case 0x4c: return VK_RETURN;
  case 0x41: return VK_DECIMAL;
  case 0x52: return VK_NUMPAD0;
  case 0x53: return VK_NUMPAD1;
  case 0x54: return VK_NUMPAD2;
  case 0x55: return VK_NUMPAD3;
  case 0x56: return VK_NUMPAD4;
  case 0x57: return VK_NUMPAD5;
  case 0x58: return VK_NUMPAD6;
  case 0x59: return VK_NUMPAD7;
  case 0x5b: return VK_NUMPAD8;
  case 0x5c: return VK_NUMPAD9;
  }

  // obey keyboard layout for other keys (ASCII virtual key code)
  NSString *chars;
  if([event modifierFlags] & NSEventModifierFlagShift)
    chars = eventCharsIgnoringMods(event);
  else
    chars = [event charactersIgnoringModifiers];

  if(![chars length])
    return keyCode & 0xFF;

  uint16_t charValue { [chars characterAtIndex:0] };
  if(charValue >= NSF1FunctionKey && charValue <= NSF24FunctionKey)
    charValue += VK_F1 - NSF1FunctionKey;
  else if(charValue >= 'a' && charValue <= 'z')
    charValue += 'A'-'a';

  // attempt to be compatible with QWERTY/AZERTY, may be innacurate
  switch(charValue) {
  case '-':  return VK_OEM_MINUS;
  case '=':  return VK_OEM_PLUS;
  case ',':  return VK_OEM_COMMA;
  case '.':  return VK_OEM_PERIOD;
  case ';': case '$': return VK_OEM_1;
  case '/': case ':': return VK_OEM_2;
  case '`':  return VK_OEM_3;
  case '[':  return VK_OEM_4;
  case ']': case ')': return VK_OEM_6;
  case '\'': return VK_OEM_7;
  }

  return charValue & 0xFF;
}

@implementation InputView
- (instancetype)initWithWindow:(Window *)window
{
  NSView *parent { (__bridge NSView *)window->nativeHandle() };
  self = [super initWithFrame:[parent frame]];
  m_window = window;

  // Fill the window to receive mouse click events
  [self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [parent setAutoresizesSubviews:YES];
  [parent addSubview:self];

  [self registerForDraggedTypes:@[NSFilenamesPboardType]];

  return self;
}

- (void)setImePosition:(NSPoint)pos
{
  m_imePos = pos;
}

- (BOOL)acceptsFirstResponder
{
  // Make ourselves first responder again when regaining focus while docked
  return YES;
}

- (BOOL)resignFirstResponder
{
  m_window->context()->clearFocus();
  return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
  return YES;
}

- (void)mouseDown:(NSEvent *)event
{
  m_window->mouseDown(WM_LBUTTONDOWN);
}

- (void)mouseUp:(NSEvent *)event
{
  m_window->mouseUp(WM_LBUTTONUP);
}

- (void)rightMouseDown:(NSEvent *)event
{
  m_window->mouseDown(WM_RBUTTONDOWN);
}

- (void)rightMouseUp:(NSEvent *)event
{
  m_window->mouseUp(WM_RBUTTONUP);
}

- (void)otherMouseDown:(NSEvent *)event
{
  m_window->mouseDown(WM_MBUTTONDOWN);
}

- (void)otherMouseUp:(NSEvent *)event
{
  m_window->mouseUp(WM_MBUTTONUP);
}

- (const char *)getSwellClass
{
  return m_window->getSwellClass();
}

- (void)keyDown:(NSEvent *)event
{
  // Send key to the system input manager. It will reply by sending insertText.
  [self interpretKeyEvents:@[event]];

  m_window->context()->keyInput(virtualKeyCode(event), true);
}

- (void)keyUp:(NSEvent *)event
{
  m_window->context()->keyInput(virtualKeyCode(event), false);
}

- (void)flagsChanged:(NSEvent *)event
{
  struct Modifier {
    unsigned short keyCode; unsigned long modFlag; uint8_t virtualKeyCode;
  };

  constexpr Modifier modifiers[] {
    { kVK_Shift,        NSEventModifierFlagShift,   VK_SHIFT   },
    { kVK_RightShift,   NSEventModifierFlagShift,   VK_SHIFT   },
    { kVK_Control,      NSEventModifierFlagControl, VK_CONTROL },
    { kVK_RightControl, NSEventModifierFlagControl, VK_CONTROL },
    { kVK_Option,       NSEventModifierFlagOption,  VK_MENU    },
    { kVK_RightOption,  NSEventModifierFlagOption,  VK_MENU    },
    { kVK_Command,      NSEventModifierFlagCommand, VK_LWIN    },
    { kVK_RightCommand, NSEventModifierFlagCommand, VK_LWIN    },
  };

  const unsigned short keyCode { [event keyCode] };
  const unsigned long modFlags { [event modifierFlags] };

  for(const auto &modifier : modifiers) {
    if(modifier.keyCode == keyCode) {
      m_window->context()->keyInput(modifier.virtualKeyCode, modFlags & modifier.modFlag);
      return;
    }
  }
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
  NSPasteboard *pboard { [sender draggingPasteboard] };
  if(![[pboard types] containsObject:NSFilenamesPboardType])
    return NSDragOperationNone;

  NSArray *nsfiles { [pboard propertyListForType:NSFilenamesPboardType] };
  const NSUInteger count { [nsfiles count] };

  std::vector<std::string> files;
  files.reserve(count);

  for(size_t i { 0 }; i < count; ++i) {
    NSString *file { [nsfiles objectAtIndex:i] };
    files.emplace_back([file UTF8String]);
  }

  m_window->context()->beginDrag(std::move(files));

  return NSDragOperationGeneric;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
  m_window->context()->endDrag(false);
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
  NSPasteboard *pboard { [sender draggingPasteboard] };
  if([[pboard types] containsObject:NSFilenamesPboardType]) {
    m_window->context()->endDrag(true);
    return YES;
  }

  return NO;
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
    ImWchar codepoint {};
    if([characters getBytes:&codepoint
                   maxLength:sizeof(codepoint)
                  usedLength:nullptr
                    encoding:NSUTF32StringEncoding
                     options:0
                       range:range
              remainingRange:&range])
    m_window->context()->charInput(codepoint);
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
  return NSMakeRect(m_imePos.x, m_imePos.y, 0, 0);
}

- (void)doCommandBySelector:(SEL)selector
{
}
@end
