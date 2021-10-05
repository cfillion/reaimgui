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

#include "docker.hpp"
#include "font.hpp"
#include "platform.hpp"
#include "viewport.hpp"

#include <cassert>
#include <imgui/imgui_internal.h>
#include <reaper_colortheme.h>
#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h>

#ifdef _WIN32
#  include "win32_unicode.hpp"
#endif

enum ButtonState {
  ButtonState_Down       = 1<<0,
  ButtonState_DownUnread = 1<<1,
};

enum DragState {
  DragState_None,
  DragState_FirstFrame = 1<<0,
  DragState_FakeClick  = 1<<1,
  DragState_Drop       = 1<<2,
};

constexpr ImGuiConfigFlags PRIVATE_CONFIG_FLAGS
  { ImGuiConfigFlags_ViewportsEnable };

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
  if(ImGuiContext *imgui { ImGui::GetCurrentContext() })
    return static_cast<Context *>(imgui->IO.UserData);
  else
    return nullptr;
}

static ImFontAtlas * const NO_DEFAULT_ATLAS
  { reinterpret_cast<ImFontAtlas *>(-1) };

static std::string generateIniFilename(const char *label)
{
  std::string filename { GetResourcePath() };

  if(!label[0]) // does not prohibit empty window titles
    throw reascript_error { "context label is required" };

  filename += WDL_DIRCHAR_STR "ReaImGui";
  RecursiveCreateDirectory(filename.c_str(), 0);

  const size_t pathSize { filename.size() };
  filename.resize(pathSize +
    (sizeof(ImGuiID) * 2) + strlen(WDL_DIRCHAR_STR ".ini"));
  snprintf(&filename[pathSize], (filename.size() - pathSize) + 1,
    WDL_DIRCHAR_STR "%0*X.ini",
    static_cast<int>(sizeof(ImGuiID) * 2), ImHashStr(label));

  return filename;
}

Context::Context(const char *label, const int userConfigFlags)
  : m_inFrame { false },
    m_dragState {}, m_cursor {}, m_mouseDown {},
    m_lastFrame { decltype(m_lastFrame)::clock::now() },
    m_name { label, ImGui::FindRenderedTextEnd(label) },
    m_iniFilename { generateIniFilename(label) },
    m_imgui { ImGui::CreateContext(NO_DEFAULT_ATLAS) },
    m_dockers { std::make_unique<DockerList>() },
    m_fonts { std::make_unique<FontList>() }
{
  static const std::string logFn
    { std::string { GetResourcePath() } + WDL_DIRCHAR_STR "imgui_log.txt" };

  setCurrent();

  ImGuiIO &io { m_imgui->IO };
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
  io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
  io.LogFilename = logFn.c_str();
  io.UserData = this;

  setUserConfigFlags(userConfigFlags);

  Platform::install();
  Viewport::install();
}

Context::~Context()
{
  setCurrent();

  if(m_inFrame)
    endFrame(false);
}

void Context::ContextDeleter::operator()(ImGuiContext *imgui)
{
  ImGui::DestroyContext(imgui);
}

int Context::userConfigFlags() const
{
  return m_imgui->IO.ConfigFlags & ~PRIVATE_CONFIG_FLAGS;
}

void Context::setUserConfigFlags(const int userFlags)
{
  m_imgui->IO.ConfigFlags = userFlags | PRIVATE_CONFIG_FLAGS;
}

bool Context::heartbeat()
{
  if(m_inFrame) {
    if(!endFrame(true))
      return false;

    keepAlive();
    m_fonts->keepAliveAll();
  }

  // Keep the frame alive for at least one full timer cycle to prevent contexts
  // created within a defer callback from being immediately destroyed.
  return Resource::heartbeat();
}

ImGuiIO &Context::IO()
{
  return m_imgui->IO;
}

void Context::setCurrent()
{
  ImGui::SetCurrentContext(m_imgui.get());
}

void Context::beginFrame()
{
  assert(!m_inFrame);

  m_inFrame = true;

  Platform::updateMonitors(); // TODO
  m_fonts->update(); // uses the monitor list

  updateFrameInfo();
  updateTheme();
  updateMouseDown();
  updateMousePos();
  updateKeyMods();
  updateSettings();

  ImGui::NewFrame();

  dragSources();
  m_dockers->drawAll();
}

void Context::enterFrame()
{
  setCurrent();

  if(!m_inFrame)
    beginFrame();
}

