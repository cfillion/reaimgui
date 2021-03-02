#include "api_helper.hpp"

#include "version.hpp"
#include "window.hpp"

DEFINE_API(void, GetVersion,
(char*,API_W(reaimgui_version))(int,API_W_SZ(reaimgui_version))
(char*,API_W(imgui_version))(int,API_W_SZ(imgui_version)),
"",
{
  if(API_W(reaimgui_version))
    snprintf(API_W(reaimgui_version), API_W_SZ(reaimgui_version), "%s", REAIMGUI_VERSION);
  if(API_W(imgui_version))
    snprintf(API_W(imgui_version), API_W_SZ(imgui_version), "%s", IMGUI_VERSION);
});

DEFINE_API(ImGui_Context*, CreateContext,
(const char*, title)(int, size_w)(int, size_h)
(int*, API_RO(pos_x))(int*, API_RO(pos_y)),
R"(Create a new Dear ImGui context and OS window. The context will remain active as long as it is used every timer cycle. Pass null x/y coordinates to auto-position the window with the arrange view.

Default values: pos_x = nil, pos_y = nil)",
{
  const int pos_x { valueOr(API_RO(pos_x), Window::centerX(size_w)) },
            pos_y { valueOr(API_RO(pos_y), Window::centerY(size_h)) };
  return new Context { title, pos_x, pos_y, size_w, size_h };
});

DEFINE_API(void, DestroyContext, (ImGui_Context*, ctx),
R"(Close and free the resources used by a context.)",
{
  assertValid(ctx);
  delete ctx;
});

// DEFINE_API(bool, IsContextValid, (ImGui_Context*, ctx),
// R"(Return whether the context is still active.)",
// {
//   return Resource::exists(ctx); // TODO: generalize IsContextValid to IsValid(void* resource, const char* type)?
// });

DEFINE_API(void*, GetNativeHwnd, (ImGui_Context*, ctx),
R"(Return the native handle for the context's OS window.)",
{
  assertValid(ctx);
  return ctx->window()->nativeHandle();
});

DEFINE_API(bool, IsCloseRequested, (ImGui_Context*, ctx),
R"(Return whether the user has requested closing the OS window since the previous frame.)",
{
  assertValid(ctx);
  return ctx->isCloseRequested();
});

DEFINE_API(void, ShowAboutWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(p_open)),
R"(Create About window. Display Dear ImGui version, credits and build/system information.

Default values: p_open = nil)",
{
  FRAME_GUARD;
  ImGui::ShowAboutWindow(API_RWO(p_open));
});

DEFINE_API(void, ShowMetricsWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(p_open)),
R"(Create Metrics/Debugger window. Display Dear ImGui internals: windows, draw commands, various internal state, etc.

Default values: p_open = nil)",
{
  FRAME_GUARD;
  ImGui::ShowMetricsWindow(API_RWO(p_open));
});

DEFINE_API(double, GetTime, (ImGui_Context*,ctx),
"Get global imgui time. Incremented every frame.",
{
  FRAME_GUARD;
  return ImGui::GetTime();
});

DEFINE_API(double, GetDeltaTime, (ImGui_Context*,ctx),
"Time elapsed since last frame, in seconds.",
{
  FRAME_GUARD;
  return ImGui::GetIO().DeltaTime;
});

DEFINE_API(int, GetFrameCount, (ImGui_Context*,ctx),
"Get global imgui frame count. incremented by 1 every frame.",
{
  FRAME_GUARD;
  return ImGui::GetFrameCount();
});

DEFINE_API(void, GetDisplaySize, (ImGui_Context*,ctx)
(double*,API_W(w))(double*,API_W(h)),
"",
{
  FRAME_GUARD;
  const ImVec2 &size { ImGui::GetIO().DisplaySize };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
});

DEFINE_API(bool, IsMouseDown, (ImGui_Context*,ctx)
(int,button),
"Is mouse button held?",
{
  FRAME_GUARD;
  return ImGui::IsMouseDown(button);
});

