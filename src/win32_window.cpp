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

#include "win32_window.hpp"

#include "context.hpp"
#include "dllimport.hpp"
#include "opengl_renderer.hpp"
#include "platform.hpp"
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

unsigned int Win32Window::dpiForMonitor(HMONITOR monitor)
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

unsigned int Win32Window::dpiForWindow(HWND window)
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

float Win32Window::scaleForDpi(const unsigned int dpi)
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

Win32Window::Class::Class()
{
  WNDCLASS wc {};
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = Window::proc;
  wc.hInstance = Win32Window::s_instance;
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);
}

Win32Window::Class::~Class()
{
  UnregisterClass(CLASS_NAME, Win32Window::s_instance);
}

Win32Window::Win32Window(ImGuiViewport *viewport, DockerHost *dockerHost)
  : Window { viewport, dockerHost }
{
}

void *Win32Window::create()
{
  static Class windowClass;

  styleFromFlags(m_viewport->Flags, &m_style, &m_exStyle);

  // Trick remove the default icon during construction, unset in show()
  DWORD exStyle { m_exStyle };
  if(!(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
    exStyle |= WS_EX_DLGMODALFRAME;

  CreateWindowEx(exStyle, CLASS_NAME, L"", m_style,
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
    parentHandle(), nullptr, s_instance, this);
  assert(m_hwnd && "CreateWindow failed");

  m_dpi = dpiForWindow(m_hwnd.get());
  m_viewport->DpiScale = scaleForDpi(m_dpi);
  const RECT &rect { scaledWindowRect(m_viewport->Pos, m_viewport->Size) };
  SetWindowPos(m_hwnd.get(), nullptr,  rect.left, rect.top,
    rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER);

  m_dc = GetDC(m_hwnd.get());
  initPixelFormat();
  initGL();
  m_renderer = new OpenGLRenderer;
  wglMakeCurrent(m_dc, nullptr);

  // will be freed upon RevokeDragDrop during destruction
  DropTarget *dropTarget = new DropTarget { m_ctx };
  RegisterDragDrop(m_hwnd.get(), dropTarget);

  return m_hwnd.get();
}

Win32Window::~Win32Window()
{
  RevokeDragDrop(m_hwnd.get());

  wglMakeCurrent(m_dc, m_gl);
  delete m_renderer;
  wglDeleteContext(m_gl);
  ReleaseDC(m_hwnd.get(), m_dc);
}

void Win32Window::initPixelFormat()
{
  PIXELFORMATDESCRIPTOR pfd {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;

  if(!SetPixelFormat(m_dc, ChoosePixelFormat(m_dc, &pfd), &pfd)) {
    ReleaseDC(m_hwnd.get(), m_dc);
    throw backend_error { "failed to set a suitable pixel format" };
  }
}

void Win32Window::initGL()
{
  HGLRC dummyGl { wglCreateContext(m_dc) }; // creates a legacy (< 2.1) context
  wglMakeCurrent(m_dc, m_gl = dummyGl);

  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB
    { reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>
      (wglGetProcAddress("wglCreateContextAttribsARB")) };

  if(wglCreateContextAttribsARB) {
    constexpr int attrs[] {
      WGL_CONTEXT_MAJOR_VERSION_ARB, OpenGLRenderer::MIN_MAJOR,
      WGL_CONTEXT_MINOR_VERSION_ARB, OpenGLRenderer::MIN_MINOR,
      0
    };

    if(HGLRC coreGl { wglCreateContextAttribsARB(m_dc, nullptr, attrs) }) {
      wglMakeCurrent(m_dc, m_gl = coreGl);
      wglDeleteContext(dummyGl);
    }
  }

  if(gl3wInit() || !gl3wIsSupported(OpenGLRenderer::MIN_MAJOR, OpenGLRenderer::MIN_MINOR)) {
    wglDeleteContext(m_gl);
    ReleaseDC(m_hwnd.get(), m_dc);
    throw backend_error { "failed to initialize OpenGL 3.2 or newer" };
  }
}

RECT Win32Window::scaledWindowRect(ImVec2 pos, ImVec2 size) const
{
  const float scale { scaleForDpi(m_dpi) };
  Platform::scalePosition(&pos, true);

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
    _AdjustWindowRectExForDpi(&rect, m_style, false, m_exStyle, m_dpi);
  else
    AdjustWindowRectEx(&rect, m_style, false, m_exStyle);

  return rect;
}

void Win32Window::show()
{
  if(!m_dockerHost && !(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
    AttachWindowTopmostButton(m_hwnd.get());

  Window::show();

  // WS_EX_DLGMODALFRAME removes the default icon but adds a border when docked
  // Unsetting it after the window is visible disables the border (+ no icon)
  const auto exStyle { GetWindowLong(m_hwnd.get(), GWL_EXSTYLE) };
  if(exStyle & WS_EX_DLGMODALFRAME)
    SetWindowLongPtr(m_hwnd.get(), GWL_EXSTYLE, m_exStyle);
}

void Win32Window::setPosition(const ImVec2 pos)
{
  const RECT &rect { scaledWindowRect(pos, m_viewport->Size) };
  SetWindowPos(m_hwnd.get(), nullptr,
    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void Win32Window::setSize(const ImVec2 size)
{
  const RECT &rect { scaledWindowRect(m_viewport->Pos, size) };
  SetWindowPos(m_hwnd.get(), nullptr,
    0, 0, rect.right - rect.left, rect.bottom - rect.top,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void Win32Window::setTitle(const char *title)
{
  SetWindowText(m_hwnd.get(), widen(title).c_str());
}

void Win32Window::update()
{
  if(isDocked())
    return;

  const DWORD prevStyle { m_style }, prevExStyle { m_exStyle };
  styleFromFlags(m_viewport->Flags, &m_style, &m_exStyle);

  if(prevStyle != m_style || prevExStyle != m_exStyle) {
    SetWindowLong(m_hwnd.get(), GWL_STYLE, m_style);
    SetWindowLong(m_hwnd.get(), GWL_EXSTYLE, m_exStyle);

    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration)
      DetachWindowTopmostButton(m_hwnd.get(), false);
    else
      AttachWindowTopmostButton(m_hwnd.get());


    HWND insertAfter;
    unsigned int flags { SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW };
    if((prevExStyle & m_exStyle) ^ WS_EX_TOPMOST) {
      if(m_exStyle & WS_EX_TOPMOST)
        insertAfter = HWND_TOPMOST;
      else
        insertAfter = HWND_NOTOPMOST;
    }
    else {
      insertAfter = nullptr;
      flags |= SWP_NOZORDER;
    }

    const RECT rect { scaledWindowRect(m_viewport->Pos, m_viewport->Size) };
    SetWindowPos(m_hwnd.get(), insertAfter, rect.left, rect.top,
      rect.right - rect.left, rect.bottom - rect.top, flags);
    m_viewport->PlatformRequestMove = m_viewport->PlatformRequestResize = true;
  }
}

void Win32Window::render(void *)
{
  wglMakeCurrent(m_dc, m_gl);
  m_renderer->render(m_viewport);
  SwapBuffers(m_dc);
  wglMakeCurrent(nullptr, nullptr);
}

float Win32Window::scaleFactor() const
{
  return m_viewport->DpiScale;
}

void Win32Window::setImePosition(ImVec2 pos)
{
  if(HIMC ime { ImmGetContext(m_hwnd.get()) }) {
    Platform::scalePosition(&pos, true);

    COMPOSITIONFORM cf;
    cf.ptCurrentPos.x = pos.x;
    cf.ptCurrentPos.y = pos.y;
    ScreenToClient(m_hwnd.get(), &cf.ptCurrentPos);
    cf.dwStyle = CFS_FORCE_POSITION;
    ImmSetCompositionWindow(ime, &cf);

    ImmReleaseContext(m_hwnd.get(), ime);
  }
}

void Win32Window::uploadFontTex()
{
  wglMakeCurrent(m_dc, m_gl);
  m_renderer->uploadFontTex();
  wglMakeCurrent(nullptr, nullptr);
}

std::optional<LRESULT> Win32Window::handleMessage
  (const unsigned int msg, WPARAM wParam, LPARAM lParam)
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
    m_dpi = LOWORD(wParam);
    m_viewport->DpiScale = scaleForDpi(m_dpi);

    const RECT *sugg { reinterpret_cast<RECT *>(lParam) };
    SetWindowPos(m_hwnd.get(), nullptr,
      sugg->left, sugg->top, sugg->right - sugg->left, sugg->bottom - sugg->top,
      SWP_NOACTIVATE | SWP_NOZORDER);

    // PlatformRequestResize doesn't work here to tell ImGui to fetch the new size
    m_viewport->Size = getSize();
    return 0;
  }
  case WM_DPICHANGED_BEFOREPARENT:
    // Tthis messages is sent when docked.
    // Only top-level windows receive WM_DPICHANGED.
    m_dpi = dpiForWindow(m_hwnd.get());
    m_viewport->DpiScale = scaleForDpi(m_dpi);
    m_viewport->Pos = getPosition();
    // WM_SIZE has been sent, no need to set m_viewport->Size here
    return 0;
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
    m_ctx->updateFocus();
    return 0;
  }

  return std::nullopt;
}
