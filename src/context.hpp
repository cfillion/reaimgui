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

#ifndef REAIMGUI_CONTEXT_HPP
#define REAIMGUI_CONTEXT_HPP

#include "color.hpp"
#include "optional.hpp"
#include "resource.hpp"

#include <array>
#include <chrono>

#include <imgui/imgui.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class Window;
struct WindowConfig;
struct ImFontAtlas;
struct ImGuiContext;

class Context : public Resource {
public:
  static constexpr const char *api_type_name { "ImGui_Context" };

  Context(const WindowConfig &);
  ~Context();

  void setCloseRequested(bool req = true) { m_closeReq = req; }
  bool isCloseRequested() const { return m_closeReq; }

  const Color &clearColor() const { return m_clearColor; }
  void setClearColor(const Color &col) { m_clearColor = col; }

  void setCurrent();
  void setDockNextFrame(int);
  void enterFrame();

  void mouseDown(unsigned int msg);
  void mouseUp(unsigned int msg);
  void mouseWheel(unsigned int msg, short delta);
  void keyInput(uint8_t key, bool down);
  void charInput(unsigned int);
  void clearFocus();

  HCURSOR cursor() const { return m_cursor; }
  Window *window() const { return m_window.get(); }
  ImGuiContext *imgui() const { return m_imgui.get(); }

protected:
  void heartbeat() override;

private:
  enum ButtonState {
    Down       = 1<<0,
    DownUnread = 1<<1,
  };

  void beginFrame();
  void endFrame(bool render, bool prinnyMode = true);
  bool anyMouseDown() const;
  void updateFrameInfo();
  void updateTheme();
  void updateCursor();
  void updateMouseDown();
  void updateMousePos();
  void updateKeyMods();

  bool m_inFrame, m_closeReq;
  Color m_clearColor;
  HCURSOR m_cursor;
  std::optional<int> m_setDockNextFrame;
  std::array<uint8_t, IM_ARRAYSIZE(ImGuiIO::MouseDown)> m_mouseDown;
  std::chrono::time_point<std::chrono::steady_clock> m_lastFrame; // monotonic

  std::unique_ptr<ImGuiContext, void(*)(ImGuiContext*)> m_imgui;
  std::unique_ptr<Window> m_window; // must be after m_imgui for correct destruction
};

#endif
