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

Context::Context(const char *title,
    const int x, const int y, const int w, const int h)
  : m_inFrame { false }, m_closeReq { false }, m_cursor {}, m_mouseDown {},
    m_lastFrame { decltype(m_lastFrame)::clock::now() },
    m_imgui { nullptr, &ImGui::DestroyContext }
{
  setupImGui();

  const RECT rect { x, y, x + w, y + h };
  m_window = std::make_unique<Window>(title, rect, this);
}

void Context::setupImGui()
{
  static std::weak_ptr<ImFontAtlas> g_fontAtlas;

  if(g_fontAtlas.expired())
    g_fontAtlas = m_fontAtlas = std::make_shared<ImFontAtlas>();
  else
    m_fontAtlas = g_fontAtlas.lock();

  m_imgui.reset(ImGui::CreateContext(m_fontAtlas.get()));
  ImGui::SetCurrentContext(m_imgui.get());

  ImGuiIO &io { ImGui::GetIO() };
  io.IniFilename = nullptr;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

#ifndef __APPLE__
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
#endif
}

Context::~Context()
{
  ImGui::SetCurrentContext(m_imgui.get());

  // Announce to REAPER the window is no longer going to be valid
  // (safe to call even when not docked)
  DockWindowRemove(m_window->nativeHandle());

  if(m_inFrame)
    endFrame(false, false);
}

void Context::heartbeat()
{
  if(m_closeReq)
    m_closeReq = false;

  if(m_inFrame)
    endFrame(true);
  else
    delete this;
}

void Context::beginFrame()
{
  assert(!m_inFrame);

  m_inFrame = true;

  updateFrameInfo();
  updateTheme();
  updateMouseDown();
  updateMousePos();
  updateKeyMods();

  ImGui::NewFrame();
  m_window->beginFrame();
}

void Context::enterFrame()
{
  ImGui::SetCurrentContext(m_imgui.get());

  if(!m_inFrame)
    beginFrame();
}

void Context::endFrame(const bool render, const bool prinnyMode) try
{
  ImGui::SetCurrentContext(m_imgui.get());

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
}
catch(const imgui_error &e) {
  char message[1024];
  snprintf(message, sizeof(message), "ImGui assertion failed: %s\n", e.what());
  ShowConsoleMsg(message); // cannot use ReaScriptError unless called by a script

  // no recovery, just destroy the context
  m_window->endFrame();
  m_fontAtlas->Locked = false; // don't assert again when destroying the atlas
  m_inFrame = false; // don't call endFrame again from the destructor

  if(prinnyMode) // don't delete twice when first called from the destructor
    delete this;
}

void Context::updateFrameInfo()
{
  ImGuiIO &io { ImGui::GetIO() };

  RECT rect;
  GetClientRect(m_window->nativeHandle(), &rect);
  io.DisplaySize = ImVec2(rect.right - rect.left, rect.bottom - rect.top);

  const float scale { m_window->scaleFactor() };
  io.DisplayFramebufferScale = ImVec2{scale, scale};

  const auto now { decltype(m_lastFrame)::clock::now() };
  io.DeltaTime = std::chrono::duration<float> { now - m_lastFrame }.count();
  m_lastFrame = now;
}

void Context::updateCursor()
{
  ImGui::SetCurrentContext(m_imgui.get());

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
  // ImGui::SetCurrentContext(m_imgui.get());

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
  // ImGui::SetCurrentContext(m_imgui.get());

  ImGuiIO &io { ImGui::GetIO() };
  HWND windowHwnd { m_window->nativeHandle() };

  if(io.WantSetMousePos) {
    POINT p { static_cast<LONG>(io.MousePos.x),
              static_cast<LONG>(io.MousePos.y) };
    ClientToScreen(windowHwnd, &p);
    SetCursorPos(p.x, p.y);
    return;
  }

  POINT p;
  GetCursorPos(&p);
  const HWND targetHwnd { WindowFromPoint(p) };
  ScreenToClient(windowHwnd, &p);

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
  ImGui::SetCurrentContext(m_imgui.get());

#ifndef WHEEL_DELTA
  constexpr float WHEEL_DELTA {
#  ifdef __APPLE__
    60.0f
#  else
    120.0f
#  endif
  };
#endif

  ImGuiIO &io { ImGui::GetIO() };
  float &wheel { msg == WM_MOUSEHWHEEL ? io.MouseWheelH : io.MouseWheel };
  wheel += static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
}

void Context::updateKeyMods()
{
  // this is only called from enterFrame, the context is already set
  // ImGui::SetCurrentContext(m_imgui.get());

  constexpr int down { 0x8000 };

  ImGuiIO &io { ImGui::GetIO() };
  io.KeyCtrl  = GetAsyncKeyState(VK_CONTROL) & down;
  io.KeyShift = GetAsyncKeyState(VK_SHIFT)   & down;
  io.KeyAlt   = GetAsyncKeyState(VK_MENU)    & down;
  io.KeySuper = GetAsyncKeyState(VK_LWIN)    & down;
}

void Context::keyInput(const uint8_t key, const bool down)
{
  ImGui::SetCurrentContext(m_imgui.get());
  ImGuiIO &io { ImGui::GetIO() };
  io.KeysDown[key] = down;
}

void Context::charInput(const unsigned int codepoint)
{
  if(codepoint < 32 || (codepoint > 126 && codepoint < 160))
    return;

  ImGui::SetCurrentContext(m_imgui.get());
  ImGuiIO &io { ImGui::GetIO() };
  io.AddInputCharacter(codepoint);
}

void Context::clearFocus()
{
  ImGui::SetCurrentContext(m_imgui.get());
  if(ImGui::GetIO().WantCaptureKeyboard)
    ImGui::ClearActiveID();
}

