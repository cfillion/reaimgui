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

#include "settings.hpp"

#include "context.hpp"
#include "window.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h>

Settings::Settings(const char *label)
  : title { label }, m_filename { GetResourcePath() }
{
  m_filename += WDL_DIRCHAR_STR "ReaImGui";
  RecursiveCreateDirectory(m_filename.c_str(), 0);

  const size_t pathSize { m_filename.size() };
  m_filename.resize(pathSize +
    (sizeof(ImGuiID) * 2) + strlen(WDL_DIRCHAR_STR ".ini"));
  snprintf(&m_filename[pathSize], (m_filename.size() - pathSize) + 1,
    WDL_DIRCHAR_STR "%0*X.ini",
    static_cast<int>(sizeof(ImGuiID) * 2), ImHashStr(label));
}

void Settings::update()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.IniFilename = io.ConfigFlags & ReaImGuiConfigFlags_NoSavedSettings ?
    nullptr : m_filename.c_str();
}

RECT Settings::initialRect(const float scale) const
{
  RECT parent, screen;
  if(pos.x == Settings::DEFAULT_POS ||
      pos.y == Settings::DEFAULT_POS) {
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

  if(pos.x == Settings::DEFAULT_POS) {
    // default to the center of the parent window
    const int parentWidth { parent.right - parent.left };
    rect.left = ((parentWidth - scaledWidth) / 2) + parent.left;
    rect.left = std::min(rect.left, screen.right - scaledWidth);
    rect.left = std::max(rect.left, screen.left);
  }
  else
    rect.left = pos.x;

  if(pos.y == Settings::DEFAULT_POS) {
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
