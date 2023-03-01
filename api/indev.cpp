/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

#include <imgui/imgui_internal.h> // GetKeyData

API_SECTION("Keyboard & Mouse");

DEFINE_SECTION(mouse, ROOT_SECTION, "Mouse",
R"(To refer to a mouse button, you may use named enums in your code e.g.
MouseButton_Left, MouseButton_Right.

You can also use regular integer: it is forever guaranteed that
0=Left, 1=Right, 2=Middle. Buttons 3 and 4 do not have a named enum.

Dragging operations are only reported after mouse has moved a certain distance
away from the initial clicking position (see 'lock_threshold' parameters and
'ConfigVar_MouseDragThreshold').)");

DEFINE_API(bool, IsMouseDown, (ImGui_Context*,ctx)
(int,button),
"Is mouse button held?")
{
  FRAME_GUARD;
  return ImGui::IsMouseDown(button);
}

DEFINE_API(double, GetMouseDownDuration, (ImGui_Context*,ctx)
(int,button),
"Duration the mouse button has been down (0.0 == just clicked)")
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  return ctx->IO().MouseDownDuration[button];
}

DEFINE_API(bool, IsMouseClicked, (ImGui_Context*,ctx)
(int,button)(bool*,API_RO(repeat),false),
R"(Did mouse button clicked? (went from !Down to Down).
Same as GetMouseClickedCount() == 1.)")
{
  FRAME_GUARD;
  return ImGui::IsMouseClicked(button, API_RO_GET(repeat));
}

DEFINE_API(void, GetMouseClickedPos, (ImGui_Context*,ctx)
(int,button)(double*,API_W(x))(double*,API_W(y)),
"")
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  const ImVec2 &pos { ctx->IO().MouseClickedPos[button] };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(bool, IsMouseReleased, (ImGui_Context*,ctx)
(int,button),
"Did mouse button released? (went from Down to !Down)")
{
  FRAME_GUARD;
  return ImGui::IsMouseReleased(button);
}

DEFINE_API(bool, IsMouseDoubleClicked, (ImGui_Context*,ctx)
(int,button),
R"(Did mouse button double-clicked? Same as GetMouseClickedCount() == 2.
(Note that a double-click will also report IsMouseClicked() == true))")
{
  FRAME_GUARD;
  return ImGui::IsMouseDoubleClicked(button);
}

DEFINE_API(int, GetMouseClickedCount, (ImGui_Context*,ctx)
(int,button),
"Return the number of successive mouse-clicks at the time where a click happen (otherwise 0).")
{
  FRAME_GUARD;
  return ImGui::GetMouseClickedCount(button);
}

DEFINE_API(bool, IsMouseHoveringRect, (ImGui_Context*,ctx)
(double,r_min_x)(double,r_min_y)(double,r_max_x)(double,r_max_y)
(bool*,API_RO(clip),true),
R"(Is mouse hovering given bounding rect (in screen space).
Clipped by current clipping settings, but disregarding of other consideration
of focus/window ordering/popup-block.)")
{
  FRAME_GUARD;
  return ImGui::IsMouseHoveringRect(
    ImVec2(r_min_x, r_min_y), ImVec2(r_max_x, r_max_y), API_RO_GET(clip));
}

DEFINE_API(bool, IsMousePosValid, (ImGui_Context*,ctx)
(double*,API_RO(mouse_pos_x))(double*,API_RO(mouse_pos_y)),
"")
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

DEFINE_API(bool, IsAnyMouseDown, (ImGui_Context*,ctx),
"Is any mouse button held?")
{
  FRAME_GUARD;
  return ImGui::IsAnyMouseDown();
}

