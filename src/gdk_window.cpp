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
#include "opengl_renderer.hpp"

#include <cassert>
#include <epoxy/gl.h>
#include <gdk/gdk.h>

#include <swell/swell.h>
#include <reaper_plugin_functions.h>

#define _LICE_H // prevent swell-internal.h from including lice.h
#define SWELL_LICE_GDI
#define SWELL_TARGET_GDK
#define Window Xorg_Window
#include <swell/swell-internal.h> // access to hwnd->m_oswindow
#undef Window

struct LICEDeleter { void operator()(LICE_IBitmap *bm) { LICE__Destroy(bm); } };
using LICEPtr = std::unique_ptr<LICE_IBitmap, LICEDeleter>;

struct Window::Impl {
  void initGl();
  void resizeTextures();
  void teardownGl();
  void checkDockChanged();
  void findOSWindow();
  bool isDocked() const { return hwnd.get() != windowOwner; }
  void liceBlit();

  HwndPtr hwnd;
  Context *ctx;
  GdkWindow *window;
  HWND windowOwner;
  GdkGLContext *gl;
  unsigned int tex, fbo;
  OpenGLRenderer *renderer;
  LICEPtr pixels; // used when docked
};

Window::Window(const char *title, RECT rect, Context *ctx)
  : m_impl { std::make_unique<Impl>() }
{
  HWND hwnd { createSwellDialog(title) };
  hwnd->m_position = rect;
  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
  ShowWindow(hwnd, SW_SHOW);

  m_impl->ctx = ctx;
  m_impl->hwnd = HwndPtr { hwnd };
  m_impl->findOSWindow();

  if(static_cast<void *>(m_impl->window) == hwnd)
    throw reascript_error { "headless SWELL is not supported" };

  // prevent invalidation (= displaying garbage) when moving another window over
  gdk_window_freeze_updates(m_impl->window);
  m_impl->pixels = LICEPtr { LICE_CreateBitmap(0, 0, 0) };

  m_impl->initGl();

  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_gdk";
}

Window::~Window()
{
  m_impl->teardownGl();
}

void Window::Impl::initGl()
{
  GError *error {};
  gl = gdk_window_create_gl_context(window, &error);
  if(error) {
    const reascript_error ex { error->message };
    g_clear_error(&error);
    throw ex;
  }

  gdk_gl_context_set_required_version(gl,
    OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR);
  gdk_gl_context_set_forward_compatible(gl, true);

  gdk_gl_context_realize(gl, &error);
  if(error) {
    const reascript_error ex { error->message };
    g_clear_error(&error);
    g_object_unref(gl);
    throw ex;
  }

  gdk_gl_context_make_current(gl);

  int major, minor;
  gdk_gl_context_get_version(gl, &major, &minor);
  if(major < OpenGLRenderer::MIN_MAJOR ||
      (major == OpenGLRenderer::MIN_MAJOR && minor < OpenGLRenderer::MIN_MINOR)) {
    g_object_unref(gl);

    char msg[1024];
    snprintf(msg, sizeof(msg), "OpenGL v%d.%d or newer required, got v%d.%d",
      OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR, major, minor);
    throw reascript_error { msg };
  }

  glGenTextures(1, &tex);
  resizeTextures(); // binds to the texture and sets its size

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  renderer = new OpenGLRenderer;
  gdk_gl_context_clear_current();
}

void Window::Impl::resizeTextures()
{
  RECT rect;
  GetClientRect(hwnd.get(), &rect);
  const int width  { rect.right - rect.left },
            height { rect.bottom - rect.top };

  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  LICE__resize(pixels.get(), width, height);
  glPixelStorei(GL_PACK_ROW_LENGTH, LICE__GetRowSpan(pixels.get()));
}

void Window::Impl::findOSWindow()
{
  windowOwner = hwnd.get();
  while(windowOwner && !windowOwner->m_oswindow) {
    HWND parent { GetParent(windowOwner) };
    windowOwner =  IsWindowVisible(parent) ? parent : nullptr;
  }

  window = windowOwner ? windowOwner->m_oswindow : nullptr;
}

void Window::Impl::checkDockChanged()
{
  GdkWindow *prevWindow { window };
  findOSWindow();

  if(window && prevWindow != window) {
    teardownGl();
    initGl();
  }
}

