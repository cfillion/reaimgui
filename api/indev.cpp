/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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

API_SECTION_DEF(mouse, ROOT_SECTION, "Mouse",
R"(To refer to a mouse button, you may use named enums in your code e.g.
MouseButton_Left, MouseButton_Right.

You can also use regular integer: it is forever guaranteed that
0=Left, 1=Right, 2=Middle. Buttons 3 and 4 do not have a named enum.

Dragging operations are only reported after mouse has moved a certain distance
away from the initial clicking position (see 'lock_threshold' parameters and
'ConfigVar_MouseDragThreshold').)");

API_FUNC(0_1, bool, IsMouseDown, (ImGui_Context*,ctx)
(int,button),
"Is mouse button held?")
{
  FRAME_GUARD;
  return ImGui::IsMouseDown(button);
}

API_FUNC(0_1, double, GetMouseDownDuration, (ImGui_Context*,ctx)
(int,button),
"Duration the mouse button has been down (0.0 == just clicked)")
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  return ctx->IO().MouseDownDuration[button];
}

API_FUNC(0_1, bool, IsMouseClicked, (ImGui_Context*,ctx)
(int,button)(bool*,API_RO(repeat),false),
R"(Did mouse button clicked? (went from !Down to Down).
Same as GetMouseClickedCount() == 1.)")
{
  FRAME_GUARD;
  return ImGui::IsMouseClicked(button, API_RO_GET(repeat));
}

API_FUNC(0_1, void, GetMouseClickedPos, (ImGui_Context*,ctx)
(int,button)(double*,API_W(x))(double*,API_W(y)),
"")
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  const ImVec2 &pos { ctx->IO().MouseClickedPos[button] };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

API_FUNC(0_1, bool, IsMouseReleased, (ImGui_Context*,ctx)
(int,button),
"Did mouse button released? (went from Down to !Down)")
{
  FRAME_GUARD;
  return ImGui::IsMouseReleased(button);
}

API_FUNC(0_1, bool, IsMouseDoubleClicked, (ImGui_Context*,ctx)
(int,button),
R"(Did mouse button double-clicked? Same as GetMouseClickedCount() == 2.
(Note that a double-click will also report IsMouseClicked() == true))")
{
  FRAME_GUARD;
  return ImGui::IsMouseDoubleClicked(button);
}

API_FUNC(0_5_10, int, GetMouseClickedCount, (ImGui_Context*,ctx)
(int,button),
"Return the number of successive mouse-clicks at the time where a click happen (otherwise 0).")
{
  FRAME_GUARD;
  return ImGui::GetMouseClickedCount(button);
}

