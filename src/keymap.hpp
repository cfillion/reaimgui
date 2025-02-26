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

#ifndef REAIMGUI_KEYMAP_HPP
#define REAIMGUI_KEYMAP_HPP

enum ImGuiKey : int;

#ifndef _WIN32
constexpr int
  VK_APPS       {0x5D},
  VK_OEM_PLUS   {0xBB},
  VK_OEM_COMMA  {0xBC},
  VK_OEM_MINUS  {0xBD},
  VK_OEM_PERIOD {0xBE},
  VK_OEM_1      {0xBA}, // ;:
  VK_OEM_2      {0xBF}, // /?
  VK_OEM_3      {0xC0}, // `~
  VK_OEM_4      {0xDB}, // [(
  VK_OEM_5      {0xDC}, // \|
  VK_OEM_6      {0xDD}, // ])
  VK_OEM_7      {0xDE}, // '"
  // VK_OEM_102    {0xE2},
  VK_LSHIFT     {0xA0},
  VK_RSHIFT     {0xA1},
  VK_LCONTROL   {0xA2},
  VK_RCONTROL   {0xA3},
  VK_LMENU      {0xA4},
  VK_RMENU      {0xA5},
  VK_RWIN       {0x5C},
  VK_BROWSER_BACK    {0xA6},
  VK_BROWSER_FORWARD {0xA7};
#endif

namespace KeyMap {
  ImGuiKey translateVirtualKey(const int);
};

#endif
