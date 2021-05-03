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

#include "context.hpp"

#include "window.hpp"

#include <cassert>
#include <reaper_colortheme.h>
#include <imgui/imgui_internal.h> // ClearActiveID
#include <reaper_plugin_functions.h>

#ifdef _WIN32
#  define PATH_SEP "\\"
#else
#  define PATH_SEP "/"
#endif

class TempCurrent {
public:
  TempCurrent(Context *ctx)
    : m_old { ImGui::GetCurrentContext() } { ctx->setCurrent(); }
  ~TempCurrent() { ImGui::SetCurrentContext(m_old); }

private:
  ImGuiContext *m_old;
};

Context *Context::current()
{
  return static_cast<Context *>(ImGui::GetIO().UserData);
}

Context::Context(const WindowConfig &winConfig)
  : m_inFrame { false }, m_closeReq { false }, m_cursor {}, m_mouseDown {},
    m_lastFrame { decltype(m_lastFrame)::clock::now() },
    m_imgui { ImGui::CreateContext(), &ImGui::DestroyContext }
{
  static const std::string logFn
    { std::string { GetResourcePath() } + PATH_SEP "imgui_log.txt" };

  setCurrent();

  ImGuiIO &io { ImGui::GetIO() };
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
  io.IniFilename = nullptr;
  io.LogFilename = logFn.c_str();
  io.UserData = this;

  Window::updateKeyMap();

  m_window = std::make_unique<Window>(winConfig, this);

  // Start a frame to prevent contexts created within a defer callback from
  // being immediately destroyed.
  beginFrame();
}

Context::~Context()
{
  setCurrent();

  if(m_inFrame)
    endFrame(false);
}

bool Context::heartbeat()
{
  if(m_closeReq)
    m_closeReq = false;

  if(m_inFrame)
    return endFrame(true);
  else
    return false;
}

void Context::beginFrame()
{
  assert(!m_inFrame);

  m_inFrame = true;

  if(m_setDockNextFrame) {
    m_window->setDock(*m_setDockNextFrame);
    m_setDockNextFrame = std::nullopt;
  }

  updateFrameInfo();
  updateTheme();
  updateMouseDown();
  updateMousePos();
  updateKeyMods();

  ImGui::NewFrame();
  m_window->beginFrame();
}

void Context::setCurrent()
{
  ImGui::SetCurrentContext(m_imgui.get());
}

void Context::setDockNextFrame(const int dock)
{
  // Docking later, as this might recreate the rendering context and invalidate
  // textures that are already used in the current frame.
  m_setDockNextFrame = dock;
}

void Context::enterFrame()
{
  setCurrent();

  if(!m_inFrame)
    beginFrame();
}

bool Context::endFrame(const bool render) try
{
  setCurrent();

  // IsWindowVisible is false when docked and another tab is active
  if(render && IsWindowVisible(m_window->nativeHandle())) {
    updateCursor();
    ImGui::Render();
    m_window->drawFrame(ImGui::GetDrawData());
  }
  else
    ImGui::EndFrame();

  m_window->endFrame();
  m_inFrame = false;

  return true;
}
catch(const imgui_error &e) {
  char message[1024];
  snprintf(message, sizeof(message), "ImGui assertion failed: %s\n", e.what());
  ShowConsoleMsg(message); // cannot use ReaScriptError unless called by a script

  // no recovery, just destroy the context
  m_window->endFrame();

  // don't assert again when destroying the font atlas
  ImGui::GetIO().Fonts->Locked = false;

  // don't call endFrame again from the destructor
  m_inFrame = false;

  return false;
}

void Context::updateFrameInfo()
{
  ImGuiIO &io { ImGui::GetIO() };

  const float scale { m_window->scaleFactor() };
  io.DisplayFramebufferScale = { scale, scale };

  RECT rect;
  GetClientRect(m_window->nativeHandle(), &rect);
  io.DisplaySize.x = rect.right - rect.left;
  io.DisplaySize.y = rect.bottom - rect.top;
#ifndef __APPLE__
  io.DisplaySize.x /= scale;
  io.DisplaySize.y /= scale;
#endif

  const auto now { decltype(m_lastFrame)::clock::now() };
  io.DeltaTime = std::chrono::duration<float> { now - m_lastFrame }.count();
  m_lastFrame = now;
}

void Context::updateCursor()
{
  setCurrent();

  if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
    return;

  static HCURSOR nativeCursors[ImGuiMouseCursor_COUNT] {
    LoadCursor(nullptr, IDC_ARROW),
    LoadCursor(nullptr, IDC_IBEAM),
    LoadCursor(nullptr, IDC_SIZEALL),
    LoadCursor(nullptr, IDC_SIZENS),
    LoadCursor(nullptr, IDC_SIZEWE),
    LoadCursor(nullptr, IDC_SIZENESW),
    LoadCursor(nullptr, IDC_SIZENWSE),
    LoadCursor(nullptr, IDC_HAND),
    LoadCursor(nullptr, IDC_NO),
  };

  // TODO
  // io.MouseDrawCursor (ImGui-drawn cursor)
  // SetCursor(nullptr) does not hide the cursor with SWELL

  // ImGui::GetMouseCursor is only valid after a frame, before Render
  // (it's reset when a new frame is started)
  const ImGuiMouseCursor imguiCursor { ImGui::GetMouseCursor() };
  const bool hidden { imguiCursor == ImGuiMouseCursor_None };
  HCURSOR cursor { hidden ? nullptr : nativeCursors[imguiCursor] };
  if(m_cursor != cursor)
    SetCursor(m_cursor = cursor);
}

