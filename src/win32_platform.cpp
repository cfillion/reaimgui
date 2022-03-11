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

#include "platform.hpp"

#include "dllimport.hpp"
#include "opengl_renderer.hpp"
#include "win32_window.hpp"

#include <imgui/imgui.h>

// Windows 10 Anniversary Update (1607) and newer
static DllImport<decltype(SetThreadDpiAwarenessContext)>
  _SetThreadDpiAwarenessContext
  { L"User32.dll", "SetThreadDpiAwarenessContext" };

class DisableDpiAwareness {
public:
  DisableDpiAwareness()
  {
    if(_SetThreadDpiAwarenessContext)
      m_prev = _SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
  }

  ~DisableDpiAwareness()
  {
    if(_SetThreadDpiAwarenessContext)
      _SetThreadDpiAwarenessContext(m_prev);
  }

private:
  DPI_AWARENESS_CONTEXT m_prev;
};

void Platform::install()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_win32";

  OpenGLRenderer::install();
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new Win32Window { viewport, dockerHost };
}

static int CALLBACK enumMonitors(HMONITOR monitor, HDC, LPRECT, LPARAM)
{
  MONITORINFO info {};
  info.cbSize = sizeof(MONITORINFO);

  { // scope for disabled DPI awareness (to still get a correct DpiScale below)
    DisableDpiAwareness raii;
    if(!GetMonitorInfo(monitor, &info))
      return true;
  }

  ImGuiPlatformMonitor imguiMonitor;
  imguiMonitor.MainPos.x  = info.rcMonitor.left;
  imguiMonitor.MainPos.y  = info.rcMonitor.top;
  imguiMonitor.MainSize.x = info.rcMonitor.right - info.rcMonitor.left;
  imguiMonitor.MainSize.y = info.rcMonitor.bottom - info.rcMonitor.top;
  imguiMonitor.WorkPos.x  = info.rcWork.left;
  imguiMonitor.WorkPos.y  = info.rcWork.top;
  imguiMonitor.WorkSize.x = info.rcWork.right - info.rcWork.left;
  imguiMonitor.WorkSize.y = info.rcWork.bottom - info.rcWork.top;
  imguiMonitor.DpiScale   = Win32Window::scaleForDpi(Win32Window::dpiForMonitor(monitor));

  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  if(info.dwFlags & MONITORINFOF_PRIMARY)
    pio.Monitors.push_front(imguiMonitor);
  else
    pio.Monitors.push_back(imguiMonitor);

  return true;
}

void Platform::updateMonitors()
{
  ImGui::GetPlatformIO().Monitors.resize(0);
  EnumDisplayMonitors(nullptr, nullptr, enumMonitors, 0);
}

ImGuiViewport *Platform::viewportUnder(const ImVec2 pos)
{
  POINT point;
  point.x = pos.x;
  point.y = pos.y;

  HWND target { WindowFromPoint(point) };

  ImGuiViewport *viewport { ImGui::FindViewportByPlatformHandle(target) };
  if(viewport && ImGui::GetMainViewport() != viewport)
    return viewport;

  return nullptr;
}

void Platform::scalePosition(ImVec2 *pos, bool toHiDpi)
{
  POINT point;
  point.x = pos->x;
  point.y = pos->y;

  HMONITOR monitor { MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST) };
  float scale = Win32Window::scaleForDpi(Win32Window::dpiForMonitor(monitor));
  if(!toHiDpi)
    scale = 1.f / scale;

  MONITORINFO info {};
  info.cbSize = sizeof(MONITORINFO);
  if(!GetMonitorInfo(monitor, &info))
    return;

  const float diffX { pos->x - info.rcMonitor.left },
              diffY { pos->y - info.rcMonitor.top  };

  { // scope for disabled DPI awareness
    DisableDpiAwareness raii;
    if(!GetMonitorInfo(monitor, &info))
      return;
  }

  pos->x = info.rcMonitor.left + (diffX * scale);
  pos->y = info.rcMonitor.top  + (diffY * scale);
}

float Platform::scaleForWindow(HWND hwnd)
{
  return Win32Window::scaleForDpi(Win32Window::dpiForWindow(hwnd));
}

HCURSOR Platform::getCursor(const ImGuiMouseCursor cur)
{
  struct Cursor {
    Cursor(const wchar_t *name) : m_cur { LoadCursor(nullptr, name) } {}
    operator HCURSOR() const { return m_cur; }
    HCURSOR m_cur;
  };

  static const Cursor cursors[ImGuiMouseCursor_COUNT] {
    IDC_ARROW, IDC_IBEAM, IDC_SIZEALL, IDC_SIZENS, IDC_SIZEWE, IDC_SIZENESW,
    IDC_SIZENWSE, IDC_HAND, IDC_NO,
  };

  return cursors[cur];
}
