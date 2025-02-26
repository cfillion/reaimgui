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

#include "menu.hpp"

#include "win32_unicode.hpp"

#ifndef _WIN32
#  include <swell/swell.h>
#endif

Menu::Menu(HMENU menu)
  : m_menu {menu}, m_owned {!menu}
{
  if(m_owned)
    m_menu = CreatePopupMenu();
}

Menu::~Menu()
{
  if(m_owned)
    DestroyMenu(m_menu);
}

static MENUITEMINFO makeItem(const TCHAR *label, const int flags)
{
  MENUITEMINFO item {};
  item.cbSize = sizeof(item);
  item.fMask = MIIM_TYPE | MIIM_STATE;
  item.fType = MFT_STRING;
  item.dwTypeData = const_cast<TCHAR *>(label);
  if(flags & Menu::Checked)
    item.fState |= MFS_CHECKED;
#ifdef _WIN32
  if(flags & Menu::Radio)
    item.fType |= MFT_RADIOCHECK;
#endif
  if(flags & Menu::Disabled)
    item.fState |= MFS_DISABLED;
  return item;
}

#ifdef _WIN32
#  define MAKE_ITEM(label, flags)               \
     const std::wstring &wlabel {widen(label)}; \
     MENUITEMINFO item {makeItem(wlabel.c_str(), flags)}
#else
#  define MAKE_ITEM(label, flags) \
     MENUITEMINFO item {makeItem(label, flags)}
#endif

void Menu::addItem(const char *label, const int command, const int flags)
{
  MAKE_ITEM(label, flags);
  item.fMask |= MIIM_ID;
  item.wID = command;
  append(item);
}

Menu Menu::addMenu(const char *label, const int flags)
{
  MAKE_ITEM(label, flags);
  item.fMask |= MIIM_SUBMENU;
  item.hSubMenu = CreatePopupMenu();
  append(item);

  return {item.hSubMenu};
}

void Menu::append(MENUITEMINFO &item)
{
  InsertMenuItem(m_menu, GetMenuItemCount(m_menu), true, &item);
}

int Menu::show(HWND parent, const short x, const short y) const
{
  return TrackPopupMenu(m_menu, TPM_NONOTIFY | TPM_RETURNCMD,
    x, y, 0, parent, nullptr);
}
