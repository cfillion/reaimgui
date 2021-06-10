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
#include "win32_unicode.hpp"

#include <cassert>
#include <GL/gl3w.h>
#include <GL/wglext.h>
#include <reaper_plugin_secrets.h>
#include <ShellScalingApi.h> // GetDpiForMonitor

#include "win32_droptarget.ipp"

static unsigned int xpScreenDpi()
{
  const HDC dc { GetDC(nullptr) };
  const int dpi { GetDeviceCaps(dc, LOGPIXELSX) };
  ReleaseDC(nullptr, dc);
  return dpi;
}

static unsigned int dpiForMonitor(HMONITOR monitor)
{
  // Windows 8.1+
  static DllImport<decltype(GetDpiForMonitor)>
    _GetDpiForMonitor
    { L"SHCore.dll", "GetDpiForMonitor" };

  if(_GetDpiForMonitor && monitor) {
    unsigned int dpiX, dpiY;
    if(S_OK == _GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))
      return dpiX;
  }

  return xpScreenDpi();
}

static unsigned int dpiForWindow(HWND window)
{
  // Windows 10 Anniversary Update (1607) and newer
  static DllImport<decltype(GetDpiForWindow)>
    _GetDpiForWindow
    { L"User32.dll", "GetDpiForWindow" };

  if(_GetDpiForWindow)
    return _GetDpiForWindow(window);
  else {
    HMONITOR monitor { MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST) };
    return dpiForMonitor(monitor);
  }
}

static float scaleForDpi(const unsigned int dpi)
{
  return static_cast<float>(dpi) / USER_DEFAULT_SCREEN_DPI;
}

static void styleFromFlags(ImGuiViewportFlags flags, DWORD *style, DWORD *exStyle)
{
  *style = WS_POPUP; // fix AttachWindowTopmostButton when a titlebar is shown
  *exStyle = WS_EX_ACCEPTFILES;

  if(!(flags & ImGuiViewportFlags_NoDecoration))
    *style |= WS_OVERLAPPEDWINDOW;

  if(flags & ImGuiViewportFlags_NoTaskBarIcon)
    *exStyle |= WS_EX_TOOLWINDOW;

  if(flags & ImGuiViewportFlags_TopMost)
    *exStyle |= WS_EX_TOPMOST;
}

struct Window::Impl {
  struct WindowClass {
    WindowClass();
    ~WindowClass();
  };

  void initPixelFormat(HWND);
  void initGL(HWND);
  RECT scaledWindowRect(ImVec2 pos, ImVec2 size);

  HDC dc;
  HGLRC gl;
  unsigned int dpi;
  OpenGLRenderer *renderer;
  DropTarget dropTarget;
  DWORD style, exStyle;
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

void Window::platformInstall()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_win32";
}

static int CALLBACK enumMonitors(HMONITOR monitor, HDC, LPRECT, LPARAM)
{
  MONITORINFO info{};
  info.cbSize = sizeof(MONITORINFO);
  if(!GetMonitorInfo(monitor, &info))
    return true;

  ImGuiPlatformMonitor imguiMonitor;
  imguiMonitor.MainPos.x  = info.rcMonitor.left;
  imguiMonitor.MainPos.y  = info.rcMonitor.top;
  imguiMonitor.MainSize.x = info.rcMonitor.right - info.rcMonitor.left;
  imguiMonitor.MainSize.y = info.rcMonitor.bottom - info.rcMonitor.top;
  imguiMonitor.WorkPos.x  = info.rcWork.left;
  imguiMonitor.WorkPos.y  = info.rcWork.top;
  imguiMonitor.WorkSize.x = info.rcWork.right - info.rcWork.left;
  imguiMonitor.WorkSize.y = info.rcWork.bottom - info.rcWork.top;
  imguiMonitor.DpiScale   = scaleForDpi(dpiForMonitor(monitor));

  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  if(info.dwFlags & MONITORINFOF_PRIMARY)
    pio.Monitors.push_front(imguiMonitor);
  else
    pio.Monitors.push_back(imguiMonitor);

  return true;
}

