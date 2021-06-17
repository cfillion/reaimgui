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

#include "cocoa_window.hpp"

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

CocoaWindow::CocoaWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
  : Window { viewport, dockerHost }
{
}

void *CocoaWindow::create()
{
  createSwellDialog();
  m_view = (__bridge NSView *)m_hwnd.get(); // SWELL_hwndChild inherits from NSView
  [m_view setWantsBestResolutionOpenGLSurface:YES]; // retina
  m_inputView = [[InputView alloc] initWithWindow:this];

  NSWindow *window { [m_view window] };
  m_defaultStyleMask = [window styleMask];
  m_defaultLevel = [window level];
  m_previousFlags = ~m_viewport->Flags; // mark all as modified
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
  m_gl = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
  if(!m_gl)
    throw backend_error { "failed to initialize OpenGL 3.2 core context" };

  [m_gl makeCurrentContext];
  m_renderer = new OpenGLRenderer;
  [NSOpenGLContext clearCurrentContext];

  return m_hwnd.get();
}

CocoaWindow::~CocoaWindow()
{
  [m_gl makeCurrentContext];
  delete m_renderer;
  [NSOpenGLContext clearCurrentContext];
}

void CocoaWindow::show()
{
  Window::show();
  [m_gl setView:m_view];

  if(!(m_viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing))
    [[m_view window] makeFirstResponder:m_inputView];
}

void CocoaWindow::setPosition(ImVec2 pos)
{
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  NSWindow *window { [m_view window] };
  const NSRect &content { [window contentRectForFrameRect:[window frame]] };
  const CGFloat titleBarHeight { [window frame].size.height - content.size.height };
  pos.y = (pio.Monitors[0].MainSize.y - pos.y) + titleBarHeight;
  [window setFrameTopLeftPoint:NSMakePoint(pos.x, pos.y)];
}

void CocoaWindow::setSize(const ImVec2 size)
{
  // most scripts expect y=0 to be the top of the window
  NSWindow *window { [m_view window] };
  [window setContentSize:NSMakeSize(size.x, size.y)];
  setPosition(m_viewport->Pos); // preserve y position from the top
  [m_gl update];
}

void CocoaWindow::setTitle(const char *title)
{
  SetWindowText(m_hwnd.get(), title);
}

void CocoaWindow::update()
{
  const ImGuiViewportFlags diff { m_previousFlags ^ m_viewport->Flags };
  m_previousFlags = m_viewport->Flags;

  NSWindow *window { [m_view window] };

  if(diff & ImGuiViewportFlags_NoDecoration) {
    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration) {
      DetachWindowTopmostButton(m_hwnd.get(), false);
      [window setStyleMask:NSWindowStyleMaskBorderless];
    }
    else {
      [window setStyleMask:m_defaultStyleMask];
      AttachWindowTopmostButton(m_hwnd.get());
      // ask dear imgui to call setText again
      static_cast<ImGuiViewportP *>(m_viewport)->LastNameHash = 0;
    }
  }

  if(diff & ImGuiViewportFlags_TopMost) {
    if(m_viewport->Flags & ImGuiViewportFlags_TopMost)
      [window setLevel:NSStatusWindowLevel];
    else
      [window setLevel:m_defaultLevel]; // SWELL uses 1 by default
  }
}

void CocoaWindow::render(void *)
{
  [m_gl makeCurrentContext];
  m_renderer->render(m_viewport);
  [m_gl flushBuffer];
  [NSOpenGLContext clearCurrentContext];
}

float CocoaWindow::scaleFactor() const
{
  return [[m_view window] backingScaleFactor];
}

void CocoaWindow::setImePosition(ImVec2 pos)
{
  pos.y = ImGui::GetPlatformIO().Monitors[0].MainSize.y - pos.y;
  pos.y -= ImGui::GetTextLineHeight();

  [m_inputView setImePosition:NSMakePoint(pos.x, pos.y)];
}

void CocoaWindow::uploadFontTex()
{
  [m_gl makeCurrentContext];
  m_renderer->uploadFontTex();
  [NSOpenGLContext clearCurrentContext];
}

std::optional<LRESULT> CocoaWindow::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM)
{
  switch(msg) {
  case WM_PAINT: // update size if it changed while we were docked & inactive
  case WM_SIZE:
    [m_gl update];
    break; // continue handling WM_SIZE in CocoaWindow::proc
  }

  return std::nullopt;
}

int Window::translateAccel(MSG *msg, accelerator_register_t *accel) // TODO
{
  auto *self { static_cast<CocoaWindow *>(accel->user) };
  HWND hwnd { self->nativeHandle() };

  if(hwnd == msg->hwnd || IsChild(hwnd, msg->hwnd)) {
    [[(__bridge NSView *)hwnd window] sendEvent:[NSApp currentEvent]];
    return Accel::EatKeystroke;
  }

  return Accel::NotOurWindow;
}