bool Context::anyMouseDown() const
{
  for(const auto state : m_mouseDown) {
    if(state & Down)
      return true;
  }

  return false;
}

void Context::mouseDown(const unsigned int msg)
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

#ifndef __APPLE__
  if(!anyMouseDown() && GetCapture() == nullptr)
    SetCapture(m_window->nativeHandle());
#endif

  m_mouseDown[btn] = Down | DownUnread;
}

void Context::mouseUp(const unsigned int msg)
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

  // keep DownUnread set to catch clicks shorted than one frame
  m_mouseDown[btn] &= ~Down;

#ifndef __APPLE__
  if(!anyMouseDown() && GetCapture() == m_window->nativeHandle())
    ReleaseCapture();
#endif
}

void Context::updateMouseDown()
{
  // this is only called from enterFrame, the context is already set
  // setCurrent();

  ImGuiIO &io { ImGui::GetIO() };

  size_t i {};
  for(auto &state : m_mouseDown) {
    io.MouseDown[i++] = (state & DownUnread) || (state & Down);
    state &= ~DownUnread;
  }
}

void Context::updateMousePos()
{
  // this is only called from enterFrame, the context is already set
  // setCurrent();

  ImGuiIO &io { ImGui::GetIO() };
  HWND windowHwnd { m_window->nativeHandle() };

  if(io.WantSetMousePos) {
    POINT p { static_cast<LONG>(io.MousePos.x),
              static_cast<LONG>(io.MousePos.y) };
#ifndef __APPLE__
    p.x *= io.DisplayFramebufferScale.x;
    p.y *= io.DisplayFramebufferScale.y;
#endif
    ClientToScreen(windowHwnd, &p);
    SetCursorPos(p.x, p.y);
    return;
  }

  POINT p;
  GetCursorPos(&p);
  const HWND targetHwnd { WindowFromPoint(p) };
  ScreenToClient(windowHwnd, &p);
#ifndef __APPLE__
  p.x /= io.DisplayFramebufferScale.x;
  p.y /= io.DisplayFramebufferScale.y;
#endif

#ifdef __APPLE__
  // Our InputView overlays SWELL's NSView.
  // Capturing is not used as macOS sends mouse up events from outside of the
  // frame when the mouse down event occured within.
  if(IsChild(windowHwnd, targetHwnd) || anyMouseDown())
#else
  if(targetHwnd == windowHwnd || GetCapture() == windowHwnd)
#endif
    io.MousePos = { static_cast<float>(p.x), static_cast<float>(p.y) };
  else
    io.MousePos = { -FLT_MAX, -FLT_MAX };
}

void Context::mouseWheel(const unsigned int msg, const short delta)
{
#ifndef WHEEL_DELTA
  constexpr float WHEEL_DELTA {
#  ifdef __APPLE__
    60.0f
#  else
    120.0f
#  endif
  };
#endif

  TempCurrent cur { this };
  ImGuiIO &io { ImGui::GetIO() };
  float &wheel { msg == WM_MOUSEHWHEEL ? io.MouseWheelH : io.MouseWheel };
  wheel += static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
}

void Context::updateKeyMods()
{
  // this is only called from enterFrame, the context is already set
  // setCurrent();

  constexpr int down { 0x8000 };

  ImGuiIO &io { ImGui::GetIO() };
  io.KeyCtrl  = GetAsyncKeyState(VK_CONTROL) & down;
  io.KeyShift = GetAsyncKeyState(VK_SHIFT)   & down;
  io.KeyAlt   = GetAsyncKeyState(VK_MENU)    & down;
  io.KeySuper = GetAsyncKeyState(VK_LWIN)    & down;
}

void Context::keyInput(const uint8_t key, const bool down)
{
  TempCurrent cur { this };
  ImGuiIO &io { ImGui::GetIO() };
  io.KeysDown[key] = down;
}

void Context::charInput(const unsigned int codepoint)
{
  if(codepoint < 32 || (codepoint > 126 && codepoint < 160))
    return;

  TempCurrent cur { this };
  ImGuiIO &io { ImGui::GetIO() };
  io.AddInputCharacter(codepoint);
}

void Context::clearFocus()
{
  TempCurrent cur { this };
  if(ImGui::GetIO().WantCaptureKeyboard)
    ImGui::ClearActiveID();
}

