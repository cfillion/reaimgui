#include "api_helper.hpp"

#include "window.hpp"

#include <stdexcept>

constexpr size_t MAX_INSTANCES { 99 };

DEFINE_API(ImGui_Context*, CreateContext,
((const char*, title))((int, x))((int, y))((int, w))((int, h)),
R"(Create a new Dear ImGui context and OS window. The context will remain active as long as it is used every timer cycle.)",
{
  if(Context::count() >= MAX_INSTANCES) {
    ReaScriptError("ReaImGui: ImGui_CreateContext: too many windows");
    return nullptr;
  }

  try {
    return new Context(title, x, y, w, h);
  }
  catch(const std::runtime_error &e) {
    char msg[1024];
    snprintf(msg, sizeof(msg), "ReaImGui: ImGui_CreateContext: %s", e.what());
    ReaScriptError(msg);
    return nullptr;
  }
});

DEFINE_API(void, DestroyContext, ((ImGui_Context*, ctx)),
R"(Close and free the resources used by a context.)",
{
  CHECK_CONTEXT(ctx);
  delete ctx;
});

DEFINE_API(bool, IsContextValid, ((ImGui_Context*, ctx)),
R"(Return whether the context is still active.)",
{
  return Context::exists(ctx);
});

DEFINE_API(void *, GetNativeHwnd, ((ImGui_Context*, ctx)),
R"(Return the native handle for the context's OS window.)",
{
  CHECK_CONTEXT(ctx, nullptr);
  return ctx->window()->nativeHandle();
});

DEFINE_API(bool, IsCloseRequested, ((ImGui_Context*, ctx)),
R"(Return whether the user has requested closing the OS window since the previous frame.)",
{
  CHECK_CONTEXT(ctx, false);
  return ctx->isCloseRequested();
});

// TODO
// DEFINE_API(int, GetContextClearColor, ((ImGui_Context*, ctx)),
// R"(Return the current clear color of the window (0xRRGGBB).
//
// See SetWindowClearColor.)",
// {
//   CHECK_CONTEXT(ctx, 0);
//   return ctx->clearColor().pack(false);
// });
//
// DEFINE_API(void, SetContextClearColor, ((ImGui_Context*, ctx))((int, rgb)),
// R"(Set the current clear color of the window (0xRRGGBB). The default is 0x000000.
//
// See GetClearColor.)",
// {
//   CHECK_CONTEXT(ctx);
//   ctx->setClearColor(Color(rgb, false));
// });

DEFINE_API(double, GetTime, ((ImGui_Context*,ctx)),
"Get global imgui time. Incremented every frame.",
{
  ENTER_CONTEXT(ctx, 0.0);
  return ImGui::GetTime();
});

DEFINE_API(double, GetDeltaTime, ((ImGui_Context*,ctx)),
"Time elapsed since last frame, in seconds.",
{
  ENTER_CONTEXT(ctx, 0.0);
  return ImGui::GetIO().DeltaTime;
});

DEFINE_API(void, ShowAboutWindow, ((ImGui_Context*,ctx))
((bool*,API_RWO(open))),
"Create About window. display Dear ImGui version, credits and build/system information.",
{
  ENTER_CONTEXT(ctx);
  ImGui::ShowAboutWindow(API_RWO(open));
});

DEFINE_API(void, ShowMetricsWindow, ((ImGui_Context*,ctx))
((bool*,API_RWO(open))),
"Create Metrics/Debugger window. display Dear ImGui internals: windows, draw commands, various internal state, etc.",
{
  ENTER_CONTEXT(ctx);
  ImGui::ShowMetricsWindow(API_RWO(open));
});

// TODO: remove this
DEFINE_API(void, FooBar, ((ImGui_Context*,ctx)),
R"()",
{
  ENTER_CONTEXT(ctx);
  ImGui::ShowDemoWindow();
});

// TODO: move somewhere else? (not context-related)
DEFINE_API(double, FLT_MIN, NO_ARGS,
"",
{
  return FLT_MIN;
});
