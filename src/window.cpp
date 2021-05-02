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

#include <reaper_plugin_secrets.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell.h>
#  include <WDL/wdltypes.h>
#endif

HINSTANCE Window::s_instance;

RECT WindowConfig::initialRect(const float scale) const
{
  RECT parent, screen;
  if(pos.x == DEFAULT_POS || pos.y == DEFAULT_POS) {
    HWND parentHwnd { Window::parentHandle() };
    GetWindowRect(parentHwnd, &parent);

#ifdef _WIN32
  HMONITOR monitor { MonitorFromWindow(parentHwnd, MONITOR_DEFAULTTONEAREST) };
  MONITORINFO minfo { sizeof(minfo) };
  GetMonitorInfo(monitor, &minfo);
  screen = minfo.rcWork;
#else
  SWELL_GetViewPort(&screen, &parent, true);
#endif
  }

  RECT rect;
  const int scaledWidth  { static_cast<int>(size.x * scale) },
            scaledHeight { static_cast<int>(size.y * scale) };

  if(pos.x == DEFAULT_POS) {
    // default to the center of the parent window
    const int parentWidth { parent.right - parent.left };
    rect.left = ((parentWidth - scaledWidth) / 2) + parent.left;
    rect.left = std::min(rect.left, screen.right - scaledWidth);
    rect.left = std::max(rect.left, screen.left);
  }
  else
    rect.left = pos.x;

  if(pos.y == DEFAULT_POS) {
    const int parentHeight { parent.bottom - parent.top };
    rect.top = ((parentHeight - scaledHeight) / 2) + parent.top;
    rect.top = std::min(rect.top, screen.bottom - scaledHeight);
    rect.top = std::max(rect.top, screen.top);
  }
  else
    rect.top = pos.y;

  rect.right  = rect.left + scaledWidth;
  rect.bottom = rect.top + scaledHeight;

  EnsureNotCompletelyOffscreen(&rect);

  return rect;
}

