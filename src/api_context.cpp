#include "api_helper.hpp"

#include "window.hpp"

using ImGui_Context = Window;

constexpr size_t MAX_INSTANCES { 99 };

DEFINE_API(ImGui_Context*, CreateContext,
((const char*, title))((int, x))((int, y))((int, w))((int, h)),
R"(Create a new window. Call ImGui_UpdateWindow at every timer cycle until destroyed to keep the window active.)",
{
  if(Window::count() >= MAX_INSTANCES)
    return nullptr;

  Window *ctx { new Window(title, x, y, w, h) };
  return ctx;
});

DEFINE_API(bool, IsContextValid, ((ImGui_Context*, ctx)),
R"(Return whether the window is still open.)",
{
  return Window::exists(ctx);
});

DEFINE_API(void, DestroyContext, ((ImGui_Context*, ctx)),
R"(Close and free the resources used by a window.)",
{
  CHECK_WINDOW(ctx);
  ctx->close();
});

DEFINE_API(void *, GetNativeHwnd, ((ImGui_Context*, ctx)),
R"(Return the native handle for the window.)",
{
  CHECK_WINDOW(ctx, nullptr);
  return ctx->handle();
});

DEFINE_API(bool, IsCloseRequested, ((ImGui_Context*, ctx)),
R"(Return whether the user has requested closing the window.)",
{
  CHECK_WINDOW(ctx, false);
  return ctx->isCloseRequested();
});

DEFINE_API(void, UpdateContext, ((ImGui_Context*, ctx)),
R"(Render a frame to the window. This must be called at every global timer cycle.

Perform all draw + update calls one window at a time for best performance.)",
{
  CHECK_WINDOW(ctx);
  ctx->enterFrame();
  ctx->endFrame(true);
});

DEFINE_API(int, GetContextClearColor, ((ImGui_Context*, ctx)),
R"(Return the current clear color of the window (0xRRGGBBAA).

See SetWindowClearColor.)",
{
  CHECK_WINDOW(ctx, 0);
  return ctx->clearColor();
});

DEFINE_API(void, SetContextClearColor, ((ImGui_Context*, ctx))((int, rgba)),
R"(Set the current clear color of the window (0xRRGGBBAA). The default is 0x000000FF.

See GetClearColor.)",
{
  CHECK_WINDOW(ctx);
  ctx->setClearColor(rgba);
});
