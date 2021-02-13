#include "backend.hpp"

#include "context.hpp"
#include "opengl_renderer.hpp"

#include <epoxy/gl.h>
#include <gdk/gdk.h>
#include <stdexcept>

#define SWELL_TARGET_GDK
#include <swell/swell-internal.h> // access to hwnd->m_oswindow

class GdkBackend : public Backend {
public:
  GdkBackend(Context *);
  ~GdkBackend() override;

  void drawFrame(ImDrawData *) override;
  float deltaTime() override;
  float scaleFactor() const override;
  bool handleMessage(unsigned int msg, WPARAM, LPARAM) override;
  void translateAccel(MSG *) override;

private:
  void initGl();
  void resizeFbTex();

  Context *m_ctx;
  GdkWindow *m_window;
  int64_t m_lastFrame;
  GdkGLContext *m_gl;
  unsigned int m_tex, m_fbo;
  OpenGLRenderer *m_renderer;
};

std::unique_ptr<Backend> Backend::create(Context *ctx)
{
  return std::make_unique<GdkBackend>(ctx);
}

GdkBackend::GdkBackend(Context *ctx)
  : m_ctx { ctx }, m_window { ctx->handle()->m_oswindow }, m_lastFrame {}
{
  gdk_window_set_events(m_window, GdkEventMask(
    gdk_window_get_events(m_window) |
    GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK
  ));

  initGl();

  glGenTextures(1, &m_tex);
  resizeFbTex(); // binds to the texture and sets its size

  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  m_renderer = new OpenGLRenderer;
  gdk_gl_context_clear_current();
}

void GdkBackend::initGl()
{
  GError *error {};
  m_gl = gdk_window_create_gl_context(m_window, &error);
  if(error) {
    const std::runtime_error ex { error->message };
    g_clear_error(&error);
    throw ex;
  }

  gdk_gl_context_set_required_version(m_gl, OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR);
  gdk_gl_context_set_forward_compatible(m_gl, true);

  gdk_gl_context_realize(m_gl, &error);
  if(error) {
    const std::runtime_error ex { error->message };
    g_clear_error(&error);
    g_object_unref(m_gl);
    throw ex;
  }

  gdk_gl_context_make_current(m_gl);

  int major, minor;
  gdk_gl_context_get_version(m_gl, &major, &minor);
  if(major < OpenGLRenderer::MIN_MAJOR ||
      (major == OpenGLRenderer::MIN_MAJOR && minor < OpenGLRenderer::MIN_MINOR)) {
    g_object_unref(m_gl);

    char msg[1024];
    snprintf(msg, sizeof(msg), "OpenGL v%d.%d or newer required, got v%d.%d",
      OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR, major, minor);
    throw std::runtime_error { msg };
  }
}

GdkBackend::~GdkBackend()
{
  gdk_gl_context_make_current(m_gl);

  glDeleteFramebuffers(1, &m_fbo);
  glDeleteTextures(1, &m_tex);

  delete m_renderer;

  // current GL context must be cleared before calling unref to avoid this bug:
  // https://gitlab.gnome.org/GNOME/gtk/-/issues/2562
  gdk_gl_context_clear_current();
  g_object_unref(m_gl);
}

void GdkBackend::drawFrame(ImDrawData *data)
{
  gdk_gl_context_make_current(m_gl);

  m_renderer->draw(data, m_ctx->clearColor());

  ImGuiIO &io { ImGui::GetIO() };
  const cairo_region_t *region { gdk_window_get_clip_region(m_window) };
  GdkDrawingContext *drawContext { gdk_window_begin_draw_frame(m_window, region) };
  cairo_t *cairoContext { gdk_drawing_context_get_cairo_context(drawContext) };
  gdk_cairo_draw_from_gl(cairoContext, m_window, m_tex, GL_TEXTURE, 1,
    0, 0, io.DisplaySize.x, io.DisplaySize.y);
  gdk_window_end_draw_frame(m_window, drawContext);

  gdk_gl_context_clear_current();
}

void GdkBackend::resizeFbTex()
{
  RECT rect;
  GetClientRect(m_ctx->handle(), &rect);
  const int width  { rect.right - rect.left },
            height { rect.bottom - rect.top };

  glBindTexture(GL_TEXTURE_2D, m_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

float GdkBackend::deltaTime()
{
  const int64_t now { g_get_monotonic_time() }; // microseconds
  if(!m_lastFrame)
    m_lastFrame = now;
  const float delta { (now - m_lastFrame) / 1'000'000.f };
  m_lastFrame = now;
  return delta;
}

float GdkBackend::scaleFactor() const
{
  return gdk_window_get_scale_factor(m_window);
}

bool GdkBackend::handleMessage(const unsigned int msg, WPARAM, LPARAM)
{
  switch(msg) {
  case WM_SIZE:
    gdk_gl_context_make_current(m_gl);
    resizeFbTex();
    gdk_gl_context_clear_current();
    return true;
  }

  return false;
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

void GdkBackend::translateAccel(MSG *msg)
{
  // No access to the orignal GDK key event, unfortunately.
  switch(msg->message) {
  case WM_KEYDOWN:
    if(unsigned int c { unmangleSwellChar(msg->wParam, msg->lParam) })
      m_ctx->charInput(c);
    if(msg->wParam < 256)
      m_ctx->keyInput(msg->wParam, true);
    break;
  case WM_KEYUP:
    if(msg->wParam < 256)
      m_ctx->keyInput(msg->wParam, false);
    break;
  }
}
