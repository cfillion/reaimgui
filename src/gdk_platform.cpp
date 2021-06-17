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

#include "gdk_window.hpp"
#include "opengl_renderer.hpp"

#include <gdk/gdk.h>
#include <imgui/imgui.h>

void Platform::install()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.BackendFlags &= ~ImGuiBackendFlags_HasMouseHoveredViewport;
  io.BackendPlatformName = "reaper_imgui_gdk";

  OpenGLRenderer::install();
  Window::install();
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new GDKWindow { viewport, dockerHost };
}

void Platform::updateMonitors()
{
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  pio.Monitors.resize(0); // recycle allocated memory (don't use clear here!)

  GdkDisplay *display { gdk_display_get_default() };

  const int count { gdk_display_get_n_monitors(display) };
  for(int i {}; i < count; ++i) {
    GdkMonitor *monitor { gdk_display_get_monitor(display, i) };

    GdkRectangle geometry, workArea;
    gdk_monitor_get_geometry(monitor, &geometry);
    gdk_monitor_get_workarea(monitor, &workArea);

    ImGuiPlatformMonitor imguiMonitor;
    imguiMonitor.MainPos.x  = geometry.x;
    imguiMonitor.MainPos.y  = geometry.y;
    imguiMonitor.MainSize.x = geometry.width;
    imguiMonitor.MainSize.y = geometry.height;
    imguiMonitor.WorkPos.x  = workArea.x;
    imguiMonitor.WorkPos.y  = workArea.y;
    imguiMonitor.WorkSize.x = workArea.width;
    imguiMonitor.WorkSize.y = workArea.height;
    imguiMonitor.DpiScale   = gdk_monitor_get_scale_factor(monitor);

    scalePosition(&imguiMonitor.MainPos);
    scalePosition(&imguiMonitor.MainSize);
    scalePosition(&imguiMonitor.WorkPos);
    scalePosition(&imguiMonitor.WorkSize);

    if(gdk_monitor_is_primary(monitor))
      pio.Monitors.push_front(imguiMonitor);
    else
      pio.Monitors.push_back(imguiMonitor);
  }
}

ImGuiViewport *Platform::viewportUnder(const ImVec2 pos)
{
  // FIXME: SWELL does not support HTTRANSPARENT or hit testing that
  // would be required for implementing ImGui's MouseHoveredViewport

  POINT point;
  point.x = pos.x;
  point.y = pos.y;

  HWND target { WindowFromPoint(point) };

  ImGuiViewport *viewport { ImGui::FindViewportByPlatformHandle(target) };
  if(viewport && viewport->PlatformUserData)
    return viewport;

  return nullptr;
}

void Platform::scalePosition(ImVec2 *pos, const bool toHiDpi)
{
  float scale { GDKWindow::globalScaleFactor() };
  if(!toHiDpi)
    scale = 1.f / scale;

  pos->x *= scale;
  pos->y *= scale;
}
