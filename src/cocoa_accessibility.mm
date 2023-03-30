/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

#include "accessibility.hpp"

#include "platform.hpp"
#include "viewport.hpp"

#include <AppKit/AppKit.h>
#include <imgui/imgui_internal.h>

@interface AccessibilityElement : NSAccessibilityElement {
  ImGuiIO *m_io;
}
- (instancetype)init;
- (void)setState:(const AccessibilityItem::State &)state
         forItem:(const AccessibilityItem &)item;
- (BOOL)accessibilityPerformPress;
- (id)accessibilityHitTest:(NSPoint)point;
@end

struct GetParentElement {
  id<NSAccessibility> operator()(Viewport *);
  id<NSAccessibility> operator()(AccessibilityItem *);
};

struct ItemBridge {
public:
  ItemBridge(const AccessibilityItem &, const AccessibilityItem::State &);
  ~ItemBridge();

  AccessibilityElement *operator*() const { return m_element; }

private:
  AccessibilityElement *m_element;
};

static AccessibilityElement *getElement(const std::shared_ptr<void> &ptr)
{
  return **std::static_pointer_cast<ItemBridge>(ptr);
}

ItemBridge::ItemBridge(const AccessibilityItem &item, const AccessibilityItem::State &state)
  : m_element { [[AccessibilityElement alloc] init] }
{
  [m_element setState:state forItem:item];

  const AccessibilityParent &parent { state.a11y->findItemParent(item.id()) };
  id<NSAccessibility> parentElement { std::visit(GetParentElement{}, parent) };
  [m_element setAccessibilityParent:parentElement];

  NSArray *children { [parentElement accessibilityChildren] };
  if(children)
    children = [children arrayByAddingObject:m_element];
  else
    children = @[m_element]; // NSAccessibilityElement initially has a nil array
  [parentElement setAccessibilityChildren:children];
  if(@available(macOS 10.13, *))
    [parentElement setAccessibilityChildrenInNavigationOrder:children];
}

ItemBridge::~ItemBridge()
{
  id<NSAccessibility> parent { [m_element accessibilityParent] };
  NSMutableArray *children { [[parent accessibilityChildren] mutableCopy] };
  [children removeObject:m_element];
  [parent setAccessibilityChildren:children];
  if(@available(macOS 10.13, *)) // enforce order of elements (random otherwise)
    [parent setAccessibilityChildrenInNavigationOrder:children];
}

id<NSAccessibility> GetParentElement::operator()(Viewport *viewport)
{
  // adding child items to the InputView for our
  // [AccessibilityElement accessibilityHitTest] be called instead of SWELL's
  return [(__bridge NSView *)viewport->nativeHandle() subviews][0];
}

id<NSAccessibility> GetParentElement::operator()(AccessibilityItem *item)
{
  return getElement(item->platformData());
}

void AccessibilityItem::setState(const State &state)
{
  if(m_platform.get())
    [getElement(m_platform) setState:state forItem:*this];
  else
    m_platform = std::make_shared<ItemBridge>(*this, state);
}

@implementation AccessibilityElement
- (instancetype)init
{
  self = [super init];
  m_io = &ImGui::GetIO();
  return self;
}

- (void)setState:(const AccessibilityItem::State &)state
         forItem:(const AccessibilityItem &)item
{
  if([[self accessibilityChildren] count] > 0)
    [self setAccessibilityRole:NSAccessibilityGroupRole]; // FIXME: move elsewhere
  else if(state.flags & ImGuiItemStatusFlags_Openable)
    [self setAccessibilityRole:NSAccessibilityDisclosureTriangleRole];
  else if(state.flags & ImGuiItemStatusFlags_Checkable)
    [self setAccessibilityRole:NSAccessibilityCheckBoxRole];
  else
    [self setAccessibilityRole:NSAccessibilityStaticTextRole];

  const ImVec4 &rect { item.rect() };
  ImVec2 pos { rect.x, rect.w };
  Platform::scalePosition(&pos, true);
  const NSRect nativeRect
    { NSMakeRect(pos.x, pos.y, rect.z - rect.x, rect.w - rect.y) };
  [self setAccessibilityFrame:nativeRect];

  NSString *label {
    [[NSString alloc] initWithBytes:state.label.data()
                             length:state.label.size()
                           encoding:NSUTF8StringEncoding]
  };
  [self setAccessibilityLabel:label];

  // [self setAccessibilityEdited:state.flags & ImGuiItemStatusFlags_Edited];

  if(state.flags & (ImGuiItemStatusFlags_Checkable |
                    ImGuiItemStatusFlags_Openable)) {
    constexpr int mask
      { ImGuiItemStatusFlags_Checked | ImGuiItemStatusFlags_Opened };
    NSNumber *value { [NSNumber numberWithBool:(state.flags & mask) != 0] };
    [self setAccessibilityValue:value];
  }
}

- (BOOL)accessibilityPerformPress
{
  const NSRect rect { [self accessibilityFrame] };
  ImVec2 pos {
    static_cast<float>(rect.origin.x + (rect.size.width / 2)),
    static_cast<float>(rect.origin.y + (rect.size.height / 2))
  };
  Platform::scalePosition(&pos, false);
  ImGui::SetCurrentContext(m_io->Ctx); // FIXME: remove once fixed upstream
  m_io->AddMousePosEvent(pos.x, pos.y);
  m_io->AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  m_io->AddMouseButtonEvent(ImGuiMouseButton_Left, false);

  // predict the effect of the click for immediate reading of the new state
  if([self accessibilityRole] == NSAccessibilityCheckBoxRole ||
     [self accessibilityRole] == NSAccessibilityDisclosureTriangleRole) {
    const BOOL value { [[self accessibilityValue] boolValue] };
    [self setAccessibilityValue:[NSNumber numberWithBool:!value]];
  }

  return YES;
}

- (id)accessibilityHitTest:(NSPoint)point
{
  // [self accessibilityFrame] does not always contain the children rects
  for(id child in [self accessibilityChildren]) {
    if(id bestMatch { [child accessibilityHitTest:point] })
      return bestMatch;
  }

  if(NSPointInRect(point, [self accessibilityFrame]))
    return self;
  else
    return nil;
}
@end
