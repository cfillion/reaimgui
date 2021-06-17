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
#include "viewport.hpp"

#include <memory>

#ifndef _WIN32
#  define TEXT(str) str
#endif

class DockerHost;

class Window : public Viewport {
public:
  enum Accel { PassToWindow = -1, NotOurWindow = 0, EatKeystroke = 1 };

  static HINSTANCE s_instance;
  static void install();
  static void updateMonitors();

  Window(ImGuiViewport *, DockerHost * = nullptr);
  ~Window() override;

  // platform callbacks
  void show() override;
  HWND nativeHandle() const override { return m_hwnd.get(); }
  void setFocus() override;
  bool hasFocus() const override;
  bool isMinimized() const override;
  void onChanged() override;

  void mouseDown(unsigned int msg);
  void mouseUp(unsigned int msg);
  bool isDocked() const { return !!m_dockerHost; }
  void invalidateTextures();

  const char *getSwellClass() const;

protected:
  static constexpr const auto *CLASS_NAME { TEXT("reaper_imgui_context") };
  static LRESULT CALLBACK proc(HWND, unsigned int, WPARAM, LPARAM);

  void createSwellDialog();
  HWND parentHandle();

  virtual void uploadFontTex() = 0;
  virtual std::optional<LRESULT> handleMessage(unsigned int msg, WPARAM, LPARAM) = 0;
  virtual int handleAccelerator(MSG *);

  struct WindowDeleter { void operator()(HWND); };
  std::unique_ptr<std::remove_pointer_t<HWND>, WindowDeleter> m_hwnd;

private:
  static int hwndInfo(HWND, INT_PTR type);
  static int translateAccel(MSG *msg, accelerator_register_t *accel);

  DockerHost *m_dockerHost;

  accelerator_register_t m_accel;
  PluginRegister m_accelReg;
  std::shared_ptr<PluginRegister> m_hwndInfo;

  float m_previousScale;
  int m_fontTexVersion;
};

#endif