DEFINE_API(double, GetMouseDownDuration, (ImGui_Context*,ctx)
(int,button),
"Duration the mouse button has been down (0.0f == just clicked)",
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  return ImGui::GetIO().MouseDownDuration[button];
});

DEFINE_API(bool, IsMouseClicked, (ImGui_Context*,ctx)
(int,button)(bool*,API_RO(repeat)),
R"(Did mouse button clicked? (went from !Down to Down)

Default values: repeat = false)",
{
  FRAME_GUARD;
  return ImGui::IsMouseClicked(button, valueOr(API_RO(repeat), false));
});

DEFINE_API(void, GetMouseClickedPos, (ImGui_Context*,ctx)
(int,button)(double*,API_W(x))(double*,API_W(y)),
"",
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  const ImVec2 &pos { ImGui::GetIO().MouseClickedPos[button] };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(bool, IsMouseReleased, (ImGui_Context*,ctx)
(int,button),
"Did mouse button released? (went from Down to !Down)",
{
  FRAME_GUARD;
  return ImGui::IsMouseReleased(button);
});

DEFINE_API(bool, IsMouseDoubleClicked, (ImGui_Context*,ctx)
(int,button),
"Did mouse button double-clicked? (note that a double-click will also report IsMouseClicked() == true)",
{
  FRAME_GUARD;
  return ImGui::IsMouseDoubleClicked(button);
});

DEFINE_API(bool, IsMouseHoveringRect, (ImGui_Context*,ctx)
(double,r_min_x)(double,r_min_y)(double,r_max_x)(double,r_max_y)
(bool*,API_RO(clip)),
R"(Is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.

Default values: clip = true)",
{
  FRAME_GUARD;
  return ImGui::IsMouseHoveringRect(
    ImVec2(r_min_x, r_min_y), ImVec2(r_max_x, r_max_y),
    valueOr(API_RO(clip), true));
});

DEFINE_API(bool, IsMousePosValid, (ImGui_Context*,ctx)
(double*,API_RO(mouse_pos_x))(double*,API_RO(mouse_pos_y)),
"Default values: mouse_pos_x = nil, mouse_pos_y = nil",
{
  FRAME_GUARD;

  ImVec2 pos;
  const bool customPos { API_RO(mouse_pos_x) && API_RO(mouse_pos_y) };
  if(customPos) {
    pos.x = *API_RO(mouse_pos_x);
    pos.y = *API_RO(mouse_pos_y);
  }

  return ImGui::IsMousePosValid(customPos ? &pos : nullptr);
});

DEFINE_API(bool, IsAnyMouseDown, (ImGui_Context*,ctx),
"Is any mouse button held?",
{
  FRAME_GUARD;
  return ImGui::IsAnyMouseDown();
});

DEFINE_API(void, GetMousePos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetIO().MousePos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(void, GetMousePosOnOpeningCurrentPopup, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetMousePosOnOpeningCurrentPopup() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(void, GetMouseWheel, (ImGui_Context*,ctx)
(double*,API_W(vertical))(double*,API_W(horizontal)),
"Mouse wheel Vertical: 1 unit scrolls about 5 lines text.",
{
  FRAME_GUARD;
  const ImGuiIO &io { ImGui::GetIO() };
  if(API_W(vertical))   *API_W(vertical)   = io.MouseWheel;
  if(API_W(horizontal)) *API_W(horizontal) = io.MouseWheelH;
});

DEFINE_API(double, GetMouseWheelH, (ImGui_Context*,ctx),
"Mouse wheel Vertical: 1 unit scrolls about 5 lines text.",
{
  FRAME_GUARD;
  return ImGui::GetIO().MouseWheelH;
});

DEFINE_API(bool, IsMouseDragging, (ImGui_Context*,ctx)
(int,button)(double*,API_RO(lock_threshold)),
R"(Is mouse dragging? (if lock_threshold < -1.0, uses io.MouseDraggingThreshold)

Default values: lock_threshold = -1.0)",
{
  FRAME_GUARD;
  return ImGui::IsMouseDragging(button, valueOr(API_RO(lock_threshold), -1.0));
});

DEFINE_API(void, GetMouseDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.",
{
  FRAME_GUARD;
  const ImVec2 &delta { ImGui::GetIO().MouseDelta };
  *API_W(x) = delta.x, *API_W(y) = delta.y;
});

DEFINE_API(void, GetMouseDragDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y))
(int*,API_RO(button))(double*,API_RO(lock_threshold)),
R"(Return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold).

