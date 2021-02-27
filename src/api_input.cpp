#include "api_helper.hpp"

#include <imgui/misc/cpp/imgui_stdlib.h>
#include <reaper_plugin_functions.h> // realloc_cmd_ptr
#include <reaper_plugin_secrets.h>   // reaper_array
#include <vector>

static void copyToBuffer(const std::string &value, char *buf, const size_t bufSize)
{
  int newSize {};
  if(value.size() >= bufSize && realloc_cmd_ptr(&buf, &newSize, value.size())) {
    // the buffer is no longer null-terminated after using realloc_cmd_ptr!
    std::memcpy(buf, value.c_str(), newSize);
  }
  else {
    const size_t limit { std::min(bufSize - 1, value.size()) };
    std::memcpy(buf, value.c_str(), limit);
    buf[limit] = '\0';
  }
}

static void sanitizeInputTextFlags(ImGuiInputTextFlags &flags)
{
  flags &= ~(
    // don't expose these to users
    ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory |
    ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackCharFilter |
    ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackResize |

    // reserved for ImGui's internal use
    ImGuiInputTextFlags_Multiline | ImGuiInputTextFlags_NoMarkEdited
  );
}

DEFINE_API(bool, InputText, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  std::string value { API_RWBIG(buf) };
  const bool rv = ImGui::InputText(label, &value, flags, nullptr, nullptr);
  copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
  return rv;
});

DEFINE_API(bool, InputTextMultiline, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(double*,API_RO(width))(double*,API_RO(height))
(int*,API_RO(flags)),
"Default values: width = 0.0, height = 0.0, flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  std::string value { API_RWBIG(buf) };
  const bool rv = ImGui::InputTextMultiline(label, &value,
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)),
    valueOr(API_RO(flags), ImGuiInputTextFlags_None),
    nullptr, nullptr);
  copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
  return rv;
});

DEFINE_API(bool, InputTextWithHint, (ImGui_Context*,ctx)
(const char*,label)(const char*,hint)
(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  std::string value { API_RWBIG(buf) };
  const bool rv = ImGui::InputTextWithHint(label, hint, &value, flags, nullptr, nullptr);
  copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
  return rv;
});

DEFINE_API(bool, InputInt, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(value))(int*,API_RO(step))(int*,API_RO(stepFast))
(int*,API_RO(flags)),
"Default values: step = 1, stepFast = 100, flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputInt(label, API_RW(value),
    valueOr(API_RO(step), 1), valueOr(API_RO(stepFast), 100), flags);
});

DEFINE_API(bool, InputInt2, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(value1))(int*,API_RW(value2))(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv { ImGui::InputInt2(label, values, flags) };
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, InputInt3, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(value1))(int*,API_RW(value2))(int*,API_RW(value3))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv { ImGui::InputInt3(label, values, flags) };
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, InputInt4, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(value1))(int*,API_RW(value2))(int*,API_RW(value3))
(int*,API_RW(value4))(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv { ImGui::InputInt4(label, values, flags) };
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, InputDouble, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value))(double*,API_RO(step))(double*,API_RO(stepFast))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: step = 1, stepFast = 100, format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputDouble(label, API_RW(value),
    valueOr(API_RO(step), 1.0), valueOr(API_RO(stepFast), 100.0),
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, InputDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::InputScalarN(label, ImGuiDataType_Double, values, 2,
    nullptr, nullptr, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, InputDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))(double*,API_RW(value3))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv = ImGui::InputScalarN(label, ImGuiDataType_Double, values, 3,
    nullptr, nullptr, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, InputDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))(double*,API_RW(value3))
(double*,API_RW(value4))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv = ImGui::InputScalarN(label, ImGuiDataType_Double, values, 4,
    nullptr, nullptr, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, InputDoubleN, (ImGui_Context*,ctx)(const char*,label)
(reaper_array*,values)(double*,API_RO(step))(double*,API_RO(stepFast))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = nil, stepFast = nil, format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size, API_RO(step), API_RO(stepFast),
    API_RO(format), flags);
});

static void sanitizeSliderFlags(ImGuiSliderFlags &flags)
{
  // dear imgui will assert if these bits are set
  flags &= ~ImGuiSliderFlags_InvalidMask_;
}

DEFINE_API(bool, DragInt, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value))(double*,API_RO(valueSpeed))
(int*,API_RO(valueMin))(int*,API_RO(valueMax))
(const char*,API_RO(format))(int*,API_RO(flags)),
R"(- CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped and can go off-bounds.
- Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
- Format string may also be set to NULL or use the default format ("%f" or "%d").
- Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
- Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits.
- Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
- We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.

Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragInt(label, API_RW(value),
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0), valueOr(API_RO(valueMax), 0),
    API_RO(format), flags
  );
});

