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
#include "dllimport.hpp"
#include "opengl_renderer.hpp"

#include <cassert>
#include <string>

#include <GL/gl3w.h>
#include <GL/wglext.h>
#include <reaper_plugin_secrets.h>
#include <ShellScalingApi.h> // GetDpiForMonitor

static std::wstring widen(const std::string &input, const UINT codepage = CP_UTF8)
{
  const int size {
    MultiByteToWideChar(codepage, 0, input.c_str(), input.size(), nullptr, 0)
  };

  std::wstring output(size, L'\0');
  MultiByteToWideChar(codepage, 0, input.c_str(), input.size(), output.data(), size);

  return output;
}

static unsigned int dpiForPoint(const POINT &point)
{
  // Windows 8.1+
  static DllImport<decltype(GetDpiForMonitor)>
    _GetDpiForMonitor
    { L"SHCore.dll", "GetDpiForMonitor" };

  HMONITOR monitor { MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST) };

  if(_GetDpiForMonitor && monitor) {
    unsigned int dpiX, dpiY;
    if(S_OK == _GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))
      return dpiX;
  }

  return USER_DEFAULT_SCREEN_DPI;
}

static unsigned int dpiForWindow(HWND window)
{
  // Windows 10 Anniversary Update (1607) and newer
  static DllImport<decltype(GetDpiForWindow)>
    _GetDpiForWindow
    { L"User32.dll", "GetDpiForWindow" };
  if(_GetDpiForWindow)
    return _GetDpiForWindow(window);
  else
    return USER_DEFAULT_SCREEN_DPI;
}

static float scaleForDpi(const unsigned int dpi)
{
  return static_cast<float>(dpi) / USER_DEFAULT_SCREEN_DPI;
}

static RECT scaledWindowRect(const WindowConfig &cfg,
  const DWORD style, const DWORD exStyle)
{
  RECT rect;

  // Windows 10 Anniversary Update (1607) and newer
  static DllImport<decltype(AdjustWindowRectExForDpi)>
    _AdjustWindowRectExForDpi
    { L"User32.dll", "AdjustWindowRectExForDpi" };
  if(_AdjustWindowRectExForDpi) {
    unsigned int dpi;
    if(cfg.x && cfg.y)
      dpi = dpiForPoint({ *cfg.x, *cfg.y });
    else
      dpi = dpiForWindow(Window::parentHandle());
    rect = cfg.clientRect(scaleForDpi(dpi));
    _AdjustWindowRectExForDpi(&rect, style, false, exStyle, dpi);
  }
  else {
    rect = cfg.clientRect();
    AdjustWindowRectEx(&rect, style, false, exStyle);
  }

  return rect;
}

constexpr wchar_t *CLASS_NAME { L"reaper_imgui_context" };

struct Window::Impl {
  struct WindowClass {
    WindowClass();
    ~WindowClass();
  };

  void initPixelFormat(HWND);
  void initGL(HWND);

  HDC dc;
  HGLRC gl;
  float scale;
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

Window::Window(const WindowConfig &cfg, Context *ctx)
  : m_cfg { cfg }, m_ctx { ctx }, m_impl { std::make_unique<Impl>() }
{
  static Impl::WindowClass windowClass;

  // WS_POPUP allows AttachWindowTopmostButton to work
  constexpr DWORD style   { WS_OVERLAPPEDWINDOW | WS_POPUP };
  // WS_EX_DLGMODALFRAME removes the default icon
  constexpr DWORD exStyle { WS_EX_DLGMODALFRAME };

  const RECT rect { scaledWindowRect(cfg, style, exStyle) };

  CreateWindowEx(exStyle, CLASS_NAME, widen(cfg.title).c_str(), style,
    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
    parentHandle(), nullptr, s_instance, this);
  assert(m_hwnd && "CreateWindow failed");

  m_impl->dc = GetDC(m_hwnd.get());
  m_impl->scale = scaleForDpi(dpiForWindow(m_hwnd.get()));

  m_impl->initPixelFormat(m_hwnd.get());
  m_impl->initGL(m_hwnd.get());
  m_impl->renderer = new OpenGLRenderer;
  wglMakeCurrent(m_impl->dc, nullptr);

  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_win32";
  io.ImeWindowHandle = m_hwnd.get();

  if(cfg.dock & 1)
    setDock(cfg.dock);
  else {
    AttachWindowTopmostButton(m_hwnd.get());
    ShowWindow(m_hwnd.get(), SW_SHOW); // after adding the topmost button
  }

  // WS_EX_DLGMODALFRAME removes the default icon but adds a border when docked
  // Unsetting it after the window is visible disables the border (+ no icon)
  SetWindowLongPtr(m_hwnd.get(), GWL_EXSTYLE, exStyle & ~WS_EX_DLGMODALFRAME);
}

void Window::Impl::initPixelFormat(HWND hwnd)
{
  PIXELFORMATDESCRIPTOR pfd {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;

  if(!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd)) {
    ReleaseDC(hwnd, dc);
    throw reascript_error { "failed to set a suitable pixel format" };
  }
}

void Window::Impl::initGL(HWND hwnd)
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
    ReleaseDC(hwnd, dc);
    throw reascript_error { "failed to initialize OpenGL 3.2 or newer" };
  }
}

Window::~Window()
{
  wglMakeCurrent(m_impl->dc, m_impl->gl);
  delete m_impl->renderer;
  wglDeleteContext(m_impl->gl);
  ReleaseDC(m_hwnd.get(), m_impl->dc);
}

void Window::beginFrame()
{
}

void Window::drawFrame(ImDrawData *drawData)
{
  wglMakeCurrent(m_impl->dc, m_impl->gl);
  m_impl->renderer->draw(drawData, m_ctx->clearColor());
  SwapBuffers(m_impl->dc);
  wglMakeCurrent(nullptr, nullptr);
}

void Window::endFrame()
{
}

float Window::scaleFactor() const
{
  return m_impl->scale;
}

std::optional<LRESULT> Window::handleMessage(const unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
  case WM_NCCREATE: {
    // Windows 10 Anniversary Update (1607) and newer
    static DllImport<decltype(EnableNonClientDpiScaling)>
      _EnableNonClientDpiScaling
      { L"User32.dll", "EnableNonClientDpiScaling" };
    if(_EnableNonClientDpiScaling)
      _EnableNonClientDpiScaling(m_hwnd.get());
    return std::nullopt;
  }
  case WM_CHAR:
    m_ctx->charInput(wParam);
    return 0;
  case WM_DPICHANGED: {
    m_impl->scale = scaleForDpi(LOWORD(wParam));
    const RECT *sugg { reinterpret_cast<RECT *>(lParam) };
    SetWindowPos(m_hwnd.get(), nullptr,
      sugg->left, sugg->top, sugg->right - sugg->left, sugg->bottom - sugg->top,
      SWP_NOACTIVATE | SWP_NOZORDER);
    return 0;
  }
  case WM_GETDLGCODE:
    return DLGC_WANTALLKEYS; // eat all inputs, don't let Tab steal focus
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    if(wParam < 256)
      m_ctx->keyInput(wParam, true);
    return 0;
  case WM_KEYUP:
  case WM_SYSKEYUP:
    if(wParam < 256)
      m_ctx->keyInput(wParam, false);
    return 0;
  case WM_KILLFOCUS:
    m_ctx->clearFocus();
    return 0;
  }

  return std::nullopt;
}
