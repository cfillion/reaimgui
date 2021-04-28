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

static RECT visibleRect(HWND window)
{
  RECT rect;
  GetWindowRect(window, &rect);

#ifdef _WIN32
  HMONITOR monitor { MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST) };
  MONITORINFO minfo { sizeof(minfo) };
  GetMonitorInfo(monitor, &minfo);
  RECT &screenRect { minfo.rcWork };
#else
  RECT screenRect;
  SWELL_GetViewPort(&screenRect, &rect, true);
#endif

  rect.left   = std::max(rect.left,   screenRect.left);
  rect.top    = std::max(rect.top,    screenRect.top);
  rect.right  = std::min(rect.right,  screenRect.right);
  rect.bottom = std::min(rect.bottom, screenRect.bottom);

  return rect;
}

RECT WindowConfig::clientRect(const float scale) const
{
  RECT parentRect;
  if(!x || !y)
    parentRect = visibleRect(Window::parentHandle());

  RECT rect;
  const int scaledWidth  { static_cast<int>(w * scale) },
            scaledHeight { static_cast<int>(h * scale) };

  if(x)
    rect.left = *x;
  else {
    // default to the center of the parent window
    const int parentWidth { parentRect.right - parentRect.left };
    rect.left = ((parentWidth - scaledWidth) / 2) + parentRect.left;
  }

  if(y)
    rect.top = *y;
  else {
    const int parentHeight { parentRect.bottom - parentRect.top };
    rect.top = ((parentHeight - scaledHeight) / 2) + parentRect.top;
  }

  rect.right  = rect.left + scaledWidth;
  rect.bottom = rect.top + scaledHeight;

  return rect;
}

LRESULT CALLBACK Window::proc(HWND handle, const unsigned int msg,
  const WPARAM wParam, const LPARAM lParam)
{
  Context *ctx {
    reinterpret_cast<Context *>(GetWindowLongPtr(handle, GWLP_USERDATA))
  };

  if(!ctx || !ctx->window())
    return DefWindowProc(handle, msg, wParam, lParam);
  else if(const auto &rv { ctx->window()->handleMessage(msg, wParam, lParam) })
    return *rv;

  switch(msg) {
  case WM_COMMAND: // docker close button sends WM_COMMAND with IDCANCEL
    if(LOWORD(wParam) != IDCANCEL)
      break;
    [[fallthrough]];
  case WM_CLOSE:
    ctx->setCloseRequested();
    if(ctx->frozen()) // let users kill frozen contexts
      delete ctx;
    return 0;
  case WM_DESTROY:
    SetWindowLongPtr(handle, GWLP_USERDATA, 0);
    return 0;
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
#ifndef GET_WHEEL_DELTA_WPARAM
#  define GET_WHEEL_DELTA_WPARAM GET_Y_LPARAM
#endif
    ctx->mouseWheel(msg, GET_WHEEL_DELTA_WPARAM(wParam));
    return 0;
  case WM_SETCURSOR:
    if(LOWORD(lParam) == HTCLIENT) {
      SetCursor(ctx->cursor()); // sets the cursor when re-entering the window
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
    ctx->mouseDown(msg);
    return 0;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    ctx->mouseUp(msg);
    return 0;
#endif // __APPLE__
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

int Window::dock() const
{
  const int dockIndex = { DockIsChildOfDock(nativeHandle(), nullptr)  };
  return dockIndex > -1 ? (dockIndex << 1) | 1 : m_cfg.dock & ~1;
}

void Window::setDock(const int dock)
{
  HWND hwnd { nativeHandle() };

  if(dock == m_cfg.dock && IsWindowVisible(hwnd))
    return;

  DockWindowRemove(hwnd);

  if(dock & 1) {
    if(!(m_cfg.dock & 1))
      updateConfig(); // store current undocked position and size (overwrites dock)
    m_cfg.dock = dock;

    constexpr const char *INI_KEY { "reaimgui" };
    Dock_UpdateDockID(INI_KEY, dock >> 1);
    DockWindowAddEx(hwnd, m_cfg.title.c_str(), INI_KEY, true);
    DockWindowActivate(hwnd);
  }
  else {
    m_cfg.dock = dock;
    Window floating { m_cfg, m_ctx };
    std::swap(m_impl, floating.m_impl);
  }
}

void Window::updateConfig()
{
  HWND hwnd { nativeHandle() };

  RECT rect;
  GetClientRect(hwnd, &rect);
  m_cfg.w = rect.right - rect.left;
  m_cfg.h = rect.bottom - rect.top;
#ifdef __APPLE__
  std::swap(rect.top, rect.bottom);
#endif
  ClientToScreen(hwnd, reinterpret_cast<POINT *>(&rect));
  m_cfg.x = rect.left;
  m_cfg.y = rect.top;

  m_cfg.dock = dock();
}

#ifndef _WIN32
HWND Window::createSwellDialog(const char *title)
{
  enum SwellDialogResFlags {
    ForceNonChild = 0x400000 | 0x8, // allows not using a resource id
    Resizable = 1,
  };

  const char *res { MAKEINTRESOURCE(ForceNonChild | Resizable) };
  HWND dialog { CreateDialog(s_instance, res, parentHandle(), proc) };
  SetWindowText(dialog, title);
  AttachWindowTopmostButton(dialog);
  return dialog;
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
  if(self->nativeHandle() == msg->hwnd)
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
