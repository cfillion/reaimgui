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

#include "docker.hpp"

#include "context.hpp"
#include "platform.hpp"
#include "win32_unicode.hpp"
#include "window.hpp"

#include <cassert>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <reaper_plugin_functions.h>

Docker::Docker(const ReaDockID id)
  : m_id {id}
{
  snprintf(m_windowTitle, sizeof(m_windowTitle), "reaimgui_docker_%X", id);
  m_windowId = ImHashStr(m_windowTitle);
}

void Docker::draw()
{
  if(!isActive()) {
    // 1) Reduce memory usage by not creating unnecessary windows
    // 2) Prevent the dock space's drop target from remaining active at 0,0 after
    //    a temporary docker tab got closed while a window is still being moved.
    ImGui::DockSpace(nodeId(), {0.f, 0.f}, ImGuiDockNodeFlags_KeepAliveOnly);
    return;
  }

  constexpr ImGuiWindowFlags windowFlags {
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoFocusOnAppearing |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
    ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings
  };

  constexpr ImGuiDockNodeFlags dockSpaceFlags {
    ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_PassthruCentralNode
  };

  // Prevent user windows from reporting a StyleVar_WindowMinSize default size
  // for a few frames when opening docked until the viewport is created.
  // This uses the size persisted from the settings.
  ImGui::SetNextWindowSize(rootNode()->Size, ImGuiCond_Once);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
  const bool visible {ImGui::Begin(m_windowTitle, nullptr, windowFlags)};
  ImGui::PopStyleVar(3);
  if(visible) // just in case, altough NoDecoration implies NoCollapse
    ImGui::DockSpace(nodeId(), {0.f, 0.f}, dockSpaceFlags);
  ImGui::End();
}

ImGuiDockNode *Docker::rootNode() const
{
  return ImGui::DockBuilderGetNode(nodeId());
}

static bool anyNodeWindow(const ImGuiDockNode *node,
  bool(*callback)(ImGuiWindow*))
{
  if(!node || node->IsEmpty())
    return false;

  for(int i {}; i < node->Windows.Size; ++i) {
    if(callback(node->Windows[i]))
      return true;
  }

  return anyNodeWindow(node->ChildNodes[0], callback) ||
         anyNodeWindow(node->ChildNodes[1], callback);
}

void Docker::update(bool deactivate)
{
  const ImGuiContext *ctx {ImGui::GetCurrentContext()};
  if(ctx->DockContext.Requests.Size > 0)
    deactivate = false;

  const bool active {anyNodeWindow(rootNode(), [](ImGuiWindow *window) {
    return window->Active || window->WasActive;
  })};

  if(active || (!ctx->MovingWindow && deactivate))
    m_active.set(0, active);

  const bool isDropTarget {Context::current()->dockers().dropTarget() == this};
  if(isDropTarget || deactivate)
    m_active.set(1, isDropTarget);
}

bool Docker::isActive() const
{
  return m_active.any();
}

bool Docker::isDropTarget() const
{
  return m_active.test(1);
}

bool Docker::isNoFocus() const
{
  return anyNodeWindow(rootNode(), [](ImGuiWindow *window) {
    return (window->Flags & ImGuiWindowFlags_NoFocusOnAppearing) != 0;
  });
}

void Docker::moveTo(Docker *target, const bool reuseHost)
{
  assert(target && target != this);

  if(target->isActive()) {
    reset(); // undock all contained windows
    return;
  }

  // The target docker is unused: move our contents to it and take its place
  // to reuse the same platform window and keep using the same docker instance.
  ImVector<const char *> remap;
  ImGui::DockBuilderCopyDockSpace(nodeId(), target->nodeId(), &remap);
  if(reuseHost)
    std::swap(m_id, target->m_id);
}

void Docker::hostViewport(ImGuiViewport *viewport)
{
  assert(!isActive());

  ImGuiWindow *window {static_cast<ImGuiViewportP *>(viewport)->Window};
  if(window->DockNodeAsHost) {
    ImVector<const char *> remap;
    ImGui::DockBuilderCopyDockSpace(window->DockNodeAsHost->ID, nodeId(), &remap);
  }
  else
    ImGui::SetWindowDock(window, nodeId(), ImGuiCond_Always);
}

