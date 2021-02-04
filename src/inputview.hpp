#ifndef REAIMGUI_INPUTVIEW_HPP
#define REAIMGUI_INPUTVIEW_HPP

#include <AppKit/AppKit.h>

class Window;

@interface InputView : NSView<NSTextInputClient>
- (instancetype)initWithWindow:(Window *)window
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
