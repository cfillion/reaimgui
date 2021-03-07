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

#include "plugin_register.hpp"

#include <memory>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class Context;
struct ImDrawData;

class Window {
public:
  enum Accel { PassToWindow = -1, NotOurWindow = 0, EatKeystroke = 1 };

  static HINSTANCE s_instance;

  // gives a default x/y coordinate to center a window to the arrange view
  static int centerX(int width);
  static int centerY(int height);

  Window(const char *title, RECT, Context *);
  Window(const Window &) = delete;
  ~Window();

  void beginFrame();
  void drawFrame(ImDrawData *);
  void endFrame();
  float scaleFactor() const;
  bool handleMessage(unsigned int msg, WPARAM, LPARAM);
  static int translateAccel(MSG *msg, accelerator_register_t *accel);

  HWND nativeHandle() const;

private:
  struct WindowDeleter { void operator()(HWND); };
  using HwndPtr = std::unique_ptr<std::remove_pointer_t<HWND>, WindowDeleter>;

  static LRESULT CALLBACK proc(HWND, unsigned int, WPARAM, LPARAM);
  static HWND createSwellDialog(const char *title);
  static HWND parentHandle();

  struct Impl;
  std::unique_ptr<Impl> m_impl;

  accelerator_register_t m_accel { &translateAccel, true, this };
  PluginRegister m_accelReg { "accelerator", &m_accel };
};

#endif
