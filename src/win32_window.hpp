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

#ifndef REAIMGUI_WIN32_WINDOW_HPP
#define REAIMGUI_WIN32_WINDOW_HPP

#include "window.hpp"

class OpenGLRenderer;

class Win32Window : public Window {
public:
  static float scaleForDpi(unsigned int);
  static unsigned int dpiForMonitor(HMONITOR);
  static unsigned int dpiForWindow(HWND);

  Win32Window(ImGuiViewport *, DockerHost *);
  ~Win32Window() override;

  void create() override;
  void show() override;
  void setPosition(ImVec2) override;
  void setSize(ImVec2) override;
  void setTitle(const char *) override;
  void update() override;
  void render(void *) override;
  float scaleFactor() const override;
  void setIME(ImGuiPlatformImeData *) override;

  void uploadFontTex() override;
  std::optional<LRESULT> handleMessage
    (const unsigned int msg, WPARAM wParam, LPARAM) override;

private:
  struct Class {
    Class();
    ~Class();
  };

  void initPixelFormat();
  void initGL();
  RECT scaledWindowRect(ImVec2 pos, ImVec2 size) const;
  void keyEvent(unsigned int, WPARAM, LPARAM);
  bool modKeyEvent(WPARAM vk, bool down);
  void unstuckModifiers();

  HDC m_dc;
  HGLRC m_gl;
  unsigned int m_dpi;
  OpenGLRenderer *m_renderer;
  DWORD m_style, m_exStyle;
};

#endif
