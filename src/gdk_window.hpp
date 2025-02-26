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

#ifndef REAIMGUI_GDK_WINDOW_HPP
#define REAIMGUI_GDK_WINDOW_HPP

#include "window.hpp"

#include <memory>

typedef struct _GdkWindow GdkWindow;
typedef struct _GtkIMContext GtkIMContext;

using ImGuiViewportFlags = int;

class GDKWindow final : public Window {
public:
  static float globalScaleFactor();

  GDKWindow(ImGuiViewport *, DockerHost *);
  ~GDKWindow() override;

  void create() override;
  void show() override;
  void setPosition(ImVec2) override;
  void setSize(ImVec2) override;
  void setTitle(const char *) override;
  void setAlpha(float) override;
  void update() override;
  float scaleFactor() const override { return globalScaleFactor(); }
  void setIME(ImGuiPlatformImeData *) override;
  std::optional<LRESULT> handleMessage
    (const unsigned int msg, WPARAM wParam, LPARAM) override;

  GdkWindow *getOSWindow() const;

private:
  static void imePreeditStart(GtkIMContext *, void *);
  static void imePreeditEnd(GtkIMContext *, void *);

  void initIME();
  void softwareBlit();
  void keyEvent(WPARAM, LPARAM, bool down);

  ImGuiViewportFlags m_previousFlags;
  int m_defaultDecorations;
  GtkIMContext *m_ime;
  bool m_imeOpen;
};

#endif
