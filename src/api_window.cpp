#include "api_helper.hpp"

#include "window.hpp"

using ImGui_Window = Window;

constexpr size_t MAX_WINDOWS { 99 };

DEFINE_API(ImGui_Window *, CreateWindow,
((const char*, title))((int, x))((int, y))((int, w))((int, h)),
R"(Create a new window. Call ImGui_UpdateWindow at every timer cycle until destroyed to keep the window active.)",
{
  if(Window::count() >= MAX_WINDOWS)
    return nullptr;

  Window *window { new Window(title, x, y, w, h) };
  return window;
});

DEFINE_API(bool, IsWindowValid, ((ImGui_Window *, window)),
R"(Return whether the window is still open.)",
{
  return Window::exists(window);
});

DEFINE_API(void, DestroyWindow, ((ImGui_Window *, window)),
R"(Close and free the resources used by a window.)",
{
  CHECK_WINDOW(window);
  window->close();
});

DEFINE_API(void *, GetWindowHwnd, ((ImGui_Window *, window)),
R"(Return the native handle for the window.)",
{
  CHECK_WINDOW(window, nullptr);
  return window->handle();
});

DEFINE_API(bool, IsWindowCloseRequested, ((ImGui_Window *, window)),
R"(Return whether the user has requested closing the window.)",
{
  CHECK_WINDOW(window, false);
  return window->isCloseRequested();
});

DEFINE_API(void, UpdateWindow, ((ImGui_Window *, window)),
R"(Render a frame to the window. This must be called at every global timer cycle.

Perform all draw + update calls one window at a time for best performance.)",
{
  CHECK_WINDOW(window);
  window->enterFrame();
  window->endFrame(true);
});

DEFINE_API(int, GetWindowClearColor, ((ImGui_Window *, window)),
R"(Return the current clear color of the window (0xRRGGBBAA).

See SetWindowClearColor.)",
{
  CHECK_WINDOW(window, 0);
  return window->clearColor();
});

DEFINE_API(void, SetWindowClearColor, ((ImGui_Window *, window))((int, rgba)),
R"(Set the current clear color of the window (0xRRGGBBAA). The default is 0x000000FF.

See GetWindowClearColor.)",
{
  CHECK_WINDOW(window);
  window->setClearColor(rgba);
});
