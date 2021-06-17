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

#include "viewport.hpp"

#include <array>
#include <memory>

using ReaDockID = unsigned int;
using ImGuiID   = unsigned int;

class Window;

class Docker {
public:
  Docker(ReaDockID id);
  Docker(Docker &) = delete;

  ReaDockID id() const { return m_id; } // REAPER dock index
  ImGuiID nodeId() const { return ~m_id; } // for SetNextWindowDockID
  ImGuiID windowId() const { return m_windowId; }

  void draw();
  bool isActive() const;
  void moveTo(Docker *other);

private:
  void reset();

  ReaDockID m_id;
  ImGuiID m_windowId;
  char m_windowTitle[20];
};

class DockerList {
public:
  static constexpr size_t DOCKER_COUNT { 16 };

  DockerList();
  void drawAll();
  Docker *findById(unsigned int);
  Docker *findByViewport(const ImGuiViewport *);

private:
  std::array<Docker, DOCKER_COUNT> m_dockers;
};

class DockerHost : public Viewport {
public:
  DockerHost(Docker *, ImGuiViewport *);

  void *create() override;
  void show() override;
  void setPosition(ImVec2) override;
  ImVec2 getPosition() const override;
  void setSize(ImVec2) override;
  ImVec2 getSize() const override;
  void setFocus() override;
  bool hasFocus() const override;
  bool isVisible() const override;
  void setTitle(const char *) override;
  void update() override;
  void render(void *) override;
  float scaleFactor() const override;
  void onChanged() override;
  void setImePosition(ImVec2) override;

private:
  void activate();

  Docker *m_docker;
  std::unique_ptr<Window> m_window;
};

#endif
