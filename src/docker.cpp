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

bool Docker::isActive() const
{
  const ImGuiDockNode *node { ImGui::DockBuilderGetNode(nodeId()) };

  if(!node || node->IsEmpty())
    return false;

  const ImGuiWindow *window { node->VisibleWindow };
  return window && (window->Active || window->WasActive) && window->DockIsActive;
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

  m_window->show();

  if(!(m_viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing))
    DockWindowActivate(hwnd);
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
    if(isActive) {
      activate();
      m_window->show();
    }
    else {
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
