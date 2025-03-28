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

#include "context.hpp"

#include "color.hpp"
#include "configvar.hpp"
#include "docker.hpp"
#include "error.hpp"
#include "font.hpp"
#include "keymap.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include "settings.hpp"
#include "texture.hpp"
#include "viewport.hpp"
#include "window.hpp"

#include <cassert>
#include <imgui/imgui_internal.h>
#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h>
#include <zlib/zlib.h>

#ifdef _WIN32
#  include "win32_unicode.hpp"
#endif

enum StateFlags {
  DnD_Cancelled = 1<<0,
  DnD_WasActive = 1<<1,

#ifdef __APPLE__
  // Right Click Emulation
  RCE_Armed  = 1<<2,
  RCE_Active = 1<<3,
#endif
};

constexpr ImGuiMouseButton DND_MouseButton {ImGuiMouseButton_Left};
constexpr ImGuiConfigFlags PRIVATE_CONFIG_FLAGS
  {ImGuiConfigFlags_ViewportsEnable};

static ImFontAtlas * const NO_DEFAULT_ATLAS
  {reinterpret_cast<ImFontAtlas *>(-1)};

class TempCurrent {
public:
  TempCurrent(Context *ctx)
    : m_old {ImGui::GetCurrentContext()} { ctx->setCurrent(); }
  ~TempCurrent() { ImGui::SetCurrentContext(m_old); }

private:
  ImGuiContext *m_old;
};

static std::string generateIniFilename(const ImGuiID id)
{
  std::string filename {GetResourcePath()};

  filename += WDL_DIRCHAR_STR "ReaImGui";
  RecursiveCreateDirectory(filename.c_str(), 0);

  const size_t pathSize {filename.size()}, idSize {sizeof(id) * 2};
  filename.resize(pathSize + idSize + strlen(WDL_DIRCHAR_STR ".ini"));
  snprintf(&filename[pathSize], (filename.size() - pathSize) + 1,
    WDL_DIRCHAR_STR "%0*X.ini", static_cast<int>(idSize), id);

  return filename;
}

Context *Context::current()
{
  if(ImGuiContext *imgui {ImGui::GetCurrentContext()})
    return static_cast<Context *>(imgui->IO.UserData);
  else
    return nullptr;
}

void Context::clearCurrent()
{
  ImGui::SetCurrentContext(nullptr);
}

Context::Context(const char *label, const int userConfigFlags)
  : m_id {ImHashStr(label)}, m_stateFlags {}, m_cursor {},
    m_lastFrame       {decltype(m_lastFrame)::clock::now()               },
    m_name            {label, ImGui::FindRenderedTextEnd(label)          },
    m_iniFilename     {generateIniFilename(m_id)                         },
    m_imgui           {ImGui::CreateContext(NO_DEFAULT_ATLAS)            },
    m_dockers         {std::make_unique<DockerList>()                    },
    m_textureManager  {std::make_unique<TextureManager>()                },
    m_fonts           {std::make_unique<FontList>(m_textureManager.get())},
    m_rendererFactory {std::make_unique<RendererFactory>()               }
{
  if(!*label) // does not prohibit empty window titles
    throw reascript_error {"context label is required"};

  static const std::string logFn
    {std::string {GetResourcePath()} + WDL_DIRCHAR_STR "imgui_log.txt"};

  setCurrent();

  ImGuiIO &io {m_imgui->IO};
  io.BackendRendererName = m_rendererFactory->name();
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
  io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
  io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  io.ConfigDebugIsDebuggerPresent = true;
  io.ConfigDockingNoSplit = Settings::DockingNoSplit;
  io.ConfigDockingTransparentPayload = Settings::DockingTransparentPayload;
  io.ConfigDockingWithShift = Settings::DockingWithShift;
  io.LogFilename = logFn.c_str();
  io.UserData = this;

  setUserConfigFlags(userConfigFlags);
  if(Settings::DockingEnable)
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  Platform::install();
  Renderer::install();
  Viewport::install();

  // prevent imgui from loading settings but not from saving them
  // (so that the saved state is reset to defaults)
  if(Settings::NoSavedSettings)
    m_imgui->SettingsLoaded = true;

  screenset_registerNew(screensetKey().data(), &screensetProc, this);
}

