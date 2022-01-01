/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

static float getArrayValue(void *data, const int index)
{
  const double value { static_cast<double *>(data)[index] };
  return static_cast<float>(value);
}

// Widgets: Data Plotting
DEFINE_API(void, PlotLines, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)(int*,API_RO(values_offset))
(const char*,API_RO(overlay_text))
(double*,API_RO(scale_min))(double*,API_RO(scale_max))
(double*,API_RO(graph_size_w))(double*,API_RO(graph_size_h)),
"Default values: values_offset = 0, overlay_text = nil, scale_min = 0.0, scale_max = 0.0, graph_size_w = 0.0, graph_size_h = 0.0",
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(overlay_text));

  ImGui::PlotLines(label, &getArrayValue, values->data, values->size,
    valueOr(API_RO(values_offset), 0), API_RO(overlay_text),
    valueOr(API_RO(scale_min), FLT_MAX), valueOr(API_RO(scale_max), FLT_MAX),
    ImVec2 { valueOr(API_RO(graph_size_w), 0.f), valueOr(API_RO(graph_size_h), 0.0f) });
});

DEFINE_API(void, PlotHistogram, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)(int*,API_RO(values_offset))
(const char*,API_RO(overlay_text))
(double*,API_RO(scale_min))(double*,API_RO(scale_max))
(double*,API_RO(graph_size_w))(double*,API_RO(graph_size_h)),
"Default values: values_offset = 0, overlay_text = nil, scale_min = FLT_MAX, scale_max = FLT_MAX, graph_size_w = 0.0, graph_size_h = 0.0",
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(overlay_text));

  ImGui::PlotHistogram(label, &getArrayValue, values->data, values->size,
    valueOr(API_RO(values_offset), 0), API_RO(overlay_text),
    valueOr(API_RO(scale_min), FLT_MAX), valueOr(API_RO(scale_max), FLT_MAX),
    ImVec2 { valueOr(API_RO(graph_size_w), 0.f), valueOr(API_RO(graph_size_h), 0.f) });
});
