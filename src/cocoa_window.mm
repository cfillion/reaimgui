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

#include "window.hpp"

#include "cocoa_inputview.hpp"
#include "context.hpp"
#include "opengl_renderer.hpp"

#include <imgui/imgui_internal.h>
#include <objc/runtime.h>
#include <reaper_plugin_secrets.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

@interface SWELLWindowOverride : NSObject
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen;
@end

@implementation SWELLWindowOverride
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen
{
  // allow setPosition to move the window outside of the woring area
  return frameRect;
}
@end

class Subclass {
public:
  Subclass(Class parent, Class chind, const char *name);
  void activate(NSObject *);

private:
  Class m_subclass;
};

Subclass::Subclass(Class parent, Class child, const char *name)
{
  unsigned int methodCount {};
  Method *methods
    { class_copyMethodList(child, &methodCount) };

  m_subclass = objc_allocateClassPair(parent, name, 0);
  for(unsigned int i {}; i < methodCount; ++i) {
    Method method { methods[i] };
    class_addMethod(m_subclass, method_getName(method),
      method_getImplementation(method), method_getTypeEncoding(method));
  }
  objc_registerClassPair(m_subclass);

  free(methods);
}

void Subclass::activate(NSObject *object)
{
  object_setClass(object, m_subclass);
}

struct Window::Impl {
  static ImGuiViewport *nextViewportUnder
    (const NSPoint, const unsigned int windowNumber);

  NSView *view;
  InputView *inputView;
  NSOpenGLContext *gl;
  OpenGLRenderer *renderer;
  unsigned int defaultStyleMask, defaultLevel;
  ImGuiViewportFlags previousFlags;
};

Window::Window(ImGuiViewport *viewport)
  : Viewport { viewport }, m_impl { std::make_unique<Impl>() }
{
}

void *Window::create()
{
  createSwellDialog();
  m_impl->view = (__bridge NSView *)m_hwnd.get(); // SWELL_hwndChild inherits from NSView
  [m_impl->view setWantsBestResolutionOpenGLSurface:YES]; // retina
  m_impl->inputView = [[InputView alloc] initWithWindow:this];

  NSWindow *window { [m_impl->view window] };
  m_impl->defaultStyleMask = [window styleMask];
  m_impl->defaultLevel = [window level];
  m_impl->previousFlags = ~m_viewport->Flags; // mark all as modified
  // imgui calls update() before show(), it will apply the flags

  static Subclass swellWindowOverride
    { [window class], [SWELLWindowOverride class], "ReaImGui_SWELLWindow" };
  swellWindowOverride.activate(window);

  constexpr NSOpenGLPixelFormatAttribute attrs[] {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFADoubleBuffer,
    kCGLPFASupportsAutomaticGraphicsSwitching,
    0
  };

  NSOpenGLPixelFormat *fmt { [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] };
  m_impl->gl = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
  if(!m_impl->gl)
    throw backend_error { "failed to initialize OpenGL 3.2 core context" };

  [m_impl->gl makeCurrentContext];
  m_impl->renderer = new OpenGLRenderer;
  [NSOpenGLContext clearCurrentContext];

  return m_hwnd.get();
}

void Window::platformInstall()
{
  // Temprarily enable repeat character input
  // WARNING: this is application-wide!
  NSUserDefaults *defaults { [NSUserDefaults standardUserDefaults] };
  [defaults registerDefaults:@{@"ApplePressAndHoldEnabled":@NO}];

  ImGuiIO &io { ImGui::GetIO() };
  io.ConfigMacOSXBehaviors = false; // don't swap Cmd/Ctrl, SWELl already does it
  io.BackendPlatformName = "reaper_imgui_cocoa";
}

Window::~Window()
{
  [m_impl->gl makeCurrentContext];
  delete m_impl->renderer;
}

void Window::updateMonitors()
{
  // TODO
  // if(!g_monitorsChanged)
  //   return;

  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  pio.Monitors.resize(0); // recycle allocated memory (don't use clear here!)

  NSArray<NSScreen *> *screens { [NSScreen screens] };
  const CGFloat mainHeight { screens[0].frame.size.height };

  for(NSScreen *screen in screens) {
    const NSRect frame { [screen frame] }, workFrame { [screen visibleFrame] };
    ImGuiPlatformMonitor monitor;
    monitor.MainPos.x  = frame.origin.x;
    monitor.MainPos.y  = mainHeight - frame.origin.y - frame.size.height;
    monitor.MainSize.x = frame.size.width;
    monitor.MainSize.y = frame.size.height;
    monitor.WorkPos.x  = workFrame.origin.x;
    monitor.WorkPos.y  = mainHeight - workFrame.origin.y - workFrame.size.height;
    monitor.WorkSize.x = workFrame.size.width;
    monitor.WorkSize.y = workFrame.size.height;
    monitor.DpiScale   = [screen backingScaleFactor];

    pio.Monitors.push_back(monitor);
  }
}

