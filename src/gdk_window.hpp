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

#ifndef REAIMGUI_GDK_WINDOW_HPP
#define REAIMGUI_GDK_WINDOW_HPP

#include "window.hpp"

#include <memory>

class OpenGLRenderer;
class GdkEventHandler;
struct LICE_IBitmap;

typedef struct _GdkGLContext GdkGLContext;
typedef struct _GdkWindow GdkWindow;
typedef struct _GtkIMContext GtkIMContext;

using ImGuiViewportFlags = int;

class GDKWindow : public Window {
public:
  static float globalScaleFactor();

  GDKWindow(ImGuiViewport *, DockerHost *);
  ~GDKWindow() override;

  void create() override;
  void show() override;
  void setPosition(ImVec2) override;
  void setSize(ImVec2) override;
  void setTitle(const char *) override;
  void update() override;
  void render(void *) override;
  float scaleFactor() const override { return globalScaleFactor(); }
  void setIME(ImGuiPlatformImeData *) override;

  void uploadFontTex() override;
  std::optional<LRESULT> handleMessage
    (const unsigned int msg, WPARAM wParam, LPARAM) override;

private:
  void initGl();
  void initIME();
  void resizeTextures();
  void teardownGl();
  void initSoftwareBlit();
  void softwareBlit();
  void keyEvent(WPARAM, LPARAM, bool down);

  GdkGLContext *m_gl;
  unsigned int m_tex, m_fbo;
  OpenGLRenderer *m_renderer;
  ImGuiViewportFlags m_previousFlags;
  int m_defaultDecorations;
  GtkIMContext *m_ime;

  // for docking
  struct LICEDeleter { void operator()(LICE_IBitmap *); };
  std::unique_ptr<LICE_IBitmap, LICEDeleter> m_pixels;
  std::shared_ptr<GdkEventHandler> m_eventHandler;
  std::shared_ptr<GdkWindow> m_offscreen;
};

#endif
