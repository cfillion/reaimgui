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

#include "cocoa_window.hpp"

#include "cocoa_events.hpp"
#include "cocoa_inject.hpp"
#include "cocoa_inputview.hpp"
#include "context.hpp"
#include "error.hpp"
#include "platform.hpp"
#include "renderer.hpp"

#include <imgui/imgui_internal.h>
#include <reaper_plugin_secrets.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

@interface SWELLWindowOverride : NSWindow
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen;
@end

@implementation SWELLWindowOverride
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen
{
  if(!CocoaInject::isTarget(self))
    return [super constrainFrameRect:frameRect toScreen:screen];

  // allow setPosition to move the window outside of the woring area
  return frameRect;
}
@end

CocoaWindow::CocoaWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
  : Window {viewport, dockerHost}
{
}

CocoaWindow::~CocoaWindow()
{
}

void CocoaWindow::create()
{
  createSwellDialog();
  m_view = (__bridge NSView *)m_hwnd; // SWELL_hwndChild inherits from NSView
  m_inputView = [[InputView alloc] initWithWindow:this];

  NSWindow *window {[m_view window]};
  m_defaultStyleMask = [window styleMask];
  m_previousScale = m_viewport->DpiScale;
  m_previousFlags = ~m_viewport->Flags; // mark all as modified
  // imgui calls update() before show(), it will apply the flags

  try {
    m_renderer = m_ctx->rendererFactory()->create(this);
  }
  catch(const backend_error &) {
    destroy();
    throw;
  }

  if(!isDocked()) {
    CocoaInject::inject([SWELLWindowOverride class], window);

    // enable transparency
    [window setOpaque:NO];
    [window setBackgroundColor:[NSColor clearColor]]; // required when decorations are enabled
  }

  static __weak EventHandler *g_handler;
  if(g_handler)
    m_eventHandler = g_handler;
  else
    g_handler = m_eventHandler = [[EventHandler alloc] init];
}

void CocoaWindow::show()
{
  Window::show();
  [m_eventHandler watchView:m_view context:m_ctx];
}

void CocoaWindow::setPosition(ImVec2 pos)
{
  Platform::scalePosition(&pos, true);

  NSWindow *window {[m_view window]};
  const NSRect &content {[window contentRectForFrameRect:[window frame]]};
  const CGFloat titleBarHeight {[window frame].size.height - content.size.height};
  [window setFrameTopLeftPoint:NSMakePoint(pos.x, pos.y + titleBarHeight)];
}

void CocoaWindow::setSize(const ImVec2 size)
{
  // most scripts expect y=0 to be the top of the window
  NSWindow *window {[m_view window]};
  [window setContentSize:NSMakeSize(size.x, size.y)];
  setPosition(m_viewport->Pos); // preserve y position from the top
}

void CocoaWindow::setFocus()
{
  NSWindow *window {[m_view window]};
  [window makeKeyAndOrderFront:nil];
  [window makeFirstResponder:m_inputView];
}

bool CocoaWindow::hasFocus() const
{
  // m_hwnd temporarily has focus between ShowWindow and WM_SETFOCUS
  // returning true here in this case is important, otherwise
  // Context::updateFocus would misdetect no active windows when it's called
  // from EventHandler::windowDidResignKey
  const HWND focus {GetFocus()};
  return focus == (__bridge HWND)m_inputView || focus == m_hwnd;
}

void CocoaWindow::setTitle(const char *title)
{
  SetWindowText(m_hwnd, title);
}

void CocoaWindow::setAlpha(const float alpha)
{
  NSWindow *window {[m_view window]};
  [window setAlphaValue:alpha];
}

