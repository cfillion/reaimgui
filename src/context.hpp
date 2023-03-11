/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

#include <chrono>
#include <string>
#include <vector>

#include <imgui/imgui.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class DockerList;
class FontList;
class RendererFactory;
class TextureManager;
struct ImGuiContext;
struct ImGuiViewport;

enum ConfigFlags {
  ReaImGuiConfigFlags_NoSavedSettings = 1<<20,
};

constexpr const char *REAIMGUI_PAYLOAD_TYPE_FILES { "_FILES" };

class Context final : public Resource {
public:
  static constexpr const char *api_type_name { "ImGui_Context" };
  static Context *current();

  Context(const char *label, int userConfigFlags = ImGuiConfigFlags_None);
  ~Context();

  // public api
  int userConfigFlags() const;
  void setUserConfigFlags(int);
  void attach(Resource *);
  void detach(Resource *);

  // api helpers
  void setCurrent();
  bool enterFrame();

  // for backends
  void mouseInput(int button, bool down);
  void mouseWheel(bool horizontal, float delta);
  void keyInput(ImGuiKey key, bool down);
  void charInput(unsigned int);
  void charInputUTF16(ImWchar16);
  void beginDrag(std::vector<std::string> &&);
  void beginDrag(HDROP);
  void endDrag(bool drop);
  void updateFocus();
  void enableViewports(bool enable);

  ImGuiIO &IO();
  DockerList &dockers() { return *m_dockers; }
  FontList &fonts() { return *m_fonts; }
  HCURSOR cursor() const { return m_cursor; }
  ImGuiContext *imgui() const { return m_imgui.get(); }
  TextureManager *textureManager() const { return m_textureManager.get(); }
  RendererFactory *rendererFactory() const { return m_rendererFactory.get(); }
  const char *name() const { return m_name.c_str(); }
  const auto &draggedFiles() const { return m_draggedFiles; }

  bool attachable(const Context *) const override { return false; }

protected:
  bool heartbeat() override;

private:
  bool beginFrame();
  bool endFrame(bool render);
  void assertOutOfFrame();

  void updateFrameInfo();
  void updateTheme();
  void updateCursor();
  void updateMouseData();
  void updateSettings();
  void updateDragDrop();

  ImGuiViewport *viewportUnder(ImVec2) const;
  ImGuiViewport *focusedViewport() const;
  void dragSources();
  void clearFocus();

  int m_dropFrameCount;
  HCURSOR m_cursor;
  std::chrono::time_point<std::chrono::steady_clock> m_lastFrame; // monotonic
  std::vector<std::string> m_draggedFiles;
  std::vector<Resource *> m_attachments;
  std::string m_name, m_iniFilename;

  struct ContextDeleter { void operator()(ImGuiContext *); };
  std::unique_ptr<ImGuiContext, ContextDeleter> m_imgui;
  std::unique_ptr<DockerList> m_dockers;
  std::unique_ptr<TextureManager> m_textureManager;
  std::unique_ptr<FontList> m_fonts;
  std::unique_ptr<RendererFactory> m_rendererFactory;
};

using ImGui_Context = Context; // user-facing alias

#endif