void Docker::reset()
{
  // the node will be re-created next frame in draw()
  ImGui::DockBuilderRemoveNode(nodeId());
  m_active.reset(0);
}

template <ImGuiID... IDs>
constexpr std::array<Docker, sizeof...(IDs)>
makeDockers(std::integer_sequence<ReaDockID, IDs...>) { return {IDs...}; }

DockerList::DockerList()
  : m_dockers {makeDockers(std::make_integer_sequence<ReaDockID, DOCKER_COUNT>{})},
    m_dropTarget {nullptr}
{
}

void DockerList::drawAll()
{
  if(!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable))
    return;

  const ImGuiPayload *payload {ImGui::GetDragDropPayload()};
  if(payload && payload->IsDataType(IMGUI_PAYLOAD_TYPE_WINDOW)) {
    const ImVec2 cursorPos {Platform::getCursorPos()};
    HWND target {Platform::windowFromPoint(cursorPos)};
    m_dropTarget = findByChildHwnd(target);
    if(!m_dropTarget && IsChild(GetMainHwnd(), target))
      m_dropTarget = findNearby(cursorPos);
  }
  else
    m_dropTarget = nullptr;

  for(Docker &docker : m_dockers) {
    docker.update(false);
    docker.draw();
  }
}

Docker *DockerList::findById(const ReaDockID id)
{
  if(id >= m_dockers.size())
    return nullptr;

  for(Docker &docker : m_dockers) {
    if(docker.id() == id)
      return &docker;
  }

  return nullptr;
}

Docker *DockerList::findByViewport(const ImGuiViewport *viewport)
{
  const auto *viewportPrivate {static_cast<const ImGuiViewportP *>(viewport)};
  if(ImGuiWindow *userWindow {viewportPrivate->Window}) {
    for(Docker &docker : m_dockers) {
      if(docker.windowId() == userWindow->ID)
        return &docker;
    }
  }

  return nullptr;
}

Docker *DockerList::findByChildHwnd(HWND window)
{
  return findById(DockIsChildOfDock(window, nullptr));
}

namespace DockPos {
  // DockGetPosition return values
  enum Pos { Unknown = -1, Bottom, Left, Top, Right, Floating };
}

const char *DockGetPositionName(const int pos)
{
  switch(static_cast<DockPos::Pos>(pos)) {
  case DockPos::Bottom:   return "bottom";
  case DockPos::Left:     return "left";
  case DockPos::Top:      return "top";
  case DockPos::Right:    return "right";
  case DockPos::Floating: return "floating";
  case DockPos::Unknown:  break;
  }

  return "unknown";
}

int CompatDockGetPosition(const int whichDock)
{
  char key[16];
  snprintf(key, sizeof(key), "dockermode%d", whichDock);
  const int mode {static_cast<int>(GetPrivateProfileInt(
    TEXT("REAPER"), WIDEN(key), DockPos::Unknown, WIDEN(get_ini_file())))};

  if(mode == DockPos::Unknown)
    return mode;

  return mode & 0x8000 ? DockPos::Floating : mode & 3;
}

static bool isDockerOrTransport(HWND window, const bool detectTransport)
{
  static const char *localizedTransport
    {LocalizeString("Transport", "DLG_188", 0)};

  TCHAR titleBuf[32] {};
  GetWindowText(window, titleBuf, std::size(titleBuf) - 1);

#ifdef _WIN32
  const std::string &narrowTitle {narrow(titleBuf)};
  const char *title {narrowTitle.c_str()};
#else
  const char *title {titleBuf};
#endif

  return !strcmp(title, "REAPER_dock") ||
         (detectTransport && !strcmp(title, localizedTransport));
}