void CocoaWindow::update()
{
  // restore keyboard focus to the input view when switching to our docker tab
  static bool no_wm_setfocus {atof(GetAppVersion()) < 6.53};
  if(no_wm_setfocus && GetFocus() == m_hwnd)
    setFocus();

  if(m_previousScale != m_viewport->DpiScale) {
    // resize macOS's GL objects when DPI changes (eg. moving to another screen)
    // NSViewFrameDidChangeNotification or WM_SIZE aren't sent
    m_renderer->setSize(m_viewport->Size);
    m_previousScale = m_viewport->DpiScale;
  }

  if(isDocked())
    return;

  const ImGuiViewportFlags diff {m_previousFlags ^ m_viewport->Flags};
  m_previousFlags = m_viewport->Flags;

  NSWindow *window {[m_view window]};

  if(diff & ImGuiViewportFlags_NoDecoration) {
    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration) {
      DetachWindowTopmostButton(m_hwnd, false);
      NSString *title {[window title]};
      [window setStyleMask:NSWindowStyleMaskBorderless]; // also clears the title
      [window setTitle:title];
    }
    else {
      [window setStyleMask:m_defaultStyleMask];
      AttachWindowTopmostButton(m_hwnd);
      // ask dear imgui to call setText again
      static_cast<ImGuiViewportP *>(m_viewport)->LastNameHash = 0;
    }
  }

  if(diff & ImGuiViewportFlags_TopMost) {
    // Using SWELL_SetWindowWantRaiseAmt instead of [NSWindow setLevel]
    // to automaticaly restore the level when the application regains focus.
    //
    // 0=normal/1=topmost as observed via a breakpoint when clicking on
    // AttachWindowTopmostButton's thumbstack button.
    const bool topmost {(m_viewport->Flags & ImGuiViewportFlags_TopMost) != 0};
    SWELL_SetWindowWantRaiseAmt(m_hwnd, topmost);
  }

  // disable shadows under the window when WindowFlags_NoBackground is set
  // (shadows wouldn't be updated along with the contents until next resize)
  if(diff & ImGuiViewportFlags_NoRendererClear) {
    const bool opaque
      {(m_viewport->Flags & ImGuiViewportFlags_NoRendererClear) != 0};
    [window setHasShadow:opaque];
  }

  if(diff & ImGuiViewportFlags_NoInputs) {
    const bool hitTestTransparent
      {(m_viewport->Flags & ImGuiViewportFlags_NoInputs) != 0};
    [window setIgnoresMouseEvents:hitTestTransparent];
  }
}

float CocoaWindow::scaleFactor() const
{
  return [[m_view window] backingScaleFactor];
}

void CocoaWindow::setIME(ImGuiPlatformImeData *data)
{
  if(!data->WantVisible) {
    NSTextInputContext *inputContext {[m_inputView inputContext]};
    [inputContext discardMarkedText];
    [inputContext invalidateCharacterCoordinates];
  }

  ImVec2 pos {data->InputPos};
  Platform::scalePosition(&pos, true);
  pos.y -= data->InputLineHeight;

  [m_inputView setImePosition:NSMakePoint(pos.x, pos.y)];
}

std::optional<LRESULT> CocoaWindow::handleMessage
  (const unsigned int msg, WPARAM wParam, LPARAM)
{
  switch(msg) {
  case WM_SETFOCUS: { // REAPER v6.53+
    // Redirect focus to the input view after m_view gets it.
    // WM_SETFOCUS is sent from becomeFirstResponder,
    // m_view will gain focus right after this returns.
    //
    // Both makeFirstResponder and making the window key are required
    // for receiving key events right away without a click
    const bool focusOnAppearing
      {!(m_viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)};
    if(focusOnAppearing || IsWindowVisible(m_hwnd)) {
      const HWND hwndCache {m_hwnd};
      dispatch_async(dispatch_get_main_queue(), ^{
        if(IsWindow(hwndCache)) // if we didn't get destroyed in the meantime
          setFocus();
      });
    }
    break;
  }
  case WM_SIZE:
    // SWELL does not send WM_MOVE when resizing moves the window origin
    m_viewport->PlatformRequestMove = true;
    break;
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
    // [NSWindow setIgnoresMouseEvents] does not include mouse wheel events
    if(m_viewport->Flags & ImGuiViewportFlags_NoInputs) {
      HWND target {Platform::windowFromPoint(Platform::getCursorPos())};
      if(target && target != m_hwnd) {
        [(__bridge NSView *)target scrollWheel:[NSApp currentEvent]];
        return 0;
      }
    }
    break;
  case WM_RBUTTONUP:
    // Prevent DefWindowProc from emitting WM_CONTEXTMENU
    // when focus is lost before the right mouse button was released
    return 0;
  }

  return std::nullopt;
}

int CocoaWindow::handleAccelerator(MSG *msg)
{
  [[m_view window] sendEvent:[NSApp currentEvent]];
  return Accel::EatKeystroke;
}

void Window::updateModifiers()
{
  // SWELL's GetAsyncKeyState omits Ctrl when right-click emulation is enabled

  struct Modifiers { int vkey; ImGuiKey key; };
  constexpr Modifiers modifiers[] {
    {kCGEventFlagMaskControl,   ImGuiMod_Ctrl },
    {kCGEventFlagMaskCommand,   ImGuiMod_Super},
    {kCGEventFlagMaskShift,     ImGuiMod_Shift},
    {kCGEventFlagMaskAlternate, ImGuiMod_Alt  },
  };

  const CGEventFlags state
    {CGEventSourceFlagsState(kCGEventSourceStateCombinedSessionState)};

  for(const auto &modifier : modifiers) {
    if(state & modifier.vkey)
      m_ctx->keyInput(modifier.key, true);
  }
}
