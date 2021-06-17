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

#ifndef REAIMGUI_VIEWPORT_HPP
#define REAIMGUI_VIEWPORT_HPP

class Context;
class Docker;
struct ImGuiViewport;
struct ImVec2;

class Viewport {
public:
  static void install();

  Viewport(ImGuiViewport *);
  Viewport(const Viewport &) = delete;
  virtual ~Viewport();

  Context *context() const { return m_ctx; }
  ImGuiViewport *viewport() const { return m_viewport; }

  virtual void *create() = 0;
  virtual void show();
  virtual void setPosition(ImVec2) = 0;
  virtual ImVec2 getPosition() const = 0;
  virtual void setSize(ImVec2) = 0;
  virtual ImVec2 getSize() const = 0;
  virtual void setFocus() = 0;
  virtual bool hasFocus() const = 0;
  virtual bool isVisible() const = 0;
  virtual void setTitle(const char *) = 0;
  virtual void update() = 0;
  virtual void render(void *) = 0;
  virtual float scaleFactor() const = 0;
  virtual void onChanged() = 0;
  virtual void setImePosition(ImVec2) = 0;

protected:
  Context *m_ctx;
  ImGuiViewport *m_viewport;
};

#endif
