#ifndef REAIMGUI_CONTEXT_HPP
#define REAIMGUI_CONTEXT_HPP

#include <array>
#include <memory>
#include <tuple>

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
  unsigned int clearColor() const;
  void setClearColor(unsigned int rgba);

  void enterFrame();
  void endFrame(bool render);
  void close();

  void mouseDown(UINT msg);
  void mouseUp(UINT msg);
  void keyInput(uint8_t key, bool down);
  void charInput(unsigned int);

private:
  static WDL_DLGRET proc(HWND, UINT, WPARAM, LPARAM);
  static int translateAccel(MSG *, accelerator_register_t *);

  enum ButtonState {
    Down       = 1<<0,
    DownUnread = 1<<1,
  };

  void setupImGui();
  void beginFrame();
  void updateFrameInfo();
  void updateCursor();
  bool anyMouseDown() const;
  void updateMouseDown();
  void updateMousePos();
  void mouseWheel(UINT msg, short delta);
  void updateKeyMods();

  HWND m_handle;
  bool m_keepAlive, m_inFrame, m_closeReq;
  std::tuple<float, float, float, float> m_clearColor;
  std::array<char, IM_ARRAYSIZE(ImGuiIO::MouseDown)> m_mouseDown;
  accelerator_register_t m_accel;

  ImGuiContext *m_imgui;
  std::unique_ptr<Backend> m_backend;
  std::shared_ptr<Watchdog> m_watchdog;
  std::shared_ptr<ImFontAtlas> m_fontAtlas;
};

#endif
