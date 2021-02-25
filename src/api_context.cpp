#include "api_helper.hpp"

#include "window.hpp"

DEFINE_API(ImGui_Context*, CreateContext,
(const char*, title)(int, x)(int, y)(int, w)(int, h),
R"(Create a new Dear ImGui context and OS window. The context will remain active as long as it is used every timer cycle.)",
{
  return new Context { title, x, y, w, h };
});

DEFINE_API(void, DestroyContext, (ImGui_Context*, ctx),
R"(Close and free the resources used by a context.)",
{
  Context::check(ctx);
  delete ctx;
});

// DEFINE_API(bool, IsContextValid, (ImGui_Context*, ctx),
// R"(Return whether the context is still active.)",
// {
//   return Resource::exists(ctx); // TODO: generalize IsContextValid to IsValid(void* resource, const char* type)?
// });

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

DEFINE_API(void, GetDisplaySize, (ImGui_Context*,ctx)
(double*,API_W(w))(double*,API_W(h)),
"",
{
  Context::check(ctx)->enterFrame();
  const ImVec2 &size { ImGui::GetIO().DisplaySize };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
});

DEFINE_API(bool, IsMouseDown, (ImGui_Context*,ctx)
(int,button),
"Is mouse button held?",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsMouseDown(button);
});

DEFINE_API(double, GetMouseDownDuration, (ImGui_Context*,ctx)
(int,button),
"Duration the mouse button has been down (0.0f == just clicked)",
{
  Context::check(ctx)->enterFrame();
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  return ImGui::GetIO().MouseDownDuration[button];
});

DEFINE_API(bool, IsMouseClicked, (ImGui_Context*,ctx)
(int,button)(bool*,API_RO(repeat)),
R"(Did mouse button clicked? (went from !Down to Down)

Defaut values: repeat=false")",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsMouseClicked(button, valueOr(API_RO(repeat), false));
});

DEFINE_API(bool, IsMouseReleased, (ImGui_Context*,ctx)
(int,button),
"Did mouse button released? (went from Down to !Down)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsMouseReleased(button);
});

DEFINE_API(bool, IsMouseDoubleClicked, (ImGui_Context*,ctx)
(int,button),
"Did mouse button double-clicked? (note that a double-click will also report IsMouseClicked() == true)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsMouseDoubleClicked(button);
});

DEFINE_API(bool, IsMouseHoveringRect, (ImGui_Context*,ctx)
(double,left)(double,top)(double,right)(double,bottom)(bool*,API_RO(clip)),
R"(Is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.

Default values: clip=true)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsMouseHoveringRect(
    ImVec2(left, top), ImVec2(right, bottom),
    valueOr(API_RO(clip), true));
});

DEFINE_API(bool, IsMousePosValid, (ImGui_Context*,ctx)
(double*,API_RO(x))(double*,API_RO(y)),
"",
{
  Context::check(ctx)->enterFrame();

  ImVec2 pos;
  const bool customPos { API_RO(x) && API_RO(y) };
  if(customPos) {
    pos.x = *API_RO(x);
    pos.y = *API_RO(y);
  }

  return ImGui::IsMousePosValid(customPos ? &pos : nullptr);
});

DEFINE_API(bool, IsAnyMouseDown, (ImGui_Context*,ctx),
"Is any mouse button held?",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsAnyMouseDown();
});

DEFINE_API(void, GetMousePos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"",
{
  Context::check(ctx)->enterFrame();

  const ImVec2 &pos { ImGui::GetIO().MousePos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(void, GetMousePosOnOpeningCurrentPopup, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)",
{
  Context::check(ctx)->enterFrame();

  const ImVec2 &pos { ImGui::GetMousePosOnOpeningCurrentPopup() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(void, GetMouseWheel, (ImGui_Context*,ctx)
(double*,API_W(vertical))(double*,API_W(horizontal)),
"Mouse wheel Vertical: 1 unit scrolls about 5 lines text.",
{
  Context::check(ctx)->enterFrame();
  const ImGuiIO &io { ImGui::GetIO() };
  if(API_W(vertical))   *API_W(vertical)   = io.MouseWheel;
  if(API_W(horizontal)) *API_W(horizontal) = io.MouseWheelH;
});

DEFINE_API(double, GetMouseWheelH, (ImGui_Context*,ctx),
"Mouse wheel Vertical: 1 unit scrolls about 5 lines text.",
{
  Context::check(ctx)->enterFrame();
  return ImGui::GetIO().MouseWheelH;
});

DEFINE_API(bool, IsMouseDragging, (ImGui_Context*,ctx)
(int,button)(double*,API_RO(lock_threshold)),
R"(Is mouse dragging? (if lock_threshold < -1.0, uses io.MouseDraggingThreshold)

Default values: lock_threshold = -1.0)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::IsMouseDragging(button, valueOr(API_RO(lock_threshold), -1.0));
});

DEFINE_API(void, GetMouseDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.",
{
  Context::check(ctx)->enterFrame();
  const ImVec2 &delta { ImGui::GetIO().MouseDelta };
  *API_W(x) = delta.x, *API_W(y) = delta.y;
});

DEFINE_API(void, GetMouseDragDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y))
(int*,API_RO(button))(double*,API_RO(lock_threshold)),
R"(Return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold).

Default values: button = ImGui_MouseButton_Left, lock_threshold = -1.0)",
{
  Context::check(ctx)->enterFrame();
  const ImVec2 &delta {
    ImGui::GetMouseDragDelta(valueOr(API_RO(button), ImGuiMouseButton_Left),
      valueOr(API_RO(lock_threshold), -1.0))
  };
  *API_W(x) = delta.x, *API_W(y) = delta.y;
});

DEFINE_API(void, ResetMouseDragDelta, (ImGui_Context*,ctx)
(int*,API_RO(button)),
"Default values: button = ImGui_MouseButton_Left",
{
  Context::check(ctx)->enterFrame();
  ImGui::ResetMouseDragDelta(valueOr(API_RO(button), ImGuiMouseButton_Left));
});

// IMGUI_API ImGuiMouseCursor GetMouseCursor();                                                // get desired cursor type, reset in ImGui::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
// IMGUI_API void          SetMouseCursor(ImGuiMouseCursor cursor_type);                       // set desired cursor type
// IMGUI_API void          CaptureMouseFromApp(bool want_capture_mouse_value = true);          // attention: misleading name! manually override io.WantCaptureMouse flag next frame (said flag is entirely left for your application to handle). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse_value;" after the next NewFrame() call.
