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

#include <reaper_plugin_secrets.h> // reaper_array

API_SECTION("Plot", "Simple data plotting using reaper_array as data source.");

static float getArrayValue(void *data, const int index)
{
  const double value {static_cast<double *>(data)[index]};
  return static_cast<float>(value);
}

API_FUNC(0_1, void, PlotLines, (Context*,ctx)
(const char*,label) (reaper_array*,values) (RO<int*>,values_offset,0)
(RO<const char*>,overlay_text)
(RO<double*>,scale_min,FLT_MAX) (RO<double*>,scale_max,FLT_MAX)
(RO<double*>,graph_size_w,0.0) (RO<double*>,graph_size_h,0.0),
"")
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(overlay_text);

  ImGui::PlotLines(label, &getArrayValue, values->data, values->size,
    API_GET(values_offset), overlay_text,
    API_GET(scale_min), API_GET(scale_max),
    ImVec2(API_GET(graph_size_w), API_GET(graph_size_h)));
}

API_FUNC(0_1, void, PlotHistogram, (Context*,ctx)
(const char*,label) (reaper_array*,values) (RO<int*>,values_offset,0)
(RO<const char*>,overlay_text)
(RO<double*>,scale_min,FLT_MAX) (RO<double*>,scale_max,FLT_MAX)
(RO<double*>,graph_size_w,0.0) (RO<double*>,graph_size_h,0.0),
"")
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(overlay_text);

  ImGui::PlotHistogram(label, &getArrayValue, values->data, values->size,
    API_GET(values_offset), overlay_text,
    API_GET(scale_min), API_GET(scale_max),
    ImVec2(API_GET(graph_size_w), API_GET(graph_size_h)));
}
