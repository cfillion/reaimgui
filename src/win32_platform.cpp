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

#include "import.hpp"
#include "win32_window.hpp"

#include <imgui/imgui.h>

// Windows 10 Anniversary Update (1607) and newer
static FuncImport<decltype(SetThreadDpiAwarenessContext)>
  _SetThreadDpiAwarenessContext
  { L"User32.dll", "SetThreadDpiAwarenessContext" };

class SetDpiAwareness {
public:
  SetDpiAwareness(const DPI_AWARENESS_CONTEXT awareness)
  {
    if(_SetThreadDpiAwarenessContext)
      m_prev = _SetThreadDpiAwarenessContext(awareness);
  }

  ~SetDpiAwareness()
  {
    if(_SetThreadDpiAwarenessContext)
      _SetThreadDpiAwarenessContext(m_prev);
  }

private:
  DPI_AWARENESS_CONTEXT m_prev;
};

static bool isPerMonitorDpiAware()
{
  // Windows 10 Anniversary Update (1607) and newer
  static FuncImport<decltype(GetThreadDpiAwarenessContext)>
    _GetThreadDpiAwarenessContext
    { L"User32.dll", "GetThreadDpiAwarenessContext" };
  static FuncImport<decltype(GetAwarenessFromDpiAwarenessContext)>
    _GetAwarenessFromDpiAwarenessContext
    { L"User32.dll", "GetAwarenessFromDpiAwarenessContext" };

  if(!_GetThreadDpiAwarenessContext || !_GetAwarenessFromDpiAwarenessContext)
    return false;

  const DPI_AWARENESS_CONTEXT context { _GetThreadDpiAwarenessContext() };
  const DPI_AWARENESS awareness { _GetAwarenessFromDpiAwarenessContext(context) };
  return awareness == DPI_AWARENESS_PER_MONITOR_AWARE;
}

void Platform::install()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_win32";
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new Win32Window { viewport, dockerHost };
}

static int CALLBACK enumMonitors(HMONITOR monitor, HDC, LPRECT, LPARAM)
{
  ImGuiPlatformMonitor imguiMonitor;
  MONITORINFO info { .cbSize = sizeof(MONITORINFO) };

  {
    // get full monitor size for imgui's FindPlatformMonitorFor{Pos,Rect}
    // required for ClampWindowRect to use the correct monitor's work area
    SetDpiAwareness raii { DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE };
    if(!GetMonitorInfo(monitor, &info))
      return true;
  }

  imguiMonitor.MainPos.x  = info.rcMonitor.left;
  imguiMonitor.MainPos.y  = info.rcMonitor.top;
  imguiMonitor.MainSize.x = info.rcMonitor.right - info.rcMonitor.left;
  imguiMonitor.MainSize.y = info.rcMonitor.bottom - info.rcMonitor.top;
  imguiMonitor.DpiScale   = Win32Window::scaleForDpi(Win32Window::dpiForMonitor(monitor));

  if(isPerMonitorDpiAware()) {
    // unscale the work area (used by imgui for clamping)
    SetDpiAwareness raii { DPI_AWARENESS_CONTEXT_UNAWARE };
    GetMonitorInfo(monitor, &info);
  }
  imguiMonitor.WorkPos.x  = info.rcWork.left;
  imguiMonitor.WorkPos.y  = info.rcWork.top;
  imguiMonitor.WorkSize.x = info.rcWork.right - info.rcWork.left;
  imguiMonitor.WorkSize.y = info.rcWork.bottom - info.rcWork.top;

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
  HWND target { WindowFromPoint(POINT(pos.x, pos.y)) };

  ImGuiViewport *viewport { ImGui::FindViewportByPlatformHandle(target) };
  if(viewport && ImGui::GetMainViewport() != viewport)
    return viewport;

  return nullptr;
}

void Platform::scalePosition(ImVec2 *pos, const bool toHiDpi, float scale)
{
  if(!isPerMonitorDpiAware())
    return;

  HMONITOR monitor { MonitorFromPoint(POINT(pos->x, pos->y), MONITOR_DEFAULTTONEAREST) };

  if(!scale)
    scale = Win32Window::scaleForDpi(Win32Window::dpiForMonitor(monitor));
  if(scale == 1.f)
    return;
  if(!toHiDpi)
    scale = 1.f / scale;

  MONITORINFO info { .cbSize = sizeof(MONITORINFO) };
  if(!GetMonitorInfo(monitor, &info))
    return;

  const ImVec2 diff { pos->x - info.rcMonitor.left, pos->y - info.rcMonitor.top };
  pos->x = info.rcMonitor.left + (diff.x * scale);
  pos->y = info.rcMonitor.top  + (diff.y * scale);
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

HWND Platform::getCapture()
{
  return GetCapture();
}

void Platform::setCapture(HWND hwnd)
{
  SetCapture(hwnd);
}

void Platform::releaseCapture()
{
  ReleaseCapture();
}
