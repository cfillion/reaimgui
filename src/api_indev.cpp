/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "api_helper.hpp"

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
  return ctx->IO().MouseDownDuration[button];
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
  const ImVec2 &pos { ctx->IO().MouseClickedPos[button] };
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
"Did mouse button double-clicked? (note that a double-click will also report ImGui_IsMouseClicked() == true)",
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
  const ImVec2 &pos { ctx->IO().MousePos };
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
  const ImGuiIO &io { ctx->IO() };
  if(API_W(vertical))   *API_W(vertical)   = io.MouseWheel;
  if(API_W(horizontal)) *API_W(horizontal) = io.MouseWheelH;
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
  const ImVec2 &delta { ctx->IO().MouseDelta };
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
  return ctx->IO().KeysDownDuration[key_code];
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

DEFINE_API(int, GetKeyPressedAmount, (ImGui_Context*,ctx)
(int,key_index)(double,repeat_delay)(double,rate),
"Uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate",
{
  FRAME_GUARD;
  return ImGui::GetKeyPressedAmount(key_index, repeat_delay, rate);
});

DEFINE_API(int, GetKeyMods, (ImGui_Context*,ctx),
"Ctrl/Shift/Alt/Super. See ImGui_KeyModFlags_*.",
{
  FRAME_GUARD;
  return ctx->IO().KeyMods;
});

DEFINE_API(bool, GetInputQueueCharacter, (ImGui_Context*,ctx)
(int,idx)(int*,API_W(unicode_char)),
"Read from ImGui's character input queue. Call with increasing idx until false is returned.",
{
  FRAME_GUARD;
  const ImGuiIO &io { ctx->IO() };
  if(idx >= 0 && idx < io.InputQueueCharacters.Size) {
    if(API_W(unicode_char)) *API_W(unicode_char) = io.InputQueueCharacters[idx];
    return true;
  }

  return false;
});

DEFINE_API(void, CaptureKeyboardFromApp, (ImGui_Context*,ctx)
(bool*,API_RO(want_capture_keyboard_value)),
R"(Manually enable or disable capture of keyboard shortcuts in the global scope for the next frame.

Default values: want_capture_keyboard_value = true)",
{
  FRAME_GUARD;
  const bool value { valueOr(API_RO(want_capture_keyboard_value), true) };
  ImGui::CaptureKeyboardFromApp(value);
});