void Window::show()
{
  commonShow();
  [m_impl->gl setView:m_impl->view];

  if(!(m_viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing))
    [[m_impl->view window] makeFirstResponder:m_impl->inputView];
}

void Window::setPosition(ImVec2 pos)
{
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  NSWindow *window { [m_impl->view window] };
  const NSRect &content { [window contentRectForFrameRect:[window frame]] };
  const CGFloat titleBarHeight { [window frame].size.height - content.size.height };
  pos.y = (pio.Monitors[0].MainSize.y - pos.y) + titleBarHeight;
  [window setFrameTopLeftPoint:NSMakePoint(pos.x, pos.y)];
}

void Window::setSize(const ImVec2 size)
{
  // most scripts expect y=0 to be the top of the window
  NSWindow *window { [m_impl->view window] };
  [window setContentSize:NSMakeSize(size.x, size.y)];
  setPosition(m_viewport->Pos); // preserve y position from the top
  [m_impl->gl update];
}

void Window::update()
{
  const ImGuiViewportFlags diff { m_impl->previousFlags ^ m_viewport->Flags };
  m_impl->previousFlags = m_viewport->Flags;

  NSWindow *window { [m_impl->view window] };

  if(diff & ImGuiViewportFlags_NoDecoration) {
    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration) {
      DetachWindowTopmostButton(m_hwnd.get(), false);
      [window setStyleMask:NSWindowStyleMaskBorderless];
    }
    else {
      [window setStyleMask:m_impl->defaultStyleMask];
      AttachWindowTopmostButton(m_hwnd.get());
      // ask dear imgui to call setText again
      static_cast<ImGuiViewportP *>(m_viewport)->LastNameHash = 0;
    }
  }

  if(diff & ImGuiViewportFlags_TopMost) {
    if(m_viewport->Flags & ImGuiViewportFlags_TopMost)
      [window setLevel:NSStatusWindowLevel];
    else
      [window setLevel:m_impl->defaultLevel]; // SWELL uses 1 by default
  }
}

void Window::render(void *)
{
  [m_impl->gl makeCurrentContext];
  m_impl->renderer->render(m_viewport);
  [m_impl->gl flushBuffer];
  [NSOpenGLContext clearCurrentContext];
}

float Window::scaleFactor() const
{
  return [[m_impl->view window] backingScaleFactor];
}

void Window::setImePosition(ImVec2 pos)
{
  pos.y = ImGui::GetPlatformIO().Monitors[0].MainSize.y - pos.y;
  pos.y -= ImGui::GetTextLineHeight();

  [m_impl->inputView setImePosition:NSMakePoint(pos.x, pos.y)];
}

void Window::uploadFontTex()
{
  [m_impl->gl makeCurrentContext];
  m_impl->renderer->uploadFontTex();
  [NSOpenGLContext clearCurrentContext];
}

void Window::translatePosition(POINT *pos, bool) const
{
  pos->y = ImGui::GetPlatformIO().Monitors[0].MainSize.y - pos->y;
}

std::optional<LRESULT> Window::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM)
{
  switch(msg) {
  case WM_PAINT: // update size if it changed while we were docked & inactive
  case WM_SIZE:
    [m_impl->gl update];
    break; // continue handling WM_SIZE in Window::proc
  }

  return std::nullopt;
}

int Window::translateAccel(MSG *msg, accelerator_register_t *accel)
{
  auto *self { static_cast<Window *>(accel->user) };

  if(self->m_hwnd.get() == msg->hwnd || IsChild(self->m_hwnd.get(), msg->hwnd)) {
    [[self->m_impl->view window] sendEvent:[NSApp currentEvent]];
    return Accel::EatKeystroke;
  }

  return Accel::NotOurWindow;
}

ImGuiViewport *Window::Impl::nextViewportUnder
  (const NSPoint pos, const unsigned int windowNumber)
{
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };

  for(int i { 1 }; i < pio.Viewports.Size; ++i) { // skip the main viewport
    ImGuiViewport *viewport { pio.Viewports[i] };
    NSView *superView { (__bridge NSView *)viewport->PlatformHandle };

    // PlatformHandle is NULL for inactive DockerHosts
    if(!superView || [[superView window] windowNumber] != windowNumber)
      continue;

    // NSView's hitTest takes a point in the coordinate system of the view's
    // superview, not of the view itself.
    NSPoint clientPos { [[superView window] convertScreenToBase:pos] };
    clientPos = [superView convertPoint:clientPos fromView:nil];

    NSView *inputView { [superView subviews][0] };
    if([inputView hitTest:clientPos])
     return viewport;
  }

  return nullptr;
}

ImGuiViewport *Window::viewportUnder(const POINT pos)
{
  const NSPoint point { NSMakePoint(pos.x, pos.y) };

  unsigned int number { 0 };
  ImGuiViewport *viewport;

  do {
    number = [NSWindow windowNumberAtPoint:point belowWindowWithWindowNumber:number];
    viewport = Impl::nextViewportUnder(point, number);
  } while(viewport && !!(viewport->Flags & ImGuiViewportFlags_NoInputs));

  return viewport;
}