API_FUNC(0_1, bool, IsMouseHoveringRect, (ImGui_Context*,ctx)
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

API_FUNC(0_1, bool, IsMousePosValid, (ImGui_Context*,ctx)
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

API_FUNC(0_1, bool, IsAnyMouseDown, (ImGui_Context*,ctx),
"Is any mouse button held?")
{
  FRAME_GUARD;
  return ImGui::IsAnyMouseDown();
}

API_FUNC(0_1, void, GetMousePos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"")
{
  FRAME_GUARD;
  const ImVec2 &pos { ctx->IO().MousePos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

API_FUNC(0_1, void, GetMousePosOnOpeningCurrentPopup, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
R"(Retrieve mouse position at the time of opening popup we have BeginPopup()
into (helper to avoid user backing that value themselves).)")
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetMousePosOnOpeningCurrentPopup() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

API_FUNC(0_1, void, GetMouseWheel, (ImGui_Context*,ctx)
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

API_FUNC(0_1, bool, IsMouseDragging, (ImGui_Context*,ctx)
(int,button)(double*,API_RO(lock_threshold),-1.0),
"Is mouse dragging? (if lock_threshold < -1.0, uses ConfigVar_MouseDragThreshold)")
{
  FRAME_GUARD;
  return ImGui::IsMouseDragging(button, API_RO_GET(lock_threshold));
}

API_FUNC(0_1, void, GetMouseDelta, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
R"(Mouse delta. Note that this is zero if either current or previous position
are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have
a huge delta.)")
{
  FRAME_GUARD;
  const ImVec2 &delta { ctx->IO().MouseDelta };
  *API_W(x) = delta.x, *API_W(y) = delta.y;
}

API_FUNC(0_1, void, GetMouseDragDelta, (ImGui_Context*,ctx)
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

API_FUNC(0_1, void, ResetMouseDragDelta, (ImGui_Context*,ctx)
(int*,API_RO(button),ImGuiMouseButton_Left),
"")
{
  FRAME_GUARD;
  ImGui::ResetMouseDragDelta(API_RO_GET(button));
}

API_ENUM(0_1, ImGui, MouseButton_Left,   "");
API_ENUM(0_1, ImGui, MouseButton_Right,  "");
API_ENUM(0_1, ImGui, MouseButton_Middle, "");

API_SECTION_DEF(cursor, mouse, "Mouse Cursor");

API_FUNC(0_1, int, GetMouseCursor, (ImGui_Context*,ctx),
"Get desired mouse cursor shape, reset every frame. This is updated during the frame.")
{
  FRAME_GUARD;
  return ImGui::GetMouseCursor();
}

API_FUNC(0_1, void, SetMouseCursor, (ImGui_Context*,ctx)
(int,cursor_type),
"Set desired mouse cursor shape. See MouseCursor_* for possible values.")
{
  FRAME_GUARD;
  IM_ASSERT(cursor_type >= -1 && cursor_type < ImGuiMouseCursor_COUNT);
  ImGui::SetMouseCursor(cursor_type);
}

API_ENUM(0_8_4, ImGui, MouseCursor_None,   "");
API_ENUM(0_1, ImGui, MouseCursor_Arrow,  "");
API_ENUM(0_1, ImGui, MouseCursor_TextInput, "When hovering over InputText, etc.");
API_ENUM(0_1, ImGui, MouseCursor_ResizeAll, "(Unused by Dear ImGui functions)");
API_ENUM(0_1, ImGui, MouseCursor_ResizeNS,
  "When hovering over a horizontal border.");
API_ENUM(0_1, ImGui, MouseCursor_ResizeEW,
  "When hovering over a vertical border or a column.");
API_ENUM(0_1, ImGui, MouseCursor_ResizeNESW,
  "When hovering over the bottom-left corner of a window.");
API_ENUM(0_1, ImGui, MouseCursor_ResizeNWSE,
  "When hovering over the bottom-right corner of a window.");
API_ENUM(0_1, ImGui, MouseCursor_Hand,
  "(Unused by Dear ImGui functions. Use for e.g. hyperlinks)");
API_ENUM(0_1, ImGui, MouseCursor_NotAllowed,
  "When hovering something with disallowed interaction. Usually a crossed circle.");

API_SECTION_DEF(keyboard, ROOT_SECTION, "Keyboard");

API_FUNC(0_1, bool, IsKeyDown, (ImGui_Context*,ctx)
(int,key),
"Is key being held.")
{
  FRAME_GUARD;
  return ImGui::IsKeyDown(static_cast<ImGuiKey>(key));
}

API_FUNC(0_1, double, GetKeyDownDuration, (ImGui_Context*,ctx)
(int,key),
"Duration the keyboard key has been down (0.0 == just pressed)")
{
  FRAME_GUARD;
  return ImGui::GetKeyData(static_cast<ImGuiKey>(key))->DownDuration;
}

API_FUNC(0_1, bool, IsKeyPressed, (ImGui_Context*,ctx)
(int,key)(bool*,API_RO(repeat),true),
R"(Was key pressed (went from !Down to Down)?
If repeat=true, uses ConfigVar_KeyRepeatDelay / ConfigVar_KeyRepeatRate.)")
{
  FRAME_GUARD;
  return ImGui::IsKeyPressed(static_cast<ImGuiKey>(key), API_RO_GET(repeat));
}

API_FUNC(0_1, bool, IsKeyReleased, (ImGui_Context*,ctx)
(int,key),
"Was key released (went from Down to !Down)?")
{
  FRAME_GUARD;
  return ImGui::IsKeyReleased(static_cast<ImGuiKey>(key));
}

API_FUNC(0_1, int, GetKeyPressedAmount, (ImGui_Context*,ctx)
(int,key)(double,repeat_delay)(double,rate),
R"(Uses provided repeat rate/delay. Return a count, most often 0 or 1 but might
be >1 if ConfigVar_RepeatRate is small enough that GetDeltaTime > RepeatRate.)")
{
  FRAME_GUARD;
  return ImGui::GetKeyPressedAmount(static_cast<ImGuiKey>(key), repeat_delay, rate);
}

API_FUNC(0_8, int, GetKeyMods, (ImGui_Context*,ctx),
"Flags for the Ctrl/Shift/Alt/Super keys. Uses Mod_* values.")
{
  FRAME_GUARD;
  return ctx->IO().KeyMods;
}

API_FUNC(0_1, bool, GetInputQueueCharacter, (ImGui_Context*,ctx)
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

API_FUNC(0_7, void, SetNextFrameWantCaptureKeyboard, (ImGui_Context*,ctx)
(bool,want_capture_keyboard),
R"(Request capture of keyboard shortcuts in REAPER's global scope for the next frame.)")
{
  FRAME_GUARD;
  ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard);
}

API_SECTION_DEF(namedKeys, keyboard, "Named Keys");
API_SECTION_P(namedKeys, "Keyboard");
API_ENUM(0_6, ImGui, Key_Tab,        "");
API_ENUM(0_6, ImGui, Key_LeftArrow,  "");
API_ENUM(0_6, ImGui, Key_RightArrow, "");
API_ENUM(0_6, ImGui, Key_UpArrow,    "");
API_ENUM(0_6, ImGui, Key_DownArrow,  "");
API_ENUM(0_6, ImGui, Key_PageUp,     "");
API_ENUM(0_6, ImGui, Key_PageDown,   "");
API_ENUM(0_6, ImGui, Key_Home,       "");
API_ENUM(0_6, ImGui, Key_End,        "");
API_ENUM(0_6, ImGui, Key_Insert,     "");
API_ENUM(0_6, ImGui, Key_Delete,     "");
API_ENUM(0_6, ImGui, Key_Backspace,  "");
API_ENUM(0_6, ImGui, Key_Space,      "");
API_ENUM(0_6, ImGui, Key_Enter,      "");
API_ENUM(0_6, ImGui, Key_Escape,     "");
API_ENUM(0_6, ImGui, Key_LeftCtrl,   "");
API_ENUM(0_6, ImGui, Key_LeftShift,  "");
API_ENUM(0_6, ImGui, Key_LeftAlt,    "");
API_ENUM(0_6, ImGui, Key_LeftSuper,  "");
API_ENUM(0_6, ImGui, Key_RightCtrl,  "");
API_ENUM(0_6, ImGui, Key_RightShift, "");
API_ENUM(0_6, ImGui, Key_RightAlt,   "");
API_ENUM(0_6, ImGui, Key_RightSuper, "");
API_ENUM(0_6, ImGui, Key_Menu,       "");
API_ENUM(0_6, ImGui, Key_0, "");
API_ENUM(0_6, ImGui, Key_1, "");
API_ENUM(0_6, ImGui, Key_2, "");
API_ENUM(0_6, ImGui, Key_3, "");
API_ENUM(0_6, ImGui, Key_4, "");
API_ENUM(0_6, ImGui, Key_5, "");
API_ENUM(0_6, ImGui, Key_6, "");
API_ENUM(0_6, ImGui, Key_7, "");
API_ENUM(0_6, ImGui, Key_8, "");
API_ENUM(0_6, ImGui, Key_9, "");
API_ENUM(0_6, ImGui, Key_A, "");
API_ENUM(0_6, ImGui, Key_B, "");
API_ENUM(0_6, ImGui, Key_C, "");
API_ENUM(0_6, ImGui, Key_D, "");
API_ENUM(0_6, ImGui, Key_E, "");
API_ENUM(0_6, ImGui, Key_F, "");
API_ENUM(0_6, ImGui, Key_G, "");
API_ENUM(0_6, ImGui, Key_H, "");
API_ENUM(0_6, ImGui, Key_I, "");
API_ENUM(0_6, ImGui, Key_J, "");
API_ENUM(0_6, ImGui, Key_K, "");
API_ENUM(0_6, ImGui, Key_L, "");
API_ENUM(0_6, ImGui, Key_M, "");
API_ENUM(0_6, ImGui, Key_N, "");
API_ENUM(0_6, ImGui, Key_O, "");
API_ENUM(0_6, ImGui, Key_P, "");
API_ENUM(0_6, ImGui, Key_Q, "");
API_ENUM(0_6, ImGui, Key_R, "");
API_ENUM(0_6, ImGui, Key_S, "");
API_ENUM(0_6, ImGui, Key_T, "");
API_ENUM(0_6, ImGui, Key_U, "");
API_ENUM(0_6, ImGui, Key_V, "");
API_ENUM(0_6, ImGui, Key_W, "");
API_ENUM(0_6, ImGui, Key_X, "");
API_ENUM(0_6, ImGui, Key_Y, "");
API_ENUM(0_6, ImGui, Key_Z, "");
API_ENUM(0_6, ImGui, Key_F1,  "");
API_ENUM(0_6, ImGui, Key_F2,  "");
API_ENUM(0_6, ImGui, Key_F3,  "");
API_ENUM(0_6, ImGui, Key_F4,  "");
API_ENUM(0_6, ImGui, Key_F5,  "");
API_ENUM(0_6, ImGui, Key_F6,  "");
API_ENUM(0_6, ImGui, Key_F7,  "");
API_ENUM(0_6, ImGui, Key_F8,  "");
API_ENUM(0_6, ImGui, Key_F9,  "");
API_ENUM(0_6, ImGui, Key_F10, "");
API_ENUM(0_6, ImGui, Key_F11, "");
API_ENUM(0_6, ImGui, Key_F12, "");
API_ENUM(0_6, ImGui, Key_Apostrophe,   "'");
API_ENUM(0_6, ImGui, Key_Comma,        ",");
API_ENUM(0_6, ImGui, Key_Minus,        "-");
API_ENUM(0_6, ImGui, Key_Period,       ".");
API_ENUM(0_6, ImGui, Key_Slash,        "/");
API_ENUM(0_6, ImGui, Key_Semicolon,    ";");
API_ENUM(0_6, ImGui, Key_Equal,        "=");
API_ENUM(0_6, ImGui, Key_LeftBracket,  "[");
API_ENUM(0_6, ImGui, Key_Backslash,    "\\");
API_ENUM(0_6, ImGui, Key_RightBracket, "]");
API_ENUM(0_6, ImGui, Key_GraveAccent,  "`");
API_ENUM(0_6, ImGui, Key_CapsLock,     "");
API_ENUM(0_6, ImGui, Key_ScrollLock,   "");
API_ENUM(0_6, ImGui, Key_NumLock,      "");
API_ENUM(0_6, ImGui, Key_PrintScreen,  "");
API_ENUM(0_6, ImGui, Key_Pause,        "");
API_ENUM(0_6, ImGui, Key_Keypad0, "");
API_ENUM(0_6, ImGui, Key_Keypad1, "");
API_ENUM(0_6, ImGui, Key_Keypad2, "");
API_ENUM(0_6, ImGui, Key_Keypad3, "");
API_ENUM(0_6, ImGui, Key_Keypad4, "");
API_ENUM(0_6, ImGui, Key_Keypad5, "");
API_ENUM(0_6, ImGui, Key_Keypad6, "");
API_ENUM(0_6, ImGui, Key_Keypad7, "");
API_ENUM(0_6, ImGui, Key_Keypad8, "");
API_ENUM(0_6, ImGui, Key_Keypad9, "");
API_ENUM(0_6, ImGui, Key_KeypadDecimal,  "");
API_ENUM(0_6, ImGui, Key_KeypadDivide,   "");
API_ENUM(0_6, ImGui, Key_KeypadMultiply, "");
API_ENUM(0_6, ImGui, Key_KeypadSubtract, "");
API_ENUM(0_6, ImGui, Key_KeypadAdd,      "");
API_ENUM(0_6, ImGui, Key_KeypadEnter,    "");
API_ENUM(0_6, ImGui, Key_KeypadEqual,    "");
API_SECTION_P(namedKeys, "Gamepad");
// TODO
API_SECTION_P(namedKeys, "Mouse Buttons",
R"(This is mirroring the data also written accessible via IsMouseDown,
GetMouseWheel etc, in a format allowing them to be accessed via standard key API.)");
API_ENUM(0_8, ImGui, Key_MouseLeft,   "");
API_ENUM(0_8, ImGui, Key_MouseRight,  "");
API_ENUM(0_8, ImGui, Key_MouseMiddle, "");
API_ENUM(0_8, ImGui, Key_MouseX1,     "");
API_ENUM(0_8, ImGui, Key_MouseX2,     "");
API_ENUM(0_8, ImGui, Key_MouseWheelX, "");
API_ENUM(0_8, ImGui, Key_MouseWheelY, "");
API_SECTION_P(namedKeys, "Modifiers");
API_ENUM(0_8, ImGui, Mod_None,  "");
API_ENUM(0_8, ImGui, Mod_Ctrl,  "");
API_ENUM(0_8, ImGui, Mod_Shift, "");
API_ENUM(0_8, ImGui, Mod_Alt,   "");
API_ENUM(0_8, ImGui, Mod_Super, "");
// cannot use dear imgui's runtime redirection of Mod_Shortcut because of
// user code relying on exact matches via GetKeyMods() == Mod_*
#ifdef __APPLE__
constexpr int ReaImGuiMod_Shortcut { ImGuiMod_Super };
#else
constexpr int ReaImGuiMod_Shortcut { ImGuiMod_Ctrl };
#endif
API_ENUM(0_8, ReaImGui, Mod_Shortcut,
  "Alias for Mod_Ctrl on Linux and Windows and Mod_Super on macOS (Cmd key).");