void Context::updateTheme()
{
  int themeSize;
  const ColorTheme *theme { static_cast<ColorTheme *>(GetColorThemeStruct(&themeSize)) };
  if(static_cast<size_t>(themeSize) < sizeof(ColorTheme))
    return;

  m_clearColor = Color::fromNative(theme->window_background);

  // TODO: Extract a few key colors from REAPER's theme, and compute nicely
  // readable colors for ImGui from them. See vendor/reaper_colortheme.h
  ImVec4 *colors { ImGui::GetStyle().Colors };
  (void)colors;
  // colors[ImGuiCol_Text]                  = Color::fromNative(0);
  // colors[ImGuiCol_TextDisabled]          = Color::fromNative(0);
  // colors[ImGuiCol_WindowBg]              = Color::fromNative(0); // Background of normal windows
  // colors[ImGuiCol_ChildBg]               = Color::fromNative(0); // Background of child windows
  // colors[ImGuiCol_PopupBg]               = Color::fromNative(0); // Background of popups, menus, tooltips windows
  // colors[ImGuiCol_Border]                = Color::fromNative(0);
  // colors[ImGuiCol_BorderShadow]          = Color::fromNative(0);
  // colors[ImGuiCol_FrameBg]               = Color::fromNative(0); // Background of checkbox, radio button, plot, slider, text input
  // colors[ImGuiCol_FrameBgHovered]        = Color::fromNative(0);
  // colors[ImGuiCol_FrameBgActive]         = Color::fromNative(0);
  // colors[ImGuiCol_TitleBg]               = Color::fromNative(0);
  // colors[ImGuiCol_TitleBgActive]         = Color::fromNative(0);
  // colors[ImGuiCol_TitleBgCollapsed]      = Color::fromNative(0);
  // colors[ImGuiCol_MenuBarBg]             = Color::fromNative(0);
  // colors[ImGuiCol_ScrollbarBg]           = Color::fromNative(0);
  // colors[ImGuiCol_ScrollbarGrab]         = Color::fromNative(0);
  // colors[ImGuiCol_ScrollbarGrabHovered]  = Color::fromNative(0);
  // colors[ImGuiCol_ScrollbarGrabActive]   = Color::fromNative(0);
  // colors[ImGuiCol_CheckMark]             = Color::fromNative(0);
  // colors[ImGuiCol_SliderGrab]            = Color::fromNative(0);
  // colors[ImGuiCol_SliderGrabActive]      = Color::fromNative(0);
  // colors[ImGuiCol_Button]                = Color::fromNative(0);
  // colors[ImGuiCol_ButtonHovered]         = Color::fromNative(0);
  // colors[ImGuiCol_ButtonActive]          = Color::fromNative(0);
  // colors[ImGuiCol_Header]                = Color::fromNative(0); // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
  // colors[ImGuiCol_HeaderHovered]         = Color::fromNative(0);
  // colors[ImGuiCol_HeaderActive]          = Color::fromNative(0);
  // colors[ImGuiCol_Separator]             = Color::fromNative(0);
  // colors[ImGuiCol_SeparatorHovered]      = Color::fromNative(0);
  // colors[ImGuiCol_SeparatorActive]       = Color::fromNative(0);
  // colors[ImGuiCol_ResizeGrip]            = Color::fromNative(0);
  // colors[ImGuiCol_ResizeGripHovered]     = Color::fromNative(0);
  // colors[ImGuiCol_ResizeGripActive]      = Color::fromNative(0);
  // colors[ImGuiCol_Tab]                   = Color::fromNative(0);
  // colors[ImGuiCol_TabHovered]            = Color::fromNative(0);
  // colors[ImGuiCol_TabActive]             = Color::fromNative(0);
  // colors[ImGuiCol_TabUnfocused]          = Color::fromNative(0);
  // colors[ImGuiCol_TabUnfocusedActive]    = Color::fromNative(0);
  // colors[ImGuiCol_PlotLines]             = Color::fromNative(0);
  // colors[ImGuiCol_PlotLinesHovered]      = Color::fromNative(0);
  // colors[ImGuiCol_PlotHistogram]         = Color::fromNative(0);
  // colors[ImGuiCol_PlotHistogramHovered]  = Color::fromNative(0);
  // colors[ImGuiCol_TableHeaderBg]         = Color::fromNative(0); // Table header background
  // colors[ImGuiCol_TableBorderStrong]     = Color::fromNative(0); // Table outer and header borders (prefer using Alpha=1.0 here)
  // colors[ImGuiCol_TableBorderLight]      = Color::fromNative(0); // Table inner borders (prefer using Alpha=1.0 here)
  // colors[ImGuiCol_TableRowBg]            = Color::fromNative(0); // Table row background (even rows)
  // colors[ImGuiCol_TableRowBgAlt]         = Color::fromNative(0); // Table row background (odd rows)
  // colors[ImGuiCol_TextSelectedBg]        = Color::fromNative(0);
  // colors[ImGuiCol_DragDropTarget]        = Color::fromNative(0);
  // colors[ImGuiCol_NavHighlight]          = Color::fromNative(0); // Gamepad/keyboard: current highlighted item
  // colors[ImGuiCol_NavWindowingHighlight] = Color::fromNative(0); // Highlight window when using CTRL+TAB
  // colors[ImGuiCol_NavWindowingDimBg]     = Color::fromNative(0); // Darken/colorize entire screen behind the CTRL+TAB window list, when active
}
