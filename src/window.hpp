#ifndef REAIMGUI_WINDOW_HPP
#define REAIMGUI_WINDOW_HPP

#include <memory>

#include <reaper_plugin.h>
#include <WDL/WDL/wdltypes.h> 

class Heartbeat;

class Window {
public:
  static REAPER_PLUGIN_HINSTANCE s_instance;
  static bool touch(Window *);
  static void heartbeat();

  Window(const char *title, int x, int y, int w, int h);
  Window(const Window &) = delete;
  ~Window();

  HWND handle() const { return m_handle; }
  void close();

private:
  static WDL_DLGRET proc(HWND, UINT, WPARAM, LPARAM);

  HWND m_handle;
  bool m_keepalive;
  std::shared_ptr<Heartbeat> m_heartbeat;
};

#endif