bool Context::endFrame(const bool render) try
{
  m_inFrame = false;

  setCurrent();

  if(render) {
    updateCursor();
    updateDragDrop();
    ImGui::Render();
  }
  else
    ImGui::EndFrame();

  ImGui::UpdatePlatformWindows();
#ifndef _WIN32
  // WM_KILLFOCUS is incomplete/missing on macOS/Linux with SWELL
  updateFocus();
#endif

  if(render)
    ImGui::RenderPlatformWindowsDefault();

  return true;
}
catch(const imgui_error &e) {
  char message[1024];
  snprintf(message, sizeof(message), "ImGui assertion failed: %s\n", e.what());
  ShowConsoleMsg(message); // cannot use ReaScriptError unless called by a script

  return false;
}
catch(const backend_error &e) {
  char message[1024];
  snprintf(message, sizeof(message), "ReaImGui error: %s\n", e.what());
  ShowConsoleMsg(message);

  return false;
}

void Context::updateFrameInfo()
{
  ImGuiIO &io { m_imgui->IO };

  // Dear ImGui doesn't call MainViewport::getSize
  io.DisplaySize =
    static_cast<Viewport *>(ImGui::GetMainViewport()->PlatformUserData)->getSize();

  const auto now { decltype(m_lastFrame)::clock::now() };
  io.DeltaTime = std::chrono::duration<float> { now - m_lastFrame }.count();
  m_lastFrame = now;
}

void Context::updateCursor()
{
  // this is only called from endFrame, the context is already set
  // setCurrent();

  if(m_imgui->IO.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
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
    if(state & ButtonState_Down)
      return true;
  }

  return false;
}

void Context::mouseInput(const int button, const bool down)
{
  if(down)
    m_mouseDown[button] = ButtonState_Down | ButtonState_DownUnread;
  else {
    // keep ButtonState_DownUnread set to catch clicks shorted than one frame
    m_mouseDown[button] &= ~ButtonState_Down;
  }
}

void Context::updateMouseDown()
{
  ImGuiIO &io { m_imgui->IO };

  size_t i {};
  for(auto &state : m_mouseDown) {
    io.MouseDown[i++] = (state & ButtonState_DownUnread) ||
                        (state & ButtonState_Down);
    state &= ~ButtonState_DownUnread;
  }
}

void Context::updateMousePos()
{
  ImGuiIO &io { m_imgui->IO };

  if(io.WantSetMousePos) {
    // convert to HiDPI on Windows, flip Y on macOS
    ImVec2 scaledPos { io.MousePos };
    Platform::scalePosition(&scaledPos, true);
    SetCursorPos(scaledPos.x, scaledPos.y);
    return;
  }
  else if(m_dragState & DragState_FakeClick) {
    io.MousePos = { -FLT_MAX, -FLT_MAX };
    return;
  }

  io.MousePos = { -FLT_MAX, -FLT_MAX };
  io.MouseHoveredViewport = 0;

  POINT point;
  GetCursorPos(&point);

  ImVec2 pos;
  pos.x = point.x;
  pos.y = point.y;

  ImGuiViewport *viewportForInput { Platform::viewportUnder(pos) };
  ImGuiViewport *viewportForPos;
  if(viewportForInput) {
    viewportForPos = viewportForInput;
    // Viewports with NoInputs set go through WindowFromPoint with
    // WM_NCHITTEST->HTTRANSPARENT when decorations are enabled on Windows
    if(!(viewportForInput->Flags & ImGuiViewportFlags_NoInputs))
      io.MouseHoveredViewport = viewportForInput->ID;
  }
  else if(HWND capture { GetCapture() })
    viewportForPos = ImGui::FindViewportByPlatformHandle(capture);
  else
    viewportForPos = nullptr;

  if(viewportForPos && ImGui::GetMainViewport() != viewportForPos) {
    Platform::scalePosition(&pos);

    io.MousePos.x = pos.x;
    io.MousePos.y = pos.y;
  }
}

void Context::mouseWheel(const bool horizontal, const short delta)
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

  ImGuiIO &io { m_imgui->IO };
  float &wheel { horizontal ? io.MouseWheelH : io.MouseWheel };
  wheel += static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
}

void Context::updateKeyMods()
{
  constexpr int down { 0x8000 };

  ImGuiIO &io { m_imgui->IO };
  io.KeyCtrl  = GetAsyncKeyState(VK_CONTROL) & down;
  io.KeyShift = GetAsyncKeyState(VK_SHIFT)   & down;
  io.KeyAlt   = GetAsyncKeyState(VK_MENU)    & down;
  io.KeySuper = GetAsyncKeyState(VK_LWIN)    & down;
}

void Context::keyInput(const uint8_t key, const bool down)
{
  m_imgui->IO.KeysDown[key] = down;
}

void Context::charInput(const ImWchar codepoint)
{
  if(codepoint < 32 || (codepoint >= 0x7f && codepoint <= 0x9f) || // control chars
      (codepoint >= 0xf700 && codepoint <= 0xf7ff)) // unicode private range
    return;

  m_imgui->IO.AddInputCharacter(codepoint);
}

