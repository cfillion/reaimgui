/* ReaImGui: ReaScript binding for dear imgui
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

#include <reaper_plugin_functions.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell.h>
#  include <WDL/wdltypes.h>
#endif

HINSTANCE Window::s_instance;

LRESULT CALLBACK Window::proc(HWND handle, const unsigned int msg,
  const WPARAM wParam, const LPARAM lParam)
{
  Context *ctx {
    reinterpret_cast<Context *>(GetWindowLongPtr(handle, GWLP_USERDATA))
  };

  if(!ctx || !ctx->window())
    return DefWindowProc(handle, msg, wParam, lParam);
  else if(ctx->window()->handleMessage(msg, wParam, lParam))
    return 0;

  switch(msg) {
  case WM_CLOSE:
    ctx->setCloseRequested();
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
    break;
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
  return dialog;
}
#endif

HWND Window::parentHandle()
{
  return GetMainHwnd();
}

static RECT getAvailableRect(HWND window)
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
  SWELL_GetViewPort(&screenRect, &rect, false);
#endif

  // limit the centering to the monitor containing most of the parent window
  rect.left   = std::max(rect.left,   rect.left);
  rect.top    = std::max(rect.top,    rect.top);
  rect.right  = std::min(rect.right,  rect.right);
  rect.bottom = std::min(rect.bottom, rect.bottom);

  return rect;
}

int Window::centerX(const int width)
{
  const RECT &parentRect { getAvailableRect(parentHandle()) };
  const int parentWidth { parentRect.right - parentRect.left };
  return ((parentWidth - width) / 2) + parentRect.left;
}

int Window::centerY(const int height)
{
  const RECT &parentRect { getAvailableRect(parentHandle()) };
  const int parentHeight { parentRect.bottom - parentRect.top };
  return ((parentHeight - height) / 2) + parentRect.top;
}

void Window::WindowDeleter::operator()(HWND window)
{
  DestroyWindow(window);
}