DEFINE_API(bool, DragInt2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value1))(int*,API_RW(value2))
(double*,API_RO(valueSpeed))
(int*,API_RO(valueMin))(int*,API_RO(valueMax))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: valueSpeed = 1.0, valueMin = 0.0, valueMax = 0.0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::DragInt2(label, values,
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0), valueOr(API_RO(valueMax), 0),
    API_RO(format), flags
  );
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, DragInt3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value1))(int*,API_RW(value2))
(int*,API_RW(value3))(double*,API_RO(valueSpeed))
(int*,API_RO(valueMin))(int*,API_RO(valueMax))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv = ImGui::DragInt3(label, values,
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0), valueOr(API_RO(valueMax), 0),
    API_RO(format), flags
  );
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, DragInt4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value1))(int*,API_RW(value2))
(int*,API_RW(value3))(int*,API_RW(value4))(double*,API_RO(valueSpeed))
(int*,API_RO(valueMin))(int*,API_RO(valueMax))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv = ImGui::DragInt4(label, values,
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0), valueOr(API_RO(valueMax), 0),
    API_RO(format), flags
  );
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, DragIntRange2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(min))(int*,API_RW(max))
(double*,API_RO(speed))(int*,API_RO(minValue))(int*,API_RO(maxValue))
(const char*,API_RO(format))(const char*,API_RO(formatMax))
(int*,API_RO(flags)),
"Default values: speed = 1.0, minValue = 0, maxValue = 0, format = '%d', formatMax = nil, flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));
  nullIfEmpty(API_RO(formatMax));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragIntRange2(label, API_RW(min), API_RW(max),
    valueOr(API_RO(speed), 1.0), valueOr(API_RO(minValue), 0),
    valueOr(API_RO(maxValue), 0), API_RO(format), API_RO(formatMax), flags);
});

DEFINE_API(bool, DragFloatRange2, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(min))(double*,API_RW(max))
(double*,API_RO(speed))(double*,API_RO(minValue))(double*,API_RO(maxValue))
(const char*,API_RO(format))(const char*,API_RO(formatMax))
(int*,API_RO(flags)),
"Default values: speed = 1.0, minValue = 0.0, maxValue = 0.0, format = '%.3f', formatMax = nil, flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));
  nullIfEmpty(API_RO(formatMax));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  float currentMin = *API_RW(min), currentMax = *API_RW(max);
  const bool retval = ImGui::DragFloatRange2(label, &currentMin, &currentMax,
    valueOr(API_RO(speed), 1.0), valueOr(API_RO(minValue), 0.0),
    valueOr(API_RO(maxValue), 0.0), API_RO(format) ? API_RO(format) : "%.3f",
    API_RO(formatMax), flags);
  *API_RW(min) = currentMin; *API_RW(max) = currentMax;

  return retval;
});

DEFINE_API(bool, DragDouble, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(value))(double*,API_RO(valueSpeed))
(double*,API_RO(valueMin))(double*,API_RO(valueMax))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragScalar(label, ImGuiDataType_Double,
    API_RW(value), valueOr(API_RO(valueSpeed), 1.0),
    API_RO(valueMin), API_RO(valueMax),
    API_RO(format) ? API_RO(format) : "%.6f", flags
  );
});

DEFINE_API(bool, DragDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))
(double*,API_RO(valueSpeed))(double*,API_RO(valueMin))
(double*,API_RO(valueMax))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::DragScalarN(label, ImGuiDataType_Double, values, 2,
    valueOr(API_RO(valueSpeed), 1.0), API_RO(valueMin), API_RO(valueMax),
    API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, DragDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))(double*,API_RW(value3))
(double*,API_RO(valueSpeed))(double*,API_RO(valueMin))
(double*,API_RO(valueMax))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv = ImGui::DragScalarN(label, ImGuiDataType_Double, values, 3,
    valueOr(API_RO(valueSpeed), 1.0), API_RO(valueMin), API_RO(valueMax),
    API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, DragDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))(double*,API_RW(value3))
(double*,API_RW(value4))(double*,API_RO(valueSpeed))
(double*,API_RO(valueMin))(double*,API_RO(valueMax))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv = ImGui::DragScalarN(label, ImGuiDataType_Double, values, 4,
    valueOr(API_RO(valueSpeed), 1.0), API_RO(valueMin), API_RO(valueMax),
    API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, DragDoubleN, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)(double*,API_RO(speed))
