#include "api_helper.hpp"

static void sanitizeSliderFlags(ImGuiSliderFlags &flags)
{
  // dear imgui will assert if these bits are set
  flags &= ~ImGuiSliderFlags_InvalidMask_; 
}

// Widgets: Drag Sliders
DEFINE_API(bool, DragInt, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value)))((double*,API_RO(valueSpeed)))
((double*,API_RO(valueMin)))((double*,API_RO(valueMax)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
R"(- CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped and can go off-bounds.
- Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
- Format string may also be set to NULL or use the default format ("%f" or "%d").
- Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
- Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits.
- Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
- We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.)",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeSliderFlags(flags);

  return ImGui::DragInt(label, API_RW(value),
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0.0), valueOr(API_RO(valueMax), 0.0),
    API_RO(format), flags
  );
});
    // IMGUI_API bool          DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, ImGuiSliderFlags flags = 0);

DEFINE_API(bool, DragDouble, ((ImGui_Context*,ctx))
((const char*,label))((double*,API_RW(value)))((double*,API_RO(valueSpeed)))
((double*,API_RO(valueMin)))((double*,API_RO(valueMax)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeSliderFlags(flags);

  return ImGui::DragScalar(label, ImGuiDataType_Double,
    API_RW(value), valueOr(API_RO(valueSpeed), 1.0),
    API_RO(valueMin), API_RO(valueMax),
    API_RO(format), flags
  );
});

    // IMGUI_API bool          DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);
    // IMGUI_API bool          DragScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, ImGuiSliderFlags flags = 0);

DEFINE_API(bool, SliderInt, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value)))((int,valueMin))((int,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"'format' is '%d' by default.",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderInt(label, API_RW(value), valueMin, valueMax,
    API_RO(format) ? API_RO(format) : "%d", flags);
});


DEFINE_API(bool, SliderDouble, ((ImGui_Context*,ctx))
((const char*,label))((double*,API_RW(value)))((double,valueMin))((double,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"'format' is '%f' by default.",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderScalar(label, ImGuiDataType_Double,
    API_RW(value), &valueMin, &valueMax,
    API_RO(format) ? API_RO(format) : "%f", flags);
});

    // IMGUI_API bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
    // IMGUI_API bool          SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0);

