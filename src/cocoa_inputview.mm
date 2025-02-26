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

#include "cocoa_inputview.hpp"

#include "context.hpp"

#define KeyMap CarbonKeyMap
#include <Carbon/Carbon.h>
#undef KeyMap

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
#include "keymap.hpp"
#include "window.hpp"

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

constexpr NSRange kEmptyRange {NSNotFound, 0};

static NSString *eventCharsIgnoringMods(NSEvent *event)
{
  // NSEvent's charactersIgnoringModifiers does not ignore Shift.
  //
  // This is undesirable because it would lead to stuck keys:
  // Shift down, 4 down (value for '$'), Shift up, 4 up (oops, value for '4'!).

  const auto inputSource {TISCopyCurrentKeyboardLayoutInputSource()};
  const auto layoutData {static_cast<CFDataRef>(
    TISGetInputSourceProperty(inputSource, kTISPropertyUnicodeKeyLayoutData))};
  const auto layout
    {reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layoutData))};

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

static int translateKeyCode(NSEvent *event)
{
  const uint16_t keyCode {[event keyCode]};

  // hard-code these keys based on their physical location on the keyboard
  // https://developer.apple.com/library/archive/documentation/mac/pdf/MacintoshToolboxEssentials.pdf
  // (figure 2-10, page 86)
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

  // case 0x0a: return VK_OEM_102;
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
  case 0x4c: return ImGuiKey_KeypadEnter;
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

  // case ??: return ImGuiKey_AppBack;
  // case ??: return ImGuiKey_AppForward;
  }

  // obey keyboard layout for other keys (ASCII virtual key code)
  NSString *chars;
  if([event modifierFlags] & NSEventModifierFlagShift)
    chars = eventCharsIgnoringMods(event);
  else
    chars = [event charactersIgnoringModifiers];

  if(![chars length])
    return keyCode & 0xFF;

  uint16_t charValue {[chars characterAtIndex:0]};

  // attempt to be compatible with QWERTY/AZERTY, may be inaccurate
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

  case NSPrintScreenFunctionKey: return VK_SNAPSHOT;
  case NSPauseFunctionKey:       return VK_PAUSE;
  }

  if(charValue >= NSF1FunctionKey && charValue <= NSF24FunctionKey)
    charValue += VK_F1 - NSF1FunctionKey;
  else if(charValue >= 'a' && charValue <= 'z')
    charValue += 'A' - 'a';

  return charValue & 0xFF;
}

@implementation InputView
- (instancetype)initWithWindow:(Window *)window
{
  NSView *parent {(__bridge NSView *)window->nativeHandle()};
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
  // Invoked when docked and losing focus to another control in the same OS
  // window or destroying a docked window
  // Calling updateFocus later because at this time focus has yet to be transferred
  // Cannot use m_window in the callback because we might get destroyed before
  // it is executed.
  Context *ctx {m_window->context()};
  dispatch_async(dispatch_get_main_queue(), ^{
    if(Resource::isValid(ctx))
      ctx->updateFocus();
  });

  return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
  return YES;
}

- (BOOL)shouldDelayWindowOrderingForEvent:(NSEvent *)event
{
  // Give focus to the window on mouseUp rather than mouseDown
  // to let mouseDown cancel it by calling preventWindowOrdering
  return m_window->viewport()->Flags & ImGuiViewportFlags_NoFocusOnClick;
}

// Only receives mouse events prior to capturing the mouse
// Susequent events (eg. mouse up) are handled via [EventHandler appMouseEvent]
- (void)mouseDown:(NSEvent *)event
{
  if(m_window->viewport()->Flags & ImGuiViewportFlags_NoFocusOnClick)
    [NSApp preventWindowOrdering];
  m_window->mouseDown(ImGuiMouseButton_Left);
}

- (void)rightMouseDown:(NSEvent *)event
{
  m_window->mouseDown(ImGuiMouseButton_Right);
}

- (void)otherMouseDown:(NSEvent *)event
{
  const auto button {[event buttonNumber]}; // 0-32
  if(button < ImGuiMouseButton_COUNT)
    m_window->mouseDown(button);
}

- (const char *)getSwellClass
{
  return m_window->getSwellClass();
}

- (void)keyDown:(NSEvent *)event
{
  // Send key to the system input manager. It will reply by sending insertText.
  [self interpretKeyEvents:@[event]];

  const ImGuiKey key {static_cast<ImGuiKey>(translateKeyCode(event))};
  m_window->context()->keyInput(key, true);
}

- (void)keyUp:(NSEvent *)event
{
  const ImGuiKey key {static_cast<ImGuiKey>(translateKeyCode(event))};
  m_window->context()->keyInput(key, false);
}

- (void)flagsChanged:(NSEvent *)event
{
  struct Key {
    unsigned short keyCode;
    ImGuiKey key;
    unsigned long mask;
  };
  struct Modifier {
    ImGuiKey modkey;
    unsigned long flag;
    Key keys[2];
  };

  constexpr Modifier modifiers[] {
    {ImGuiMod_Ctrl,  NSEventModifierFlagControl, {
      {kVK_Control,      ImGuiKey_LeftCtrl,  0x0001},
      {kVK_RightControl, ImGuiKey_RightCtrl, 0x2000}
    }},
    {ImGuiMod_Shift, NSEventModifierFlagShift, {
      {kVK_Shift,      ImGuiKey_LeftShift,  0x0002},
      {kVK_RightShift, ImGuiKey_RightShift, 0x0004}
    }},
    {ImGuiMod_Super, NSEventModifierFlagCommand, {
      {kVK_Command,      ImGuiKey_LeftSuper,  0x0008},
      {kVK_RightCommand, ImGuiKey_RightSuper, 0x0010},
    }},
    {ImGuiMod_Alt,   NSEventModifierFlagOption, {
      {kVK_Option,      ImGuiKey_LeftAlt,  0x0020},
      {kVK_RightOption, ImGuiKey_RightAlt, 0x0040},
    }},
  };

  const unsigned short keyCode {[event keyCode]};
  const unsigned long modFlags {[event modifierFlags]};

  for(const auto &modifier : modifiers) {
    m_window->context()->keyInput(modifier.modkey, modFlags & modifier.flag);

    for(const auto &key : modifier.keys) {
      if(key.keyCode == keyCode)
        m_window->context()->keyInput(key.key, modFlags & key.mask);
    }
  }
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
  NSPasteboard *pboard {[sender draggingPasteboard]};
  if(![[pboard types] containsObject:NSFilenamesPboardType])
    return NSDragOperationNone;

  NSArray *nsfiles {[pboard propertyListForType:NSFilenamesPboardType]};
  const NSUInteger count {[nsfiles count]};

  std::vector<std::string> files;
  files.reserve(count);

  for(size_t i {}; i < count; ++i) {
    NSString *file {[nsfiles objectAtIndex:i]};
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
  NSPasteboard *pboard {[sender draggingPasteboard]};
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
  const NSEvent *event {[NSApp currentEvent]};

  if([event modifierFlags] & NSEventModifierFlagCommand)
    return;

  const NSString *characters;
  if([string isKindOfClass:[NSAttributedString class]])
    characters = [string string];
  else
    characters = string;

  NSRange range {NSMakeRange(0, [characters length])};

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
