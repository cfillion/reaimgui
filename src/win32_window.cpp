#include "window.hpp"

#include "context.hpp"
#include "opengl_renderer.hpp"

#include <GL/gl3w.h>
#include <GL/wglext.h>
#include <stdexcept>
#include <string>

static std::wstring widen(const char *input, const UINT codepage = CP_UTF8)
{
  const int size = MultiByteToWideChar(codepage, 0, input, -1, nullptr, 0) - 1;

  std::wstring output(size, 0);
  MultiByteToWideChar(codepage, 0, input, -1, &output[0], size);

  return output;
}

constexpr wchar_t *CLASS_NAME { L"reaper_imgui_context" };

struct Window::Impl {
  struct WindowClass {
    WindowClass();
    ~WindowClass();
  };

  void initPixelFormat();
  void initGL();

  Context *ctx;
  HwndPtr hwnd;
  HDC dc;
  HGLRC gl;
  OpenGLRenderer *renderer;
};

Window::Impl::WindowClass::WindowClass()
{
  WNDCLASS wc {};
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = Window::proc;
  wc.hInstance = Window::s_instance;
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);
}

Window::Impl::WindowClass::~WindowClass()
{
  UnregisterClass(CLASS_NAME, Window::s_instance);
}

Window::Window(const char *title, RECT rect, Context *ctx)
  : m_impl { std::make_unique<Impl>() }
{
  static Impl::WindowClass windowClass;

  // WS_EX_DLGMODALFRAME removes the default icon
  constexpr int style { WS_OVERLAPPEDWINDOW | WS_VISIBLE };
  constexpr int exStyle { WS_EX_DLGMODALFRAME };

  AdjustWindowRectEx(&rect, style, false, exStyle);

  HWND hwnd {
    CreateWindowEx(exStyle, CLASS_NAME, widen(title).c_str(), style,
      rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
      parentHandle(), nullptr, s_instance, nullptr)
  };
  assert(hwnd && "CreateWindow failed");
  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));

  m_impl->ctx = ctx;
  m_impl->hwnd = HwndPtr { hwnd };
  m_impl->dc = GetDC(hwnd);

  m_impl->initPixelFormat();
  m_impl->initGL();
  m_impl->renderer = new OpenGLRenderer;
  wglMakeCurrent(m_impl->dc, nullptr);

  ImGuiIO &io { ImGui::GetIO() };
  io.ImeWindowHandle = hwnd;
}

void Window::Impl::initPixelFormat()
{
  PIXELFORMATDESCRIPTOR pfd {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;

  if(!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd)) {
    ReleaseDC(hwnd.get(), dc);
    throw std::runtime_error { "failed to set a suitable pixel format" };
  }
}

void Window::Impl::initGL()
{
  HGLRC dummyGl { wglCreateContext(dc) }; // creates a legacy (< 2.1) context
  wglMakeCurrent(dc, gl = dummyGl);

  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB
    { reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
      wglGetProcAddress("wglCreateContextAttribsARB")) };

  if(wglCreateContextAttribsARB) {
    constexpr int attrs[] {
      WGL_CONTEXT_MAJOR_VERSION_ARB, OpenGLRenderer::MIN_MAJOR,
      WGL_CONTEXT_MINOR_VERSION_ARB, OpenGLRenderer::MIN_MINOR,
      0
    };

    if(HGLRC coreGl { wglCreateContextAttribsARB(dc, nullptr, attrs) }) {
      wglMakeCurrent(dc, gl = coreGl);
      wglDeleteContext(dummyGl);
    }
  }

  if(gl3wInit() || !gl3wIsSupported(OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR)) {
    wglDeleteContext(gl);
    ReleaseDC(hwnd.get(), dc);
    throw std::runtime_error { "failed to initialize OpenGL 3.2 or newer" };
  }
}

Window::~Window()
{
  wglMakeCurrent(m_impl->dc, m_impl->gl);
  delete m_impl->renderer;
  wglDeleteContext(m_impl->gl);
  ReleaseDC(m_impl->hwnd.get(), m_impl->dc);
}

HWND Window::nativeHandle() const
{
  return m_impl->hwnd.get();
}

void Window::beginFrame()
{
}

void Window::drawFrame(ImDrawData *drawData)
{
  wglMakeCurrent(m_impl->dc, m_impl->gl);
  m_impl->renderer->draw(drawData, m_impl->ctx->clearColor());
  SwapBuffers(m_impl->dc);
  wglMakeCurrent(nullptr, nullptr);
}

void Window::endFrame()
{
}

float Window::scaleFactor() const
{
  return 1; // TODO
}

bool Window::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    if(wParam < 256)
      m_impl->ctx->keyInput(wParam, true);
    return true;
  case WM_KEYUP:
  case WM_SYSKEYUP:
    if(wParam < 256)
      m_impl->ctx->keyInput(wParam, false);
    return true;
  case WM_CHAR:
    m_impl->ctx->charInput(wParam);
    return true;
  }

  return false;
}
