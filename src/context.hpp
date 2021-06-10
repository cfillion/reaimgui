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

#include "resource.hpp"
#include "settings.hpp"

#include <array>
#include <chrono>
#include <vector>

#include <imgui/imgui.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class FontList;
class Window;
struct ImGuiContext;
struct ImGuiViewport;

enum ConfigFlags {
  ReaImGuiConfigFlags_NoSavedSettings = 1<<20,
};

constexpr const char *REAIMGUI_PAYLOAD_TYPE_FILES { "_FILES" };

class Context : public Resource {
public:
  static constexpr const char *api_type_name { "ImGui_Context" };
  static Context *current();

  Context(const char *name, int configFlags = ImGuiConfigFlags_None);
  ~Context();

  void setCurrent();
  bool inFrame() const { return m_inFrame; }
  void enterFrame();

  void mouseInput(int button, bool down);
  bool anyMouseDown() const;
  void mouseWheel(bool horizontal, short delta);
  void keyInput(uint8_t key, bool down);
  void charInput(ImWchar);
  void beginDrag(std::vector<std::string> &&);
  void beginDrag(HDROP);
  void endDrag(bool drop);
  void clearFocus();
  // void markSettingsDirty();

  ImGuiIO &IO();
  FontList *fonts() { return m_fonts.get(); }
  // Settings &settings() { return m_settings; }
  HCURSOR cursor() const { return m_cursor; }
  ImGuiContext *imgui() const { return m_imgui.get(); }
  const auto &draggedFiles() const { return m_draggedFiles; }

protected:
  bool heartbeat() override;

private:
  void beginFrame();
  bool endFrame(bool render);

  void updateFrameInfo();
  void updateTheme();
  void updateCursor();
  void updateMouseDown();
  void updateMousePos();
  void updateKeyMods();
  void updateDragDrop();
  void updateFocus();

  ImGuiViewport *viewportUnder(POINT) const;
  ImGuiViewport *focusedViewport(bool *hasOwnedViewport = nullptr) const;
  void dragSources();

  bool m_inFrame;
  int m_dragState;
  HCURSOR m_cursor;
  std::array<uint8_t, IM_ARRAYSIZE(ImGuiIO::MouseDown)> m_mouseDown;
  std::chrono::time_point<std::chrono::steady_clock> m_lastFrame; // monotonic
  std::vector<std::string> m_draggedFiles;
  Settings m_settings;

  struct ContextDeleter { void operator()(ImGuiContext *); };
  std::unique_ptr<ImGuiContext, ContextDeleter> m_imgui;
  std::unique_ptr<FontList> m_fonts;
};

#endif