(double*,API_RO(min))(double*,API_RO(max))(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: speed = 1.0, min = nil, max = nil, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragScalarN(label, ImGuiDataType_Double,
    values->data, values->size, valueOr(API_RO(speed), 1.0),
    API_RO(min), API_RO(max), API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, SliderInt, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value))(int,valueMin)(int,valueMax)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderInt(label, API_RW(value), valueMin, valueMax,
    API_RO(format) ? API_RO(format) : "%d", flags);
});

DEFINE_API(bool, SliderInt2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value1))(int*,API_RW(value2))
(int,valueMin)(int,valueMax)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::SliderInt2(label, values, valueMin, valueMax,
    API_RO(format) ? API_RO(format) : "%d", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, SliderInt3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value1))(int*,API_RW(value2))
(int*,API_RW(value3))(int,valueMin)(int,valueMax)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv = ImGui::SliderInt3(label, values, valueMin, valueMax,
    API_RO(format) ? API_RO(format) : "%d", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, SliderInt4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(value1))(int*,API_RW(value2))
(int*,API_RW(value3))(int*,API_RW(value4))(int,valueMin)(int,valueMax)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv = ImGui::SliderInt4(label, values, valueMin, valueMax,
    API_RO(format) ? API_RO(format) : "%d", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, SliderDouble, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(value))(double,valueMin)(double,valueMax)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderScalar(label, ImGuiDataType_Double,
    API_RW(value), &valueMin, &valueMax,
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, SliderDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))
(double,valueMin)(double,valueMax)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::SliderScalarN(label, ImGuiDataType_Double, values, 2,
    &valueMin, &valueMax, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, SliderDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))(double*,API_RW(value3))
(double,valueMin)(double,valueMax)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv = ImGui::SliderScalarN(label, ImGuiDataType_Double, values, 3,
    &valueMin, &valueMax, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, SliderDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(value1))(double*,API_RW(value2))(double*,API_RW(value3))
(double*,API_RW(value4))(double,valueMin)(double,valueMax)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv = ImGui::SliderScalarN(label, ImGuiDataType_Double, values, 4,
    &valueMin, &valueMax, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, SliderDoubleN, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)
(double,valueMin)(double,valueMax)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderScalarN(label, ImGuiDataType_Double,
    values->data, values->size, &valueMin, &valueMax,
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, VSliderInt, (ImGui_Context*,ctx)
(const char*,label)(double,width)(double,height)(int*,API_RW(value))
(int,valueMin)(int,valueMax)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::VSliderInt(label, ImVec2(width, height), API_RW(value),
    valueMin, valueMax, API_RO(format) ? API_RO(format) : "%d", flags);
});

DEFINE_API(bool, VSliderDouble, (ImGui_Context*,ctx)
(const char*,label)(double,width)(double,height)(double*,API_RW(value))
(double,valueMin)(double,valueMax)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  Context::check(ctx)->enterFrame();
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::VSliderScalar(label, ImVec2(width, height),
    ImGuiDataType_Double, API_RW(value), &valueMin, &valueMax,
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

DEFINE_API(bool, ColorEdit4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(rgba))(int*,API_RO(flags)),
R"(Color is in 0xRRGGBBAA or, if ImGui_ColorEditFlags_NoAlpha is set, 0xXXRRGGBB (XX is ignored and will not be modified).

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  Context::check(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  float col[4];
  Color(*API_RW(rgba), alpha).unpack(col);
  const bool ret { ImGui::ColorEdit4(label, col, flags) };

  // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
  *API_RW(rgba) = Color{col}.pack(alpha, *API_RW(rgba));

  return ret;
});

DEFINE_API(bool, ColorEdit3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(rgb))(int*,API_RO(flags)),
R"(Color is in 0xXXRRGGBB. XX is ignored and will not be modified.

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  // unneeded, only to show Edit3 in the error message instead of Edit4
  Context::check(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  flags |= ImGuiColorEditFlags_NoAlpha;
  return API_ColorEdit4(ctx, label, API_RW(rgb), &flags);
});

DEFINE_API(bool, ColorPicker4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(rgba))(int*,API_RO(flags))(int*,API_RO(refCol)),
"Default values: flags = ImGui_ColorEditFlags_None, refCol = nil",
{
  Context::check(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };

  float col[4], refCol[4];
  Color(*API_RW(rgba), alpha).unpack(col);
  if(API_RO(refCol))
    Color(*API_RO(refCol), alpha).unpack(refCol);

  const bool ret {
    ImGui::ColorPicker4(label, col, flags, API_RO(refCol) ? refCol : nullptr)
  };

  // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
  *API_RW(rgba) = Color{col}.pack(alpha, *API_RW(rgba));

  return ret;
});

