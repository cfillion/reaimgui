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

#ifndef REAIMGUI_SETTINGS_HPP
#define REAIMGUI_SETTINGS_HPP

struct RendererType;

#ifdef SETTINGS_IMPLEMENT
#  define SETTING
#else
#  define SETTING extern
#endif

namespace Settings {
  void open();
  void setup();
  void save();
  void teardown();

  SETTING bool NoSavedSettings;
  SETTING bool DockingEnable,
               DockingNoSplit, DockingWithShift, DockingTransparentPayload;
  SETTING const RendererType *Renderer;
  SETTING bool ForceSoftware;
}

#undef SETTING

#endif