DEFINE_API(void, GetMousePos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"")
{
  FRAME_GUARD;
  const ImVec2 &pos { ctx->IO().MousePos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(void, GetMousePosOnOpeningCurrentPopup, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
R"(Retrieve mouse position at the time of opening popup we have BeginPopup()
into (helper to avoid user backing that value themselves).)")
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetMousePosOnOpeningCurrentPopup() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(void, GetMouseWheel, (ImGui_Context*,ctx)
(double*,API_W(vertical))(double*,API_W(horizontal)),
R"(Vertical: 1 unit scrolls about 5 lines text. >0 scrolls Up, <0 scrolls Down.
Hold SHIFT to turn vertical scroll into horizontal scroll

Horizontal: >0 scrolls Left, <0 scrolls Right.
Most users don't have a mouse with a horizontal wheel.)")
{
  FRAME_GUARD;
  const ImGuiIO &io { ctx->IO() };
  if(API_W(vertical))   *API_W(vertical)   = io.MouseWheel;
  if(API_W(horizontal)) *API_W(horizontal) = io.MouseWheelH;
}

DEFINE_API(bool, IsMouseDragging, (ImGui_Context*,ctx)
(int,button)(double*,API_RO(lock_threshold),-1.0),
"Is mouse dragging? (if lock_threshold < -1.0, uses ConfigVar_MouseDragThreshold)")
{
  FRAME_GUARD;
  return ImGui::IsMouseDragging(button, API_RO_GET(lock_threshold));
}

DEFINE_API(void, GetMouseDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
R"(Mouse delta. Note that this is zero if either current or previous position
are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have
a huge delta.)")
{
  FRAME_GUARD;
  const ImVec2 &delta { ctx->IO().MouseDelta };
  *API_W(x) = delta.x, *API_W(y) = delta.y;
}

DEFINE_API(void, GetMouseDragDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y))
(int*,API_RO(button),ImGuiMouseButton_Left)(double*,API_RO(lock_threshold),-1.0),
R"(Return the delta from the initial clicking position while the mouse button is
pressed or was just released. This is locked and return 0.0 until the mouse
moves past a distance threshold at least once (if lock_threshold < -1.0, uses
ConfigVar_MouseDragThreshold).)")
{
  FRAME_GUARD;
  const ImVec2 &delta {
    ImGui::GetMouseDragDelta(API_RO_GET(button), API_RO_GET(lock_threshold))
  };
  if(API_W(x)) *API_W(x) = delta.x;
  if(API_W(y)) *API_W(y) = delta.y;
}

DEFINE_API(void, ResetMouseDragDelta, (ImGui_Context*,ctx)
(int*,API_RO(button),ImGuiMouseButton_Left),
"")
{
  FRAME_GUARD;
  ImGui::ResetMouseDragDelta(API_RO_GET(button));
}

DEFINE_ENUM(ImGui, MouseButton_Left,   "");
DEFINE_ENUM(ImGui, MouseButton_Right,  "");
DEFINE_ENUM(ImGui, MouseButton_Middle, "");

DEFINE_SECTION(cursor, mouse, "Mouse Cursor");

DEFINE_API(int, GetMouseCursor, (ImGui_Context*,ctx),
"Get desired mouse cursor shape, reset every frame. This is updated during the frame.")
{
  FRAME_GUARD;
  return ImGui::GetMouseCursor();
}

DEFINE_API(void, SetMouseCursor, (ImGui_Context*,ctx)
(int,cursor_type),
"Set desired mouse cursor shape. See MouseCursor_* for possible values.")
{
  FRAME_GUARD;
  IM_ASSERT(cursor_type >= -1 && cursor_type < ImGuiMouseCursor_COUNT);
  ImGui::SetMouseCursor(cursor_type);
}

DEFINE_ENUM(ImGui, MouseCursor_None,      "");
DEFINE_ENUM(ImGui, MouseCursor_Arrow,     "");
DEFINE_ENUM(ImGui, MouseCursor_TextInput, "When hovering over InputText, etc.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeAll, "(Unused by Dear ImGui functions)");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNS,
  "When hovering over a horizontal border.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeEW,
  "When hovering over a vertical border or a column.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNESW,
  "When hovering over the bottom-left corner of a window.");
DEFINE_ENUM(ImGui, MouseCursor_ResizeNWSE,
  "When hovering over the bottom-right corner of a window.");
DEFINE_ENUM(ImGui, MouseCursor_Hand,
  "(Unused by Dear ImGui functions. Use for e.g. hyperlinks)");
DEFINE_ENUM(ImGui, MouseCursor_NotAllowed,
  "When hovering something with disallowed interaction. Usually a crossed circle.");

DEFINE_SECTION(keyboard, ROOT_SECTION, "Keyboard");

DEFINE_API(bool, IsKeyDown, (ImGui_Context*,ctx)
(int,key),
"Is key being held.")
{
  FRAME_GUARD;
  return ImGui::IsKeyDown(static_cast<ImGuiKey>(key));
}

DEFINE_API(double, GetKeyDownDuration, (ImGui_Context*,ctx)
(int,key),
"Duration the keyboard key has been down (0.0 == just pressed)")
{
  FRAME_GUARD;
  return ImGui::GetKeyData(static_cast<ImGuiKey>(key))->DownDuration;
}

DEFINE_API(bool, IsKeyPressed, (ImGui_Context*,ctx)
(int,key)(bool*,API_RO(repeat),true),
R"(Was key pressed (went from !Down to Down)?
If repeat=true, uses ConfigVar_KeyRepeatDelay / ConfigVar_KeyRepeatRate.)")
{
  FRAME_GUARD;
  return ImGui::IsKeyPressed(static_cast<ImGuiKey>(key), API_RO_GET(repeat));
}

DEFINE_API(bool, IsKeyReleased, (ImGui_Context*,ctx)
(int,key),
"Was key released (went from Down to !Down)?")
{
  FRAME_GUARD;
  return ImGui::IsKeyReleased(static_cast<ImGuiKey>(key));
}

DEFINE_API(int, GetKeyPressedAmount, (ImGui_Context*,ctx)
(int,key)(double,repeat_delay)(double,rate),
R"(Uses provided repeat rate/delay. Return a count, most often 0 or 1 but might
be >1 if ConfigVar_RepeatRate is small enough that GetDeltaTime > RepeatRate.)")
{
  FRAME_GUARD;
  return ImGui::GetKeyPressedAmount(static_cast<ImGuiKey>(key), repeat_delay, rate);
}

DEFINE_API(int, GetKeyMods, (ImGui_Context*,ctx),
"Flags for the Ctrl/Shift/Alt/Super keys. Uses Mod_* values.")
{
  FRAME_GUARD;
  return ctx->IO().KeyMods;
}

DEFINE_API(bool, GetInputQueueCharacter, (ImGui_Context*,ctx)
(int,idx)(int*,API_W(unicode_char)),
R"(Read from ImGui's character input queue.
Call with increasing idx until false is returned.)")
{
  FRAME_GUARD;
  const ImGuiIO &io { ctx->IO() };
  if(idx >= 0 && idx < io.InputQueueCharacters.Size) {
    if(API_W(unicode_char))
      *API_W(unicode_char) = io.InputQueueCharacters[idx];
    return true;
  }

  return false;
}

DEFINE_API(void, SetNextFrameWantCaptureKeyboard, (ImGui_Context*,ctx)
(bool,want_capture_keyboard),
R"(Request capture of keyboard shortcuts in REAPER's global scope for the next frame.)")
{
  FRAME_GUARD;
  ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard);
}

