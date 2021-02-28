#include "api_helper.hpp"

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
  Context::check(ctx)->enterFrame();
  assertValid(values);
  nullIfEmpty(API_RO(overlay_text));

  ImGui::PlotLines(label, &getArrayValue, values->data, values->size,
    valueOr(API_RO(values_offset), 0), API_RO(overlay_text),
    valueOr(API_RO(scale_min), FLT_MAX), valueOr(API_RO(scale_max), FLT_MAX),
    ImVec2(valueOr(API_RO(graph_size_w), 0.0), valueOr(API_RO(graph_size_h), 0.0)));
});

DEFINE_API(void, PlotHistogram, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)(int*,API_RO(values_offset))
(const char*,API_RO(overlay_text))
(double*,API_RO(scale_min))(double*,API_RO(scale_max))
(double*,API_RO(graph_size_w))(double*,API_RO(graph_size_h)),
"Default values: values_offset = 0, overlay_text = nil, scale_min = FLT_MAX, scale_max = FLT_MAX, graph_size_w = 0.0, graph_size_h = 0.0",
{
  Context::check(ctx)->enterFrame();
  assertValid(values);
  nullIfEmpty(API_RO(overlay_text));

  ImGui::PlotHistogram(label, &getArrayValue, values->data, values->size,
    valueOr(API_RO(values_offset), 0), API_RO(overlay_text),
    valueOr(API_RO(scale_min), FLT_MAX), valueOr(API_RO(scale_max), FLT_MAX),
    ImVec2(valueOr(API_RO(graph_size_w), 0.0), valueOr(API_RO(graph_size_h), 0.0)));
});
