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

#include "platform.hpp"

#include "configvar.hpp"
#include "context.hpp"
#include "error.hpp"
#include "import.hpp"
#include "win32_window.hpp"

#include <imgui/imgui.h>
#include <shellscalingapi.h> // GetProcessDpiAwareness for Windows 8.1

// Windows 10 Anniversary Update (1607) and newer
static FuncImport<decltype(SetThreadDpiAwarenessContext)>
  _SetThreadDpiAwarenessContext
  {L"User32.dll", "SetThreadDpiAwarenessContext"};

static int alwaysallowkb_val;

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
  static std::optional<bool> result;
  if(result)
    return *result;

  // Windows 10 Anniversary Update (1607) and newer
  FuncImport<decltype(GetThreadDpiAwarenessContext)>
    _GetThreadDpiAwarenessContext
    {L"User32.dll", "GetThreadDpiAwarenessContext"};
  FuncImport<decltype(GetAwarenessFromDpiAwarenessContext)>
    _GetAwarenessFromDpiAwarenessContext
    {L"User32.dll", "GetAwarenessFromDpiAwarenessContext"};
  if(_GetThreadDpiAwarenessContext && _GetAwarenessFromDpiAwarenessContext) {
    const DPI_AWARENESS_CONTEXT context {_GetThreadDpiAwarenessContext()};
    const DPI_AWARENESS awareness {_GetAwarenessFromDpiAwarenessContext(context)};
    result = awareness == DPI_AWARENESS_PER_MONITOR_AWARE;
    return *result;
  }

  // Windows 8.1
  FuncImport<decltype(GetProcessDpiAwareness)>
    _GetProcessDpiAwareness {L"Shcore.dll", "GetProcessDpiAwareness"};
  if(_GetProcessDpiAwareness) {
    PROCESS_DPI_AWARENESS awareness {};
    if(FAILED(_GetProcessDpiAwareness(nullptr, &awareness)))
      return false;
    result = awareness == PROCESS_PER_MONITOR_DPI_AWARE;
    return *result;
  }

  result = false;
  return *result;
}

void Platform::install()
{
  ImGuiIO &io {ImGui::GetIO()};
  io.BackendPlatformName = "reaper_imgui_win32";
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new Win32Window {viewport, dockerHost};
}

