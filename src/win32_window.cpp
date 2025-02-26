/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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

#include "configvar.hpp"
#include "context.hpp"
#include "error.hpp"
#include "import.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include "win32_droptarget.hpp"
#include "win32_unicode.hpp"

#include <reaper_plugin_secrets.h>
#include <shellscalingapi.h> // GetDpiForMonitor
#define GetThemeColor Win32_GetThemeColor // solve conflict with REAPER API
#include <dwmapi.h> // Dwm* functions for compositing
#undef GetThemeColor

static unsigned int xpScreenDpi()
{
  const HDC dc {GetDC(nullptr)};
  const int dpi {GetDeviceCaps(dc, LOGPIXELSX)};
  ReleaseDC(nullptr, dc);
  return dpi;
}

unsigned int Win32Window::dpiForMonitor(HMONITOR monitor)
{
  // Windows 8.1+
  static FuncImport<decltype(GetDpiForMonitor)>
    _GetDpiForMonitor {L"SHCore.dll", "GetDpiForMonitor"};

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
  static FuncImport<decltype(GetDpiForWindow)>
    _GetDpiForWindow {L"User32.dll", "GetDpiForWindow"};

  if(_GetDpiForWindow)
    return _GetDpiForWindow(window);
  else {
    HMONITOR monitor {MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST)};
    return dpiForMonitor(monitor);
  }
}

float Win32Window::scaleForDpi(const unsigned int dpi)
{
  return static_cast<float>(dpi) / USER_DEFAULT_SCREEN_DPI;
}

// Preferences > General > Advanced UI/system tweaks >
// Use large (non-tool) window frames for windows
static bool useBigFrame()
{
  static const ConfigVar<int> bigwndframes {"bigwndframes"};
  return bigwndframes.value_or(false);
}

void Win32Window::updateStyles()
{
  m_style = WS_POPUP; // fix AttachWindowTopmostButton when a titlebar is shown
  m_exStyle = WS_EX_ACCEPTFILES;

  if(!(m_viewport->Flags & ImGuiViewportFlags_NoDecoration)) {
    m_style |= WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX;
    if(!useBigFrame())
      m_exStyle |= WS_EX_TOOLWINDOW;
  }

  // tooltips & other short-lived windows
  if(m_viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
    m_exStyle |= WS_EX_TOOLWINDOW;

  if(m_viewport->Flags & ImGuiViewportFlags_TopMost)
    m_exStyle |= WS_EX_TOPMOST;
}

static BOOL CALLBACK reparentChildren(HWND hwnd, LPARAM data)
{
  HWND owner {reinterpret_cast<HWND>(data)};
  if(GetWindow(hwnd, GW_OWNER) == owner) {
    SetWindowLongPtr(hwnd, GWLP_HWNDPARENT,
      reinterpret_cast<LONG_PTR>(GetWindow(owner, GW_OWNER)));
  }
  return TRUE;
}

Win32Window::Class::Class()
{
  const WNDCLASS wc {
    .style         = CS_OWNDC,
    .lpfnWndProc   = &Window::proc,
    .hInstance     = Win32Window::s_instance,
    .lpszClassName = CLASS_NAME,
  };
  RegisterClass(&wc);
}

Win32Window::Class::~Class()
{
  UnregisterClass(CLASS_NAME, Win32Window::s_instance);
}

Win32Window::Win32Window(ImGuiViewport *viewport, DockerHost *dockerHost)
  : Window {viewport, dockerHost}
{
}

Win32Window::~Win32Window()
{
}

void Win32Window::create()
{
  static Class windowClass;

  updateStyles();

  // Trick remove the default icon during construction, unset in show()
  DWORD exStyle {m_exStyle};
  if(!(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
    exStyle |= WS_EX_DLGMODALFRAME;

  // Give a sensible window position guess (accurate if no decorations)
  // so that m_dpi gets initialized to the correct value
  // (would be the primary monitor's DPI otherwise, causing scalePosition to be
  // given an incorrect scale and possibly moving the window out of view)
  ImVec2 initialPos {m_viewport->Pos};
  Platform::scalePosition(&initialPos, true);
  CreateWindowEx(exStyle, CLASS_NAME, L"", m_style,
    initialPos.x, initialPos.y, 0, 0,
    parentHandle(), nullptr, s_instance, this);
  if(!m_hwnd)
    throw backend_error {"failed to create native window"};

  m_dpi = dpiForWindow(m_hwnd);
  m_viewport->DpiScale = scaleForDpi(m_dpi);
  const RECT &rect {scaledWindowRect(m_viewport->Pos, m_viewport->Size)};
  SetWindowPos(m_hwnd, nullptr,  rect.left, rect.top,
    rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER);

  try {
    m_renderer = m_ctx->rendererFactory()->create(this);
  }
  catch(const backend_error &) {
    destroy();
    throw;
  }

  // will be freed upon RevokeDragDrop during destruction
  DropTarget *dropTarget = new DropTarget {m_ctx};
  RegisterDragDrop(m_hwnd, dropTarget);

  // disable IME by default
  ImmAssociateContextEx(m_hwnd, nullptr, 0);

  // enable compositing for transparency
  HRGN region {CreateRectRgn(0, 0, -1, -1)};
  const DWM_BLURBEHIND bb {
    .dwFlags  = DWM_BB_ENABLE | DWM_BB_BLURREGION,
    .fEnable  = true,
    .hRgnBlur = region, // no actual blur/shadow
  };
  DwmEnableBlurBehindWindow(m_hwnd, &bb);
  DeleteObject(region);
}

void Win32Window::destroy()
{
  // ImGui destroys windows in creation order. Give ownership of our owned
  // windows to our own owner to avoid a broken chain leading to Windows
  // possibly focusing a window from another application.
  EnumThreadWindows(GetCurrentThreadId(),
    &reparentChildren, reinterpret_cast<LPARAM>(m_hwnd));

  Window::destroy();
}

RECT Win32Window::scaledWindowRect(ImVec2 pos, ImVec2 size) const
{
  const float scale {m_viewport->DpiScale};
  Platform::scalePosition(&pos, true, m_viewport);

  RECT rect;
  rect.left = pos.x;
  rect.top  = pos.y;
  rect.right  = rect.left + (size.x * scale);
  rect.bottom = rect.top  + (size.y * scale);

  // Windows 10 Anniversary Update (1607) and newer
  static FuncImport<decltype(AdjustWindowRectExForDpi)>
    _AdjustWindowRectExForDpi
    {L"User32.dll", "AdjustWindowRectExForDpi"};

  if(_AdjustWindowRectExForDpi)
    _AdjustWindowRectExForDpi(&rect, m_style, false, m_exStyle, m_dpi);
  else
    AdjustWindowRectEx(&rect, m_style, false, m_exStyle);

  return rect;
}

void Win32Window::show()
{
  if(!isDocked() && !(m_viewport->Flags & ImGuiViewportFlags_NoDecoration))
    AttachWindowTopmostButton(m_hwnd);

  if(isDocked())
    Window::show();
  else {
    // prevent ShowWindow from bringing the parent window to the front
    const LONG_PTR parent {SetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT, 0)};
    Window::show();
    SetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT, parent);
  }

  // WS_EX_DLGMODALFRAME removes the default icon but adds a border when docked
  // Unsetting it after the window is visible disables the border (+ no icon)
  const auto exStyle {GetWindowLong(m_hwnd, GWL_EXSTYLE)};
  if(exStyle & WS_EX_DLGMODALFRAME)
    SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, m_exStyle);
}

