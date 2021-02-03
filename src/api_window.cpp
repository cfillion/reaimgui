#include "api_helper.hpp"

#include "window.hpp"

using IMGUI_Window = Window;

DEFINE_API(IMGUI_Window *, CreateWindow,
((const char*, title))((int, x))((int, y))((int, w))((int, h)),
R"(Create a new window.)",
{
  Window *window { new Window(title, x, y, w, h) };
  return window;
});

DEFINE_API(bool, DestroyWindow, ((IMGUI_Window *, window)),
R"(Close and free the memory of a window.)",
{
  if(!Window::touch(window))
    return false;

  window->close();
  return true;
});

DEFINE_API(void *, GetWindowHwnd, ((IMGUI_Window *, window)),
R"(Get the native handle for the window.)",
{
  if(!Window::touch(window))
    return nullptr;

  return window->handle();
});
