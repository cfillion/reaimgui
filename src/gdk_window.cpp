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
#define Font Xorg_Font
#define Window Xorg_Window
#include <swell/swell-internal.h> // access to hwnd->m_oswindow
#undef Window

struct LICEDeleter { void operator()(LICE_IBitmap *bm) { LICE__Destroy(bm); } };

struct Window::Impl {
  void initGl();
  void resizeTextures();
  void teardownGl();
  void checkOSWindowChanged();
  void findOSWindow();
  bool isDocked() const { return windowOwner && hwnd != windowOwner; }
  void liceBlit();

  HWND hwnd, windowOwner;
  GdkWindow *window;
  GdkGLContext *gl;
  unsigned int tex, fbo;
  OpenGLRenderer *renderer;
  std::unique_ptr<LICE_IBitmap, LICEDeleter> pixels; // used when docked
  ImGuiViewportFlags previousFlags;
  GdkWMDecoration defaultDecorations;
};

Window::Window(ImGuiViewport *viewport, Context *ctx)
  : m_viewport { viewport }, m_ctx { ctx }, m_impl { std::make_unique<Impl>() }
{
  createSwellDialog();
  SetWindowLongPtr(m_hwnd.get(), GWL_EXSTYLE, WS_EX_ACCEPTFILES);

  // WS_CHILD does gdk_window_set_override_redirect(true)
  // SWELL only supports setting WS_CHILD before ShowWindow
  if(m_viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
    SetWindowLongPtr(m_hwnd.get(), GWL_STYLE, WS_CHILD);

  m_impl->window = nullptr;
  m_impl->gl = nullptr;
  m_impl->previousFlags = ~viewport->Flags; // update will be called before show
}

Window::~Window()
{
  if(m_impl->gl)
    m_impl->teardownGl();
}

void Window::platformInstall()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_gdk";
}