void Window::updateMonitors()
{
  ImGui::GetPlatformIO().Monitors.resize(0);
  EnumDisplayMonitors(nullptr, nullptr, enumMonitors, 0);
}

Window::Window(ImGuiViewport *viewport, Context *ctx)
  : m_viewport { viewport }, m_ctx { ctx }, m_impl { std::make_unique<Impl>() }
{
  static Impl::WindowClass windowClass;

  styleFromFlags(viewport->Flags, &m_impl->style, &m_impl->exStyle);

  // Trick remove the default icon during construction, unset in show()
  DWORD exStyle { m_impl->exStyle };
  if(!(viewport->Flags & ImGuiViewportFlags_NoDecoration))
    exStyle |= WS_EX_DLGMODALFRAME;

  CreateWindowEx(exStyle, CLASS_NAME, L"", m_impl->style,
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
    parentHandle(), nullptr, s_instance, this);
  assert(m_hwnd && "CreateWindow failed");

  m_impl->dpi = dpiForWindow(m_hwnd.get());
  m_viewport->DpiScale = scaleForDpi(m_impl->dpi);
  const RECT &rect { m_impl->scaledWindowRect(viewport->Pos, viewport->Size) };
  SetWindowPos(m_hwnd.get(), nullptr,  rect.left, rect.top,
    rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER);

  m_impl->dc = GetDC(m_hwnd.get());
  m_impl->initPixelFormat(m_hwnd.get());
  m_impl->initGL(m_hwnd.get());
  m_impl->renderer = new OpenGLRenderer;
  wglMakeCurrent(m_impl->dc, nullptr);

  m_impl->dropTarget.setContext(ctx);
  RegisterDragDrop(m_hwnd.get(), &m_impl->dropTarget);
}

Window::~Window()
{
  RevokeDragDrop(m_hwnd.get());

  wglMakeCurrent(m_impl->dc, m_impl->gl);
  delete m_impl->renderer;
  wglDeleteContext(m_impl->gl);
  ReleaseDC(m_hwnd.get(), m_impl->dc);
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
    throw backend_error { "failed to set a suitable pixel format" };
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
    throw backend_error { "failed to initialize OpenGL 3.2 or newer" };
  }
}

RECT Window::Impl::scaledWindowRect(const ImVec2 pos, const ImVec2 size)
{
  const float scale { scaleForDpi(dpi) };

  RECT rect;
  rect.left = pos.x;
  rect.top  = pos.y;
  rect.right  = rect.left + (size.x * scale);
  rect.bottom = rect.top  + (size.y * scale);

  // Windows 10 Anniversary Update (1607) and newer
  static DllImport<decltype(AdjustWindowRectExForDpi)>
    _AdjustWindowRectExForDpi
    { L"User32.dll", "AdjustWindowRectExForDpi" };

  if(_AdjustWindowRectExForDpi)
    _AdjustWindowRectExForDpi(&rect, style, false, exStyle, dpi);
  else
    AdjustWindowRectEx(&rect, style, false, exStyle);

  return rect;
}

