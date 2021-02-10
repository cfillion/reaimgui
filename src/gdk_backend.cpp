#include "backend.hpp"

#include "context.hpp"

#include <epoxy/gl.h>
#include <gdk/gdk.h>
#include <stdexcept>

#define SWELL_TARGET_GDK
#include <swell/swell-internal.h> // access to hwnd->m_oswindow

#include <imgui/backends/imgui_impl_opengl3.h>

class GdkBackend : public Backend {
public:
  GdkBackend(Context *);
  ~GdkBackend() override;

  void beginFrame() override;
  void enterFrame() override;
  void endFrame(ImDrawData *) override;
  float deltaTime() override;
  float scaleFactor() const override;
  void translateAccel(MSG *) override;

private:
  Context *m_ctx;
  GdkWindow *m_window;
  gint64 m_lastFrame;
  ImDrawData *m_drawData;

  GdkGLContext *m_gl;
  GdkDrawingContext *m_drawContext;

  GLuint m_tex, m_fbo;
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

  GError *error {};
  m_gl = gdk_window_create_gl_context(m_window, &error);
  if(error) {
    const std::runtime_error ex { error->message };
    g_clear_error(&error);
    throw ex;
  }

  gdk_gl_context_set_required_version(m_gl, 3, 2);
  gdk_gl_context_set_forward_compatible(m_gl, true);
  gdk_gl_context_set_use_es(m_gl, 0);

  gdk_gl_context_realize(m_gl, &error);
  if(error) {
    const std::runtime_error ex { error->message };
    g_clear_error(&error);
    g_object_unref(&m_gl);
    throw ex;
  }

  gdk_gl_context_make_current(m_gl);

  glGenTextures(1, &m_tex);
  glBindTexture(GL_TEXTURE_2D, m_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, // TODO: optimize resizes
    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  ImGui_ImplOpenGL3_Init();
}

GdkBackend::~GdkBackend()
{
  gdk_gl_context_make_current(m_gl);
  glDeleteFramebuffers(1, &m_fbo);
  glDeleteTextures(1, &m_tex);

  ImGui_ImplOpenGL3_Shutdown();

  // current GL context must be cleared before calling unref to avoid this bug:
  // https://gitlab.gnome.org/GNOME/gtk/-/issues/2562
  gdk_gl_context_clear_current();
  g_object_unref(m_gl);
}

void GdkBackend::beginFrame()
{
  const cairo_region_t *region { gdk_window_get_clip_region(m_window) };
  m_drawContext = gdk_window_begin_draw_frame(m_window, region);

  gdk_gl_context_make_current(m_gl);

  ImGuiIO &io { ImGui::GetIO() };
  glBindTexture(GL_TEXTURE_2D, m_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, io.DisplaySize.x, io.DisplaySize.y,
    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  ImGui_ImplOpenGL3_NewFrame();
}

void GdkBackend::enterFrame()
{
  gdk_gl_context_make_current(m_gl);
}

void GdkBackend::endFrame(ImDrawData *drawData)
{
  gdk_gl_context_make_current(m_gl);

  m_ctx->clearColor().apply(glClearColor);
  glClear(GL_COLOR_BUFFER_BIT);

  if(drawData)
    ImGui_ImplOpenGL3_RenderDrawData(drawData);

  ImGuiIO &io { ImGui::GetIO() };
  cairo_t *cairoContext { gdk_drawing_context_get_cairo_context(m_drawContext) };
  gdk_cairo_draw_from_gl(cairoContext, m_window, m_tex, GL_TEXTURE, 1,
    0, 0, io.DisplaySize.x, io.DisplaySize.y);
  gdk_window_end_draw_frame(m_window, m_drawContext);
}

float GdkBackend::deltaTime()
{
  const gint64 now { g_get_monotonic_time() }; // microseconds
  if(!m_lastFrame)
    m_lastFrame = now;
  const float delta = { (now - m_lastFrame) / 1'000'000.f };
  m_lastFrame = now;
  return delta;
}

float GdkBackend::scaleFactor() const
{
  return gdk_window_get_scale_factor(m_window);
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