void Window::Impl::initGl()
{
  GError *error {};
  gl = gdk_window_create_gl_context(window, &error);
  if(error) {
    const backend_error ex { error->message };
    g_clear_error(&error);
    throw ex;
  }

  gdk_gl_context_set_required_version(gl,
    OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR);
  gdk_gl_context_set_forward_compatible(gl, true);

  gdk_gl_context_realize(gl, &error);
  if(error) {
    const backend_error ex { error->message };
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
    throw backend_error { msg };
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
  GetClientRect(hwnd, &rect);
  const int width  { rect.right - rect.left },
            height { rect.bottom - rect.top };

  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
    0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

  if(pixels) {
    LICE__resize(pixels.get(), width, height);
    glPixelStorei(GL_PACK_ROW_LENGTH, LICE__GetRowSpan(pixels.get()));
  }
}

void Window::Impl::findOSWindow()
{
  windowOwner = hwnd;
  while(windowOwner && !windowOwner->m_oswindow) {
    HWND parent { GetParent(windowOwner) };
    windowOwner = IsWindowVisible(parent) ? parent : nullptr;
  }

  window = windowOwner ? windowOwner->m_oswindow : nullptr;

  if(static_cast<void *>(window) == hwnd)
    window = nullptr; // headless SWELL
}

void Window::Impl::checkOSWindowChanged()
{
  GdkWindow *prevWindow { window };
  findOSWindow();

  // if(!pixels && isDocked())
  //   pixels.reset(LICE_CreateBitmap(0, 0, 0));

  if(window && prevWindow != window) {
    teardownGl();
    initGl();

    // Context *ctx { static_cast<Context *>(GetProp(hwnd, CLASS_NAME)) };
    // ctx->invalidateTextures(); TODO
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

void Window::updateMonitors()
{
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  pio.Monitors.resize(0); // recycle allocated memory (don't use clear here!)

  GdkDisplay *display { gdk_display_get_default() };

  const int count { gdk_display_get_n_monitors(display) };
  for(int i {}; i < count; ++i) {
    GdkMonitor *monitor { gdk_display_get_monitor(display, i) };

    GdkRectangle geometry, workArea;
    gdk_monitor_get_geometry(monitor, &geometry);
    gdk_monitor_get_workarea(monitor, &workArea);

    ImGuiPlatformMonitor imguiMonitor;
    imguiMonitor.MainPos.x  = geometry.x;
    imguiMonitor.MainPos.y  = geometry.y;
    imguiMonitor.MainSize.x = geometry.width;
    imguiMonitor.MainSize.y = geometry.height;
    imguiMonitor.WorkPos.x  = workArea.x;
    imguiMonitor.WorkPos.y  = workArea.y;
    imguiMonitor.WorkSize.x = workArea.width;
    imguiMonitor.WorkSize.y = workArea.height;
    imguiMonitor.DpiScale   = gdk_monitor_get_scale_factor(monitor);

    if(gdk_monitor_is_primary(monitor))
      pio.Monitors.push_front(imguiMonitor);
    else
      pio.Monitors.push_back(imguiMonitor);
  }
}

void Window::show()
{
  commonShow();

  m_impl->findOSWindow();
  m_impl->initGl();

  // prevent invalidation (= displaying garbage) when moving another window over
  // if(!(settings.dock & 1)) TODO
  //   // LICE bitmap must be null when docking to avoid drawing a garbage frame
  //   m_impl->pixels.reset(LICE_CreateBitmap(0, 0, 0));
  gdk_window_freeze_updates(m_impl->window); // TODO: undocked only
}

void Window::setPosition(const ImVec2 pos)
{
  SetWindowPos(m_hwnd.get(), nullptr, pos.x, pos.y, 0, 0,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void Window::setSize(const ImVec2 size)
{
  SetWindowPos(m_hwnd.get(), nullptr, 0, 0,
    size.x * scaleFactor(), size.y * scaleFactor(),
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void Window::update()
{
  const ImGuiViewportFlags diff { m_impl->previousFlags ^ m_viewport->Flags };
  m_impl->previousFlags = m_viewport->Flags;

  if(GetFocus() == m_hwnd.get())
    SWELL_SetClassName(m_hwnd.get(), getSwellClass());

  if(diff & ImGuiViewportFlags_NoDecoration) {
    auto style { GetWindowLongPtr(m_hwnd.get(), GWL_STYLE) };

    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration)
      style &= ~WS_CAPTION;
    else
      style |= WS_CAPTION;

    SetWindowLongPtr(m_hwnd.get(), GWL_STYLE, style);

    // SetWindowLongPtr hides the window
    // it sets an internal "need show" flag that's used by SetWindowPos
    if(m_impl->window) {
      setPosition(m_viewport->Pos);
      setSize(m_viewport->Size);
    }
  }

  if(diff & ImGuiViewportFlags_TopMost) {
    if(m_viewport->Flags & ImGuiViewportFlags_TopMost)
      SWELL_SetWindowLevel(m_hwnd.get(), 1);
    else
      SWELL_SetWindowLevel(m_hwnd.get(), 0);
  }

  if(m_impl->window) // after show()
    m_impl->checkOSWindowChanged();
}

void Window::render(void *)
{
  gdk_gl_context_make_current(m_impl->gl);

  const bool softwareBlit { m_impl->isDocked() };
  m_impl->renderer->render(m_viewport, softwareBlit);

  // if(softwareBlit) {
  //   // REAPER is also drawing to the same GdkWindow so we must share it.
  //   // Switch to slower render path, copying pixels into a LICE bitmap.
  //   glReadPixels(0, 0,
  //     LICE__GetWidth(m_impl->pixels.get()), LICE__GetHeight(m_impl->pixels.get()),
  //     GL_BGRA, GL_UNSIGNED_BYTE, LICE__GetBits(m_impl->pixels.get()));
  //   InvalidateRect(m_hwnd.get(), nullptr, false);
  //   gdk_gl_context_clear_current();
  //   return;
  // }

  ImDrawData *drawData { m_viewport->DrawData };

  const cairo_region_t *region { gdk_window_get_clip_region(m_impl->window) };
  GdkDrawingContext *drawContext { gdk_window_begin_draw_frame(m_impl->window, region) };
  cairo_t *cairoContext { gdk_drawing_context_get_cairo_context(drawContext) };
  gdk_cairo_draw_from_gl(cairoContext, m_impl->window,
    m_impl->tex, GL_TEXTURE, 1, 0, 0,
    drawData->DisplaySize.x * drawData->FramebufferScale.x,
    drawData->DisplaySize.y * drawData->FramebufferScale.y);
  gdk_window_end_draw_frame(m_impl->window, drawContext);

  // required for making the window visible on GNOME
  gdk_window_thaw_updates(m_impl->window); // schedules an update
  gdk_window_freeze_updates(m_impl->window);

  gdk_gl_context_clear_current();
}

void Window::Impl::liceBlit()
{
  constexpr int LICE_BLIT_MODE_COPY      { 0x00000 },
                LICE_BLIT_IGNORE_SCALING { 0x20000 };

  PAINTSTRUCT ps;
  if(!BeginPaint(hwnd, &ps))
    return;

  const int width  { LICE__GetWidth(pixels.get())  },
            height { LICE__GetHeight(pixels.get()) };

  LICE_Blit(ps.hdc->surface, pixels.get(),
    ps.hdc->surface_offs.x, ps.hdc->surface_offs.y,
    0, 0, width, height, 1.0f, LICE_BLIT_MODE_COPY | LICE_BLIT_IGNORE_SCALING);

  EndPaint(hwnd, &ps);
}

float Window::scaleFactor() const
{
  static float scale { SWELL_GetScaling256() / 256.f };
  return scale;
}

void Window::setImePosition(ImVec2)
{
}

void Window::uploadFontTex()
{
  gdk_gl_context_make_current(m_impl->gl);
  m_impl->renderer->uploadFontTex();
  gdk_gl_context_clear_current();
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

std::optional<LRESULT> Window::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
  case WM_CREATE:
    m_impl->hwnd = m_hwnd.get();
    return 0;
  case WM_DROPFILES: {
    HDROP drop { reinterpret_cast<HDROP>(wParam) };
    m_ctx->beginDrag(drop);
    DragFinish(drop);
    m_ctx->endDrag(true);
    return 0;
  }
  case WM_SIZE:
    if(m_impl->gl) {
      gdk_gl_context_make_current(m_impl->gl);
      m_impl->resizeTextures();
      gdk_gl_context_clear_current();
    }
    break; // continue handling in Window::proc
  case WM_KEYDOWN:
    // No access to the orignal GDK key event, unfortunately.
    if(unsigned int c { unmangleSwellChar(wParam, lParam) })
      m_ctx->charInput(c);
    if(wParam < 256)
      m_ctx->keyInput(wParam, true);
    return 0;
  case WM_KEYUP:
    if(wParam < 256)
      m_ctx->keyInput(wParam, false);
    return 0;
  case WM_PAINT:
    if(m_impl->pixels)
      m_impl->liceBlit();
    return 0;
  }

  return std::nullopt;
}