static int CALLBACK enumMonitors(HMONITOR monitor, HDC, LPRECT, LPARAM)
{
  MONITORINFO info {.cbSize = sizeof(MONITORINFO)};
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
  {
    SetDpiAwareness raii {DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE};
    imguiMonitor.DpiScale = Win32Window::scaleForDpi(Win32Window::dpiForMonitor(monitor));
  }

  const ImVec2 workOffs {imguiMonitor.WorkPos.x  - imguiMonitor.MainPos.x,
                         imguiMonitor.WorkPos.y  - imguiMonitor.MainPos.y};
  imguiMonitor.WorkPos.x = imguiMonitor.MainPos.x + (workOffs.x / imguiMonitor.DpiScale);
  imguiMonitor.WorkPos.y = imguiMonitor.MainPos.y + (workOffs.y / imguiMonitor.DpiScale);
  imguiMonitor.WorkSize.x /= imguiMonitor.DpiScale;
  imguiMonitor.WorkSize.y /= imguiMonitor.DpiScale;

  ImGuiPlatformIO &pio {ImGui::GetPlatformIO()};
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

static HWND windowBehind(HWND candidate, const POINT point)
{
  HWND hit {nullptr};

  while((candidate = GetWindow(candidate, GW_HWNDNEXT))) {
    do {
      RECT clientRect;
      GetClientRect(candidate, &clientRect);
      POINT clientPoint {point};
      ScreenToClient(candidate, &clientPoint);
      if(PtInRect(&clientRect, clientPoint)) {
        hit = candidate;
        if(!(candidate = GetWindow(candidate, GW_CHILD)))
          return hit;
      }
      else
        break;
    } while(candidate);
  }

  return hit;
}

HWND Platform::windowFromPoint(const ImVec2 nativePoint)
{
  const POINT point(nativePoint.x, nativePoint.y);
  HWND hit {WindowFromPoint(point)};

  // 1. Honor WM_NCHITTEST returning HTTRANSPARENT over
  //    non-client areas (native decorations)
  // 2. Make the native titlebar always passthrough to match macOS/Linux
  if(hit && GetWindowLong(hit, GWL_STYLE) & WS_CAPTION) {
    RECT clientRect;
    GetClientRect(hit, &clientRect);
    ClientToScreen(hit, reinterpret_cast<POINT *>(&clientRect));
    ClientToScreen(hit, reinterpret_cast<POINT *>(&clientRect) + 1);
    if(!PtInRect(&clientRect, point))
      hit = windowBehind(hit, point);
  }

  return hit;
}

ImVec2 Platform::getCursorPos()
{
  POINT point;
  GetCursorPos(&point);
  return ImVec2(point.x, point.y);
}

void Platform::scalePosition(ImVec2 *pos, const bool toNative, const ImGuiViewport *viewport)
{
  if(!isPerMonitorDpiAware()) {
    bool isHiDpi {false};
    const ImVector<ImGuiPlatformMonitor> &monitors {ImGui::GetPlatformIO().Monitors};
    for(int i {}; i < monitors.Size; ++i) {
      if(monitors[i].DpiScale != 1.f) {
        isHiDpi = true;
        break;
      }
    }
    if(!isHiDpi)
      return;
    throw backend_error {
      R"(Unsupported HiDPI mode: select "Multimonitor aware v2" in )"
      "Preferences > General > Adavanced UI/system settings."
    };
  }

  const POINT point(pos->x, pos->y);
  HMONITOR monitor;
  HWND window;
  float scale;

  // Make {Monitor,Window}FromPoint use the same coordinate space as the input
  SetDpiAwareness raii {toNative ? DPI_AWARENESS_CONTEXT_UNAWARE : DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE};

  if(viewport)
    window = static_cast<HWND>(viewport->PlatformHandle);
  else {
    window = WindowFromPoint(point);
    if(window && Window::contextFromHwnd(window) != Context::current())
      window = nullptr;
  }

  // Use the monitor and scale of the window under the point so that windows
  // straddling multiple monitors will get correct coordinates everywhere
  if(window) {
    monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);

    if(viewport)
      scale = viewport->DpiScale;
    else {
      SetDpiAwareness aware {DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE};
      scale = Win32Window::scaleForDpi(Win32Window::dpiForWindow(window));
    }
  }
  else {
    monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);

    SetDpiAwareness aware {DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE};
    scale = Win32Window::scaleForDpi(Win32Window::dpiForMonitor(monitor));
  }

  SetDpiAwareness aware {DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE};
  MONITORINFO info {.cbSize = sizeof(MONITORINFO)};
  if(!GetMonitorInfo(monitor, &info))
    return;

  if(!toNative)
    scale = 1.f / scale;

  // Truncate decimals when scaling from native so that converting back and
  // forth to native gives back the same result (native coordinates are integers)
  const ImVec2 diff {pos->x - info.rcMonitor.left, pos->y - info.rcMonitor.top};
  pos->x = info.rcMonitor.left + static_cast<LONG>(diff.x * scale);
  pos->y = info.rcMonitor.top  + static_cast<LONG>(diff.y * scale);
}

float Platform::scaleForWindow(HWND hwnd)
{
  return Win32Window::scaleForDpi(Win32Window::dpiForWindow(hwnd));
}

HCURSOR Platform::getCursor(const ImGuiMouseCursor cur)
{
  if(cur == ImGuiMouseCursor_None)
    return nullptr;

  struct Cursor {
    Cursor(const wchar_t *name) : m_cur {LoadCursor(nullptr, name)} {}
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
  // temporarily overriding "Allow keyboard commands when mouse-editing"
  // to receive VK_MENU key events while the mouse is captured
  ConfigVar<int> alwaysallowkb {"alwaysallowkb"};
  alwaysallowkb_val = *alwaysallowkb;
  *alwaysallowkb = 1;
  SetCapture(hwnd);
}

void Platform::releaseCapture()
{
  *ConfigVar<int> {"alwaysallowkb"} = alwaysallowkb_val;
  ReleaseCapture();
}
