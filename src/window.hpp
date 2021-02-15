#ifndef REAIMGUI_WINDOW_HPP
#define REAIMGUI_WINDOW_HPP

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
  static HINSTANCE s_instance;

  Window(const char *title, RECT, Context *);
  Window(const Window &) = delete;
  ~Window();

  void beginFrame();
  void drawFrame(ImDrawData *);
  void endFrame();
  float scaleFactor() const;
  bool handleMessage(unsigned int msg, WPARAM, LPARAM);

  HWND nativeHandle() const;

private:
  struct WindowDeleter { void operator()(HWND); };
  using HwndPtr = std::unique_ptr<std::remove_pointer_t<HWND>, WindowDeleter>;

  static LRESULT CALLBACK proc(HWND, unsigned int, WPARAM, LPARAM);
  static HWND createSwellDialog(const char *title);
  static HWND parentHandle();

  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif
