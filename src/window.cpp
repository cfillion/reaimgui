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

#ifndef _WIN32
#  include <swell/swell.h>
#  include <WDL/wdltypes.h>
#endif

HINSTANCE Window::s_instance;

LRESULT CALLBACK Window::proc(HWND handle, const unsigned int msg,
  const WPARAM wParam, const LPARAM lParam)
{
  Window *self;
#ifdef _WIN32
  if(msg == WM_NCCREATE) {
    void *ptr { reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams };
#else
  if(msg == WM_CREATE) {
    auto &ptr { lParam };
#endif
    self = reinterpret_cast<Window *>(ptr);
    self->m_hwnd.reset(handle);
    SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  else
    self = reinterpret_cast<Window *>(GetWindowLongPtr(handle, GWLP_USERDATA));

  if(!self)
    return DefWindowProc(handle, msg, wParam, lParam);
  else if(const auto &rv { self->handleMessage(msg, wParam, lParam) })
    return *rv;

  switch(msg) {
  case WM_COMMAND: // docker close button sends WM_COMMAND with IDCANCEL
    if(LOWORD(wParam) != IDCANCEL)
      break;
    [[fallthrough]];
  case WM_CLOSE:
    self->m_ctx->setCloseRequested();
    return 0;
  case WM_MOVE:
  case WM_SIZE:
    if(self->m_ctx->window() == self) // only after window construction is over
      self->updateSettings();
    return 0;
  case WM_DESTROY:
    SetWindowLongPtr(handle, GWLP_USERDATA, 0);
    return 0;
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
#ifndef GET_WHEEL_DELTA_WPARAM
#  define GET_WHEEL_DELTA_WPARAM GET_Y_LPARAM
#endif
    self->m_ctx->mouseWheel(msg, GET_WHEEL_DELTA_WPARAM(wParam));
    return 0;
  case WM_SETCURSOR:
    if(LOWORD(lParam) == HTCLIENT) {
      SetCursor(self->m_ctx->cursor()); // sets the cursor when re-entering the window
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
    self->m_ctx->mouseDown(msg);
    return 0;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    self->m_ctx->mouseUp(msg);
    return 0;
#endif // __APPLE__
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

void Window::updateKeyMap()
{
  ImGuiIO &io { ImGui::GetIO() };
  io.KeyMap[ImGuiKey_Tab]         = VK_TAB;
  io.KeyMap[ImGuiKey_LeftArrow]   = VK_LEFT;
  io.KeyMap[ImGuiKey_RightArrow]  = VK_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow]     = VK_UP;
  io.KeyMap[ImGuiKey_DownArrow]   = VK_DOWN;
  io.KeyMap[ImGuiKey_PageUp]      = VK_PRIOR;
  io.KeyMap[ImGuiKey_PageDown]    = VK_NEXT;
  io.KeyMap[ImGuiKey_Home]        = VK_HOME;
  io.KeyMap[ImGuiKey_End]         = VK_END;
  io.KeyMap[ImGuiKey_Insert]      = VK_INSERT;
  io.KeyMap[ImGuiKey_Delete]      = VK_DELETE;
  io.KeyMap[ImGuiKey_Backspace]   = VK_BACK;
  io.KeyMap[ImGuiKey_Space]       = VK_SPACE;
  io.KeyMap[ImGuiKey_Enter]       = VK_RETURN;
  io.KeyMap[ImGuiKey_Escape]      = VK_ESCAPE;
  io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
  io.KeyMap[ImGuiKey_A]           = 'A';
  io.KeyMap[ImGuiKey_C]           = 'C';
  io.KeyMap[ImGuiKey_V]           = 'V';
  io.KeyMap[ImGuiKey_X]           = 'X';
  io.KeyMap[ImGuiKey_Y]           = 'Y';
  io.KeyMap[ImGuiKey_Z]           = 'Z';
}

int Window::dock() const
{
  const int dockIndex = { DockIsChildOfDock(m_hwnd.get(), nullptr)  };
  return dockIndex > -1 ? (dockIndex << 1) | 1 : m_ctx->settings().dock & ~1;
}

void Window::setDock(const int dock)
{
  Settings &settings { m_ctx->settings() };
  if(dock == settings.dock && IsWindowVisible(m_hwnd.get()))
    return;

  DockWindowRemove(m_hwnd.get());

  if(dock & 1) {
    if(!(settings.dock & 1)) // store undocked position and size
      updateSettings();      // (overwrites settings.dock first)
    settings.dock = dock;

    constexpr const char *INI_KEY { "reaimgui" };
    Dock_UpdateDockID(INI_KEY, dock >> 1);
    DockWindowAddEx(m_hwnd.get(), settings.title.c_str(), INI_KEY, true);
    DockWindowActivate(m_hwnd.get());
  }
  else {
    settings.dock = dock;
    Window floating { m_ctx };
    std::swap(m_hwnd, floating.m_hwnd);
    std::swap(m_impl, floating.m_impl);
    SetWindowLongPtr(m_hwnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }
}

void Window::updateSettings()
{
  Settings &settings { m_ctx->settings() };
  settings.dock = dock();

  // only persist position and size when undocked
  if(!(settings.dock & 1)) {
    RECT rect;
    GetClientRect(m_hwnd.get(), &rect);
    settings.size.x = rect.right - rect.left;
    settings.size.y = rect.bottom - rect.top;
#ifdef __APPLE__
    std::swap(rect.top, rect.bottom);
#else
    const float scale { scaleFactor() };
    settings.size.x /= scale;
    settings.size.y /= scale;
#endif
    ClientToScreen(m_hwnd.get(), reinterpret_cast<POINT *>(&rect));
    settings.pos.x = rect.left;
    settings.pos.y = rect.top;
  }

  m_ctx->markSettingsDirty();
}

#ifndef _WIN32
void Window::createSwellDialog()
{
  enum SwellDialogResFlags {
    ForceNonChild = 0x400000 | 0x8, // allows not using a resource id
    Resizable = 1,
  };

  const char *res { MAKEINTRESOURCE(ForceNonChild | Resizable) };
  LPARAM param { reinterpret_cast<LPARAM>(this) };
  CreateDialogParam(s_instance, res, parentHandle(), proc, param);
  SetWindowText(m_hwnd.get(), m_ctx->settings().title.c_str());
  AttachWindowTopmostButton(m_hwnd.get());
}

const char *Window::getSwellClass()
{
  // eat global shortcuts when a text input is focused
  ImGuiIO &io { ImGui::GetIO() };
  return io.WantCaptureKeyboard ? "Lua_LICE_gfx_standalone" : "reaper_imgui_context";
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
  if(self->m_hwnd.get() == msg->hwnd)
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
