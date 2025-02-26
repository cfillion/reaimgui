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

#include "gdk_window.hpp"

#include <gdk/gdk.h>
#include <imgui/imgui.h>

// using RegisterClipboardFormat instead of CF_TEXT for compatibility with REAPER v5
// (prior to WDL commit 0f77b72adf1cdbe98fd56feb41eb097a8fac5681)
#undef CF_TEXT
#define CF_TEXT RegisterClipboardFormat("SWELL__CF_TEXT")

static const char *getClipboardText(void *)
{
  static std::string text;

  OpenClipboard(nullptr);
  if(HANDLE mem {GetClipboardData(CF_TEXT)}) {
    text = static_cast<const char *>(GlobalLock(mem));
    GlobalUnlock(mem);
  }
  else
    text.clear();
  CloseClipboard();

  return text.c_str();
}

static void setClipboardText(void *, const char *text)
{
  const size_t size {strlen(text) + 1};
  HANDLE mem {GlobalAlloc(GMEM_MOVEABLE, size)};
  memcpy(GlobalLock(mem), text, size);
  GlobalUnlock(mem);

  OpenClipboard(nullptr);
  EmptyClipboard();
  SetClipboardData(CF_TEXT, mem);
  CloseClipboard();
}

void Platform::install()
{
  ImGuiIO &io {ImGui::GetIO()};
  io.BackendPlatformName = "reaper_imgui_gdk";
  io.GetClipboardTextFn = &getClipboardText;
  io.SetClipboardTextFn = &setClipboardText;
}

Window *Platform::createWindow(ImGuiViewport *viewport, DockerHost *dockerHost)
{
  return new GDKWindow {viewport, dockerHost};
}

void Platform::updateMonitors()
{
  ImGuiPlatformIO &pio {ImGui::GetPlatformIO()};
  pio.Monitors.resize(0); // recycle allocated memory (don't use clear here!)

  GdkDisplay *display {gdk_display_get_default()};

  const int count {gdk_display_get_n_monitors(display)};
  for(int i {}; i < count; ++i) {
    GdkMonitor *monitor {gdk_display_get_monitor(display, i)};

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

HWND Platform::windowFromPoint(const ImVec2 nativePoint)
{
  POINT point;
  point.x = nativePoint.x;
  point.y = nativePoint.y;

  HWND window {WindowFromPoint(point)};

  if(window && HTTRANSPARENT ==
      SendMessage(window, WM_NCHITTEST, 0, MAKELPARAM(point.x, point.y))) {
    // For compatibility when running v6.73 or older (SWELL prior to da86a62)
    //
    // Trick WindowFromPoint into skipping this window by overwriting
    // HWND::m_visible. Storing the original value as a sanity-check
    // in case the offset isn't always valid in odd configurations.
    char *visible {reinterpret_cast<char *>(window) + 0x2b0};
    const char originalValue {*visible};
    *visible = 0;
    window = WindowFromPoint(point);
    *visible = originalValue;
  }

  return window;
}

ImVec2 Platform::getCursorPos()
{
  POINT point;
  GetCursorPos(&point);
  return ImVec2(point.x, point.y);
}

void Platform::scalePosition(ImVec2 *pos, const bool toHiDpi, const ImGuiViewport *)
{
  float scale {GDKWindow::globalScaleFactor()};
  if(!toHiDpi)
    scale = 1.f / scale;

  // Truncate decimals when scaling from native so that converting back and
  // forth to native gives back the same result (native coordinates are integers)
  pos->x = static_cast<long>(pos->x * scale);
  pos->y = static_cast<long>(pos->y * scale);
}

float Platform::scaleForWindow(HWND hwnd)
{
  return GDKWindow::globalScaleFactor();
}

HCURSOR Platform::getCursor(const ImGuiMouseCursor cur)
{
  struct Cursor {
    Cursor(const GdkCursorType type)
      : m_cur {gdk_cursor_new_for_display(gdk_display_get_default(), type)} {}
    operator HCURSOR() const { return reinterpret_cast<HCURSOR>(m_cur); }
    GdkCursor *m_cur;
  };

  static const Cursor cursors[ImGuiMouseCursor_COUNT + 1] {
    GDK_BLANK_CURSOR,
    GDK_ARROW, GDK_XTERM, GDK_FLEUR, GDK_TOP_SIDE, GDK_RIGHT_SIDE,
    GDK_BOTTOM_LEFT_CORNER, GDK_BOTTOM_RIGHT_CORNER, GDK_HAND1, GDK_PIRATE,
  };

  return cursors[cur + 1]; // ImGuiMouseCursor_None is -1, shift to 0
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
