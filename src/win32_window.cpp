/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#include <GL/gl3w.h>
#include <GL/wglext.h>
#include <reaper_plugin_secrets.h>
#include <ShellScalingApi.h> // GetDpiForMonitor
#define GetThemeColor Win32_GetThemeColor // solve conflict with REAPER API
#include <dwmapi.h> // Dwm* functions for compositing
#undef GetThemeColor

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

void Win32Window::updateStyles()
{
  m_style = WS_POPUP; // fix AttachWindowTopmostButton when a titlebar is shown
  m_exStyle = WS_EX_ACCEPTFILES;

  if(!(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
    m_style |= WS_OVERLAPPEDWINDOW;

  if(m_viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
    m_exStyle |= WS_EX_TOOLWINDOW;

  if(m_viewport->Flags & ImGuiViewportFlags_TopMost)
    m_exStyle |= WS_EX_TOPMOST;
}

static BOOL CALLBACK reparentChildren(HWND hwnd, LPARAM data)
{
  HWND owner { reinterpret_cast<HWND>(data) };
  if(GetWindow(hwnd, GW_OWNER) == owner) {
    SetWindowLongPtr(hwnd, GWLP_HWNDPARENT,
      reinterpret_cast<LONG_PTR>(GetWindow(owner, GW_OWNER)));
  }
  return TRUE;
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
  : Window { viewport, dockerHost }, m_gl {}, m_renderer { nullptr }
{
}

void Win32Window::create()
{
  static Class windowClass;

  updateStyles();

  // Trick remove the default icon during construction, unset in show()
  DWORD exStyle { m_exStyle };
  if(!(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
    exStyle |= WS_EX_DLGMODALFRAME;

  // give a sensible window position guess (accurate if no decorations)
  // so that m_dpi gets initialized to the correct value
  // (would be the primary monitor's DPI otherwise, causing scalePosition to be
  // given an incorrect scale and possibly moving the window out of view)
  ImVec2 initialPos { m_viewport->Pos };
  Platform::scalePosition(&initialPos, true);
  CreateWindowEx(exStyle, CLASS_NAME, L"", m_style,
    initialPos.x, initialPos.y, 0, 0,
    parentHandle(), nullptr, s_instance, this);
  if(!m_hwnd)
    throw backend_error { "failed to create native window" };

  m_dpi = dpiForWindow(m_hwnd.get());
  m_viewport->DpiScale = scaleForDpi(m_dpi);
  const RECT &rect { scaledWindowRect(m_viewport->Pos, m_viewport->Size) };
  SetWindowPos(m_hwnd.get(), nullptr,  rect.left, rect.top,
    rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER);

  m_dc = GetDC(m_hwnd.get());
  initPixelFormat();
  initGL();
  wglMakeCurrent(m_dc, nullptr);

  // will be freed upon RevokeDragDrop during destruction
  DropTarget *dropTarget = new DropTarget { m_ctx };
  RegisterDragDrop(m_hwnd.get(), dropTarget);

  // disable IME by default
  ImmAssociateContextEx(m_hwnd.get(), nullptr, 0);

  // enable compositing for transparency
  HRGN region { CreateRectRgn(0, 0, -1, -1) };
  DWM_BLURBEHIND bb {};
  bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
  bb.fEnable = true;
  bb.hRgnBlur = region; // no actual blur/shadow
  DwmEnableBlurBehindWindow(m_hwnd.get(), &bb);
  DeleteObject(region);
}

Win32Window::~Win32Window()
{
}

void Win32Window::destroy()
{
  // ImGui destroys windows in creation order. Give ownership of our owned
  // windows to our own owner to avoid a broken chain leading to Windows
  // possibly focusing a window from another application.
  EnumThreadWindows(GetCurrentThreadId(), &reparentChildren, reinterpret_cast<LPARAM>(m_hwnd.get()));

  // the window may already have been destroyed at this point
  // when exiting REAPER or somone called DestroyWindow on us
  if(m_gl) {
    wglMakeCurrent(m_dc, m_gl);
    if(m_renderer)
      delete m_renderer;
    wglDeleteContext(m_gl);
  }
  ReleaseDC(m_hwnd.get(), m_dc);

  Window::destroy();
}

void Win32Window::initPixelFormat()
{
  PIXELFORMATDESCRIPTOR pfd {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cAlphaBits = pfd.cBlueBits = pfd.cGreenBits = pfd.cRedBits = 8;
  pfd.cColorBits = pfd.cRedBits + pfd.cGreenBits + pfd.cBlueBits + pfd.cAlphaBits;

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
    static int minor { 2 };
    do {
      // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
      const int attrs[] {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        0
      };

      if(HGLRC coreGl { wglCreateContextAttribsARB(m_dc, nullptr, attrs) }) {
        wglMakeCurrent(m_dc, m_gl = coreGl);
        wglDeleteContext(dummyGl);
        break;
      }
    } while(--minor >= 1);
  }

  if(gl3wInit())
    throw backend_error { "failed to initialize OpenGL context" };

  m_renderer = new OpenGLRenderer;
}

RECT Win32Window::scaledWindowRect(ImVec2 pos, ImVec2 size) const
{
  const float scale { scaleForDpi(m_dpi) };
  Platform::scalePosition(&pos, true, scale);

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
  if(!isDocked() && !(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
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

void Win32Window::setAlpha(const float alpha)
{
  // may conflict with updateStyles()/update(), but viewport flags are
  // unlikely to change while alpha isn't 1

  if(alpha == 1.0f) {
    SetWindowLong(m_hwnd.get(), GWL_EXSTYLE, m_exStyle);
    return;
  }

  SetWindowLong(m_hwnd.get(), GWL_EXSTYLE, m_exStyle | WS_EX_LAYERED);
  SetLayeredWindowAttributes(m_hwnd.get(), 0, 255 * alpha, LWA_ALPHA);
}

void Win32Window::update()
{
  unstuckModifiers();

  if(isDocked())
    return;

  const DWORD prevStyle { m_style }, prevExStyle { m_exStyle };
  updateStyles();

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
  if(m_needTexUpload) {
    m_renderer->uploadFontTex(m_fontAtlas);
    m_needTexUpload = false;
  }
  m_renderer->render(m_viewport);
  SwapBuffers(m_dc);
  wglMakeCurrent(nullptr, nullptr);
}

float Win32Window::scaleFactor() const
{
  return m_viewport->DpiScale;
}

void Win32Window::setIME(ImGuiPlatformImeData *data)
{
  ImmAssociateContextEx(m_hwnd.get(), nullptr, data->WantVisible ? IACE_DEFAULT : 0);

  if(HIMC ime { ImmGetContext(m_hwnd.get()) }) {
    ImVec2 pos { data->InputPos };
    Platform::scalePosition(&pos, true);

    COMPOSITIONFORM composition;
    composition.dwStyle = CFS_FORCE_POSITION;
    composition.ptCurrentPos.x = pos.x;
    composition.ptCurrentPos.y = pos.y;
    ScreenToClient(m_hwnd.get(), &composition.ptCurrentPos);
    ImmSetCompositionWindow(ime, &composition);

    CANDIDATEFORM candidate;
    candidate.dwStyle = CFS_CANDIDATEPOS;
    candidate.ptCurrentPos = composition.ptCurrentPos;
    ImmSetCandidateWindow(ime, &candidate);

    ImmReleaseContext(m_hwnd.get(), ime);
  }
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
  case WM_ACTIVATEAPP:
    if(m_viewport->Flags & ImGuiViewportFlags_TopMost) {
      SetWindowPos(m_hwnd.get(), wParam ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }
    break;
  case WM_DPICHANGED: {
    m_dpi = LOWORD(wParam);
    m_viewport->DpiScale = scaleForDpi(m_dpi);

    const RECT *sugg { reinterpret_cast<RECT *>(lParam) };
    SetWindowPos(m_hwnd.get(), nullptr,
      sugg->left, sugg->top, sugg->right - sugg->left, sugg->bottom - sugg->top,
      SWP_NOACTIVATE | SWP_NOZORDER);

    // PlatformRequestResize doesn't work here to tell ImGui to fetch the new size
    m_viewport->Pos  = getPosition();
    m_viewport->Size = getSize();
    return 0;
  }
  case WM_DPICHANGED_BEFOREPARENT:
    // This message is sent when docked.
    // Only top-level windows receive WM_DPICHANGED.
    m_dpi = dpiForWindow(m_hwnd.get());
    m_viewport->DpiScale = scaleForDpi(m_dpi);
    m_viewport->Pos = getPosition();
    // WM_SIZE has been sent, no need to set m_viewport->Size here
    return 0;
  case WM_GETDLGCODE:
    return DLGC_WANTALLKEYS; // eat all inputs, don't let Tab steal focus
  case WM_XBUTTONDOWN:
    mouseDown(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4);
    return 0;
  case WM_XBUTTONUP:
    mouseUp(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4);
    return 0;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
    keyEvent(msg, wParam, lParam);
    return 0;
  case WM_CHAR:
    if(wParam >= 0 && wParam < 0x10000)
      m_ctx->charInputUTF16(wParam);
    return 0;
  case WM_NCHITTEST:
    if(m_viewport->Flags & ImGuiViewportFlags_NoInputs)
      return HTTRANSPARENT;
    break;
  case WM_KILLFOCUS:
    m_ctx->updateFocus();
    return 0;
  case WM_DESTROY:
    RevokeDragDrop(m_hwnd.get());
    break;
  }

  return std::nullopt;
}

static bool IsVkDown(const int vk)
{
  return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void Win32Window::keyEvent(unsigned int msg,
  const WPARAM vk, const LPARAM lParam)
{
  if(vk > 255)
    return;

  const bool down { msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN };

  if(vk == VK_RETURN && (HIWORD(lParam) & KF_EXTENDED))
    m_ctx->keyInput(ImGuiKey_KeypadEnter, down);
  else if(!modKeyEvent(vk, down))
    m_ctx->keyInput(vk, down);
}

struct ModKey { uint8_t vk; ImGuiKey ik; };
struct Modifier { uint8_t modVK; ImGuiKey modKey; ModKey keys[2]; };
constexpr Modifier modifiers[] {
  { VK_CONTROL, ImGuiKey_ModCtrl, {
    { VK_LCONTROL, ImGuiKey_LeftCtrl }, { VK_RCONTROL, ImGuiKey_RightCtrl },
  }},
  { VK_SHIFT, ImGuiKey_ModShift, {
    { VK_LSHIFT, ImGuiKey_LeftShift }, { VK_RSHIFT, ImGuiKey_RightShift },
  }},
  { VK_MENU, ImGuiKey_ModAlt, {
    { VK_LMENU, ImGuiKey_LeftAlt }, { VK_RMENU, ImGuiKey_RightAlt },
  }},
  { VK_LWIN, ImGuiKey_ModSuper, {
    { VK_LWIN, ImGuiKey_LeftSuper }, { VK_RWIN, ImGuiKey_RightSuper },
  }},
};

bool Win32Window::modKeyEvent(const WPARAM vk, const bool down)
{
  for(const auto &modifier : modifiers) {
    if(vk != modifier.modVK)
      continue;
    if(IsVkDown(modifier.modVK) == down)
      m_ctx->keyInput(modifier.modKey, down);
    for(const auto &modkey : modifier.keys) {
      if(IsVkDown(modkey.vk) == down)
        m_ctx->keyInput(modkey.ik, down);
    }
    return true;
  }
  return false;
}

void Win32Window::unstuckModifiers()
{
  // Windows doesn't send KEYUP events when both LSHIFT+RSHIFT are down and one is released
  for(const auto &modifier : modifiers) {
    for(const auto &modkey : modifier.keys) {
      if(ImGui::IsKeyDown(modkey.ik) && !IsVkDown(modkey.vk))
        m_ctx->keyInput(modkey.ik, false);
    }
  }
}
