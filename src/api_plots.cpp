#include "api_helper.hpp"

#include <reaper_plugin_secrets.h> // reaper_array

static float getArrayValue(void *data, const int index)
{
  const double value { static_cast<double *>(data)[index] };
  return static_cast<float>(value);
}

// Widgets: Data Plotting
DEFINE_API(void, PlotLines, ((ImGui_Context*,ctx))
((const char*,label))((reaper_array*,values))((int*,API_RO(valuesOffset)))
((const char*,API_RO(overlayText)))
((double*,API_RO(scaleMin)))((double*,API_RO(scaleMax)))
((double*,API_RO(graphWidth)))((double*,API_RO(graphHeight))),
"",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(overlayText));

  ImGui::PlotLines(label, &getArrayValue, values->data, values->size,
    valueOr(API_RO(valuesOffset), 0), API_RO(overlayText),
    valueOr(API_RO(scaleMin), FLT_MAX), valueOr(API_RO(scaleMax), FLT_MAX),
    ImVec2(valueOr(API_RO(graphWidth), 0.0), valueOr(API_RO(graphHeight), 0.0)));
});

DEFINE_API(void, PlotHistogram, ((ImGui_Context*,ctx))
((const char*,label))((reaper_array*,values))((int*,API_RO(valuesOffset)))
((const char*,API_RO(overlayText)))
((double*,API_RO(scaleMin)))((double*,API_RO(scaleMax)))
((double*,API_RO(graphWidth)))((double*,API_RO(graphHeight))),
"",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(overlayText));

  ImGui::PlotHistogram(label, &getArrayValue, values->data, values->size,
    valueOr(API_RO(valuesOffset), 0), API_RO(overlayText),
    valueOr(API_RO(scaleMin), FLT_MAX), valueOr(API_RO(scaleMax), FLT_MAX),
    ImVec2(valueOr(API_RO(graphWidth), 0.0), valueOr(API_RO(graphHeight), 0.0)));
});
