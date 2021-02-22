#ifndef REAIMGUI_CONTEXT_HPP
#define REAIMGUI_CONTEXT_HPP

#include "color.hpp"

#include <array>
#include <chrono>
#include <memory>

#include <imgui/imgui.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class Window;
struct Heartbeat;
struct ImFontAtlas;
struct ImGuiContext;

class Context {
public:
  static bool exists(Context *);
  static Context *check(Context *);
  static size_t count();
  static void heartbeat();

  Context(const char *title, int x, int y, int w, int h);
  Context(const Context &) = delete;
  ~Context();

  void setCloseRequested(bool req = true) { m_closeReq = req; }
  bool isCloseRequested() const { return m_closeReq; }

  const Color &clearColor() const { return m_clearColor; }
  void setClearColor(const Color &col) { m_clearColor = col; }

  void enterFrame();

  void mouseDown(unsigned int msg);
  void mouseUp(unsigned int msg);
  void mouseWheel(unsigned int msg, short delta);
  void keyInput(uint8_t key, bool down);
  void charInput(unsigned int);

  HCURSOR cursor() const { return m_cursor; }
  Window *window() const { return m_window.get(); }

private:
  enum ButtonState {
    Down       = 1<<0,
    DownUnread = 1<<1,
  };

  void setupImGui();
  void beginFrame();
  void endFrame(bool render);
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
  std::array<uint8_t, IM_ARRAYSIZE(ImGuiIO::MouseDown)> m_mouseDown;
  std::chrono::time_point<std::chrono::steady_clock> m_lastFrame; // monotonic

  std::unique_ptr<ImGuiContext, void(*)(ImGuiContext*)> m_imgui;
  std::unique_ptr<Window> m_window; // must be after m_imgui for correct destruction
  std::shared_ptr<Heartbeat> m_heartbeat;
  std::shared_ptr<ImFontAtlas> m_fontAtlas;
};

#endif