DEFINE_API(bool, ColorPicker3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(rgb))(int*,API_RO(flags)),
R"(Color is in 0xXXRRGGBB. XX is ignored and will not be modified.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  // unneeded, only to show Picker3 in the error message instead of Picker4
  Context::check(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  flags |= ImGuiColorEditFlags_NoAlpha;
  return API_ColorPicker4(ctx, label, API_RW(rgb), &flags, nullptr);
});

DEFINE_API(bool, ColorButton, (ImGui_Context*,ctx)
(const char*,desc_id)(int,rgba)(int*,API_RO(flags))
(double*,API_RO(width))(double*,API_RO(height)),
R"(Display a color square/button, hover for details, return true when pressed.

Default values: flags = ImGui_ColorEditFlags_None, width = 0.0, height = 0.0)",
{
  Context::check(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  const ImVec4 col { Color(rgba, alpha) };

  return ImGui::ColorButton(desc_id, col, flags,
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)));
});

DEFINE_API(void, SetColorEditOptions, (ImGui_Context*,ctx)
(int,flags),
"Picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.",
{
  Context::check(ctx)->enterFrame();
  sanitizeColorEditFlags(flags);
  ImGui::SetColorEditOptions(flags);
});

// Allowing ReaScripts to input a null-separated string would be unsafe.
// REAPER's buf, buf_sz mechanism does not handle strings containing null
// bytes, so the user would have to specify the size manually. This would
// enable reading from arbitrary memory locations.
static std::vector<const char *> splitString(char *list)
{
  constexpr char ITEM_SEP { '\x1f' }; // ASCII Unit Separator
  const auto len { strlen(list) };

  if(len < 1 || list[len - 1] != ITEM_SEP || list[len] != '\0')
    throw reascript_error { "items are not terminated with \\31 (unit separator)" };

  std::vector<const char *> strings;

  do {
    strings.push_back(list);
    while(*list != ITEM_SEP) ++list;
    *list = '\0';
  } while(*++list);

  return strings;
}

// Widgets: Combo Box
DEFINE_API(bool, BeginCombo, (ImGui_Context*,ctx)(const char*,label)
(const char*,previewValue)(int*,API_RO(flags)),
R"(The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.

Default values: flags = ImGui_ComboFlags_None)",
{
  Context::check(ctx)->enterFrame();

  return ImGui::BeginCombo(label, previewValue,
    valueOr(API_RO(flags), ImGuiComboFlags_None));
});

DEFINE_API(void, EndCombo, (ImGui_Context*,ctx),
"Only call EndCombo() if BeginCombo() returns true!",
{
  Context::check(ctx)->enterFrame();
  ImGui::EndCombo();
});

DEFINE_API(bool, Combo, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(currentItem))(char*,items)
(int*,API_RO(popupMaxHeightInItems)),
R"(Helper over BeginCombo()/EndCombo() for convenience purpose. Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

Default values: popupMaxHeightInItems = -1)",
{
  Context::check(ctx)->enterFrame();

  const auto &strings { splitString(items) };
  return ImGui::Combo(label, API_RW(currentItem), strings.data(), strings.size(),
    valueOr(API_RO(popupMaxHeightInItems), -1));
});

// Widgets: List Boxes
DEFINE_API(bool, ListBox, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(currentItem))(char*,items)(int*,API_RO(heightInItems)),
R"(This is an helper over BeginListBox()/EndListBox() for convenience purpose. This is analoguous to how Combos are created.

Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

Default values: heightInItems = -1)",
{
  Context::check(ctx)->enterFrame();

  const auto &strings { splitString(items) };
  return ImGui::ListBox(label, API_RW(currentItem), strings.data(), strings.size(),
    valueOr(API_RO(heightInItems), -1));
});

DEFINE_API(bool, BeginListBox, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RO(width))(double*,API_RO(height)),
R"(Open a framed scrolling region.  This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.

The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() or any items.

- Choose frame width:   width  > 0.0: custom  /  width  < 0.0 or -FLT_MIN: right-align   /  width  = 0.0 (default): use current ItemWidth
- Choose frame height:  height > 0.0: custom  /  height < 0.0 or -FLT_MIN: bottom-align  /  height = 0.0 (default): arbitrary default height which can fit ~7 items

Default values: width = 0, height = 0

See ImGui_EndListBox.)",
{
  Context::check(ctx)->enterFrame();

  return ImGui::BeginListBox(label,
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)));
});

DEFINE_API(void, EndListBox, (ImGui_Context*,ctx),
R"(Only call EndListBox() if BeginListBox() returned true!

See ImGui_BeginListBox.)",
{
  Context::check(ctx)->enterFrame();
  ImGui::EndListBox();
});
