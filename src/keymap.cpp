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

#include "keymap.hpp"

#include <imgui/imgui.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

ImGuiKey KeyMap::translateVirtualKey(const int vkey)
{
  if(vkey >= '0' && vkey <= '9')
    return static_cast<ImGuiKey>(ImGuiKey_0 + (vkey - '0'));
  if(vkey >= 'A' && vkey <= 'Z')
    return static_cast<ImGuiKey>(ImGuiKey_A + (vkey - 'A'));
  if(vkey >= VK_F1 && vkey <= VK_F24)
    return static_cast<ImGuiKey>(ImGuiKey_F1 + (vkey - VK_F1));

  switch(vkey) {
  case VK_TAB:        return ImGuiKey_Tab;
  case VK_LEFT:       return ImGuiKey_LeftArrow;
  case VK_RIGHT:      return ImGuiKey_RightArrow;
  case VK_UP:         return ImGuiKey_UpArrow;
  case VK_DOWN:       return ImGuiKey_DownArrow;
  case VK_PRIOR:      return ImGuiKey_PageUp;
  case VK_NEXT:       return ImGuiKey_PageDown;
  case VK_HOME:       return ImGuiKey_Home;
  case VK_END:        return ImGuiKey_End;
  case VK_INSERT:     return ImGuiKey_Insert;
  case VK_DELETE:     return ImGuiKey_Delete;
  case VK_BACK:       return ImGuiKey_Backspace;
  case VK_SPACE:      return ImGuiKey_Space;
  case VK_RETURN:     return ImGuiKey_Enter;
  case VK_ESCAPE:     return ImGuiKey_Escape;
  case VK_OEM_7:      return ImGuiKey_Apostrophe;
  case VK_OEM_COMMA:  return ImGuiKey_Comma;
  case VK_OEM_MINUS:  return ImGuiKey_Minus;
  case VK_OEM_PERIOD: return ImGuiKey_Period;
  case VK_OEM_2:      return ImGuiKey_Slash;
  case VK_OEM_1:      return ImGuiKey_Semicolon;
  case VK_OEM_PLUS:   return ImGuiKey_Equal;
  case VK_OEM_4:      return ImGuiKey_LeftBracket;
  case VK_OEM_5:      return ImGuiKey_Backslash;
  case VK_OEM_6:      return ImGuiKey_RightBracket;
  case VK_OEM_3:      return ImGuiKey_GraveAccent;
  case VK_CAPITAL:    return ImGuiKey_CapsLock;
  case VK_SCROLL:     return ImGuiKey_ScrollLock;
  case VK_NUMLOCK:    return ImGuiKey_NumLock;
  case VK_SNAPSHOT:   return ImGuiKey_PrintScreen;
  case VK_PAUSE:      return ImGuiKey_Pause;
  case VK_NUMPAD0:    return ImGuiKey_Keypad0;
  case VK_NUMPAD1:    return ImGuiKey_Keypad1;
  case VK_NUMPAD2:    return ImGuiKey_Keypad2;
  case VK_NUMPAD3:    return ImGuiKey_Keypad3;
  case VK_NUMPAD4:    return ImGuiKey_Keypad4;
  case VK_NUMPAD5:    return ImGuiKey_Keypad5;
  case VK_NUMPAD6:    return ImGuiKey_Keypad6;
  case VK_NUMPAD7:    return ImGuiKey_Keypad7;
  case VK_NUMPAD8:    return ImGuiKey_Keypad8;
  case VK_NUMPAD9:    return ImGuiKey_Keypad9;
  case VK_DECIMAL:    return ImGuiKey_KeypadDecimal;
  case VK_DIVIDE:     return ImGuiKey_KeypadDivide;
  case VK_MULTIPLY:   return ImGuiKey_KeypadMultiply;
  case VK_SUBTRACT:   return ImGuiKey_KeypadSubtract;
  case VK_ADD:        return ImGuiKey_KeypadAdd;
  case VK_APPS:       return ImGuiKey_Menu;
  case VK_BROWSER_BACK:    return ImGuiKey_AppBack;
  case VK_BROWSER_FORWARD: return ImGuiKey_AppForward;
  default:            return ImGuiKey_None;
  }
}
