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

#include "window.hpp"

#include "context.hpp"
#include "font.hpp"
#include "platform.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#ifndef _WIN32
#  include <swell/swell.h>
#  include <WDL/wdltypes.h>
#endif

HINSTANCE Window::s_instance;

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
  io.KeyMap[ImGuiKey_KeypadEnter] = VK_RETURN;
  io.KeyMap[ImGuiKey_A]           = 'A';
  io.KeyMap[ImGuiKey_C]           = 'C';
  io.KeyMap[ImGuiKey_V]           = 'V';
  io.KeyMap[ImGuiKey_X]           = 'X';
  io.KeyMap[ImGuiKey_Y]           = 'Y';
  io.KeyMap[ImGuiKey_Z]           = 'Z';
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
  }
  else {
    self = reinterpret_cast<Window *>(GetWindowLongPtr(handle, GWLP_USERDATA));

    if(!self)
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
    self->mouseDown(ImGuiMouseButton_Left);
    return 0;
  case WM_LBUTTONUP:
    self->mouseUp(ImGuiMouseButton_Left);
    return 0;
  case WM_MBUTTONDOWN:
    self->mouseDown(ImGuiMouseButton_Middle);
    return 0;
  case WM_MBUTTONUP:
    self->mouseUp(ImGuiMouseButton_Middle);
    return 0;
  case WM_RBUTTONDOWN:
    self->mouseDown(ImGuiMouseButton_Right);
    return 0;
  case WM_RBUTTONUP:
    self->mouseUp(ImGuiMouseButton_Right);
    return 0;
#endif // __APPLE__
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

Window::Window(ImGuiViewport *viewport, DockerHost *dockerHost)
  : Viewport { viewport }, m_dockerHost { dockerHost },
    m_accel { &translateAccel, true, this },
    m_accelReg { "accelerator", &m_accel }, m_previousScale { 0.f },
    m_fontTexVersion { -1 }, m_mouseDown { 0 }, m_noFocus { false }
{
  static std::weak_ptr<PluginRegister> g_hwndInfo; // v6.29+

  if(g_hwndInfo.expired())
    g_hwndInfo = m_hwndInfo = std::make_shared<PluginRegister>
      ("hwnd_info", reinterpret_cast<void *>(&Window::hwndInfo));
  else
    m_hwndInfo = g_hwndInfo.lock();

  // HACK: See Window::show. Not using ViewportFlags because it would always be
  // set when using BeginPopup.
  ImGuiViewportP *viewportPrivate { static_cast<ImGuiViewportP *>(viewport) };
  if(ImGuiWindow *userWindow { viewportPrivate->Window })
    m_noFocus = userWindow->Flags & ImGuiWindowFlags_NoFocusOnAppearing;

  // Cannot initialize m_hwnd during construction due to handleMessage being
  // virtual. This task is delayed to created() called once fully constructed.
}

Window::~Window()
{
  RemoveProp(m_hwnd.get(), CLASS_NAME);
  // Disable message passing to the derived class (not available at this point)
  SetWindowLongPtr(m_hwnd.get(), GWLP_USERDATA, 0);
  // Announce to REAPER the window is no longer going to be valid
  // (DockWindowRemove is safe to call even when not docked)
  DockWindowRemove(m_hwnd.get()); // may send messages
}

void Window::WindowDeleter::operator()(HWND window)
{
  DestroyWindow(window);
}

void Window::show()
{
  if(isDocked())
    return;

  // HACK: Undo this weird thing ImGui does right before calling ShowWindow
  if(ImGui::GetFrameCount() < 3 && !m_noFocus)
    m_viewport->Flags &= ~ImGuiViewportFlags_NoFocusOnAppearing;

  if(m_viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
    ShowWindow(m_hwnd.get(), SW_SHOWNA);
  else
    ShowWindow(m_hwnd.get(), SW_SHOW);
}

void Window::setFocus()
{
  SetFocus(m_hwnd.get());
}

bool Window::hasFocus() const
{
  return GetFocus() == m_hwnd.get();
}

bool Window::isMinimized() const
{
  // IsWindowVisible is false when docked and another tab is active
  return !IsWindowVisible(m_hwnd.get());
}

void Window::onChanged()
{
  const bool scaleChanged { m_previousScale != m_viewport->DpiScale };
  m_previousScale = m_viewport->DpiScale;

  const int fontTexVersion { m_ctx->fonts().setScale(m_viewport->DpiScale) };
  if(scaleChanged || fontTexVersion != m_fontTexVersion) {
    uploadFontTex();
    m_fontTexVersion = fontTexVersion;
  }
}

void Window::mouseDown(const ImGuiMouseButton btn)
{
  // Not needed on macOS for receiving mouse up messages outside of the
  // windows's boundaries. It is instead used by Context::updateMouseData.
  if(GetCapture() == nullptr)
    SetCapture(m_hwnd.get());

  // give keyboard focus when docked (+ focus on right/middle click on macOS)
  setFocus();

  m_ctx->mouseInput(btn, true);
  m_mouseDown |= 1 << btn;
}

void Window::mouseUp(const ImGuiMouseButton btn)
{
  m_ctx->mouseInput(btn, false);
  m_mouseDown &= ~(1 << btn);

  if(GetCapture() == m_hwnd.get() && m_mouseDown == 0)
    ReleaseCapture();
}

void Window::invalidateTextures()
{
  // uploadFontTex will be called on the next onChanged()
  m_fontTexVersion = -1;
}

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

int Window::translateAccel(MSG *msg, accelerator_register_t *accel)
{
  auto *self { static_cast<Window *>(accel->user) };
  HWND hwnd { self->m_hwnd.get() };
  if(hwnd == msg->hwnd || IsChild(hwnd, msg->hwnd))
    return self->handleAccelerator(msg);
  else
    return Accel::NotOurWindow;
}

int Window::handleAccelerator(MSG *)
{
  return Accel::PassToWindow; // default implementation
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
