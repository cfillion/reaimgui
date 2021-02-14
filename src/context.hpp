#ifndef REAIMGUI_CONTEXT_HPP
#define REAIMGUI_CONTEXT_HPP

#include "color.hpp"

#include <array>
#include <chrono>
#include <memory>

#include <imgui/imgui.h>
#include <reaper_plugin.h>
#include <WDL/wdltypes.h>

class Backend;
class Watchdog;
struct ImFontAtlas;
struct ImGuiContext;

class Context {
public:
  static REAPER_PLUGIN_HINSTANCE s_instance;
  static bool exists(Context *);
  static size_t count();
  static void heartbeat();

  Context(const char *title, int x, int y, int w, int h);
  Context(const Context &) = delete;
  ~Context();

  HWND handle() const { return m_handle; }
  bool isCloseRequested() const { return m_closeReq; }
  const Color &clearColor() const { return m_clearColor; }
  void setClearColor(const Color &col) { m_clearColor = col; }

  void enterFrame();
  void close();

  void mouseDown(unsigned int msg);
  void mouseUp(unsigned int msg);
  void keyInput(uint8_t key, bool down);
  void charInput(unsigned int);

private:
  static LRESULT CALLBACK proc(HWND, unsigned int, WPARAM, LPARAM);

  enum ButtonState {
    Down       = 1<<0,
    DownUnread = 1<<1,
  };

  void setupImGui();
  void beginFrame();
  void endFrame(bool render);
  void updateFrameInfo();
  void updateCursor();
  bool anyMouseDown() const;
  void updateMouseDown();
  void updateMousePos();
  void mouseWheel(unsigned int msg, short delta);
  void updateKeyMods();

  HWND m_handle;
  bool m_inFrame, m_closeReq;
  Color m_clearColor;
  std::array<char, IM_ARRAYSIZE(ImGuiIO::MouseDown)> m_mouseDown;
  std::chrono::time_point<std::chrono::steady_clock> m_lastFrame; // monotonic

  ImGuiContext *m_imgui;
  std::unique_ptr<Backend> m_backend;
  std::shared_ptr<Watchdog> m_watchdog;
  std::shared_ptr<ImFontAtlas> m_fontAtlas;
};

#endif