void Window::show()
{
  if(!(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
    AttachWindowTopmostButton(m_hwnd.get());

  commonShow();

  // WS_EX_DLGMODALFRAME removes the default icon but adds a border when docked
  // Unsetting it after the window is visible disables the border (+ no icon)
  const auto exStyle { GetWindowLong(m_hwnd.get(), GWL_EXSTYLE) };
  if(exStyle & WS_EX_DLGMODALFRAME)
    SetWindowLongPtr(m_hwnd.get(), GWL_EXSTYLE, m_impl->exStyle);
}

void Window::setPosition(const ImVec2 pos)
{
  const RECT &rect { m_impl->scaledWindowRect(pos, m_viewport->Size) };
  SetWindowPos(m_hwnd.get(), nullptr,
    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void Window::setSize(ImVec2 size)
{
  const RECT &rect { m_impl->scaledWindowRect(m_viewport->Pos, size) };
  SetWindowPos(m_hwnd.get(), nullptr,
    0, 0, rect.right - rect.left, rect.bottom - rect.top,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void Window::setTitle(const char *title)
{
  SetWindowText(m_hwnd.get(), widen(title).c_str());
}

void Window::update()
{
  const DWORD prevStyle { m_impl->style }, prevExStyle { m_impl->exStyle };
  styleFromFlags(m_viewport->Flags, &m_impl->style, &m_impl->exStyle);

  if(prevStyle != m_impl->style || prevExStyle != m_impl->exStyle) {
    SetWindowLong(m_hwnd.get(), GWL_STYLE, m_impl->style);
    SetWindowLong(m_hwnd.get(), GWL_EXSTYLE, m_impl->exStyle);

    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration)
      DetachWindowTopmostButton(m_hwnd.get(), false);
    else
      AttachWindowTopmostButton(m_hwnd.get());


    HWND insertAfter;
    unsigned int flags { SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW };
    if((prevExStyle & m_impl->exStyle) ^ WS_EX_TOPMOST) {
      if(m_impl->exStyle & WS_EX_TOPMOST)
        insertAfter = HWND_TOPMOST;
      else
        insertAfter = HWND_NOTOPMOST;
    }
    else {
      insertAfter = nullptr;
      flags |= SWP_NOZORDER;
    }

    const RECT rect { m_impl->scaledWindowRect(m_viewport->Pos, m_viewport->Size) };
    SetWindowPos(m_hwnd.get(), insertAfter, rect.left, rect.top,
      rect.right - rect.left, rect.bottom - rect.top, flags);
    m_viewport->PlatformRequestMove = m_viewport->PlatformRequestResize = true;
  }
}

void Window::render(void *)
{
  wglMakeCurrent(m_impl->dc, m_impl->gl);
  m_impl->renderer->render(m_viewport);
  SwapBuffers(m_impl->dc);
  wglMakeCurrent(nullptr, nullptr);
}

float Window::scaleFactor() const
{
  return m_viewport->DpiScale;
}

void Window::setImePosition(const ImVec2 pos)
{
  if(HIMC ime { ImmGetContext(m_hwnd.get()) }) {
    COMPOSITIONFORM cf;
    cf.ptCurrentPos.x = pos.x;
    cf.ptCurrentPos.y = pos.y;
    cf.dwStyle = CFS_FORCE_POSITION;
    ImmSetCompositionWindow(ime, &cf);
    ImmReleaseContext(m_hwnd.get(), ime);
  }
}

void Window::uploadFontTex()
{
  wglMakeCurrent(m_impl->dc, m_impl->gl);
  m_impl->renderer->uploadFontTex();
  wglMakeCurrent(nullptr, nullptr);
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
    break;
  }
  case WM_CHAR:
    m_ctx->charInput(wParam);
    return 0;
  case WM_DPICHANGED: {
    m_impl->dpi = LOWORD(wParam);
    m_viewport->DpiScale = scaleForDpi(m_impl->dpi);
    const RECT *sugg { reinterpret_cast<RECT *>(lParam) };
    SetWindowPos(m_hwnd.get(), nullptr,
      sugg->left, sugg->top, sugg->right - sugg->left, sugg->bottom - sugg->top,
      SWP_NOACTIVATE | SWP_NOZORDER);

    // PlatformRequestResize doesn't work here to tell ImGui to fetch the new size
    m_viewport->Size = getSize();
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
  case WM_NCHITTEST:
    if(m_viewport->Flags & ImGuiViewportFlags_NoInputs)
      return HTTRANSPARENT;
    break;
  case WM_KILLFOCUS:
    m_ctx->clearFocus();
    return 0;
  }

  return std::nullopt;
}
