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

#ifndef REAIMGUI_RENDERER_HPP
#define REAIMGUI_RENDERER_HPP

#include <memory>

class Renderer;
class Window;
struct ImVec2;

class RendererFactory {
public:
  // hard-coded to opengl for now
  const char *name() const { return "reaper_imgui_opengl3"; }
  std::unique_ptr<Renderer> create(Window *);

  template<typename T>
  auto getSharedData() const { return std::static_pointer_cast<T>(m_shared.lock()); }

  template<typename T>
  void setSharedData(T d) { m_shared = d; }

protected:
  std::weak_ptr<void> m_shared;
};

class Renderer {
public:
  static void install();

  Renderer(Window *);
  virtual ~Renderer();

  virtual void setSize(ImVec2) = 0;
  virtual void render(void *) = 0;
  virtual void swapBuffers(void *) = 0;

protected:
  Window *m_window;
};

#endif
