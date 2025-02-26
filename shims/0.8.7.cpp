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

#include "shims.hpp"

#include "../api/listclipper.hpp"

SHIM("0.8.7",
  (void, ListClipper_IncludeRangeByIndices, ListClipper*)
);

// dear imgui v1.89.6 breaking change
SHIM_ALIAS(0_5_10, ListClipper_ForceDisplayRangeByIndices,
  ListClipper_IncludeRangeByIndices)