void Win32Window::setPosition(const ImVec2 pos)
{
  const RECT &rect {scaledWindowRect(pos, m_viewport->Size)};
  SetWindowPos(m_hwnd, nullptr,
    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void Win32Window::setSize(const ImVec2 size)
{
  const RECT &rect {scaledWindowRect(m_viewport->Pos, size)};
  SetWindowPos(m_hwnd, nullptr,
    0, 0, rect.right - rect.left, rect.bottom - rect.top,
    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void Win32Window::setTitle(const char *title)
{
  SetWindowText(m_hwnd, widen(title).c_str());
}

void Win32Window::setAlpha(const float alpha)
{
  // may conflict with updateStyles()/update(), but viewport flags are
  // unlikely to change while alpha isn't 1

  if(alpha == 1.0f) {
    SetWindowLong(m_hwnd, GWL_EXSTYLE, m_exStyle);
    return;
  }

  SetWindowLong(m_hwnd, GWL_EXSTYLE, m_exStyle | WS_EX_LAYERED);
  SetLayeredWindowAttributes(m_hwnd, 0, 255 * alpha, LWA_ALPHA);
}

void Win32Window::update()
{
  unstuckModifiers();

  if(isDocked())
    return;

  const DWORD prevStyle {m_style}, prevExStyle {m_exStyle};
  updateStyles();

  if(prevStyle != m_style || prevExStyle != m_exStyle) {
    SetWindowLong(m_hwnd, GWL_STYLE, m_style);
    SetWindowLong(m_hwnd, GWL_EXSTYLE, m_exStyle);

    if(m_viewport->Flags & ImGuiViewportFlags_NoDecoration)
      DetachWindowTopmostButton(m_hwnd, false);
    else
      AttachWindowTopmostButton(m_hwnd);

    HWND insertAfter;
    unsigned int flags {SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW};
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

    const RECT rect {scaledWindowRect(m_viewport->Pos, m_viewport->Size)};
    SetWindowPos(m_hwnd, insertAfter, rect.left, rect.top,
      rect.right - rect.left, rect.bottom - rect.top, flags);
    m_viewport->PlatformRequestMove = m_viewport->PlatformRequestResize = true;
  }
}

float Win32Window::scaleFactor() const
{
  return m_viewport->DpiScale;
}

void Win32Window::setIME(ImGuiPlatformImeData *data)
{
  ImmAssociateContextEx(m_hwnd, nullptr, data->WantVisible ? IACE_DEFAULT : 0);

  if(HIMC ime {ImmGetContext(m_hwnd)}) {
    ImVec2 pos {data->InputPos};
    Platform::scalePosition(&pos, true);

    COMPOSITIONFORM composition;
    composition.dwStyle = CFS_FORCE_POSITION;
    composition.ptCurrentPos.x = pos.x;
    composition.ptCurrentPos.y = pos.y;
    ScreenToClient(m_hwnd, &composition.ptCurrentPos);
    ImmSetCompositionWindow(ime, &composition);

    CANDIDATEFORM candidate;
    candidate.dwStyle = CFS_CANDIDATEPOS;
    candidate.ptCurrentPos = composition.ptCurrentPos;
    ImmSetCandidateWindow(ime, &candidate);

    ImmReleaseContext(m_hwnd, ime);
  }
}

std::optional<LRESULT> Win32Window::handleMessage
  (const unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
  case WM_NCCREATE: {
    // Windows 10 Anniversary Update (1607) and newer
    static FuncImport<decltype(EnableNonClientDpiScaling)>
      _EnableNonClientDpiScaling {L"User32.dll", "EnableNonClientDpiScaling"};
    if(_EnableNonClientDpiScaling)
      _EnableNonClientDpiScaling(m_hwnd);
    break;
  }
  case WM_ACTIVATEAPP:
    if(m_viewport->Flags & ImGuiViewportFlags_TopMost) {
      SetWindowPos(m_hwnd, wParam ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE);
    }
    break;
  case WM_MOUSEACTIVATE:
    if(m_viewport->Flags & ImGuiViewportFlags_NoFocusOnClick)
      return MA_NOACTIVATE;
    break;
  case WM_DPICHANGED: {
    m_dpi = LOWORD(wParam);
    m_viewport->DpiScale = scaleForDpi(m_dpi);

    const RECT *sugg {reinterpret_cast<RECT *>(lParam)};
    SetWindowPos(m_hwnd, nullptr,
      sugg->left, sugg->top, sugg->right - sugg->left, sugg->bottom - sugg->top,
      SWP_NOACTIVATE | SWP_NOZORDER);

    // PlatformRequestResize doesn't work here to tell ImGui to fetch the new size
    m_viewport->Pos  = getPosition();
    m_viewport->Size = getSize();

    // imgui won't call this if unscaled size didn't change
    if(m_renderer)
      m_renderer->setSize(m_viewport->Size);
    return 0;
  }
  case WM_DPICHANGED_BEFOREPARENT:
    // This message is sent when docked.
    // Only top-level windows receive WM_DPICHANGED.
    m_dpi = dpiForWindow(m_hwnd);
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
    if(wParam < 0 || wParam > 0xffff)
      break;

    // https://learn.microsoft.com/en-us/windows/win32/inputdev/using-keyboard-input#processing-character-messages
    switch(wParam) {
    case '\b':
    case 0x1B: // \e
    case '\r':
    case '\n':
    case '\t':
      break;
    default:
      m_ctx->charInputUTF16(wParam);
      return 0;
    }
  case WM_KILLFOCUS:
    m_ctx->updateFocus();
    return 0;
  case WM_DESTROY:
    RevokeDragDrop(m_hwnd);
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

  const bool down {msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN};

  // Windows doesn't send a WM_KEYDOWN for Print Screen
  if(vk == VK_SNAPSHOT && !down)
    m_ctx->keyInput(ImGuiKey_PrintScreen, true);

  if(vk == VK_RETURN && (HIWORD(lParam) & KF_EXTENDED))
    m_ctx->keyInput(ImGuiKey_KeypadEnter, down);
  else if(!modKeyEvent(vk, down))
    m_ctx->keyInput(static_cast<ImGuiKey>(vk), down);
}

struct ModKey { uint8_t vk; ImGuiKey ik; };
struct Modifier { uint8_t modVK; ImGuiKey modKey; ModKey keys[2]; };
constexpr Modifier modifiers[] {
  {VK_CONTROL, ImGuiMod_Ctrl, {
    {VK_LCONTROL, ImGuiKey_LeftCtrl }, {VK_RCONTROL, ImGuiKey_RightCtrl},
  }},
  {VK_SHIFT, ImGuiMod_Shift, {
    {VK_LSHIFT, ImGuiKey_LeftShift }, {VK_RSHIFT, ImGuiKey_RightShift},
  }},
  {VK_MENU, ImGuiMod_Alt, {
    {VK_LMENU, ImGuiKey_LeftAlt }, {VK_RMENU, ImGuiKey_RightAlt},
  }},
  {VK_LWIN, ImGuiMod_Super, {
    {VK_LWIN, ImGuiKey_LeftSuper }, {VK_RWIN, ImGuiKey_RightSuper},
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
