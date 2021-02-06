#include "api_helper.hpp"

static float getArrayValue(void *data, const int index)
{
  const double value { static_cast<reaper_array *>(data)->data[index] };
  return static_cast<float>(value);
}

// Widgets: Data Plotting
DEFINE_API(void, PlotLines, ((Window*,window))
((const char*,label))((reaper_array*,values))((int*,valuesOffsetInOptional))
((const char*,overlayTextInOptional))
((double*,scaleMinInOptional))((double*,scaleMaxInOptional))
((double*,graphWidthInOptional))((double*,graphHeightInOptional)),
"",
{
  USE_WINDOW(window);
  ImGui::PlotLines(label, &getArrayValue, values->data, values->size,
    VALUE_OR(valuesOffsetInOptional, 0),
    NULL_IF_EMPTY(overlayTextInOptional),
    VALUE_OR(scaleMinInOptional, FLT_MAX), VALUE_OR(scaleMaxInOptional, FLT_MAX),
    ImVec2(
      VALUE_OR(graphWidthInOptional, 0.0), VALUE_OR(graphHeightInOptional, 0.0)
    )
  );
});

DEFINE_API(void, PlotHistogram, ((Window*,window))
((const char*,label))((reaper_array*,values))((int*,valuesOffsetInOptional))
((const char*,overlayTextInOptional))
((double*,scaleMinInOptional))((double*,scaleMaxInOptional))
((double*,graphWidthInOptional))((double*,graphHeightInOptional)),
"",
{
  USE_WINDOW(window);
  ImGui::PlotHistogram(label, &getArrayValue, values->data, values->size,
    VALUE_OR(valuesOffsetInOptional, 0),
    overlayTextInOptional,
    VALUE_OR(scaleMinInOptional, FLT_MAX), VALUE_OR(scaleMaxInOptional, FLT_MAX),
    ImVec2(
      VALUE_OR(graphWidthInOptional, 0.0), VALUE_OR(graphHeightInOptional, 0.0)
    )
  );
});
