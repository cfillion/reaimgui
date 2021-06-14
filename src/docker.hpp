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

#ifndef REAIMGUI_DOCKER_HPP
#define REAIMGUI_DOCKER_HPP

#include <array>

using ReaDockID = unsigned int;
using ImGuiID   = unsigned int;
struct ImGuiViewport;

class Docker {
public:
  Docker(ReaDockID id);

  ReaDockID id() const { return m_id; } // REAPER dock index
  void setId(unsigned int newId) { m_id = newId; }
  bool appearing() const { return m_appearing; }
  ImGuiID nodeId() const { return ~m_id; } // for SetNextWindowDockID
  ImGuiID windowId() const { return m_windowId; }

  void draw();
  void moveTo(unsigned int id);
  void remove();

private:
  ReaDockID m_id;
  ImGuiID m_windowId;
  bool m_appearing;
  char m_windowTitle[20];
};

class DockerList {
public:
  static constexpr size_t DOCKER_COUNT { 16 };

  DockerList();
  void drawActive();
  Docker *findById(unsigned int);
  Docker *findByViewport(const ImGuiViewport *);
  void onDockChanged(Docker *docker, ReaDockID newId);

private:
  std::array<Docker, DOCKER_COUNT> m_dockers;
};

#endif
