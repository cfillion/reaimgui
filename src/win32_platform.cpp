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

#include "platform.hpp"

#include "opengl_renderer.hpp"
#include "win32_window.hpp"

#include <imgui/imgui.h>

void Platform::install()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.BackendPlatformName = "reaper_imgui_win32";

  OpenGLRenderer::install();
  Window::install();
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new Win32Window { viewport, dockerHost };
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
  if(viewport && viewport->PlatformUserData)
    return viewport;

  return nullptr;
}

void Platform::translatePosition(ImVec2 *, bool toHiDpi)
{
}
