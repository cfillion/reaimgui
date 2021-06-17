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

#ifndef REAIMGUI_GDK_WINDOW_HPP
#define REAIMGUI_GDK_WINDOW_HPP

#include "window.hpp"

#include <memory>

class OpenGLRenderer;
struct _GdkGLContext;
struct _GDKWindow;
struct LICE_IBitmap;

typedef struct _GdkWindow GdkWindow;
typedef struct _GdkGLContext GdkGLContext;

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
  void onChanged() override;
  void setImePosition(ImVec2) override;

  void uploadFontTex() override;
  std::optional<LRESULT> handleMessage
    (const unsigned int msg, WPARAM wParam, LPARAM) override;

private:
  void initGl();
  void resizeTextures();
  void teardownGl();
  void checkOSWindowChanged();
  void findOSWindow();
  void liceBlit();

  HWND m_windowOwner;
  GdkWindow *m_window;
  GdkGLContext *m_gl;
  unsigned int m_tex, m_fbo;
  OpenGLRenderer *m_renderer;
  ImGuiViewportFlags m_previousFlags;
  int m_defaultDecorations;

  struct LICEDeleter { void operator()(LICE_IBitmap *bm); };
  std::unique_ptr<LICE_IBitmap, LICEDeleter> m_pixels; // used when docked
};

#endif