DEFINE_SECTION(namedKeys, keyboard, "Named Keys");
API_SECTION_P(namedKeys, "Keyboard");
DEFINE_ENUM(ImGui, Key_Tab,        "");
DEFINE_ENUM(ImGui, Key_LeftArrow,  "");
DEFINE_ENUM(ImGui, Key_RightArrow, "");
DEFINE_ENUM(ImGui, Key_UpArrow,    "");
DEFINE_ENUM(ImGui, Key_DownArrow,  "");
DEFINE_ENUM(ImGui, Key_PageUp,     "");
DEFINE_ENUM(ImGui, Key_PageDown,   "");
DEFINE_ENUM(ImGui, Key_Home,       "");
DEFINE_ENUM(ImGui, Key_End,        "");
DEFINE_ENUM(ImGui, Key_Insert,     "");
DEFINE_ENUM(ImGui, Key_Delete,     "");
DEFINE_ENUM(ImGui, Key_Backspace,  "");
DEFINE_ENUM(ImGui, Key_Space,      "");
DEFINE_ENUM(ImGui, Key_Enter,      "");
DEFINE_ENUM(ImGui, Key_Escape,     "");
DEFINE_ENUM(ImGui, Key_LeftCtrl,   "");
DEFINE_ENUM(ImGui, Key_LeftShift,  "");
DEFINE_ENUM(ImGui, Key_LeftAlt,    "");
DEFINE_ENUM(ImGui, Key_LeftSuper,  "");
DEFINE_ENUM(ImGui, Key_RightCtrl,  "");
DEFINE_ENUM(ImGui, Key_RightShift, "");
DEFINE_ENUM(ImGui, Key_RightAlt,   "");
DEFINE_ENUM(ImGui, Key_RightSuper, "");
DEFINE_ENUM(ImGui, Key_Menu,       "");
DEFINE_ENUM(ImGui, Key_0, "");
DEFINE_ENUM(ImGui, Key_1, "");
DEFINE_ENUM(ImGui, Key_2, "");
DEFINE_ENUM(ImGui, Key_3, "");
DEFINE_ENUM(ImGui, Key_4, "");
DEFINE_ENUM(ImGui, Key_5, "");
DEFINE_ENUM(ImGui, Key_6, "");
DEFINE_ENUM(ImGui, Key_7, "");
DEFINE_ENUM(ImGui, Key_8, "");
DEFINE_ENUM(ImGui, Key_9, "");
DEFINE_ENUM(ImGui, Key_A, "");
DEFINE_ENUM(ImGui, Key_B, "");
DEFINE_ENUM(ImGui, Key_C, "");
DEFINE_ENUM(ImGui, Key_D, "");
DEFINE_ENUM(ImGui, Key_E, "");
DEFINE_ENUM(ImGui, Key_F, "");
DEFINE_ENUM(ImGui, Key_G, "");
DEFINE_ENUM(ImGui, Key_H, "");
DEFINE_ENUM(ImGui, Key_I, "");
DEFINE_ENUM(ImGui, Key_J, "");
DEFINE_ENUM(ImGui, Key_K, "");
DEFINE_ENUM(ImGui, Key_L, "");
DEFINE_ENUM(ImGui, Key_M, "");
DEFINE_ENUM(ImGui, Key_N, "");
DEFINE_ENUM(ImGui, Key_O, "");
DEFINE_ENUM(ImGui, Key_P, "");
DEFINE_ENUM(ImGui, Key_Q, "");
DEFINE_ENUM(ImGui, Key_R, "");
DEFINE_ENUM(ImGui, Key_S, "");
DEFINE_ENUM(ImGui, Key_T, "");
DEFINE_ENUM(ImGui, Key_U, "");
DEFINE_ENUM(ImGui, Key_V, "");
DEFINE_ENUM(ImGui, Key_W, "");
DEFINE_ENUM(ImGui, Key_X, "");
DEFINE_ENUM(ImGui, Key_Y, "");
DEFINE_ENUM(ImGui, Key_Z, "");
DEFINE_ENUM(ImGui, Key_F1,  "");
DEFINE_ENUM(ImGui, Key_F2,  "");
DEFINE_ENUM(ImGui, Key_F3,  "");
DEFINE_ENUM(ImGui, Key_F4,  "");
DEFINE_ENUM(ImGui, Key_F5,  "");
DEFINE_ENUM(ImGui, Key_F6,  "");
DEFINE_ENUM(ImGui, Key_F7,  "");
DEFINE_ENUM(ImGui, Key_F8,  "");
DEFINE_ENUM(ImGui, Key_F9,  "");
DEFINE_ENUM(ImGui, Key_F10, "");
DEFINE_ENUM(ImGui, Key_F11, "");
DEFINE_ENUM(ImGui, Key_F12, "");
DEFINE_ENUM(ImGui, Key_Apostrophe,   "'");
DEFINE_ENUM(ImGui, Key_Comma,        ",");
DEFINE_ENUM(ImGui, Key_Minus,        "-");
DEFINE_ENUM(ImGui, Key_Period,       ".");
DEFINE_ENUM(ImGui, Key_Slash,        "/");
DEFINE_ENUM(ImGui, Key_Semicolon,    ";");
DEFINE_ENUM(ImGui, Key_Equal,        "=");
DEFINE_ENUM(ImGui, Key_LeftBracket,  "[");
DEFINE_ENUM(ImGui, Key_Backslash,    "\\");
DEFINE_ENUM(ImGui, Key_RightBracket, "]");
DEFINE_ENUM(ImGui, Key_GraveAccent,  "`");
DEFINE_ENUM(ImGui, Key_CapsLock,     "");
DEFINE_ENUM(ImGui, Key_ScrollLock,   "");
DEFINE_ENUM(ImGui, Key_NumLock,      "");
DEFINE_ENUM(ImGui, Key_PrintScreen,  "");
DEFINE_ENUM(ImGui, Key_Pause,        "");
DEFINE_ENUM(ImGui, Key_Keypad0, "");
DEFINE_ENUM(ImGui, Key_Keypad1, "");
DEFINE_ENUM(ImGui, Key_Keypad2, "");
DEFINE_ENUM(ImGui, Key_Keypad3, "");
DEFINE_ENUM(ImGui, Key_Keypad4, "");
DEFINE_ENUM(ImGui, Key_Keypad5, "");
DEFINE_ENUM(ImGui, Key_Keypad6, "");
DEFINE_ENUM(ImGui, Key_Keypad7, "");
DEFINE_ENUM(ImGui, Key_Keypad8, "");
DEFINE_ENUM(ImGui, Key_Keypad9, "");
DEFINE_ENUM(ImGui, Key_KeypadDecimal,  "");
DEFINE_ENUM(ImGui, Key_KeypadDivide,   "");
DEFINE_ENUM(ImGui, Key_KeypadMultiply, "");
DEFINE_ENUM(ImGui, Key_KeypadSubtract, "");
DEFINE_ENUM(ImGui, Key_KeypadAdd,      "");
DEFINE_ENUM(ImGui, Key_KeypadEnter,    "");
DEFINE_ENUM(ImGui, Key_KeypadEqual,    "");
API_SECTION_P(namedKeys, "Gamepad");
// TODO
API_SECTION_P(namedKeys, "Mouse Buttons",
R"(This is mirroring the data also written accessible via IsMouseDown,
GetMouseWheel etc, in a format allowing them to be accessed via standard key API.)");
DEFINE_ENUM(ImGui, Key_MouseLeft,   "");
DEFINE_ENUM(ImGui, Key_MouseRight,  "");
DEFINE_ENUM(ImGui, Key_MouseMiddle, "");
DEFINE_ENUM(ImGui, Key_MouseX1,     "");
DEFINE_ENUM(ImGui, Key_MouseX2,     "");
DEFINE_ENUM(ImGui, Key_MouseWheelX, "");
DEFINE_ENUM(ImGui, Key_MouseWheelY, "");
API_SECTION_P(namedKeys, "Modifiers");
DEFINE_ENUM(ImGui, Mod_None,  "");
DEFINE_ENUM(ImGui, Mod_Ctrl,  "");
DEFINE_ENUM(ImGui, Mod_Shift, "");
DEFINE_ENUM(ImGui, Mod_Alt,   "");
DEFINE_ENUM(ImGui, Mod_Super, "");
// cannot use dear imgui's runtime redirection of Mod_Shortcut because of
// user code relying on exact matches via GetKeyMods() == Mod_*
#ifdef __APPLE__
constexpr int ReaImGuiMod_Shortcut { ImGuiMod_Super };
#else
constexpr int ReaImGuiMod_Shortcut { ImGuiMod_Ctrl };
#endif
DEFINE_ENUM(ReaImGui, Mod_Shortcut,
  "Alias for Mod_Ctrl on Linux and Windows and Mod_Super on macOS (Cmd key).");
