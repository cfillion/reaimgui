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

#include "helper.hpp"

DEFINE_API(__LINE__, bool, IsMouseDown, (ImGui_Context*,ctx)
(int,button),
"Is mouse button held?",
{
  FRAME_GUARD;
  return ImGui::IsMouseDown(button);
}

DEFINE_API(__LINE__, double, GetMouseDownDuration, (ImGui_Context*,ctx)
(int,button),
"Duration the mouse button has been down (0.0f == just clicked)",
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  return ctx->IO().MouseDownDuration[button];
}

DEFINE_API(__LINE__, bool, IsMouseClicked, (ImGui_Context*,ctx)
(int,button)(bool*,API_RO(repeat)),
R"(Did mouse button clicked? (went from !Down to Down)

Default values: repeat = false)",
{
  FRAME_GUARD;
  return ImGui::IsMouseClicked(button, valueOr(API_RO(repeat), false));
}

DEFINE_API(__LINE__, void, GetMouseClickedPos, (ImGui_Context*,ctx)
(int,button)(double*,API_W(x))(double*,API_W(y)),
"",
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  const ImVec2 &pos { ctx->IO().MouseClickedPos[button] };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(__LINE__, bool, IsMouseReleased, (ImGui_Context*,ctx)
(int,button),
"Did mouse button released? (went from Down to !Down)",
{
  FRAME_GUARD;
  return ImGui::IsMouseReleased(button);
}

DEFINE_API(__LINE__, bool, IsMouseDoubleClicked, (ImGui_Context*,ctx)
(int,button),
"Did mouse button double-clicked? (note that a double-click will also report ImGui_IsMouseClicked() == true)",
{
  FRAME_GUARD;
  return ImGui::IsMouseDoubleClicked(button);
}

DEFINE_API(__LINE__, bool, IsMouseHoveringRect, (ImGui_Context*,ctx)
(double,r_min_x)(double,r_min_y)(double,r_max_x)(double,r_max_y)
(bool*,API_RO(clip)),
R"(Is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.

Default values: clip = true)",
{
  FRAME_GUARD;
  return ImGui::IsMouseHoveringRect(
    ImVec2(r_min_x, r_min_y), ImVec2(r_max_x, r_max_y),
    valueOr(API_RO(clip), true));
}

DEFINE_API(__LINE__, bool, IsMousePosValid, (ImGui_Context*,ctx)
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
}

DEFINE_API(__LINE__, bool, IsAnyMouseDown, (ImGui_Context*,ctx),
"Is any mouse button held?",
{
  FRAME_GUARD;
  return ImGui::IsAnyMouseDown();
}

DEFINE_API(__LINE__, void, GetMousePos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"",
{
  FRAME_GUARD;
  const ImVec2 &pos { ctx->IO().MousePos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(__LINE__, void, GetMousePosOnOpeningCurrentPopup, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetMousePosOnOpeningCurrentPopup() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(__LINE__, void, GetMouseWheel, (ImGui_Context*,ctx)
(double*,API_W(vertical))(double*,API_W(horizontal)),
"Mouse wheel Vertical: 1 unit scrolls about 5 lines text.",
{
  FRAME_GUARD;
  const ImGuiIO &io { ctx->IO() };
  if(API_W(vertical))   *API_W(vertical)   = io.MouseWheel;
  if(API_W(horizontal)) *API_W(horizontal) = io.MouseWheelH;
}

DEFINE_API(__LINE__, bool, IsMouseDragging, (ImGui_Context*,ctx)
(int,button)(double*,API_RO(lock_threshold)),
R"(Is mouse dragging? (if lock_threshold < -1.0, uses io.MouseDraggingThreshold)

Default values: lock_threshold = -1.0)",
{
  FRAME_GUARD;
  return ImGui::IsMouseDragging(button, valueOr(API_RO(lock_threshold), -1.0));
}

DEFINE_API(__LINE__, void, GetMouseDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.",
{
  FRAME_GUARD;
  const ImVec2 &delta { ctx->IO().MouseDelta };
  *API_W(x) = delta.x, *API_W(y) = delta.y;
}

DEFINE_API(__LINE__, void, GetMouseDragDelta, (ImGui_Context*,ctx)
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
}

DEFINE_API(__LINE__, void, ResetMouseDragDelta, (ImGui_Context*,ctx)
(int*,API_RO(button)),
"Default values: button = ImGui_MouseButton_Left",
{
  FRAME_GUARD;
  ImGui::ResetMouseDragDelta(valueOr(API_RO(button), ImGuiMouseButton_Left));
}

DEFINE_API(__LINE__, int, GetMouseCursor, (ImGui_Context*,ctx),
"Get desired cursor type, reset every frame. This is updated during the frame.",
{
  FRAME_GUARD;
  return ImGui::GetMouseCursor();
}

DEFINE_API(__LINE__, void, SetMouseCursor, (ImGui_Context*,ctx)
(int,cursor_type),
"Set desired cursor type",
{
  FRAME_GUARD;
  IM_ASSERT(cursor_type >= 0 && cursor_type < ImGuiMouseCursor_COUNT);
  ImGui::SetMouseCursor(cursor_type);
}

DEFINE_API(__LINE__, bool, IsKeyDown, (ImGui_Context*,ctx)
(int,key_code),
"Is key being held.",
{
  FRAME_GUARD;
  return ImGui::IsKeyDown(key_code);
}

DEFINE_API(__LINE__, double, GetKeyDownDuration, (ImGui_Context*,ctx)
(int,key_code),
"Duration the keyboard key has been down (0.0f == just pressed)",
{
  FRAME_GUARD;
  IM_ASSERT(key_code >= 0 && key_code < IM_ARRAYSIZE(ImGuiIO::KeysDownDuration));
  return ctx->IO().KeysDownDuration[key_code];
}

DEFINE_API(__LINE__, bool, IsKeyPressed, (ImGui_Context*,ctx)
(int,key_code)(bool*,API_RO(repeat)),
R"(Was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate

Default values: repeat = true)",
{
  FRAME_GUARD;
  return ImGui::IsKeyPressed(key_code, valueOr(API_RO(repeat), true));
}

DEFINE_API(__LINE__, bool, IsKeyReleased, (ImGui_Context*,ctx)
(int,key_code),
"Was key released (went from Down to !Down)?",
{
  FRAME_GUARD;
  return ImGui::IsKeyReleased(key_code);
}

DEFINE_API(__LINE__, int, GetKeyPressedAmount, (ImGui_Context*,ctx)
(int,key_index)(double,repeat_delay)(double,rate),
"Uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate",
{
  FRAME_GUARD;
  return ImGui::GetKeyPressedAmount(key_index, repeat_delay, rate);
}

DEFINE_API(__LINE__, int, GetKeyMods, (ImGui_Context*,ctx),
"Ctrl/Shift/Alt/Super. See ImGui_KeyModFlags_*.",
{
  FRAME_GUARD;
  return ctx->IO().KeyMods;
}

DEFINE_API(__LINE__, bool, GetInputQueueCharacter, (ImGui_Context*,ctx)
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
}

DEFINE_API(__LINE__, void, CaptureKeyboardFromApp, (ImGui_Context*,ctx)
(bool*,API_RO(want_capture_keyboard_value)),
R"(Manually enable or disable capture of keyboard shortcuts in the global scope for the next frame.

Default values: want_capture_keyboard_value = true)",
{
  FRAME_GUARD;
  const bool value { valueOr(API_RO(want_capture_keyboard_value), true) };
  ImGui::CaptureKeyboardFromApp(value);
}

// ImGuiKeyModFlags
DEFINE_ENUM(ImGui, KeyModFlags_None,  "");
DEFINE_ENUM(ImGui, KeyModFlags_Ctrl,  "");
DEFINE_ENUM(ImGui, KeyModFlags_Shift, "");
DEFINE_ENUM(ImGui, KeyModFlags_Alt,   "");
DEFINE_ENUM(ImGui, KeyModFlags_Super, "");

// ImGuiMouseButton
DEFINE_ENUM(ImGui, MouseButton_Left,   "");
DEFINE_ENUM(ImGui, MouseButton_Right,  "");
DEFINE_ENUM(ImGui, MouseButton_Middle, "");

// ImGuiMouseCursor
// DEFINE_ENUM(ImGui, MouseCursor_None,       "");
DEFINE_ENUM(ImGui, MouseCursor_Arrow,      "");
DEFINE_ENUM(ImGui, MouseCursor_TextInput,  "When hovering over ImGui_InputText, etc.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeAll,  "(Unused by Dear ImGui functions)");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNS,   "When hovering over an horizontal border.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeEW,   "When hovering over a vertical border or a column.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNESW, "When hovering over the bottom-left corner of a window.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNWSE, "When hovering over the bottom-right corner of a window.");
DEFINE_ENUM(ImGui, MouseCursor_Hand,       "(Unused by Dear ImGui functions. Use for e.g. hyperlinks)");
DEFINE_ENUM(ImGui, MouseCursor_NotAllowed, "When hovering something with disallowed interaction. Usually a crossed circle.");