void Context::updateTheme()
{
  int themeSize;
  const ColorTheme *theme { static_cast<ColorTheme *>(GetColorThemeStruct(&themeSize)) };
  if(static_cast<size_t>(themeSize) < sizeof(ColorTheme))
    return;

  m_clearColor = Color::fromTheme(theme->window_background);

  // TODO: Extract a few key colors from REAPER's theme, and compute nicely
  // readable colors for ImGui from them. See vendor/reaper_colortheme.h
  ImVec4 *colors { ImGui::GetStyle().Colors };
  (void)colors;
  // colors[ImGuiCol_Text]                  = Color::fromTheme(0);
  // colors[ImGuiCol_TextDisabled]          = Color::fromTheme(0);
  // colors[ImGuiCol_WindowBg]              = Color::fromTheme(0); // Background of normal windows
  // colors[ImGuiCol_ChildBg]               = Color::fromTheme(0); // Background of child windows
  // colors[ImGuiCol_PopupBg]               = Color::fromTheme(0); // Background of popups, menus, tooltips windows
  // colors[ImGuiCol_Border]                = Color::fromTheme(0);
  // colors[ImGuiCol_BorderShadow]          = Color::fromTheme(0);
  // colors[ImGuiCol_FrameBg]               = Color::fromTheme(0); // Background of checkbox, radio button, plot, slider, text input
  // colors[ImGuiCol_FrameBgHovered]        = Color::fromTheme(0);
  // colors[ImGuiCol_FrameBgActive]         = Color::fromTheme(0);
  // colors[ImGuiCol_TitleBg]               = Color::fromTheme(0);
  // colors[ImGuiCol_TitleBgActive]         = Color::fromTheme(0);
  // colors[ImGuiCol_TitleBgCollapsed]      = Color::fromTheme(0);
  // colors[ImGuiCol_MenuBarBg]             = Color::fromTheme(0);
  // colors[ImGuiCol_ScrollbarBg]           = Color::fromTheme(0);
  // colors[ImGuiCol_ScrollbarGrab]         = Color::fromTheme(0);
  // colors[ImGuiCol_ScrollbarGrabHovered]  = Color::fromTheme(0);
  // colors[ImGuiCol_ScrollbarGrabActive]   = Color::fromTheme(0);
  // colors[ImGuiCol_CheckMark]             = Color::fromTheme(0);
  // colors[ImGuiCol_SliderGrab]            = Color::fromTheme(0);
  // colors[ImGuiCol_SliderGrabActive]      = Color::fromTheme(0);
  // colors[ImGuiCol_Button]                = Color::fromTheme(0);
  // colors[ImGuiCol_ButtonHovered]         = Color::fromTheme(0);
  // colors[ImGuiCol_ButtonActive]          = Color::fromTheme(0);
  // colors[ImGuiCol_Header]                = Color::fromTheme(0); // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
  // colors[ImGuiCol_HeaderHovered]         = Color::fromTheme(0);
  // colors[ImGuiCol_HeaderActive]          = Color::fromTheme(0);
  // colors[ImGuiCol_Separator]             = Color::fromTheme(0);
  // colors[ImGuiCol_SeparatorHovered]      = Color::fromTheme(0);
  // colors[ImGuiCol_SeparatorActive]       = Color::fromTheme(0);
  // colors[ImGuiCol_ResizeGrip]            = Color::fromTheme(0);
  // colors[ImGuiCol_ResizeGripHovered]     = Color::fromTheme(0);
  // colors[ImGuiCol_ResizeGripActive]      = Color::fromTheme(0);
  // colors[ImGuiCol_Tab]                   = Color::fromTheme(0);
  // colors[ImGuiCol_TabHovered]            = Color::fromTheme(0);
  // colors[ImGuiCol_TabActive]             = Color::fromTheme(0);
  // colors[ImGuiCol_TabUnfocused]          = Color::fromTheme(0);
  // colors[ImGuiCol_TabUnfocusedActive]    = Color::fromTheme(0);
  // colors[ImGuiCol_PlotLines]             = Color::fromTheme(0);
  // colors[ImGuiCol_PlotLinesHovered]      = Color::fromTheme(0);
  // colors[ImGuiCol_PlotHistogram]         = Color::fromTheme(0);
  // colors[ImGuiCol_PlotHistogramHovered]  = Color::fromTheme(0);
  // colors[ImGuiCol_TableHeaderBg]         = Color::fromTheme(0); // Table header background
  // colors[ImGuiCol_TableBorderStrong]     = Color::fromTheme(0); // Table outer and header borders (prefer using Alpha=1.0 here)
  // colors[ImGuiCol_TableBorderLight]      = Color::fromTheme(0); // Table inner borders (prefer using Alpha=1.0 here)
  // colors[ImGuiCol_TableRowBg]            = Color::fromTheme(0); // Table row background (even rows)
  // colors[ImGuiCol_TableRowBgAlt]         = Color::fromTheme(0); // Table row background (odd rows)
  // colors[ImGuiCol_TextSelectedBg]        = Color::fromTheme(0);
  // colors[ImGuiCol_DragDropTarget]        = Color::fromTheme(0);
  // colors[ImGuiCol_NavHighlight]          = Color::fromTheme(0); // Gamepad/keyboard: current highlighted item
  // colors[ImGuiCol_NavWindowingHighlight] = Color::fromTheme(0); // Highlight window when using CTRL+TAB
  // colors[ImGuiCol_NavWindowingDimBg]     = Color::fromTheme(0); // Darken/colorize entire screen behind the CTRL+TAB window list, when active
}
