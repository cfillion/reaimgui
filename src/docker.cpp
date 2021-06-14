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

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

Docker::Docker(const ReaDockID id)
  : m_id { id }, m_appearing { true }
{
  snprintf(m_windowTitle, sizeof(m_windowTitle), "reaimgui_docker_%02X", id);
  m_windowId = ImHashStr(m_windowTitle);
}

void Docker::draw()
{
  constexpr ImGuiWindowFlags parentWindowFlags {
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
    ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground
  };

  constexpr ImGuiDockNodeFlags dockSpaceFlags {
    ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_PassthruCentralNode
  };

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
  const int visible { ImGui::Begin(m_windowTitle, nullptr, parentWindowFlags) };
  ImGui::PopStyleVar(3);
  if(visible)
    ImGui::DockSpace(nodeId(), { 0.f, 0.f }, dockSpaceFlags);
  ImGui::End();

  m_appearing = false;
}

static void remapNodeContents(ImGuiDockNode *sourceNode, ImGuiDockNode *targetNode)
{
  if(sourceNode->IsSplitNode()) {
    targetNode->SplitAxis = sourceNode->SplitAxis;
    for(size_t i {}; i < std::size(targetNode->ChildNodes); ++i) {
      std::swap(sourceNode->ChildNodes[i], targetNode->ChildNodes[i]);
      targetNode->ChildNodes[i]->ParentNode = targetNode;
    }
  }

  std::swap(sourceNode->Windows, targetNode->Windows);

  for(int i {}; i < targetNode->Windows.Size; ++i) {
    ImGuiWindow *window { targetNode->Windows[i] };
    window->DockId   = targetNode->ID;
    window->DockNode = targetNode;
  }
}

void Docker::moveTo(const ReaDockID newId)
{
  ImGuiID targetNodeId { ~newId };

  if(ImGuiDockNode *targetNode { ImGui::DockBuilderGetNode(targetNodeId) }) {
    bool horizontal { true };
    while(ImGuiDockNode *childNode { targetNode->ChildNodes[1] }) {
      targetNode = childNode;
      targetNodeId = childNode->ID;
      horizontal ^= true;
    }

    if(!targetNode->IsEmpty()) {
      const ImGuiDir direction { horizontal ? ImGuiDir_Right : ImGuiDir_Down };
      ImGui::DockBuilderSplitNode(targetNodeId, direction, 0.5f,
        &targetNodeId, nullptr);
    }
  }
  else
    ImGui::DockBuilderAddNode(targetNodeId);

  // don't trust ImGuiDockNode pointers to remain valid after operations
  ImGuiDockNode *sourceNode { ImGui::DockBuilderGetNode(nodeId()) },
                *targetNode { ImGui::DockBuilderGetNode(targetNodeId) };

  remapNodeContents(sourceNode, targetNode);
}

void Docker::remove()
{
  ImGui::DockBuilderRemoveNode(nodeId());
  m_appearing = true;
}

template <ImGuiID... IDs>
constexpr std::array<Docker, sizeof...(IDs)>
makeDockers(std::integer_sequence<ReaDockID, IDs...>) { return { IDs... }; }

DockerList::DockerList()
  : m_dockers { makeDockers(std::make_integer_sequence<ReaDockID, DOCKER_COUNT>{}) }
{
}

void DockerList::drawActive()
{
  for(Docker &docker : m_dockers) {
    if(ImGuiDockNode *node { ImGui::DockBuilderGetNode(docker.nodeId()) }) {
      if(docker.appearing() || !node->IsEmpty())
        docker.draw();
      else
        node->LastFrameAlive = ImGui::GetFrameCount();
    }
  }
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
  if(ImGuiWindow *window { static_cast<const ImGuiViewportP *>(viewport)->Window }) {
    for(Docker &docker : m_dockers) {
      if(docker.windowId() == window->ID)
        return &docker;
    }
  }

  return nullptr;
}

void DockerList::onDockChanged(Docker *docker, const ReaDockID newId)
{
  assert(newId < DOCKER_COUNT);

  const bool reuseWindow { !ImGui::DockBuilderGetNode(~newId) };
  docker->moveTo(newId);

  if(reuseWindow) {
    findById(newId)->setId(docker->id());
    docker->setId(newId);
  }
  else
    docker->remove();
}
