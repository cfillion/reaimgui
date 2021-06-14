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
#include "font.hpp"

#include <imgui/imgui.h>

#ifndef _WIN32
#  include <swell/swell.h>
#  include <WDL/wdltypes.h>
#endif

HINSTANCE Window::s_instance;

static void createWindow(ImGuiViewport *viewport)
{
  Window *window = new Window { viewport, Context::current() };

  // set these only if constructions succeeds
  viewport->PlatformUserData = window;
  viewport->PlatformHandle = window->nativeHandle();
}

static void destroyWindow(ImGuiViewport *viewport)
{
  if(viewport->Flags & ImGuiViewportFlags_OwnedByApp)
    return; // don't destroy the "main" viewport that we didn't create

  delete static_cast<Window *>(viewport->PlatformUserData);
  viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

template<auto fn, typename... Args>
static auto instanceProxy(ImGuiViewport *vp, Args... args)
{
  using R = std::result_of_t<decltype(fn)(Window *, Args...)>;

  if(Window *wnd { static_cast<Window *>(vp->PlatformUserData) })
    return (wnd->*fn)(args...);

  if constexpr(!std::is_void_v<R>)
    return R{};
}

void Window::install()
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

  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  pio.Platform_CreateWindow       = &createWindow;
  pio.Platform_DestroyWindow      = &destroyWindow;
  pio.Platform_ShowWindow         = &instanceProxy<&Window::show>;
  pio.Platform_SetWindowPos       = &instanceProxy<&Window::setPosition>;
  pio.Platform_GetWindowPos       = &instanceProxy<&Window::getPosition>;
  pio.Platform_SetWindowSize      = &instanceProxy<&Window::setSize>;
  pio.Platform_GetWindowSize      = &instanceProxy<&Window::getSize>;
  pio.Platform_SetWindowFocus     = &instanceProxy<&Window::setFocus>;
  pio.Platform_GetWindowFocus     = &instanceProxy<&Window::hasFocus>;
  pio.Platform_GetWindowMinimized = &instanceProxy<&Window::isVisible>;
  pio.Platform_SetWindowTitle     = &instanceProxy<&Window::setTitle>;
  // TODO: SetWindowAlpha
  pio.Platform_UpdateWindow       = &instanceProxy<&Window::update>;
  pio.Platform_RenderWindow       = &instanceProxy<&Window::render>;
  pio.Platform_GetWindowDpiScale  = &instanceProxy<&Window::scaleFactor>;
  pio.Platform_OnChangedViewport  = &instanceProxy<&Window::onChangedViewport>;
  pio.Platform_SetImeInputPos     = &instanceProxy<&Window::setImePosition>;

  platformInstall();
}

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
    SetProp(handle, CLASS_NAME, self->m_ctx);
    self->installHooks();
  }
  else {
    self = reinterpret_cast<Window *>(GetWindowLongPtr(handle, GWLP_USERDATA));

    if(!self)// || self->m_ctx->window() != self) // TODO: when redoing undocking
      return DefWindowProc(handle, msg, wParam, lParam);
  }

  if(const std::optional<LRESULT> &rv { self->handleMessage(msg, wParam, lParam) })
    return *rv;

  switch(msg) {
  case WM_COMMAND: // docker close button sends WM_COMMAND with IDCANCEL
    if(LOWORD(wParam) != IDCANCEL)
      break;
    [[fallthrough]];
  case WM_CLOSE:
    self->m_viewport->PlatformRequestClose = true;
    return 0;
  case WM_MOVE:
    self->m_viewport->PlatformRequestMove = true;
    return 0;
  case WM_SIZE:
    self->m_viewport->PlatformRequestResize = true;
    return 0;
  case WM_DESTROY:
    RemoveProp(handle, CLASS_NAME);
    SetWindowLongPtr(handle, GWLP_USERDATA, 0);
    return 0;
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
#ifndef GET_WHEEL_DELTA_WPARAM
#  define GET_WHEEL_DELTA_WPARAM GET_Y_LPARAM
#endif
    self->m_ctx->mouseWheel(msg == WM_MOUSEHWHEEL, GET_WHEEL_DELTA_WPARAM(wParam));
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
    self->mouseDown(msg);
    return 0;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    self->mouseUp(msg);
    return 0;
#endif // __APPLE__
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

void Window::commonShow()
{
  // FIXME: Undo this weird thing ImGui does before calling ShowWindow
  if(ImGui::GetFrameCount() < 3)
    m_viewport->Flags &= ~ImGuiViewportFlags_NoFocusOnAppearing;

  if(m_viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
    ShowWindow(m_hwnd.get(), SW_SHOWNA);
  else
    ShowWindow(m_hwnd.get(), SW_SHOW);
}

ImVec2 Window::getPosition() const
{
  POINT pos {};
  ClientToScreen(m_hwnd.get(), &pos);

#ifdef __APPLE__
  pos.y = ImGui::GetPlatformIO().Monitors[0].MainSize.y - pos.y;
#endif

  return { static_cast<float>(pos.x), static_cast<float>(pos.y) };
}

ImVec2 Window::getSize() const
{
  RECT rect;
  GetClientRect(m_hwnd.get(), &rect);
  auto width  { rect.right - rect.left },
       height { rect.bottom - rect.top };
#ifndef __APPLE__
  width  /= m_viewport->DpiScale;
  height /= m_viewport->DpiScale;
#endif
  return { static_cast<float>(width), static_cast<float>(height) };
}

void Window::setFocus()
{
  SetFocus(m_hwnd.get());
}

bool Window::hasFocus() const
{
  // the focused control is a child of the window's hwnd on macOS (InputView)
  HWND foreground { GetForegroundWindow() }, self { m_hwnd.get() };
#ifdef __APPLE__
  return IsChild(self, foreground);
#else
  return foreground == self;
#endif
}

bool Window::isVisible() const
{
  // IsWindowVisible is false when docked and another tab is active
  return !IsWindowVisible(m_hwnd.get());
}

#ifndef _WIN32
void Window::setTitle(const char *title)
{
  SetWindowText(m_hwnd.get(), title);
}
#endif

void Window::onChangedViewport()
{
  const bool scaleChanged { m_previousScale != m_viewport->DpiScale };
  m_previousScale = m_viewport->DpiScale;

  const int fontTexVersion { m_ctx->fonts()->setScale(m_viewport->DpiScale) };
  if(scaleChanged || fontTexVersion != m_fontTexVersion) {
    uploadFontTex();
    m_fontTexVersion = fontTexVersion;
  }
}

#ifndef __APPLE__
void Window::translatePosition(POINT *point, const bool toHiDpi) const
{
  const auto fromOriginX { point->x - m_viewport->Pos.x },
             fromOriginY { point->y - m_viewport->Pos.y };

  float scale { m_viewport->DpiScale };
  if(!toHiDpi)
    scale = 1.f / scale;

  point->x = m_viewport->Pos.x + (fromOriginX * scale);
  point->y = m_viewport->Pos.y + (fromOriginY * scale);
}
#endif

void Window::mouseDown(const unsigned int msg)
{
  size_t btn;

  switch(msg) {
  case WM_LBUTTONDOWN:
    btn = ImGuiMouseButton_Left;
    break;
  case WM_MBUTTONDOWN:
    btn = ImGuiMouseButton_Middle;
    break;
  case WM_RBUTTONDOWN:
    btn = ImGuiMouseButton_Right;
    break;
  default:
    return;
  }

  if(GetCapture() == nullptr)
    SetCapture(m_hwnd.get());

  m_ctx->mouseInput(btn, true);
}

void Window::mouseUp(const unsigned int msg)
{
  size_t btn;

  switch(msg) {
  case WM_LBUTTONUP:
    btn = ImGuiMouseButton_Left;
    break;
  case WM_MBUTTONUP:
    btn = ImGuiMouseButton_Middle;
    break;
  case WM_RBUTTONUP:
    btn = ImGuiMouseButton_Right;
    break;
  default:
    return;
  }

  m_ctx->mouseInput(btn, false);

  if(GetCapture() == m_hwnd.get() && !m_ctx->anyMouseDown())
    ReleaseCapture();
}

// int Window::dock() const
// {
//   const int dockIndex { DockIsChildOfDock(m_hwnd.get(), nullptr) };
//   return dockIndex > -1 ? (dockIndex << 1) | 1 : m_ctx->settings().dock & ~1;
// }

// void Window::setDock(const int dock)
// {
//   Settings &settings { m_ctx->settings() };
//   if(dock == settings.dock && IsWindowVisible(m_hwnd.get()))
//     return;
//
//   DockWindowRemove(m_hwnd.get());
//
//   if(dock & 1) {
//     if(!(settings.dock & 1)) // store undocked position and size
//       updateSettings();      // (overwrites settings.dock first)
//     settings.dock = dock;
//
//     constexpr const char *INI_KEY { "reaimgui" };
//     Dock_UpdateDockID(INI_KEY, dock >> 1);
//     DockWindowAddEx(m_hwnd.get(), settings.title.c_str(), INI_KEY, true);
//     DockWindowActivate(m_hwnd.get());
//   }
//   else {
//     settings.dock = dock;
//     Window floating { m_ctx };
//     std::swap(m_hwnd, floating.m_hwnd);
//     std::swap(m_impl, floating.m_impl);
//     SetWindowLongPtr(m_hwnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
//     m_ctx->invalidateTextures();
//   }
// }

// void Window::updateSettings()
// {
//   Settings &settings { m_ctx->settings() };
//   settings.dock = dock();
//
//   // only persist position and size when undocked
//   if(!(settings.dock & 1)) {
//     RECT rect;
// #ifdef __linux__
//     GetWindowRect(m_hwnd.get(), &rect);
// #else
//     GetClientRect(m_hwnd.get(), &rect);
// #endif
//     settings.size.x = rect.right - rect.left;
//     settings.size.y = rect.bottom - rect.top;
// #ifndef __APPLE__
//     const float scale { scaleFactor() };
//     settings.size.x /= scale;
//     settings.size.y /= scale;
// #endif
// #ifndef __linux__
//     ClientToScreen(m_hwnd.get(), reinterpret_cast<POINT *>(&rect));
// #endif
//     settings.pos.x = rect.left;
//     settings.pos.y = rect.top;
//   }
//
//   m_ctx->markSettingsDirty();
// }

#ifndef _WIN32
void Window::createSwellDialog()
{
  enum SwellDialogResFlags {
    ForceNonChild = 0x400008, // allows not using a resource id
    Resizable = 1,
  };

  const char *res { MAKEINTRESOURCE(ForceNonChild | Resizable) };
  LPARAM param { reinterpret_cast<LPARAM>(this) };
  CreateDialogParam(s_instance, res, parentHandle(), proc, param);
}

const char *Window::getSwellClass() const
{
  // eat global shortcuts when a text input is focused before v6.29's hwnd_info
  return m_ctx->IO().WantCaptureKeyboard ? "Lua_LICE_gfx_standalone" : CLASS_NAME;
}
#endif

HWND Window::parentHandle()
{
  ImGuiViewport *parent { ImGui::FindViewportByID(m_viewport->ParentViewportId) };

  if(!parent)
    parent = ImGui::GetMainViewport();

  return static_cast<HWND>(parent->PlatformHandle);
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

void Window::installHooks()
{
  static std::weak_ptr<PluginRegister> g_hwndInfo; // v6.29+

  if(g_hwndInfo.expired())
    g_hwndInfo = m_hwndInfo = std::make_shared<PluginRegister>
      ("hwnd_info", reinterpret_cast<void *>(&Window::hwndInfo));
  else
    m_hwndInfo = g_hwndInfo.lock();
}

int Window::hwndInfo(HWND hwnd, const intptr_t infoType)
{
  enum InfoType { IsInTextField };
  enum RetVal { Unknown = 0, InTextField = 1, NotInTextField = -1 };

  Context *ctx;
  do {
    ctx = static_cast<Context *>(GetProp(hwnd, CLASS_NAME));
#ifdef __APPLE__
  // hwnd is the InputView when it has focus
  } while(!ctx && (hwnd = GetParent(hwnd)));
#else
  } while(false);
#endif

  if(infoType == IsInTextField && Resource::exists(ctx)) {
    // Called for handling global shortcuts (v6.29+)
    // getSwellClass emulates this in older versions (but only on macOS & Linux)
    return ctx->IO().WantCaptureKeyboard ? InTextField : NotInTextField;
  }

  return Unknown;
}

#ifndef __APPLE__
ImGuiViewport *Window::viewportUnder(const POINT pos)
{
  HWND target { WindowFromPoint(pos) };

  ImGuiViewport *viewport { ImGui::FindViewportByPlatformHandle(target) };
  if(viewport && viewport->PlatformUserData)
    return viewport;

  return nullptr;
}
#endif

void Window::WindowDeleter::operator()(HWND window)
{
  // Announce to REAPER the window is no longer going to be valid
  // (safe to call even when not docked)
  DockWindowRemove(window);
  DestroyWindow(window);
}
