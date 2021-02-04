// vi: ft=objcpp

@interface TextInput : NSView<NSTextInputClient>
{
  NSMutableAttributedString *m_markedText;
  Window *m_window;
}
@end

constexpr NSRange kEmptyRange { NSNotFound, 0 };

@implementation TextInput
- (instancetype)initWithWindow:(Window *)window
{
  self = [super init];
  m_window = window;
  return self;
}

- (void)keyDown:(NSEvent *)event
{
  [self interpretKeyEvents:[NSArray arrayWithObject:event]];
  [super keyDown:event];
}

// extracted from GLFW (zlib license) with minimal changes
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

    m_window->charInput(codepoint);
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
  // const NSRect frame { [[self superview] frame] };
  // return NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0);
  return NSMakeRect(0, 0, 0, 0); // this doesn't seem to be used
}

- (void)doCommandBySelector:(SEL)selector
{
}
@end
