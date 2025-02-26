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

#ifndef REAIMGUI_MENU_HPP
#define REAIMGUI_MENU_HPP

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#endif

class Menu {
public:
  enum ItemFlags {
    Checked  = 1<<0,
    Radio    = 1<<1,
    Disabled = 1<<2,
  };

  Menu(HMENU menu = nullptr);
  Menu(const Menu &) = delete;
  ~Menu();

  void addItem(const char *, int cmd, int flags = 0);
  Menu addMenu(const char *, int flags = 0);
  int show(HWND parent, short x, short y) const;

private:
  void append(MENUITEMINFO &);

  HMENU m_menu;
  bool m_owned;
};

#endif
