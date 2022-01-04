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

#include "docker.hpp"

#include "context.hpp"
#include "platform.hpp"
#include "window.hpp"

#include <cassert>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <reaper_plugin_functions.h>

Docker::Docker(const ReaDockID id)
  : m_id { id }
{
  snprintf(m_windowTitle, sizeof(m_windowTitle), "reaimgui_docker_%X", id);
  m_windowId = ImHashStr(m_windowTitle);
}

void Docker::draw()
{
  constexpr ImGuiWindowFlags windowFlags {
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
    ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoSavedSettings
  };

  constexpr ImGuiDockNodeFlags dockSpaceFlags {
    ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_PassthruCentralNode
  };

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
  const int visible { ImGui::Begin(m_windowTitle, nullptr, windowFlags) };
  ImGui::PopStyleVar(3);
  if(visible) // just in case, altough NoDecoration implies NoCollapse
    ImGui::DockSpace(nodeId(), { 0.f, 0.f }, dockSpaceFlags);
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

bool Docker::isActive() const
{
  return anyNodeWindow(rootNode(), [](ImGuiWindow *window) {
    return
      (window->Active || window->WasActive) &&
      (window->DockIsActive || window->DockTabIsVisible);
  });
}

bool Docker::isNoFocus() const
{
  return anyNodeWindow(rootNode(), [](ImGuiWindow *window) {
    return (window->Flags & ImGuiWindowFlags_NoFocusOnAppearing) != 0;
  });
}

void Docker::moveTo(Docker *target)
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
  reset(); // clear out the previous node to hide it
  std::swap(m_id, target->m_id);
}

void Docker::reset()
{
  // the node will be re-created next frame in draw()
  ImGui::DockBuilderRemoveNode(nodeId());
}

template <ImGuiID... IDs>
constexpr std::array<Docker, sizeof...(IDs)>
makeDockers(std::integer_sequence<ReaDockID, IDs...>) { return { IDs... }; }

DockerList::DockerList()
  : m_dockers { makeDockers(std::make_integer_sequence<ReaDockID, DOCKER_COUNT>{}) }
{
}

void DockerList::drawAll()
{
  if(!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable))
    return;

  for(Docker &docker : m_dockers)
    docker.draw();
}

Docker *DockerList::findById(const ReaDockID id)
{
  for(Docker &docker : m_dockers) {
    if(docker.id() == id)
      return &docker;
  }

  return nullptr;
}

Docker *DockerList::findByViewport(const ImGuiViewport *viewport)
{
  const auto *viewportPrivate { static_cast<const ImGuiViewportP *>(viewport) };
  if(ImGuiWindow *userWindow { viewportPrivate->Window }) {
    for(Docker &docker : m_dockers) {
      if(docker.windowId() == userWindow->ID)
        return &docker;
    }
  }

  return nullptr;
}

DockerHost::DockerHost(Docker *docker, ImGuiViewport *viewport)
  : Viewport { viewport }, m_docker { docker }
{
}

void DockerHost::activate()
{
  m_window.reset(Platform::createWindow(m_viewport, this));
  m_window->create();

  HWND hwnd { m_window->nativeHandle() };
  m_viewport->PlatformHandle = hwnd;

  constexpr const char *INI_KEY { "reaimgui" };
  Dock_UpdateDockID(INI_KEY, m_docker->id());
  DockWindowAddEx(hwnd, m_ctx->name(), INI_KEY, true);

  // ImGuiViewportFlags_NoFocusOnAppearing is not inherited from the
  // docked windows, but would from the Begin in Docker::draw
  if(!m_docker->isNoFocus())
    DockWindowActivate(hwnd);

  m_window->show();
}

void DockerHost::create()
{
}

HWND DockerHost::nativeHandle() const
{
  return m_window ? m_window->nativeHandle() : nullptr;
}

void DockerHost::show()
{
}

void DockerHost::setPosition(ImVec2)
{
}

ImVec2 DockerHost::getPosition() const
{
  return m_window ? m_window->getPosition() : ImVec2{};
}

void DockerHost::setSize(ImVec2)
{
}

ImVec2 DockerHost::getSize() const
{
  return m_window ? m_window->getSize() : ImVec2{};
}

void DockerHost::setFocus()
{
  if(m_window)
    m_window->setFocus();
}

bool DockerHost::hasFocus() const
{
  return m_window ? m_window->hasFocus() : false;
}

bool DockerHost::isMinimized() const
{
  return m_window ? m_window->isMinimized() : true;
}

void DockerHost::setTitle(const char *)
{
}

void DockerHost::update()
{
  if(m_window)
    m_window->update();
}

void DockerHost::render(void *payload)
{
  if(m_window)
    m_window->render(payload);
}

float DockerHost::scaleFactor() const
{
  return m_window ? m_window->scaleFactor() : 1.f;
}

void DockerHost::onChanged()
{
  const bool isActive { m_docker->isActive() };
  if(isActive ^ !!m_window) {
    if(isActive)
      activate();
    else if(!ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            !ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      m_viewport->PlatformHandle = nullptr;
      m_window.reset();
    }
  }

  ImGuiViewportP *viewport { static_cast<ImGuiViewportP *>(m_viewport) };
  if(ImGuiWindow *userWindow { viewport->Window }) {
    userWindow->Pos = viewport->Pos = viewport->LastPlatformPos = getPosition();
    userWindow->Size = userWindow->SizeFull = viewport->LastRendererSize =
      viewport->Size = viewport->LastPlatformSize = getSize();
  }

  if(m_window) {
    const int dockIndex { DockIsChildOfDock(m_window->nativeHandle(), nullptr) };
    if(static_cast<ReaDockID>(dockIndex) != m_docker->id())
      m_docker->moveTo(m_ctx->dockers().findById(dockIndex));

    m_window->onChanged();
  }
}

void DockerHost::setImePosition(const ImVec2 pos)
{
  if(m_window)
    m_window->setImePosition(pos);
}
