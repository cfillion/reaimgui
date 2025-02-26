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

#ifndef REAIMGUI_PLATFORM_HPP
#define REAIMGUI_PLATFORM_HPP

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class DockerHost;
class Window;
struct ImGuiViewport;
struct ImVec2;
typedef int ImGuiMouseCursor;

namespace Platform {
  void install();
  Window *createWindow(ImGuiViewport *, DockerHost * = nullptr);
  void updateMonitors();
  HWND windowFromPoint(ImVec2 nativePoint);
  ImVec2 getCursorPos(); // in native coordinates
  void scalePosition(ImVec2 *, bool toHiDpi = false, const ImGuiViewport * = nullptr);
  float scaleForWindow(HWND);
  HCURSOR getCursor(ImGuiMouseCursor);
  HWND getCapture();
  void setCapture(HWND);
  void releaseCapture();
};

#endif
