/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2026  Christian Fillion
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

#ifndef REAIMGUI_LOCALIZE_HPP
#define REAIMGUI_LOCALIZE_HPP

#ifdef _WIN32
#  include <windows.h>
#else
#  include <swell/swell-types.h>
#  include "win32_unicode.hpp"
#endif

int DialogBoxParamL(
  HINSTANCE inst, const TCHAR *tpl, HWND parent, DLGPROC proc, LPARAM param);

static inline int DialogBoxL(
  HINSTANCE inst, const TCHAR *tpl, HWND parent, DLGPROC proc)
{
  return DialogBoxParamL(inst, tpl, parent, proc, 0);
}

HWND CreateDialogParamL(
  HINSTANCE inst, const TCHAR *tpl, HWND parent, DLGPROC proc, LPARAM param);

static inline HWND CreateDialogL(
  HINSTANCE inst, const TCHAR *tpl, HWND parent, DLGPROC proc)
{
  return CreateDialogParamL(inst, tpl, parent, proc, 0);
}

#endif
