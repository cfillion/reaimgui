#include "api_helper.hpp"

#include "window.hpp"

constexpr size_t MAX_INSTANCES { 99 };

DEFINE_API(ImGui_Context*, CreateContext,
(const char*, title)(int, x)(int, y)(int, w)(int, h),
R"(Create a new Dear ImGui context and OS window. The context will remain active as long as it is used every timer cycle.)",
{
  if(Context::count() >= MAX_INSTANCES)
    throw reascript_error { "exceeded maximum limit" };

  return new Context(title, x, y, w, h);
});

DEFINE_API(void, DestroyContext, (ImGui_Context*, ctx),
R"(Close and free the resources used by a context.)",
{
  Context::check(ctx);
  delete ctx;
});

DEFINE_API(bool, IsContextValid, (ImGui_Context*, ctx),
R"(Return whether the context is still active.)",
{
  return Context::exists(ctx);
});

DEFINE_API(void *, GetNativeHwnd, (ImGui_Context*, ctx),
R"(Return the native handle for the context's OS window.)",
{
  Context::check(ctx);
  return ctx->window()->nativeHandle();
});

DEFINE_API(bool, IsCloseRequested, (ImGui_Context*, ctx),
R"(Return whether the user has requested closing the OS window since the previous frame.)",
{
  Context::check(ctx);
  return ctx->isCloseRequested();
});

// TODO
// DEFINE_API(int, GetContextClearColor, (ImGui_Context*, ctx),
// R"(Return the current clear color of the window (0xRRGGBB).
//
// See SetWindowClearColor.)",
// {
//   Context::check(ctx);
//   return ctx->clearColor().pack(false);
// });
//
// DEFINE_API(void, SetContextClearColor, (ImGui_Context*, ctx)(int, rgb),
// R"(Set the current clear color of the window (0xRRGGBB). The default is 0x000000.
//
// See GetClearColor.)",
// {
//   Context::check(ctx);
//   ctx->setClearColor(Color(rgb, false));
// });

DEFINE_API(double, GetTime, (ImGui_Context*,ctx),
"Get global imgui time. Incremented every frame.",
{
  Context::check(ctx)->enterFrame();
  return ImGui::GetTime();
});

DEFINE_API(double, GetDeltaTime, (ImGui_Context*,ctx),
"Time elapsed since last frame, in seconds.",
{
  Context::check(ctx)->enterFrame();
  return ImGui::GetIO().DeltaTime;
});

DEFINE_API(void, ShowAboutWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(open)),
"Create About window. Display Dear ImGui version, credits and build/system information.",
{
  Context::check(ctx)->enterFrame();
  ImGui::ShowAboutWindow(API_RWO(open));
});

DEFINE_API(void, ShowMetricsWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(open)),
"Create Metrics/Debugger window. Display Dear ImGui internals: windows, draw commands, various internal state, etc.",
{
  Context::check(ctx)->enterFrame();
  ImGui::ShowMetricsWindow(API_RWO(open));
});

// TODO: move somewhere else? (not context-related)
DEFINE_API(double, FLT_MIN, NO_ARGS,
"",
{
  return FLT_MIN;
});
