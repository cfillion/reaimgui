/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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

API_FUNC(0_1, bool, IsMouseDown, (Context*,ctx)
(int,button),
"Is mouse button held?")
{
  FRAME_GUARD;
  return ImGui::IsMouseDown(button);
}

API_FUNC(0_1, double, GetMouseDownDuration, (Context*,ctx)
(int,button),
"Duration the mouse button has been down (0.0 == just clicked)")
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  return ctx->IO().MouseDownDuration[button];
}

API_FUNC(0_1, bool, IsMouseClicked, (Context*,ctx)
(int,button) (RO<bool*>,repeat,false),
R"(Did mouse button clicked? (went from !Down to Down).
Same as GetMouseClickedCount() == 1.)")
{
  FRAME_GUARD;
  return ImGui::IsMouseClicked(button, API_GET(repeat));
}

API_FUNC(0_1, void, GetMouseClickedPos, (Context*,ctx)
(int,button) (W<double*>,x) (W<double*>,y),
"")
{
  FRAME_GUARD;
  IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(ImGuiIO::MouseDownDuration));
  const ImVec2 &pos {ctx->IO().MouseClickedPos[button]};
  if(x) *x = pos.x;
  if(y) *y = pos.y;
}

API_FUNC(0_1, bool, IsMouseReleased, (Context*,ctx)
(int,button),
"Did mouse button released? (went from Down to !Down)")
{
  FRAME_GUARD;
  return ImGui::IsMouseReleased(button);
}

API_FUNC(0_1, bool, IsMouseDoubleClicked, (Context*,ctx)
(int,button),
R"(Did mouse button double-clicked? Same as GetMouseClickedCount() == 2.
(Note that a double-click will also report IsMouseClicked() == true))")
{
  FRAME_GUARD;
  return ImGui::IsMouseDoubleClicked(button);
}

API_FUNC(0_5_10, int, GetMouseClickedCount, (Context*,ctx)
(int,button),
"Return the number of successive mouse-clicks at the time where a click happen (otherwise 0).")
{
  FRAME_GUARD;
  return ImGui::GetMouseClickedCount(button);
}

API_FUNC(0_1, bool, IsMouseHoveringRect, (Context*,ctx)
(double,r_min_x) (double,r_min_y) (double,r_max_x) (double,r_max_y)
(RO<bool*>,clip,true),
R"(Is mouse hovering given bounding rect (in screen space).
Clipped by current clipping settings, but disregarding of other consideration
of focus/window ordering/popup-block.)")
{
  FRAME_GUARD;
  return ImGui::IsMouseHoveringRect(
    ImVec2(r_min_x, r_min_y), ImVec2(r_max_x, r_max_y), API_GET(clip));
}

API_FUNC(0_1, bool, IsMousePosValid, (Context*,ctx)
(RO<double*>,mouse_pos_x) (RO<double*>,mouse_pos_y),
"")
{
  FRAME_GUARD;

  ImVec2 custom_pos, *custom_pos_ptr;
  if(mouse_pos_x && mouse_pos_y) {
    custom_pos.x = *mouse_pos_x, custom_pos.y = *mouse_pos_y;
    custom_pos_ptr = &custom_pos;
  }
  else
    custom_pos_ptr = nullptr;

  return ImGui::IsMousePosValid(custom_pos_ptr);
}

API_FUNC(0_1, bool, IsAnyMouseDown, (Context*,ctx),
"Is any mouse button held?")
{
  FRAME_GUARD;
  return ImGui::IsAnyMouseDown();
}

API_FUNC(0_1, void, GetMousePos, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
"")
{
  FRAME_GUARD;
  const ImVec2 &pos {ctx->IO().MousePos};
  if(x) *x = pos.x;
  if(y) *y = pos.y;
}

