#include "window.hpp"

#include "heartbeat.hpp"

#include <unordered_set>

#include <reaper_plugin_functions.h>

static std::unordered_set<Window *> g_windows;

REAPER_PLUGIN_HINSTANCE Window::s_instance;

enum SwellDialogResFlags {
  ForceNonChild = 0x400000 | 0x8, // allows not using a resource id
  Resizable = 1,
};

WDL_DLGRET Window::proc(HWND handle, const UINT msg,
  const WPARAM wParam, const LPARAM lParam)
{
  Window *self {
    reinterpret_cast<Window *>(GetWindowLongPtr(handle, GWLP_USERDATA))
  };

  if(!self)
    return false;

  switch(msg) {
  case WM_CLOSE:
    DestroyWindow(handle);
    break;
  case WM_DESTROY:
    delete self;
    break;
  }

  return DefWindowProc(handle, msg, wParam, lParam);
}

bool Window::touch(Window *win)
{
  if(!g_windows.count(win))
    return false;

  return win->m_keepalive = true;
}

void Window::heartbeat()
{
  auto it = g_windows.begin();

  while(it != g_windows.end()) {
    Window *win = *it++;

    if(win->m_keepalive)
      win->m_keepalive = false;
    else
      win->close();
  }
}

Window::Window(const char *title,
    const int x, const int y, const int w, const int h)
  : m_keepalive { true }, m_heartbeat { Heartbeat::get() }
{
  g_windows.emplace(this);

  m_handle = CreateDialogParam(s_instance,
    MAKEINTRESOURCE(ForceNonChild | Resizable),
    GetMainHwnd(),
    proc, reinterpret_cast<LPARAM>(this));

  SetWindowLongPtr(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  SetWindowText(m_handle, title);
  SetWindowPos(m_handle, HWND_TOP, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
  ShowWindow(m_handle, SW_SHOW);
}

Window::~Window()
{
  g_windows.erase(this);
}

void Window::close()
{
  DestroyWindow(m_handle);
}