void Window::Impl::teardownGl()
{
  gdk_gl_context_make_current(gl);

  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &tex);

  delete renderer;

  // current GL context must be cleared before calling unref to avoid this bug:
  // https://gitlab.gnome.org/GNOME/gtk/-/issues/2562
  gdk_gl_context_clear_current();
  g_object_unref(gl);
}

HWND Window::nativeHandle() const
{
  return m_impl->hwnd.get();
}

void Window::beginFrame()
{
}

void Window::drawFrame(ImDrawData *data)
{
  m_impl->checkDockChanged();

  if(!m_impl->window) // we're in a hidden docker
    return;

  gdk_gl_context_make_current(m_impl->gl);

  m_impl->renderer->draw(data, m_impl->ctx->clearColor());

  if(m_impl->isDocked()) {
    // REAPER is also drawing to the same GdkWindow so we must share it.
    // Switch to slower render path, copying pixels into a LICE bitmap.
    glReadPixels(0, 0,
      LICE__GetWidth(m_impl->pixels.get()), LICE__GetHeight(m_impl->pixels.get()),
      GL_BGRA, GL_UNSIGNED_BYTE, LICE__GetBits(m_impl->pixels.get()));
    InvalidateRect(m_impl->hwnd.get(), nullptr, false);
    gdk_gl_context_clear_current();
    return;
  }

  ImGuiIO &io { ImGui::GetIO() };
  const cairo_region_t *region { gdk_window_get_clip_region(m_impl->window) };
  GdkDrawingContext *drawContext { gdk_window_begin_draw_frame(m_impl->window, region) };
  cairo_t *cairoContext { gdk_drawing_context_get_cairo_context(drawContext) };
  gdk_cairo_draw_from_gl(cairoContext, m_impl->window, m_impl->tex,
    GL_TEXTURE, 1, 0, 0, io.DisplaySize.x, io.DisplaySize.y);
  gdk_window_end_draw_frame(m_impl->window, drawContext);

  gdk_gl_context_clear_current();
}

void Window::Impl::liceBlit()
{
  constexpr int LICE_BLIT_MODE_COPY { 0 };

  PAINTSTRUCT ps;
  if(!BeginPaint(hwnd.get(), &ps))
    return;

  const int width  { LICE__GetWidth(pixels.get())  },
            height { LICE__GetHeight(pixels.get()) };

  // flip vertically
  LICE_ScaledBlit(ps.hdc->surface, pixels.get(),
    ps.hdc->surface_offs.x, ps.hdc->surface_offs.y, width, height,
    0, height, width, -height, 1.0f, LICE_BLIT_MODE_COPY);

  EndPaint(hwnd.get(), &ps);
}

void Window::endFrame()
{
}

float Window::scaleFactor() const
{
  if(!m_impl->window)
    return 1.0f;

  return gdk_window_get_scale_factor(m_impl->window);
}

static unsigned int unmangleSwellChar(WPARAM wParam, LPARAM lParam)
{
  // Trying to guess the character to print from SWELL's event data.
  // Matching the behavior of OnEditKeyDown from swell-wnd-generic.cpp

  if(lParam & (FCONTROL | FALT | FLWIN) || wParam < 32)
    return 0;

  if(wParam >= 'A' && wParam <= 'Z') {
    // This does not support caps lock.
    if(!(lParam & FSHIFT))
      wParam += 'a' - 'A';
  }
  else if(wParam >= VK_NUMPAD0 && wParam <= VK_DIVIDE) {
    if(wParam <= VK_NUMPAD9)
      wParam += '0' - VK_NUMPAD0;
    else
      wParam += '*' - VK_MULTIPLY;
  }
  else if(lParam & FVIRTKEY && (wParam < '0' || wParam > '9') && wParam != VK_SPACE)
    return 0; // virtual keys that aren't letters or numbers aren't printable

  return wParam;
}

bool Window::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
  case WM_SIZE:
    gdk_gl_context_make_current(m_impl->gl);
    m_impl->resizeTextures();
    gdk_gl_context_clear_current();
    return true;
  case WM_KEYDOWN:
    // No access to the orignal GDK key event, unfortunately.
    if(unsigned int c { unmangleSwellChar(wParam, lParam) })
      m_impl->ctx->charInput(c);
    if(wParam < 256)
      m_impl->ctx->keyInput(wParam, true);
    return true;
  case WM_KEYUP:
    if(wParam < 256)
      m_impl->ctx->keyInput(wParam, false);
    return true;
  case WM_PAINT:
    if(m_impl->isDocked())
      m_impl->liceBlit();
    return true;
  }

  return false;
}
