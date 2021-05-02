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

#include "context.hpp"
#include "inputview.hpp"
#include "opengl_renderer.hpp"

#include <WDL/wdltypes.h>
#include <reaper_plugin_functions.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

struct Window::Impl {
  NSView *view;
  InputView *inputView;
  ImDrawData *lastDrawData {};
  NSOpenGLContext *gl;
  OpenGLRenderer *renderer;
};

Window::Window(const WindowConfig &cfg, Context *ctx)
  : m_cfg { cfg }, m_ctx { ctx }, m_impl { std::make_unique<Impl>() }
{
  const RECT rect { cfg.initialRect() };
  const float x { static_cast<float>(rect.left) },
              y { static_cast<float>(rect.top)  },
              w { static_cast<float>(rect.right - rect.left) },
              h { static_cast<float>(rect.bottom - rect.top) };

  createSwellDialog();
  m_impl->view = (__bridge NSView *)m_hwnd.get(); // SWELL_hwndChild inherits from NSView

  if(m_cfg.dock & 1)
    setDock(m_cfg.dock);
  else {
    [[m_impl->view window] setFrameOrigin:NSPoint { x, y }];
    [[m_impl->view window] setContentSize:NSSize  { w, h }];
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

  ImGuiIO &io { ImGui::GetIO() };
  io.ConfigMacOSXBehaviors = false; // don't swap Cmd/Ctrl, SWELl already does it
  io.BackendPlatformName = "reaper_imgui_cocoa";
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
}

float Window::scaleFactor() const
{
  return [[m_impl->view window] backingScaleFactor];
}

std::optional<LRESULT> Window::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM)
{
  switch(msg) {
  case WM_ACTIVATE:
    // Only sent when not docked (InputView::resignFirstResponder otherwise)
    if(wParam == WA_INACTIVE)
      m_ctx->clearFocus();
    return 0;
  case WM_PAINT: // update size if it changed while we were docked & inactive
  case WM_SIZE:
    [m_impl->gl update];
    if(m_impl->lastDrawData)
      drawFrame(m_impl->lastDrawData);
    return 0;
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
