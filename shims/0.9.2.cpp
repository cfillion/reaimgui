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

#include "shims.hpp"

SHIM("0.9.2",
  (int, Mod_Ctrl)
  (int, Mod_Super)
);

// dear imgui v1.90.7 swaps Ctrl<>Super on macOS
SHIM_ALIAS(0_8, Mod_Shortcut, Mod_Ctrl)
#ifdef __APPLE__
SHIM_ALIAS(0_8, Mod_Ctrl,  Mod_Super)
SHIM_ALIAS(0_8, Mod_Super, Mod_Ctrl)
#else
SHIM_ALIAS(0_8, Mod_Ctrl,  Mod_Ctrl)
SHIM_ALIAS(0_8, Mod_Super, Mod_Ctrl)
#endif
