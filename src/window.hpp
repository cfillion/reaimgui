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

#ifndef REAIMGUI_WINDOW_HPP
#define REAIMGUI_WINDOW_HPP

#include "optional.hpp"
#include "plugin_register.hpp"

#include <memory>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#  define TEXT(str) str
#endif

class Context;
class Docker;
struct ImDrawData;
struct ImGuiViewport;
struct ImVec2;

class Window {
public:
  enum Accel { PassToWindow = -1, NotOurWindow = 0, EatKeystroke = 1 };

  static HINSTANCE s_instance;
  static void install();
  static void updateMonitors();
  static ImGuiViewport *viewportUnder(POINT);

  Window(ImGuiViewport *, Context *);
  Window(const Window &) = delete;
  ~Window();

  // platform callbacks
  void show();
  void setPosition(ImVec2);
  ImVec2 getPosition() const;
  void setSize(ImVec2);
  ImVec2 getSize() const;
  void setFocus();
  bool hasFocus() const;
  bool isVisible() const;
  void setTitle(const char *);
  void update();
  void render(void *);
  float scaleFactor() const;
  void onChangedViewport();
  void setImePosition(ImVec2);
  void translatePosition(POINT *, bool toHiDpi = false) const;

  void mouseDown(unsigned int msg);
  void mouseUp(unsigned int msg);

  const char *getSwellClass() const;
  Context *context() const { return m_ctx; }
  HWND nativeHandle() const { return m_hwnd.get(); }
  ImGuiViewport *viewport() const { return m_viewport; }

protected:
  static constexpr const auto *CLASS_NAME { TEXT("reaper_imgui_context") };
  static int translateAccel(MSG *msg, accelerator_register_t *accel);

  void createSwellDialog();
  void commonShow();

  static void platformInstall();
  void uploadFontTex();
  std::optional<LRESULT> handleMessage(unsigned int msg, WPARAM, LPARAM);

  ImGuiViewport *m_viewport;
  Context *m_ctx;
  Docker *m_docker;

  struct Impl;
  std::unique_ptr<Impl> m_impl;

  struct WindowDeleter { void operator()(HWND); };
  std::unique_ptr<std::remove_pointer_t<HWND>, WindowDeleter> m_hwnd;

private:
  static LRESULT CALLBACK proc(HWND, unsigned int, WPARAM, LPARAM);
  static int hwndInfo(HWND, INT_PTR type);

  void installHooks();
  HWND parentHandle();

  float m_previousScale {};
  int m_fontTexVersion {};

  accelerator_register_t m_accel { &translateAccel, true, this };
  PluginRegister m_accelReg { "accelerator", &m_accel };
  std::shared_ptr<PluginRegister> m_hwndInfo;
};

#endif
