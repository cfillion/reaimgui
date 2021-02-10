#include "api_helper.hpp"

#include <stdexcept>

constexpr size_t MAX_INSTANCES { 99 };

DEFINE_API(ImGui_Context*, CreateContext,
((const char*, title))((int, x))((int, y))((int, w))((int, h)),
R"(Create a new window. Call ImGui_UpdateWindow at every timer cycle until destroyed to keep the window active.)",
{
  if(Context::count() >= MAX_INSTANCES)
    return nullptr;

  Context *ctx {};
  try {
    ctx = new Context(title, x, y, w, h);
  }
  catch(const std::runtime_error &e) {
    char msg[1024];
    snprintf(msg, sizeof(msg), "ReaImGui: failed to create context: %s", e.what());
    ReaScriptError(msg);
  }
  return ctx;
});

DEFINE_API(bool, IsContextValid, ((ImGui_Context*, ctx)),
R"(Return whether the window is still open.)",
{
  return Context::exists(ctx);
});

DEFINE_API(void, DestroyContext, ((ImGui_Context*, ctx)),
R"(Close and free the resources used by a window.)",
{
  CHECK_CONTEXT(ctx);
  ctx->close();
});

DEFINE_API(void *, GetNativeHwnd, ((ImGui_Context*, ctx)),
R"(Return the native handle for the window.)",
{
  CHECK_CONTEXT(ctx, nullptr);
  return ctx->handle();
});

DEFINE_API(bool, IsCloseRequested, ((ImGui_Context*, ctx)),
R"(Return whether the user has requested closing the window.)",
{
  CHECK_CONTEXT(ctx, false);
  return ctx->isCloseRequested();
});

DEFINE_API(void, UpdateContext, ((ImGui_Context*, ctx)),
R"(Render a frame to the window. This must be called at every global timer cycle.

Perform all draw + update calls one window at a time for best performance.)",
{
  CHECK_CONTEXT(ctx);
  ctx->enterFrame();
  ctx->endFrame(true);
});

DEFINE_API(int, GetContextClearColor, ((ImGui_Context*, ctx)),
R"(Return the current clear color of the window (0xRRGGBB).

See SetWindowClearColor.)",
{
  CHECK_CONTEXT(ctx, 0);
  return ctx->clearColor().pack(false);
});

DEFINE_API(void, SetContextClearColor, ((ImGui_Context*, ctx))((int, rgb)),
R"(Set the current clear color of the window (0xRRGGBB). The default is 0x000000.

See GetClearColor.)",
{
  CHECK_CONTEXT(ctx);
  ctx->setClearColor(Color(rgb, false));
});

DEFINE_API(double, GetTime, ((ImGui_Context*,ctx)),
"Get global imgui time. Incremented every frame.",
{
  ENTER_CONTEXT(ctx, 0.0);
  return ImGui::GetTime();
});

// TODO: remove this
DEFINE_API(void, FooBar, ((ImGui_Context*,ctx)),
R"()",
{
  CHECK_CONTEXT(ctx);
  ctx->enterFrame();
  ImGui::ShowDemoWindow();
});
