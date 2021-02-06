#include "api_helper.hpp"

static void sanitizeSliderFlags(ImGuiSliderFlags &flags)
{
  // dear imgui will assert if these bits are set
  flags &= ~ImGuiSliderFlags_InvalidMask_; 
}

// Widgets: Drag Sliders
DEFINE_API(bool, DragInt, ((Window*,window))
((const char*,label))((int*,valueInOut))((double*,valueSpeedInOptional))
((double*,valueMinInOptional))((double*,valueMaxInOptional))
((const char*,formatInOptional))((int*,flagsInOptional)),
R"(- CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped and can go off-bounds.
- Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
- Format string may also be set to NULL or use the default format ("%f" or "%d").
- Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
- Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits.
- Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
- We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.)",
{
  USE_WINDOW(window, false);

  ImGuiSliderFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeSliderFlags(flags);

  return ImGui::DragInt(label, valueInOut,
    VALUE_OR(valueSpeedInOptional, 1.0),
    VALUE_OR(valueMinInOptional, 0),
    VALUE_OR(valueMaxInOptional, 0),
    NULL_IF_EMPTY(formatInOptional),
    flags
  );
});
    // IMGUI_API bool          DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, ImGuiSliderFlags flags = 0);

DEFINE_API(bool, DragDouble, ((Window*,window))
((const char*,label))((double*,valueInOut))((double*,valueSpeedInOptional))
((double*,valueMinInOptional))((double*,valueMaxInOptional))
((const char*,formatInOptional))((int*,flagsInOptional)),
"",
{
  USE_WINDOW(window, false);

  ImGuiSliderFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeSliderFlags(flags);

  return ImGui::DragScalar(label, ImGuiDataType_Double,
    valueInOut, VALUE_OR(valueSpeedInOptional, 1.0),
    valueMinInOptional, valueMaxInOptional,
    NULL_IF_EMPTY(formatInOptional),
    flags
  );
});

    // IMGUI_API bool          DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);

DEFINE_API(bool, SliderInt, ((Window*,window))
((const char*,label))((int*,valueInOut))((int,valueMin))((int,valueMax))
((const char*,formatInOptional))((int*,flagsInOptional)),
"'format' is '%d' by default.",
{
  USE_WINDOW(window, false);

  ImGuiSliderFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderInt(label, valueInOut, valueMin, valueMax,
    NULL_IF_EMPTY(formatInOptional) ? formatInOptional : "%d",
    flags
  );
});


DEFINE_API(bool, SliderDouble, ((Window*,window))
((const char*,label))((double*,valueInOut))((double,valueMin))((double,valueMax))
((const char*,formatInOptional))((int*,flagsInOptional)),
"'format' is '%f' by default.",
{
  USE_WINDOW(window, false);

  ImGuiSliderFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderScalar(label, ImGuiDataType_Double,
    valueInOut, &valueMin, &valueMax,
    NULL_IF_EMPTY(formatInOptional) ? formatInOptional : "%f",
    flags
  );
});

    // IMGUI_API bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
    // IMGUI_API bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0);