API_FUNC(0_1, void, GetMousePosOnOpeningCurrentPopup, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
R"(Retrieve mouse position at the time of opening popup we have BeginPopup()
into (helper to avoid user backing that value themselves).)")
{
  FRAME_GUARD;
  const ImVec2 &pos {ImGui::GetMousePosOnOpeningCurrentPopup()};
  if(x) *x = pos.x;
  if(y) *y = pos.y;
}

API_FUNC(0_1, void, GetMouseWheel, (Context*,ctx)
(W<double*>,vertical) (W<double*>,horizontal),
R"(Vertical: 1 unit scrolls about 5 lines text. >0 scrolls Up, <0 scrolls Down.
Hold SHIFT to turn vertical scroll into horizontal scroll

Horizontal: >0 scrolls Left, <0 scrolls Right.
Most users don't have a mouse with a horizontal wheel.)")
{
  FRAME_GUARD;
  const ImGuiIO &io {ctx->IO()};
  if(vertical)   *vertical   = io.MouseWheel;
  if(horizontal) *horizontal = io.MouseWheelH;
}

API_FUNC(0_1, bool, IsMouseDragging, (Context*,ctx)
(int,button) (RO<double*>,lock_threshold,-1.0),
"Is mouse dragging? (uses ConfigVar_MouseDragThreshold if lock_threshold < 0.0)")
{
  FRAME_GUARD;
  return ImGui::IsMouseDragging(button, API_GET(lock_threshold));
}

API_FUNC(0_1, void, GetMouseDelta, (Context*,ctx)
(W<double*>,x) (W<double*>,y),
R"(Mouse delta. Note that this is zero if either current or previous position
are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have
a huge delta.)")
{
  FRAME_GUARD;
  const ImVec2 &delta {ctx->IO().MouseDelta};
  if(x) *x = delta.x;
  if(y) *y = delta.y;
}

API_FUNC(0_1, void, GetMouseDragDelta, (Context*,ctx)
(W<double*>,x) (W<double*>,y)
(RO<int*>,button,ImGuiMouseButton_Left) (RO<double*>,lock_threshold,-1.0),
R"(Return the delta from the initial clicking position while the mouse button is
pressed or was just released. This is locked and return 0.0 until the mouse
moves past a distance threshold at least once (uses ConfigVar_MouseDragThreshold
if lock_threshold < 0.0).)")
{
  FRAME_GUARD;
  const ImVec2 &delta {
    ImGui::GetMouseDragDelta(API_GET(button), API_GET(lock_threshold))
  };
  if(x) *x = delta.x;
  if(y) *y = delta.y;
}

API_FUNC(0_1, void, ResetMouseDragDelta, (Context*,ctx)
(RO<int*>,button,ImGuiMouseButton_Left),
"")
{
  FRAME_GUARD;
  ImGui::ResetMouseDragDelta(API_GET(button));
}

API_ENUM(0_1, ImGui, MouseButton_Left,   "");
API_ENUM(0_1, ImGui, MouseButton_Right,  "");
API_ENUM(0_1, ImGui, MouseButton_Middle, "");

API_SECTION_DEF(cursor, mouse, "Mouse Cursor");

API_FUNC(0_1, int, GetMouseCursor, (Context*,ctx),
"Get desired mouse cursor shape, reset every frame. This is updated during the frame.")
{
  FRAME_GUARD;
  return ImGui::GetMouseCursor();
}

API_FUNC(0_1, void, SetMouseCursor, (Context*,ctx)
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

API_FUNC(0_9, bool, IsKeyDown, (Context*,ctx)
(int,key),
"Is key being held.")
{
  FRAME_GUARD;
  return ImGui::IsKeyDown(static_cast<ImGuiKey>(key));
}

API_FUNC(0_9, double, GetKeyDownDuration, (Context*,ctx)
(int,key),
"Duration the keyboard key has been down (0.0 == just pressed)")
{
  FRAME_GUARD;
  return ImGui::GetKeyData(static_cast<ImGuiKey>(key))->DownDuration;
}

API_FUNC(0_9, bool, IsKeyPressed, (Context*,ctx)
(int,key) (RO<bool*>,repeat,true),
R"(Was key pressed (went from !Down to Down)?
If repeat=true, uses ConfigVar_KeyRepeatDelay / ConfigVar_KeyRepeatRate.)")
{
  FRAME_GUARD;
  return ImGui::IsKeyPressed(static_cast<ImGuiKey>(key), API_GET(repeat));
}

API_FUNC(0_9, bool, IsKeyReleased, (Context*,ctx)
(int,key),
"Was key released (went from Down to !Down)?")
{
  FRAME_GUARD;
  return ImGui::IsKeyReleased(static_cast<ImGuiKey>(key));
}

API_FUNC(0_9, int, GetKeyPressedAmount, (Context*,ctx)
(int,key) (double,repeat_delay) (double,rate),
R"(Uses provided repeat rate/delay. Return a count, most often 0 or 1 but might
be >1 if ConfigVar_RepeatRate is small enough that GetDeltaTime > RepeatRate.)")
{
  FRAME_GUARD;
  return ImGui::GetKeyPressedAmount(static_cast<ImGuiKey>(key), repeat_delay, rate);
}

API_FUNC(0_8, int, GetKeyMods, (Context*,ctx),
"Flags for the Ctrl/Shift/Alt/Super keys. Uses Mod_* values.")
{
  FRAME_GUARD;
  return ctx->IO().KeyMods;
}

API_FUNC(0_1, bool, GetInputQueueCharacter, (Context*,ctx)
(int,idx) (W<int*>,unicode_char),
R"(Read from ImGui's character input queue.
Call with increasing idx until false is returned.)")
{
  FRAME_GUARD;
  const ImGuiIO &io {ctx->IO()};
  if(idx >= 0 && idx < io.InputQueueCharacters.Size) {
    if(unicode_char)
      *unicode_char = io.InputQueueCharacters[idx];
    return true;
  }

  return false;
}

API_FUNC(0_7, void, SetNextFrameWantCaptureKeyboard, (Context*,ctx)
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
API_ENUM(0_9, ImGui, Key_F13, "");
API_ENUM(0_9, ImGui, Key_F14, "");
API_ENUM(0_9, ImGui, Key_F15, "");
API_ENUM(0_9, ImGui, Key_F16, "");
API_ENUM(0_9, ImGui, Key_F17, "");
API_ENUM(0_9, ImGui, Key_F18, "");
API_ENUM(0_9, ImGui, Key_F19, "");
API_ENUM(0_9, ImGui, Key_F20, "");
API_ENUM(0_9, ImGui, Key_F21, "");
API_ENUM(0_9, ImGui, Key_F22, "");
API_ENUM(0_9, ImGui, Key_F23, "");
API_ENUM(0_9, ImGui, Key_F24, "");
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
API_ENUM(0_9, ImGui, Key_AppBack,
R"(Available on some keyboard/mouses. Often referred as "Browser Back".)");
API_ENUM(0_9, ImGui, Key_AppForward, "");
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
API_ENUM(0_8,   ImGui, Mod_None,  "");
API_ENUM(0_9_2, ImGui, Mod_Ctrl,  "Cmd when ConfigVar_MacOSXBehaviors is enabled.");
API_ENUM(0_8,   ImGui, Mod_Shift, "");
API_ENUM(0_8,   ImGui, Mod_Alt,   "");
API_ENUM(0_9_2, ImGui, Mod_Super, "Ctrl when ConfigVar_MacOSXBehaviors is enabled.");

API_SECTION_DEF(shortcuts, ROOT_SECTION, "Shortcuts", R"(
Key chords can combine a Key_* and a Mod_* value. For example: `Mod_Ctrl | Key_C`.
Only Mod_* values can be combined a Key_* value. Another Key_* value cannot be combined.

Several callers may register interest in a shortcut, and only one owner gets it.

    Parent -> call Shortcut(Ctrl+S) // When Parent is focused, Parent gets the shortcut.
    Child1 -> call Shortcut(Ctrl+S) // When Child1 is focused, Child1 gets the shortcut (Child1 overrides Parent shortcuts)
    Child2 -> no call               // When Child2 is focused, Parent gets the shortcut.

The whole system is order independent, so if Child1 makes its calls before Parent, results will be identical.
This is an important property as it facilitate working with foreign code or larger codebase.

To understand the difference:
- IsKeyChordPressed compares modifiers and calls IsKeyPressed -> function has no side-effect.
- Shortcut submits a route, routes are resolved, if it currently can be routed it
  calls IsKeyChordPressed -> function has (desirable) side-effects as it can
  prevents another call from getting the route.

Registered routes may be visualized via Metrics/Debugger > Inputs (ShowMetricsWindow).)");

API_FUNC(0_9, bool, IsKeyChordPressed, (Context*,ctx)
(int,key_chord),
R"(Was key chord (mods + key) pressed? You can pass e.g. `Mod_Ctrl | Key_S`
as a key chord. This doesn't do any routing or focus check, consider using the
Shortcut function instead.)")
{
  FRAME_GUARD;
  return ImGui::IsKeyChordPressed(key_chord);
}

API_FUNC(0_9_2, bool, Shortcut, (Context*,ctx)
(int,key_chord) (RO<int*>,flags,ImGuiInputFlags_None),
"")
{
  FRAME_GUARD;
  return ImGui::Shortcut(key_chord, API_GET(flags));
}

API_FUNC(0_9_2, void, SetNextItemShortcut, (Context*,ctx)
(int,key_chord) (RO<int*>,flags,ImGuiInputFlags_None),
"")
{
  FRAME_GUARD;
  ImGui::SetNextItemShortcut(key_chord, API_GET(flags));
}

API_SECTION_DEF(shortcutFlags, shortcuts, "Flags");
API_ENUM(0_9_2, ImGui, InputFlags_None, "");
API_ENUM(0_9_2, ImGui, InputFlags_Repeat,
  "Enable repeat. Return true on successive repeats.");
API_ENUM(0_9_2, ImGui, InputFlags_Tooltip,
  "Automatically display a tooltip when hovering item");
API_ENUM(0_9_2, ImGui, InputFlags_RouteOverFocused,
R"(Global route: higher priority than focused route
   (unless active item in focused route).)");
API_ENUM(0_9_2, ImGui, InputFlags_RouteOverActive,
R"(Global route: higher priority than active item. Unlikely you need to
   use that: will interfere with every active items, e.g. Ctrl+A registered by
   InputText will be overridden by this. May not be fully honored as user/internal
   code is likely to always assume they can access keys when active.)");
API_ENUM(0_9_2, ImGui, InputFlags_RouteUnlessBgFocused,
R"(Option: global route: will not be applied if underlying background/void is
   focused (== no Dear ImGui windows are focused). Useful for overlay applications.)");
API_ENUM(0_9_2, ImGui, InputFlags_RouteFromRootWindow,
R"(Option: route evaluated from the point of view of root window rather than current window.)");
API_SECTION_P(shortcutFlags, "Routing policies",
R"(RouteGlobal+OverActive >> RouteActive or RouteFocused (if owner is active item)
   \>> RouteGlobal+OverFocused >> RouteFocused (if in focused window stack) >> RouteGlobal.

   Default policy is RouteFocused. Can select only one policy among all available.)");
API_ENUM(0_9_2, ImGui, InputFlags_RouteActive, "Route to active item only.");
API_ENUM(0_9_2, ImGui, InputFlags_RouteFocused,
R"(Route to windows in the focus stack. Deep-most focused window takes inputs.
   Active item takes inputs over deep-most focused window.)");
API_ENUM(0_9_2, ImGui, InputFlags_RouteGlobal,
  "Global route (unless a focused window or active item registered the route).");
API_ENUM(0_9_2, ImGui, InputFlags_RouteAlways,
  "Do not register route, poll keys directly.");