Context::~Context()
{
  setCurrent();
  screenset_unregisterByParam(this);

  if(m_imgui->WithinFrameScope)
    endFrame(false);

  // destroy windows while this and m_imgui are still valid
  ImGui::DestroyPlatformWindows();
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

void Context::assertOutOfFrame()
{
  if(m_imgui->WithinFrameScope)
    throw reascript_error {"cannot modify font texture: a frame has already begun"};
}

void Context::attach(Resource *obj)
{
  if(m_attachments.size() >= 0x400)
    throw reascript_error {"exceeded maximum object attachment limit"};
  if(std::find(m_attachments.begin(), m_attachments.end(), obj)
      != m_attachments.end())
    throw reascript_error {"the object is already attached to this context"};
  else if(!obj->attachable(this))
    throw reascript_error {"the object cannot be attached to this context"};

  if(Font *font {dynamic_cast<Font *>(obj)}) {
    assertOutOfFrame();
    m_fonts->add(font);
  }

  m_attachments.push_back(obj);
}

void Context::detach(Resource *obj)
{
  const auto it {std::find(m_attachments.begin(), m_attachments.end(), obj)};
  if(it == m_attachments.end())
    throw reascript_error {"the object is not attached to this context"};

  if(Font *font {dynamic_cast<Font *>(obj)}) {
    assertOutOfFrame();
    m_fonts->remove(font);
  }

  m_attachments.erase(it);
}

bool Context::heartbeat()
{
  if(m_imgui->WithinFrameScope) {
    if(!endFrame(true))
      return false;

    for(Resource *obj : m_attachments)
      obj->keepAlive();
  }

  // Keep the frame alive for at least one full timer cycle to prevent contexts
  // created within a defer callback from being immediately destroyed.
  return Resource::heartbeat();
}

ImGuiIO &Context::IO()
{
  return m_imgui->IO;
}

ImGuiStyle &Context::style()
{
  return m_imgui->Style;
}

void Context::setCurrent()
{
  ImGui::SetCurrentContext(m_imgui.get());
}

bool Context::beginFrame() try
{
  assert(!m_imgui->WithinFrameScope);

  Platform::updateMonitors(); // TODO only if changed
  m_fonts->update(); // uses the monitor list

  // remove unused textures before texture IDs are given out for this frame
  m_textureManager->cleanup();

  updateFrameInfo();
  updateMouseData();
  updateSettings();

  ImGui::NewFrame();

  dragSources();
  m_dockers->drawAll();

  return true;
}
catch(const backend_error &e) {
  Error::report(this, e);
  return false;
}

bool Context::enterFrame()
{
  setCurrent();

  if(m_imgui->WithinFrameScope)
    return true;
  else
    return beginFrame();
}

bool Context::endFrame(const bool render) try
{
  setCurrent();

  if(!render) {
    ImGui::EndFrame();
    return true;
  }

  updateCursor();
  updateDragDrop();
  ImGui::Render();
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();

#ifdef FOCUS_POLLING
  // WM_KILLFOCUS/WM_ACTIVATE+WA_INACTIVE are incomplete or missing in SWELL
  updateFocus();
#endif

  return true;
}
catch(const imgui_error &e) {
  Error::report(this, e);
  // don't retry EndFrame and error out again during destruction
  m_imgui->WithinFrameScope = false;
  return false;
}
catch(const backend_error &e) {
  Error::report(this, e);
  return false;
}

void Context::updateFrameInfo()
{
  ImGuiIO &io {m_imgui->IO};

  // Dear ImGui doesn't call MainViewport::getSize
  ImGuiViewport *mainViewport {ImGui::GetMainViewport()};
  Viewport *mainViewportInstance
    {static_cast<Viewport *>(mainViewport->PlatformUserData)};
  mainViewport->Pos = mainViewportInstance->getPosition();
  io.DisplaySize = mainViewport->Size = mainViewportInstance->getSize();

  const auto now {decltype(m_lastFrame)::clock::now()};
  io.DeltaTime = std::chrono::duration<float> {now - m_lastFrame}.count();
  m_lastFrame = now;

  // Workaround to prevent stuck keys when keyboard capture is released in
  // reaction to pressing down an action's global shortcut key
  // (REAPER won't send the key up event to us)
  if(io.WantCaptureKeyboard && isAnyKeyDown())
    ImGui::SetNextFrameWantCaptureKeyboard(true);
}

void Context::updateCursor()
{
  if(m_imgui->IO.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
    return;

  // ImGui::GetMouseCursor is only valid after a frame, before Render
  // (it's reset when a new frame is started)
  HCURSOR nativeCursor {Platform::getCursor(ImGui::GetMouseCursor())};
  if(m_cursor != nativeCursor)
    SetCursor(m_cursor = nativeCursor);
}

void Context::updateMouseData()
{
  ImGuiIO &io {m_imgui->IO};

  if(io.WantSetMousePos) {
    ImVec2 scaledPos {io.MousePos};
    Platform::scalePosition(&scaledPos, true);
    SetCursorPos(scaledPos.x, scaledPos.y);
    return;
  }

  ImVec2 pos {Platform::getCursorPos()};

  ImGuiID hoveredViewport {0};
  ImGuiViewport *viewportForPos {nullptr};
  HWND capture {m_draggedFiles.empty() ? Platform::getCapture() : nullptr};
  if(ImGuiViewport *viewportForInput {viewportUnder(pos)}) {
    if(!capture || Window::contextFromHwnd(capture) == this) {
      viewportForPos = viewportForInput;
      hoveredViewport = viewportForInput->ID;
    }
  }
  else if(capture)
    viewportForPos = ImGui::FindViewportByPlatformHandle(capture);

  io.AddMouseViewportEvent(hoveredViewport);

  if(viewportForPos && ImGui::GetMainViewport() != viewportForPos &&
      !(m_stateFlags & DnD_Cancelled))
    Platform::scalePosition(&pos, false, viewportForPos);
  else
    pos.x = pos.y = -FLT_MAX;

  io.AddMousePosEvent(pos.x, pos.y);
}

void Context::mouseInput(int button, const bool down)
{
#ifdef __APPLE__
  if(button == ImGuiMouseButton_Left &&
      m_stateFlags & (down ? RCE_Armed : RCE_Active)) {
    button = ImGuiMouseButton_Right;
    if(down)
      m_stateFlags |= RCE_Active;
    else
      m_stateFlags &= ~RCE_Active;
  }
#endif

  // always disable imgui's right click emulation which forwards Ctrl unlike REAPER's
  // https://github.com/ocornut/imgui/pull/2343#issuecomment-2118602304
  const bool oldMacOSBehaviors {m_imgui->IO.ConfigMacOSXBehaviors};
  m_imgui->IO.ConfigMacOSXBehaviors = false;
  m_imgui->IO.AddMouseButtonEvent(button, down);
  m_imgui->IO.ConfigMacOSXBehaviors = oldMacOSBehaviors;
}

void Context::mouseWheel(const bool horizontal, float delta)
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

  delta /= WHEEL_DELTA;

  if(horizontal)
    m_imgui->IO.AddMouseWheelEvent(delta, 0.0f);
  else
    m_imgui->IO.AddMouseWheelEvent(0.0f, delta);
}

void Context::keyInput(ImGuiKey key, const bool down)
{
#ifdef __APPLE__
  // Preferences > Editing Behavior > Mouse >
  // Control+left-click emulates right-click
  static ConfigVar<int> rightclickemulate {"rightclickemulate"};
  if(rightclickemulate.value_or(false)) {
    switch(key) {
    case ImGuiMod_Ctrl:
    case ImGuiKey_LeftCtrl:
    case ImGuiKey_RightCtrl:
      if(down)
        m_stateFlags |= RCE_Armed;
      else
        m_stateFlags &= ~RCE_Armed;
      return;
    default:
      break;
    }
  }
#endif

  if(ImGui::IsLegacyKey(key))
    key = KeyMap::translateVirtualKey(key);

  m_imgui->IO.AddKeyEvent(key, down);
}

void Context::charInput(const unsigned int codepoint)
{
  if(codepoint < 32 || (codepoint >= 0x7f && codepoint <= 0x9f) || // control chars
      (codepoint >= 0xf700 && codepoint <= 0xf7ff)) // unicode private range
    return;

  m_imgui->IO.AddInputCharacter(codepoint);
}

void Context::charInputUTF16(const ImWchar16 unit)
{
  m_imgui->IO.AddInputCharacterUTF16(unit);
}

void Context::updateSettings()
{
  ImGuiIO &io {m_imgui->IO};
  if(io.ConfigFlags & ReaImGuiConfigFlags_NoSavedSettings)
    io.IniFilename = nullptr;
  else
    io.IniFilename = m_iniFilename.c_str();
}

void Context::updateDragDrop()
{
  // Ensuring drag and drop was active for at least one frame is required for
  // allowing beginDrag+endDrag to be called on the same frame on systems
  // that only notify on drop (SWELL-GDK)
  const bool active_now {ImGui::IsDragDropActive()};
  if(m_stateFlags & DnD_WasActive && !active_now) {
    m_draggedFiles.clear();
    m_stateFlags &= ~(DnD_WasActive | DnD_Cancelled);
  }
  else if(active_now)
    m_stateFlags |= DnD_WasActive;
}

void Context::dragSources()
{
  constexpr ImGuiDragDropFlags flags
    {ImGuiDragDropFlags_SourceExtern | ImGuiDragDropFlags_PayloadAutoExpire};

  if(m_draggedFiles.empty())
    return;

  // Checking &DnD_WasActive is required to support single-frame drag/drop
  // when input queue tickling is disabled
  if(m_stateFlags & DnD_WasActive &&
      !ImGui::IsMouseDown(DND_MouseButton) &&
      !ImGui::IsMouseReleased(DND_MouseButton))
    return;

  if(!ImGui::BeginDragDropSource(flags))
    return;

  ImGui::SetDragDropPayload(REAIMGUI_PAYLOAD_TYPE_FILES, nullptr, 0);
  for(const std::string &file : m_draggedFiles) {
    size_t fnPos {file.rfind(WDL_DIRCHAR_STR)};
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

void Context::beginDrag(std::vector<std::string> &&files)
{
  m_draggedFiles = std::move(files);

  m_imgui->IO.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
  m_imgui->IO.AddMouseButtonEvent(DND_MouseButton, true);
}

#ifndef __APPLE__
void Context::beginDrag(HDROP drop)
{
  unsigned int count {DragQueryFile(drop, -1, nullptr, 0)};
  std::vector<std::string> files {count};
  for(unsigned int i {0}; i < count; ++i) {
    std::string &file {files[i]};
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
  if(!drop) {
    // Keep the mouse position invalid until m_draggedFiles is cleared
    // to prevent accidental drop (IDropSource may call endDrag(false)
    // while the mouse is still over our window, but near another app's
    // window border or title bar).
    m_stateFlags |= DnD_Cancelled;

    m_imgui->IO.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
  }

  m_imgui->IO.AddMouseButtonEvent(DND_MouseButton, false);
}

ImGuiViewport *Context::viewportUnder(const ImVec2 nativePos) const
{
  HWND target {Platform::windowFromPoint(nativePos)};
#ifdef __APPLE__
  target = GetParent(target);
#endif
  if(!target)
    return nullptr;

  ImGuiViewport *viewport {ImGui::FindViewportByPlatformHandle(target)};
  if(viewport && ImGui::GetMainViewport() != viewport)
    return viewport;

  return nullptr;
}

ImGuiViewport *Context::focusedViewport() const
{
  const ImGuiPlatformIO &pio {m_imgui->PlatformIO};
  for(int i {1}; i < pio.Viewports.Size; ++i) { // skip the main viewport
    ImGuiViewport *viewport {pio.Viewports[i]};
    Viewport *instance {static_cast<Viewport *>(viewport->PlatformUserData)};

    if(instance && instance->hasFocus())
      return viewport;
  }

  return nullptr;
}

void Context::updateFocus()
{
  // Don't clear focus before any windows have been opened
  // so that the first window can have it
  // (only required when polling updateFocus every frame, eg. on Linux)
  bool hasHiddenWindows {false};
#ifdef FOCUS_POLLING
  for(int i {}; i < m_imgui->Windows.Size; ++i) {
    if(m_imgui->Windows[i]->Hidden) {
      hasHiddenWindows = true;
      break;
    }
  }
#endif

  if(!hasHiddenWindows && !focusedViewport())
    clearFocus();
}

void Context::clearFocus()
{
  TempCurrent cur {this};

  if(ImGui::GetTopMostPopupModal())
    ImGui::ClearActiveID(); // don't close the current modal
  else
    ImGui::FocusWindow(nullptr); // also calls ClearActiveID

  // let UpdateViewportsNewFrame detect when the same viewport regains focus
  m_imgui->PlatformLastFocusedViewportId = ImGui::GetMainViewport()->ID;

  // ClearInputMouse resets MousePos to -FLT_MAX
  // Restoring it to gain focus on first click on Linux
  const ImVec2 mousePos {m_imgui->IO.MousePos};
  m_imgui->IO.ClearInputKeys();
  m_imgui->IO.ClearInputMouse();
  m_imgui->IO.MousePos = mousePos;
#ifdef __APPLE__
  m_stateFlags &= ~(RCE_Armed | RCE_Active);
#endif

  HWND capture {Platform::getCapture()};
  if(capture && Window::contextFromHwnd(capture) == this) {
    Window *window
      {reinterpret_cast<Window *>(GetWindowLongPtr(capture, GWLP_USERDATA))};
    window->releaseMouse();
  }
}

void Context::enableViewports(const bool enable)
{
  const ImGuiPlatformIO &pio {m_imgui->PlatformIO};
  for(int i {1}; i < pio.Viewports.Size; ++i) { // skip the main viewport
    ImGuiViewport *viewport {pio.Viewports[i]};
    Viewport *instance {static_cast<Viewport *>(viewport->PlatformUserData)};
    EnableWindow(instance->nativeHandle(), enable);
  }
}

void Context::invalidateViewportsPos()
{
  const ImGuiPlatformIO &pio {m_imgui->PlatformIO};
  for(int i {}; i < pio.Viewports.Size; ++i)
    pio.Viewports[i]->PlatformRequestMove = true;
}

bool Context::isAnyKeyDown() const
{
  for(const ImGuiKeyData &keyData : m_imgui->IO.KeysData) {
    if(keyData.Down)
      return true;
  }
  return false;
}

std::string Context::screensetKey() const
{
  return std::format("reaimgui:{:0{}X}", m_id, sizeof(m_id) * 2);
}

LRESULT Context::screensetProc(const int action, const char *id,
  void *user, void *param, const int paramSize)
{
  constexpr int SCREENSET_ACTION_GET_STATE_SIZE {0x102}; // v7.15+
  auto *self {static_cast<Context *>(user)};

  switch(action) {
  case SCREENSET_ACTION_LOAD_STATE:
    if(param) // null if save returned 0
      self->loadScreenset(static_cast<char *>(param), paramSize);
    return 0;
  case SCREENSET_ACTION_GET_STATE_SIZE:
    param = nullptr;
    [[fallthrough]];
  case SCREENSET_ACTION_SAVE_STATE:
    return self->saveScreenset(static_cast<char *>(param), paramSize);
  }

  return 0;
}

struct ScreensetHeader {
  uint32_t version, size;
};

void Context::loadScreenset(const char *buffer, unsigned long bufferSize)
{
  if(bufferSize < sizeof(ScreensetHeader))
    return;

  auto header {reinterpret_cast<const ScreensetHeader *>(buffer)};
  buffer += sizeof(*header), bufferSize -= sizeof(*header);

  if(Color::fromBigEndian(header->version) != 0)
    return;

  unsigned long dataSize {Color::fromBigEndian(header->size)};
  std::unique_ptr<char[]> data {new(std::nothrow) char[dataSize]};
  if(!data)
    return;

  if(Z_OK != uncompress(reinterpret_cast<unsigned char *>(data.get()), &dataSize,
      reinterpret_cast<const unsigned char *>(buffer), bufferSize))
    return;

  if(!dataSize || dataSize != Color::fromBigEndian(header->size))
    return;

  TempCurrent cur {this};
  ImGui::LoadIniSettingsFromMemory(data.get(), dataSize);
}

long Context::saveScreenset(char *buffer, unsigned long bufferSize)
{
  if(m_imgui->SettingsDirtyTimer > 0.0f) {
    TempCurrent cur {this};
    [[maybe_unused]] const char *data {ImGui::SaveIniSettingsToMemory()};
    assert(data == m_imgui->SettingsIniData.c_str());
  }

  const char *data {m_imgui->SettingsIniData.c_str()};
  const unsigned long dataSize {m_imgui->SettingsIniData.size() + 0ul};

  if(!buffer) // SCREENSET_ACTION_GET_STATE_SIZE
    return compressBound(dataSize);

  if(bufferSize < sizeof(ScreensetHeader))
    return 0;

  new(buffer) ScreensetHeader
    {Color::toBigEndian(0), Color::toBigEndian(dataSize)};
  buffer += sizeof(ScreensetHeader), bufferSize -= sizeof(ScreensetHeader);

  if(Z_OK != compress(reinterpret_cast<unsigned char *>(buffer), &bufferSize,
      reinterpret_cast<const unsigned char *>(data), dataSize))
    return 0;

  return sizeof(ScreensetHeader) + bufferSize;
}
