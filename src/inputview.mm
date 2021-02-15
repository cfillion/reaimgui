#include "inputview.hpp"

#include "context.hpp"

#include <swell/swell-types.h>

constexpr NSRange kEmptyRange { NSNotFound, 0 };

@implementation InputView
{
  NSMutableAttributedString *m_markedText;
  Context *m_context;
}

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
  if ([string isKindOfClass:[NSAttributedString class]])
    characters = [string string];
  else
    characters = string;

  NSRange range { NSMakeRange(0, [characters length]) };

  while(range.length) {
    uint32_t codepoint {};

    if (![characters getBytes:&codepoint
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