void Context::updateSettings()
{
  ImGuiIO &io { m_imgui->IO };
  if(io.ConfigFlags & ReaImGuiConfigFlags_NoSavedSettings)
    io.IniFilename = nullptr;
  else
    io.IniFilename = m_iniFilename.c_str();
}

void Context::updateDragDrop()
{
  if(m_dragState & DragState_FirstFrame) {
    m_dragState &= ~DragState_FirstFrame;
    return; // don't clear data until next frame
  }

  if(m_dragState & DragState_Drop)
    m_draggedFiles.clear();

  m_dragState = DragState_None;
}

void Context::dragSources()
{
  // this is only called from beginFrame, the context is already set
  // setCurrent();

  const int flags { ImGuiDragDropFlags_SourceExtern |
                    ImGuiDragDropFlags_SourceAutoExpirePayload };

  if(!m_draggedFiles.empty() && ImGui::BeginDragDropSource(flags)) {
    ImGui::SetDragDropPayload(REAIMGUI_PAYLOAD_TYPE_FILES, nullptr, 0);
    for(const std::string &file : m_draggedFiles) {
      size_t fnPos { file.rfind(WDL_DIRCHAR_STR) };
      if(fnPos == std::string::npos)
        fnPos = 0;
      else
        ++fnPos;
      if(m_draggedFiles.size() > 1)
        ImGui::Bullet();
      ImGui::TextUnformatted(&file[fnPos]);
    }
    ImGui::EndDragDropSource();
  }
}

void Context::beginDrag(std::vector<std::string> &&files)
{
  m_draggedFiles = std::move(files);
  m_mouseDown[ImGuiMouseButton_Left] = ButtonState_Down | ButtonState_DownUnread;
  m_dragState = DragState_FirstFrame | DragState_FakeClick;
}

#ifndef __APPLE__
void Context::beginDrag(HDROP drop)
{
  unsigned int count { DragQueryFile(drop, -1, nullptr, 0) };
  std::vector<std::string> files { count };
  for(unsigned int i { 0 }; i < count; ++i) {
    std::string &file { files[i] };
#ifdef _WIN32
    std::wstring wideFile(DragQueryFile(drop, i, nullptr, 0), L'\0');
    DragQueryFile(drop, i, wideFile.data(), wideFile.size() + 1);
    file = narrow(wideFile);
#else
    file.resize(DragQueryFile(drop, i, nullptr, 0));
    DragQueryFile(drop, i, file.data(), file.size() + 1);
#endif
  }
  beginDrag(std::move(files));
}
#endif

void Context::endDrag(const bool drop)
{
  m_mouseDown[ImGuiMouseButton_Left] &= ~ButtonState_Down;
  if(drop)
    m_dragState = DragState_Drop | (m_dragState & DragState_FirstFrame);
  else {
    m_dragState = DragState_FakeClick;
    m_draggedFiles.clear();
  }
}

ImGuiViewport *Context::focusedViewport() const
{
  const ImGuiPlatformIO &pio { m_imgui->PlatformIO };
  for(int i { 1 }; i < pio.Viewports.Size; ++i) { // skip the main viewport
    ImGuiViewport *viewport { pio.Viewports[i] };
    Viewport *instance { static_cast<Viewport *>(viewport->PlatformUserData) };

    if(instance && instance->hasFocus())
      return viewport;
  }

  return nullptr;
}

void Context::updateFocus()
{
  // don't clear focus before any windows have been opened
  // (so that the first window can have it)
  const bool hasOwnedViewport { m_imgui->PlatformIO.Viewports.Size > 1 };

  if(hasOwnedViewport && !focusedViewport())
    clearFocus();
}

void Context::clearFocus()
{
  TempCurrent cur { this };

  if(ImGui::GetTopMostPopupModal())
    ImGui::ClearActiveID(); // don't close the current modal
  else
    ImGui::FocusWindow(nullptr); // also calls ClearActiveID

  m_imgui->IO.ClearInputKeys();
}

void Context::updateTheme()
{
  // this is only called from beginFrame, the context is already set
  // setCurrent();

  int themeSize;
  const ColorTheme *theme { static_cast<ColorTheme *>(GetColorThemeStruct(&themeSize)) };
  if(static_cast<size_t>(themeSize) < sizeof(ColorTheme))
    return;

  // TODO: Extract a few key colors from REAPER's theme, and compute nicely
  // readable colors for ImGui from them. See vendor/reaper_colortheme.h
  ImVec4 *colors { ImGui::GetStyle().Colors };
  (void)theme; (void)colors;
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