LRESULT CALLBACK Window::proc(HWND handle, const unsigned int msg,
  const WPARAM wParam, const LPARAM lParam)
{
  Window *self;
#ifdef _WIN32
  if(msg == WM_NCCREATE) {
    void *ptr { reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams };
#else
  if(msg == WM_CREATE) {
    auto &ptr { lParam };
#endif
    self = reinterpret_cast<Window *>(ptr);
    self->m_hwnd.reset(handle);
    SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  else
    self = reinterpret_cast<Window *>(GetWindowLongPtr(handle, GWLP_USERDATA));

  if(!self)
    return DefWindowProc(handle, msg, wParam, lParam);
  else if(const auto &rv { self->handleMessage(msg, wParam, lParam) })
    return *rv;

  switch(msg) {
  case WM_COMMAND: // docker close button sends WM_COMMAND with IDCANCEL
    if(LOWORD(wParam) != IDCANCEL)
      break;
    [[fallthrough]];
  case WM_CLOSE:
    self->m_ctx->setCloseRequested();
    return 0;
  case WM_DESTROY:
    SetWindowLongPtr(handle, GWLP_USERDATA, 0);
    return 0;
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
#ifndef GET_WHEEL_DELTA_WPARAM
#  define GET_WHEEL_DELTA_WPARAM GET_Y_LPARAM
#endif
    self->m_ctx->mouseWheel(msg, GET_WHEEL_DELTA_WPARAM(wParam));
    return 0;
  case WM_SETCURSOR:
    if(LOWORD(lParam) == HTCLIENT) {
      SetCursor(self->m_ctx->cursor()); // sets the cursor when re-entering the window
      return 1;
    }
#ifdef _WIN32
    break; // lets Windows set the cursor over resize handles
#else
    return 1; // tells SWELL to not reset the cursor to IDC_ARROW on mouse events
#endif
#ifndef __APPLE__ // these are handled by InputView, bypassing SWELL
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    SetFocus(handle); // give keyboard focus when docked
    self->m_ctx->mouseDown(msg);
    return 0;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    self->m_ctx->mouseUp(msg);
    return 0;
#endif // __APPLE__
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

void Window::updateKeyMap()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.KeyMap[ImGuiKey_Tab]         = VK_TAB;
  io.KeyMap[ImGuiKey_LeftArrow]   = VK_LEFT;
  io.KeyMap[ImGuiKey_RightArrow]  = VK_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow]     = VK_UP;
  io.KeyMap[ImGuiKey_DownArrow]   = VK_DOWN;
  io.KeyMap[ImGuiKey_PageUp]      = VK_PRIOR;
  io.KeyMap[ImGuiKey_PageDown]    = VK_NEXT;
  io.KeyMap[ImGuiKey_Home]        = VK_HOME;
  io.KeyMap[ImGuiKey_End]         = VK_END;
  io.KeyMap[ImGuiKey_Insert]      = VK_INSERT;
  io.KeyMap[ImGuiKey_Delete]      = VK_DELETE;
  io.KeyMap[ImGuiKey_Backspace]   = VK_BACK;
  io.KeyMap[ImGuiKey_Space]       = VK_SPACE;
  io.KeyMap[ImGuiKey_Enter]       = VK_RETURN;
  io.KeyMap[ImGuiKey_Escape]      = VK_ESCAPE;
  io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
  io.KeyMap[ImGuiKey_A]           = 'A';
  io.KeyMap[ImGuiKey_C]           = 'C';
  io.KeyMap[ImGuiKey_V]           = 'V';
  io.KeyMap[ImGuiKey_X]           = 'X';
  io.KeyMap[ImGuiKey_Y]           = 'Y';
  io.KeyMap[ImGuiKey_Z]           = 'Z';
}

int Window::dock() const
{
  const int dockIndex = { DockIsChildOfDock(m_hwnd.get(), nullptr)  };
  return dockIndex > -1 ? (dockIndex << 1) | 1 : m_cfg.dock & ~1;
}

void Window::setDock(const int dock)
{
  if(dock == m_cfg.dock && IsWindowVisible(m_hwnd.get()))
    return;

  DockWindowRemove(m_hwnd.get());

  if(dock & 1) {
    if(!(m_cfg.dock & 1))
      updateConfig(); // store current undocked position and size (overwrites dock)
    m_cfg.dock = dock;

    constexpr const char *INI_KEY { "reaimgui" };
    Dock_UpdateDockID(INI_KEY, dock >> 1);
    DockWindowAddEx(m_hwnd.get(), m_cfg.title.c_str(), INI_KEY, true);
    DockWindowActivate(m_hwnd.get());
  }
  else {
    m_cfg.dock = dock;
    Window floating { m_cfg, m_ctx };
    std::swap(m_hwnd, floating.m_hwnd);
    std::swap(m_impl, floating.m_impl);
    SetWindowLongPtr(m_hwnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }
}

void Window::updateConfig()
{
  RECT rect;
  GetClientRect(m_hwnd.get(), &rect);
  m_cfg.size.x = rect.right - rect.left;
  m_cfg.size.y = rect.bottom - rect.top;
#ifdef __APPLE__
  std::swap(rect.top, rect.bottom);
#else
  const float scale { scaleFactor() };
  m_cfg.size.x /= scale;
  m_cfg.size.y /= scale;
#endif
  ClientToScreen(m_hwnd.get(), reinterpret_cast<POINT *>(&rect));
  m_cfg.pos.x = rect.left;
  m_cfg.pos.y = rect.top;

  m_cfg.dock = dock();
}

#ifndef _WIN32
void Window::createSwellDialog()
{
  enum SwellDialogResFlags {
    ForceNonChild = 0x400000 | 0x8, // allows not using a resource id
    Resizable = 1,
  };

  const char *res { MAKEINTRESOURCE(ForceNonChild | Resizable) };
  LPARAM param { reinterpret_cast<LPARAM>(this) };
  CreateDialogParam(s_instance, res, parentHandle(), proc, param);
  SetWindowText(m_hwnd.get(), m_cfg.title.c_str());
  AttachWindowTopmostButton(m_hwnd.get());
}
#endif

HWND Window::parentHandle()
{
  return GetMainHwnd();
}

#ifndef __APPLE__
int Window::translateAccel(MSG *msg, accelerator_register_t *accel)
{
  auto *self { static_cast<Window *>(accel->user) };
  if(self->m_hwnd.get() == msg->hwnd)
    return Accel::PassToWindow;
  else
    return Accel::NotOurWindow;
}
#endif

void Window::WindowDeleter::operator()(HWND window)
{
  // Announce to REAPER the window is no longer going to be valid
  // (safe to call even when not docked)
  DockWindowRemove(window);
  DestroyWindow(window);
}