static RECT closedDockersHitBox()
{
  RECT rect, rulerRect;
  HWND main {GetMainHwnd()}, ruler {GetDlgItem(main, 0x3ed)};
  GetClientRect(main, &rect); // not including decorations
  ClientToScreen(main, reinterpret_cast<POINT *>(&rect));
  ClientToScreen(main, reinterpret_cast<POINT *>(&rect) + 1);

  if(!ruler) // for safety in case the ruler's control ID ever changes
    return rect;

  GetWindowRect(ruler, &rulerRect);

  // Transport: Show transport docked above ruler
  const bool skipTransport {GetToggleCommandState(41604) == 1};

  // Add space taken by the tabbar, transport and toolbar above the ruler
  // Order of controls (top to bottom) depending on the transport bar position:
  // Top of main window: <tabbar> <transport> <toolbar> <dock> <ruler>
  // Above ruler:        <tabbar> <toolbar> <dock> <transport> <ruler>
  for(HWND child {GetWindow(main, GW_CHILD)};
      child; child = GetWindow(child, GW_HWNDNEXT)) {
    if(isDockerOrTransport(child, skipTransport))
      continue;

    RECT childRect;
    GetWindowRect(child, &childRect);
#ifdef __APPLE__
    if(childRect.bottom > rulerRect.top && childRect.bottom < rect.top)
#else
    if(childRect.bottom < rulerRect.top && childRect.bottom > rect.top)
#endif
      rect.top = childRect.bottom;
  }

  return rect;
}

const Docker *DockerList::findNearby(const ImVec2 point) const
{
  constexpr int HANDLE_SIZE {32}; // * Platform::scaleForWindow(main)?

  struct Side { LONG RECT::*dir; float ImVec2::*coord; DockPos::Pos dockPos; };
  constexpr Side sides[] {
    {&RECT::left,   &ImVec2::x, DockPos::Left  },
    {&RECT::top,    &ImVec2::y, DockPos::Top   },
    {&RECT::right,  &ImVec2::x, DockPos::Right },
    {&RECT::bottom, &ImVec2::y, DockPos::Bottom},
  };

  const RECT rect {closedDockersHitBox()};

  DockPos::Pos wantPos {DockPos::Unknown};
  for(const auto side : sides) {
    if(point.*(side.coord) > rect.*(side.dir) - HANDLE_SIZE &&
       point.*(side.coord) < rect.*(side.dir) + HANDLE_SIZE) {
      wantPos = side.dockPos;
      break;
    }
  }

  if(wantPos == DockPos::Unknown)
    return nullptr;

  for(const Docker &docker : m_dockers) {
    if(DockGetPosition(docker.id()) == wantPos)
      return &docker;
  }

  return nullptr;
}

static void restoreMovingWindowFocus()
{
  // Workaround for:
  // - Moved window going under floating dockers
  // - On Linux: Moving floating windows lose focus when a
  //   docker drop target is destroyed.
  const ImGuiContext *ctx {ImGui::GetCurrentContext()};
  if(ctx->MovingWindow && ctx->MovingWindow->ViewportOwned) {
    if(void *viewport {ctx->MovingWindow->Viewport->PlatformUserData})
      static_cast<Viewport *>(viewport)->setFocus();
  }
}

static void DockWindowActivate2(HWND window)
{
  const ImGuiContext *ctx {ImGui::GetCurrentContext()};
  const bool allowStealFocus {!ctx->MovingWindow};

#ifdef _WIN32
  // Workaround for DockWindowActivate stealing focus from the moving window
  HWND dockerWindow;
  bool wasEnabled;
  if(!allowStealFocus) {
    dockerWindow = GetParent(window);
    wasEnabled = IsWindowEnabled(dockerWindow);
    EnableWindow(dockerWindow, false);
  }
#endif

  DockWindowActivate(window);

  // DockWindowActivate alone is not enough to give keyboard focus if the
  // window is the only tab in the docker (the docker was not already open)
  if(allowStealFocus)
    SetFocus(window);
#ifndef _WIN32
  // Bring back to front on macOS and Linux when moving over over floating dockers
  else
    restoreMovingWindowFocus();
#endif

#ifdef _WIN32
  if(!allowStealFocus)
    EnableWindow(dockerWindow, wasEnabled);
#endif
}

