#include "backend.hpp"

#include "context.hpp"
#include "opengl_renderer.hpp"

#include <GL/gl3w.h>
#include <GL/wglext.h>
#include <stdexcept>

class Win32Backend : public Backend {
public:
  Win32Backend(Context *);
  ~Win32Backend() override;

  void drawFrame(ImDrawData *) override;
  float deltaTime() override;
  float scaleFactor() const override;
  bool handleMessage(unsigned int, WPARAM, LPARAM) override;
  void translateAccel(MSG *) override;

private:
  void initPixelFormat();
  void initGL();

  Context *m_ctx;
  LARGE_INTEGER m_lastFrame, m_ticksPerSecond;
  HDC m_dc;
  HGLRC m_gl;
  OpenGLRenderer *m_renderer;
};

std::unique_ptr<Backend> Backend::create(Context *ctx)
{
  return std::make_unique<Win32Backend>(ctx);
}

Win32Backend::Win32Backend(Context *ctx)
  : m_ctx { ctx }, m_dc { GetDC(ctx->handle()) }
{
  QueryPerformanceCounter(&m_lastFrame);
  QueryPerformanceFrequency(&m_ticksPerSecond);

  initPixelFormat();
  initGL();
  m_renderer = new OpenGLRenderer;
  wglMakeCurrent(m_dc, nullptr);

  ImGuiIO &io { ImGui::GetIO() };
  io.ImeWindowHandle = ctx->handle();
}

void Win32Backend::initPixelFormat()
{
  PIXELFORMATDESCRIPTOR pfd {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;

  if(!SetPixelFormat(m_dc, ChoosePixelFormat(m_dc, &pfd), &pfd)) {
    ReleaseDC(m_ctx->handle(), m_dc);
    throw std::runtime_error { "failed to set a suitable pixel format" };
  }
}

void Win32Backend::initGL()
{
  HGLRC dummy { wglCreateContext(m_dc) }; // creates a legacy (< 2.1) context
  wglMakeCurrent(m_dc, dummy);

  const int attrs[] {
    WGL_CONTEXT_MAJOR_VERSION_ARB, OpenGLRenderer::MIN_MAJOR,
    WGL_CONTEXT_MINOR_VERSION_ARB, OpenGLRenderer::MIN_MINOR,
    0
  };

  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB
    { reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
      wglGetProcAddress("wglCreateContextAttribsARB")) };

  if(wglCreateContextAttribsARB) {
    m_gl = wglCreateContextAttribsARB(m_dc, nullptr, attrs);
    if(m_gl) {
      wglMakeCurrent(m_dc, m_gl);
      wglDeleteContext(dummy);
    }
  }
  else
    m_gl = dummy;

  if(gl3wInit() || !gl3wIsSupported(OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR)) {
    wglDeleteContext(m_gl);
    ReleaseDC(m_ctx->handle(), m_dc);
    throw std::runtime_error { "failed to initialize OpenGL 3.2 or newer" };
  }
}

Win32Backend::~Win32Backend()
{
  wglMakeCurrent(m_dc, m_gl);
  delete m_renderer;
  wglDeleteContext(m_gl);
  ReleaseDC(m_ctx->handle(), m_dc);
}

void Win32Backend::drawFrame(ImDrawData *drawData)
{
  wglMakeCurrent(m_dc, m_gl);
  m_renderer->draw(drawData, m_ctx->clearColor());
  SwapBuffers(m_dc);
  wglMakeCurrent(m_dc, nullptr);
}

float Win32Backend::deltaTime()
{
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  const float delta {
    static_cast<float>(now.QuadPart - m_lastFrame.QuadPart)
    / m_ticksPerSecond.QuadPart
  };
  m_lastFrame = now;
  return delta;
}

float Win32Backend::scaleFactor() const
{
  return 1; // TODO
}

bool Win32Backend::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    if(wParam < 256)
      m_ctx->keyInput(wParam, true);
    return true;
  case WM_KEYUP:
  case WM_SYSKEYUP:
    if(wParam < 256)
      m_ctx->keyInput(wParam, false);
    return true;
  }

  return false;
}

void Win32Backend::translateAccel(MSG *)
{
}
