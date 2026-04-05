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

#include "localize.hpp"

#include <array> // std::size

#include <reaper_plugin_secrets.h>

#define PREFIX "REAIMGUI_"

int DialogBoxParamL(
  HINSTANCE inst, const TCHAR *tpl, HWND parent, DLGPROC proc, LPARAM param)
{
  void *p[4];
  DLGPROC new_proc = __localizePrepareDialog(PREFIX, inst,
    reinterpret_cast<const char *>(tpl), proc, param, p, std::size(p));
  if(new_proc)
    proc = new_proc, param = (LPARAM)(INT_PTR)p;
  return DialogBoxParam(inst, tpl, parent, proc, param);
}

HWND CreateDialogParamL(
  HINSTANCE inst, const TCHAR *tpl, HWND parent, DLGPROC proc, LPARAM param)
{
  void *p[4];
  DLGPROC new_proc = __localizePrepareDialog(PREFIX, inst,
    reinterpret_cast<const char *>(tpl), proc, param, p, std::size(p));
  if(new_proc)
    proc = new_proc, param = (LPARAM)(INT_PTR)p;
  return CreateDialogParam(inst, tpl, parent, proc, param);
}