Default values: button = ImGui_MouseButton_Left, lock_threshold = -1.0)",
{
  FRAME_GUARD;
  const ImVec2 &delta {
    ImGui::GetMouseDragDelta(valueOr(API_RO(button), ImGuiMouseButton_Left),
      valueOr(API_RO(lock_threshold), -1.0))
  };
  if(API_W(x)) *API_W(x) = delta.x;
  if(API_W(y)) *API_W(y) = delta.y;
});

DEFINE_API(void, ResetMouseDragDelta, (ImGui_Context*,ctx)
(int*,API_RO(button)),
"Default values: button = ImGui_MouseButton_Left",
{
  FRAME_GUARD;
  ImGui::ResetMouseDragDelta(valueOr(API_RO(button), ImGuiMouseButton_Left));
});

DEFINE_API(int, GetMouseCursor, (ImGui_Context*,ctx),
"Get desired cursor type, reset every frame. This is updated during the frame.",
{
  FRAME_GUARD;
  return ImGui::GetMouseCursor();
});

DEFINE_API(void, SetMouseCursor, (ImGui_Context*,ctx)
(int,cursor_type),
"Set desired cursor type",
{
  FRAME_GUARD;
  IM_ASSERT(cursor_type >= 0 && cursor_type < ImGuiMouseCursor_COUNT);
  ImGui::SetMouseCursor(cursor_type);
});

DEFINE_API(bool, IsKeyDown, (ImGui_Context*,ctx)
(int,key_code),
"Is key being held.",
{
  FRAME_GUARD;
  return ImGui::IsKeyDown(key_code);
});

DEFINE_API(double, GetKeyDownDuration, (ImGui_Context*,ctx)
(int,key_code),
"Duration the keyboard key has been down (0.0f == just pressed)",
{
  FRAME_GUARD;
  IM_ASSERT(key_code >= 0 && key_code < IM_ARRAYSIZE(ImGuiIO::KeysDownDuration));
  return ImGui::GetIO().KeysDownDuration[key_code];
});

DEFINE_API(bool, IsKeyPressed, (ImGui_Context*,ctx)
(int,key_code)(bool*,API_RO(repeat)),
R"(Was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate

Default values: repeat = true)",
{
  FRAME_GUARD;
  return ImGui::IsKeyPressed(key_code, valueOr(API_RO(repeat), true));
});

DEFINE_API(bool, IsKeyReleased, (ImGui_Context*,ctx)
(int,key_code),
"Was key released (went from Down to !Down)?",
{
  FRAME_GUARD;
  return ImGui::IsKeyReleased(key_code);
});

DEFINE_API(int, GetKeyMods, (ImGui_Context*,ctx),
"Ctrl/Shift/Alt/Super. See ImGui_KeyModFlags_*.",
{
  FRAME_GUARD;
  return ImGui::GetIO().KeyMods;
});

DEFINE_API(bool, GetInputQueueCharacter, (ImGui_Context*,ctx)
(int,idx)(int*,API_W(unicode_char)),
"Read from ImGui's character input queue. Call with increasing idx until false is returned.",
{
  FRAME_GUARD;
  const ImGuiIO &io { ImGui::GetIO() };
  if(idx >= 0 && idx < io.InputQueueCharacters.Size) {
    if(API_W(unicode_char)) *API_W(unicode_char) = io.InputQueueCharacters[idx];
    return true;
  }

  return false;
});
