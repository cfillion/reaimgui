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

#include <WDL/wdltypes.h>
#include <reaper_plugin_functions.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

struct Window::Impl {
  static void setImePosition(int x, int y);

  NSView *view;
  InputView *inputView;
  ImDrawData *lastDrawData {};
  NSOpenGLContext *gl;
  OpenGLRenderer *renderer;
};

Window::Window(Context *ctx)
  : m_ctx { ctx }, m_impl { std::make_unique<Impl>() }
{
  createSwellDialog();
  m_impl->view = (__bridge NSView *)m_hwnd.get(); // SWELL_hwndChild inherits from NSView

  const Settings &settings { ctx->settings() };
  if(settings.dock & 1)
    setDock(settings.dock);
  else {
    const RECT &rect { settings.initialRect() };
    NSWindow *window { [m_impl->view window] };
    [window setFrameOrigin:NSMakePoint(rect.left, rect.top)];
    [window setContentSize:NSMakeSize(rect.right - rect.left, rect.bottom - rect.top)];
    ShowWindow(m_hwnd.get(), SW_SHOW);
  }

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
    throw reascript_error { "failed to initialize OpenGL 3.2 core context" };

  [m_impl->view setWantsBestResolutionOpenGLSurface:YES]; // retina
  [m_impl->gl setView:m_impl->view];

  [m_impl->gl makeCurrentContext];
  m_impl->renderer = new OpenGLRenderer;
  [m_impl->gl flushBuffer]; // avoid a quick flash of undefined pixels
  [NSOpenGLContext clearCurrentContext];

  m_impl->inputView = [[InputView alloc] initWithContext:ctx parent:m_impl->view];

  // Temprarily enable repeat character input
  // WARNING: this is application-wide!
  NSUserDefaults *defaults { [NSUserDefaults standardUserDefaults] };
  [defaults registerDefaults:@{@"ApplePressAndHoldEnabled":@NO}];

  ImGuiIO &io { ctx->IO() };
  io.ConfigMacOSXBehaviors = false; // don't swap Cmd/Ctrl, SWELl already does it
  io.BackendPlatformName = "reaper_imgui_cocoa";
  io.ImeSetInputScreenPosFn = &Impl::setImePosition;
}

Window::~Window()
{
  [m_impl->gl makeCurrentContext];
  delete m_impl->renderer;
}

void Window::beginFrame()
{
  m_impl->lastDrawData = nullptr;
}

void Window::drawFrame(ImDrawData *drawData)
{
  m_impl->lastDrawData = drawData;
  [m_impl->gl makeCurrentContext];
  m_impl->renderer->draw(drawData, m_ctx->clearColor());
  [m_impl->gl flushBuffer];
  [NSOpenGLContext clearCurrentContext];
}

void Window::endFrame()
{
  // WM_ACTIVATE (wParam = WA_INACTIVE) is not fired when docked.
  // InputView::resignFirstResponder handles change of focus within the docker's
  // window, and isKeyWindow below handles the docker window itself losing focus.
  if(![[m_impl->view window] isKeyWindow])
    m_ctx->clearFocus();
}

float Window::scaleFactor() const
{
  return [[m_impl->view window] backingScaleFactor];
}

std::optional<LRESULT> Window::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM)
{
  switch(msg) {
  case WM_PAINT: // update size if it changed while we were docked & inactive
  case WM_SIZE:
    [m_impl->gl update];
    if(m_impl->lastDrawData)
      drawFrame(m_impl->lastDrawData);
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

void Window::Impl::setImePosition(const int x, const int y)
{
  InputView *inputView { Context::current()->window()->m_impl->inputView };
  [inputView setImePosition:NSMakePoint(x, y + ImGui::GetTextLineHeight())];
}