DockerHost::DockerHost(Docker *docker, ImGuiViewport *viewport)
  : Viewport {viewport}, m_docker {docker},
    m_window {Platform::createWindow(viewport, this)}
{
}

void DockerHost::create()
{
  m_window->create();
  m_window->setTitle(m_ctx->name()); // for p=2649553

  HWND hwnd {m_window->nativeHandle()};
  m_viewport->PlatformHandle = hwnd;

  const auto &key {m_ctx->screensetKey()};
  Dock_UpdateDockID(key.c_str(), m_docker->id());
  DockWindowAddEx(hwnd, m_ctx->name(), key.c_str(), true);

  // ImGuiViewportFlags_NoFocusOnAppearing is not inherited from the
  // docked windows, but would from the Begin in Docker::draw
  if(!m_docker->isNoFocus())
    DockWindowActivate2(hwnd);
}

void DockerHost::show()
{
  m_window->show();
}

void DockerHost::destroy()
{
  m_viewport->PlatformHandle = nullptr;
  m_window->destroy();
  restoreMovingWindowFocus();

  // Required for dear imgui to create a new viewport for windows
  // that switched from this docker to an newly created node
  // eg. SetNextWindowDockID(-1) then later SetNextWindowDockID(1)
  //
  // Begin() creates a new viewport if the current does not fit the contents
  // and is not minimized. It can be minimized if the docker tab was not
  // the active one at the time of the DockID(1).
  m_viewport->Size = {}, m_viewport->Flags &= ~ImGuiViewportFlags_IsMinimized;
}

HWND DockerHost::nativeHandle() const
{
  return m_window->nativeHandle();
}

void DockerHost::setPosition(ImVec2)
{
}

ImVec2 DockerHost::getPosition() const
{
  return m_window->getPosition();
}

void DockerHost::setSize(ImVec2)
{
}

ImVec2 DockerHost::getSize() const
{
  return m_window->getSize();
}

void DockerHost::setFocus()
{
  m_window->setFocus();
}

bool DockerHost::hasFocus() const
{
  return m_window->hasFocus();
}

bool DockerHost::isMinimized() const
{
  return m_window->isMinimized();
}

void DockerHost::setTitle(const char *)
{
}

void DockerHost::setAlpha(float)
{
}

float DockerHost::scaleFactor() const
{
  return m_window->scaleFactor();
}

void DockerHost::onChanged()
{
  // Can briefly be false after switching to a new imgui node with SetDockID(>0)
  // Viewport size must not be updated so that the window can be un-docked.
  // Otherwise it would stay attached to the viewport and be invisible.
  if(!m_docker->isActive())
    return;

  m_window->onChanged();

  ImGuiViewportP *viewport {static_cast<ImGuiViewportP *>(m_viewport)};
  if(ImGuiWindow *userWindow {viewport->Window}) {
    userWindow->Pos = viewport->Pos = viewport->LastPlatformPos = getPosition();
    userWindow->Size = userWindow->SizeFull =
      viewport->Size = viewport->LastPlatformSize = getSize();
    // not touching LastRendererSize let Renderer::setSize update textures
  }
}

void DockerHost::update()
{
  // Deactivate only after DockSpace handles drag/drop events
  // so that we don't close the docker in the frame during which a window is
  // dropped (when a dock request is created).
  m_docker->update(true);

  m_window->update();

  // Checking m_window->isMinimized here would prevent restoring
  // the moving window to the foreground over floating dockers on macOS
  if(m_docker->isDropTarget())
    DockWindowActivate2(m_window->nativeHandle());

  const int dockIndex {DockIsChildOfDock(m_window->nativeHandle(), nullptr)};
  if(static_cast<ReaDockID>(dockIndex) != m_docker->id())
    m_docker->moveTo(m_ctx->dockers().findById(dockIndex), true);
}

void DockerHost::setIME(ImGuiPlatformImeData *data)
{
  m_window->setIME(data);
}
