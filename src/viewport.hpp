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

#ifndef REAIMGUI_VIEWPORT_HPP
#define REAIMGUI_VIEWPORT_HPP

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class Context;
class Docker;
struct ImGuiPlatformImeData;
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

  // create/destroy called once virtual methods are available
  virtual void create() = 0;
  virtual void destroy() = 0;
  virtual HWND nativeHandle() const = 0;
  virtual void show() = 0;
  virtual void setPosition(ImVec2) = 0;
  virtual ImVec2 getPosition() const;
  virtual void setSize(ImVec2) = 0;
  virtual ImVec2 getSize() const;
  virtual void setFocus() = 0;
  virtual bool hasFocus() const = 0;
  virtual bool isMinimized() const = 0;
  virtual void setTitle(const char *) = 0;
  virtual void setAlpha(float) = 0;
  virtual void update() = 0;
  virtual float scaleFactor() const = 0;
  virtual void onChanged() = 0;
  virtual void setIME(ImGuiPlatformImeData *) = 0;

protected:
  Context *m_ctx;
  ImGuiViewport *m_viewport;
};

#endif
