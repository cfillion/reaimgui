#include "window.hpp"

#include "context.hpp"
#include "opengl_renderer.hpp"

#include <epoxy/gl.h>
#include <gdk/gdk.h>
#include <stdexcept>

#include <swell/swell.h>

#define SWELL_TARGET_GDK
#define Window Xorg_Window
#include <swell/swell-internal.h> // access to hwnd->m_oswindow
#undef Window

struct Window::Impl {
  void initGl();
  void resizeFbTex();

  HwndPtr hwnd;
  Context *ctx;
  GdkWindow *window;
  GdkGLContext *gl;
  unsigned int tex, fbo;
  OpenGLRenderer *renderer;
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
  m_impl->window = m_impl->hwnd->m_oswindow;

  if(static_cast<void *>(m_impl->window) == hwnd)
    throw std::runtime_error { "headless SWELL is not supported" };

  // prevent invalidation (= displaying garbage) when moving another window over
  gdk_window_freeze_updates(m_impl->window);

  m_impl->initGl();

  glGenTextures(1, &m_impl->tex);
  m_impl->resizeFbTex(); // binds to the texture and sets its size

  glGenFramebuffers(1, &m_impl->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_impl->fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
    GL_TEXTURE_2D, m_impl->tex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  m_impl->renderer = new OpenGLRenderer;
  gdk_gl_context_clear_current();
}

void Window::Impl::initGl()
{
  GError *error {};
  gl = gdk_window_create_gl_context(window, &error);
  if(error) {
    const std::runtime_error ex { error->message };
    g_clear_error(&error);
    throw ex;
  }

  gdk_gl_context_set_required_version(gl,
    OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR);
  gdk_gl_context_set_forward_compatible(gl, true);

  gdk_gl_context_realize(gl, &error);
  if(error) {
    const std::runtime_error ex { error->message };
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
    throw std::runtime_error { msg };
  }
}

void Window::Impl::resizeFbTex()
{
  RECT rect;
  GetClientRect(hwnd.get(), &rect);
  const int width  { rect.right - rect.left },
            height { rect.bottom - rect.top };

  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

Window::~Window()
{
  gdk_gl_context_make_current(m_impl->gl);

  glDeleteFramebuffers(1, &m_impl->fbo);
  glDeleteTextures(1, &m_impl->tex);

  delete m_impl->renderer;

  // current GL context must be cleared before calling unref to avoid this bug:
  // https://gitlab.gnome.org/GNOME/gtk/-/issues/2562
  gdk_gl_context_clear_current();
  g_object_unref(m_impl->gl);
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
  gdk_gl_context_make_current(m_impl->gl);

  m_impl->renderer->draw(data, m_impl->ctx->clearColor());

  ImGuiIO &io { ImGui::GetIO() };
  GdkWindow *window { m_impl->window };
  const cairo_region_t *region { gdk_window_get_clip_region(window) };
  GdkDrawingContext *drawContext { gdk_window_begin_draw_frame(window, region) };
  cairo_t *cairoContext { gdk_drawing_context_get_cairo_context(drawContext) };
  gdk_cairo_draw_from_gl(cairoContext, window, m_impl->tex, GL_TEXTURE, 1,
    0, 0, io.DisplaySize.x, io.DisplaySize.y);
  gdk_window_end_draw_frame(m_impl->window, drawContext);

  gdk_gl_context_clear_current();
}

void Window::endFrame()
{
}

float Window::scaleFactor() const
{
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
    m_impl->resizeFbTex();
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
  }

  return false;
}
