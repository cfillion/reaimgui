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
#endif

class Context;
struct ImDrawData;

struct WindowConfig {
  RECT clientRect(float scale = 1.f) const;

  std::string title;
  std::optional<int> x, y;
  int w, h;
  int dock;
};

class Window {
public:
  enum Accel { PassToWindow = -1, NotOurWindow = 0, EatKeystroke = 1 };

  static HINSTANCE s_instance;
  static HWND parentHandle();
  static void updateKeyMap();

  Window(const WindowConfig &, Context *);
  Window(const Window &) = delete;
  ~Window();

  void beginFrame();
  void drawFrame(ImDrawData *);
  void endFrame();
  float scaleFactor() const;
  std::optional<LRESULT> handleMessage(unsigned int msg, WPARAM, LPARAM);

  int dock() const;
  void setDock(int);
  HWND nativeHandle() const { return m_hwnd.get(); }

private:
  struct WindowDeleter { void operator()(HWND); };

  static LRESULT CALLBACK proc(HWND, unsigned int, WPARAM, LPARAM);
  static int translateAccel(MSG *msg, accelerator_register_t *accel);

  void createSwellDialog();
  void updateConfig();

  WindowConfig m_cfg;
  Context *m_ctx;
  std::unique_ptr<std::remove_pointer_t<HWND>, WindowDeleter> m_hwnd;
  accelerator_register_t m_accel { &translateAccel, true, this };
  PluginRegister m_accelReg { "accelerator", &m_accel };

  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif
