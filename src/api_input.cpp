#include "api_helper.hpp"

static int inputTextCallback(ImGuiInputTextCallbackData *data)
{
  if(data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    realloc_cmd_ptr(&data->Buf, &data->BufSize, data->BufSize);

  return 0;
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

static void sanitizeSliderFlags(ImGuiSliderFlags &flags)
{
  // dear imgui will assert if these bits are set
  flags &= ~ImGuiSliderFlags_InvalidMask_;
}

// Widgets: Input with Keyboard
DEFINE_API(bool, InputText, ((ImGui_Context*,ctx))
((const char*,label))((char*,API_WBIG(buf)))((int,API_WBIG_SZ(buf)))
((int*,API_RO(flags))),
"Default values: flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputText(label, API_WBIG(buf), API_WBIG_SZ(buf),
    flags, &inputTextCallback, nullptr);
});

DEFINE_API(bool, InputTextMultiline, ((ImGui_Context*,ctx))
((const char*,label))((char*,API_WBIG(buf)))((int,API_WBIG_SZ(buf)))
((double*,API_RO(width)))((double*,API_RO(height)))
((int*,API_RO(flags))),
"Default values: width = 0, height = 0, flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false)

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputTextMultiline(label, API_WBIG(buf), API_WBIG_SZ(buf),
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)),
    valueOr(API_RO(flags), ImGuiInputTextFlags_None),
    &inputTextCallback, nullptr);
});

DEFINE_API(bool, InputTextWithHint, ((ImGui_Context*,ctx))
((const char*,label))((const char*,hint))
((char*,API_WBIG(buf)))((int,API_WBIG_SZ(buf)))
((int*,API_RO(flags))),
"Default values: flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputTextWithHint(label, hint, API_WBIG(buf), API_WBIG_SZ(buf),
    flags, &inputTextCallback, nullptr);
});

DEFINE_API(bool, InputInt, ((ImGui_Context*,ctx))((const char*,label))
((int*,API_RW(value)))((int*,API_RO(step)))((int*,API_RO(stepFast)))
((int*,API_RO(flags))),
"Default values: step = 1, stepFast = 100, flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputInt(label, API_RW(value),
    valueOr(API_RO(step), 1), valueOr(API_RO(stepFast), 100), flags);
});

DEFINE_API(bool, InputInt2, ((ImGui_Context*,ctx))((const char*,label))
((int*,API_RW(value1)))((int*,API_RW(value2)))((int*,API_RO(flags))),
"Default values: flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv { ImGui::InputInt2(label, values, flags) };
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, InputInt3, ((ImGui_Context*,ctx))((const char*,label))
((int*,API_RW(value1)))((int*,API_RW(value2)))((int*,API_RW(value3)))
((int*,API_RO(flags))),
"Default values: flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv { ImGui::InputInt3(label, values, flags) };
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, InputInt4, ((ImGui_Context*,ctx))((const char*,label))
((int*,API_RW(value1)))((int*,API_RW(value2)))((int*,API_RW(value3)))
((int*,API_RW(value4)))((int*,API_RO(flags))),
"Default values: flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv { ImGui::InputInt4(label, values, flags) };
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, InputDouble, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value)))((double*,API_RO(step)))((double*,API_RO(stepFast)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: step = 1, stepFast = 100, format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputDouble(label, API_RW(value),
    valueOr(API_RO(step), 1.0), valueOr(API_RO(stepFast), 100.0),
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, InputDouble2, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::InputScalarN(label, ImGuiDataType_Double, values, 2,
    nullptr, nullptr, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, InputDouble3, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))((double*,API_RW(value3)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, InputDouble4, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))((double*,API_RW(value3)))
((double*,API_RW(value4)))((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, InputDoubleN, ((ImGui_Context*,ctx))((const char*,label))
((reaper_array*,values))((double*,API_RO(step)))((double*,API_RO(stepFast)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = nil, stepFast = nil, format = '%.6f', flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size, API_RO(step), API_RO(stepFast),
    API_RO(format), flags);
});

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
- We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.

Default values: valueSpeed = 1.0, valueMin = 0.0, valueMax = 0.0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragInt(label, API_RW(value),
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0.0), valueOr(API_RO(valueMax), 0.0),
    API_RO(format), flags
  );
});

DEFINE_API(bool, DragInt2, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value1)))((int*,API_RW(value2)))
((double*,API_RO(valueSpeed)))
((double*,API_RO(valueMin)))((double*,API_RO(valueMax)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: valueSpeed = 1.0, valueMin = 0.0, valueMax = 0.0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::DragInt2(label, values,
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0.0), valueOr(API_RO(valueMax), 0.0),
    API_RO(format), flags
  );
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, DragInt3, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value1)))((int*,API_RW(value2)))
((int*,API_RW(value3)))((double*,API_RO(valueSpeed)))
((double*,API_RO(valueMin)))((double*,API_RO(valueMax)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: valueSpeed = 1.0, valueMin = 0.0, valueMax = 0.0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv = ImGui::DragInt3(label, values,
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0.0), valueOr(API_RO(valueMax), 0.0),
    API_RO(format), flags
  );
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, DragInt4, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value1)))((int*,API_RW(value2)))
((int*,API_RW(value3)))((int*,API_RW(value4)))((double*,API_RO(valueSpeed)))
((double*,API_RO(valueMin)))((double*,API_RO(valueMax)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: valueSpeed = 1.0, valueMin = 0.0, valueMax = 0.0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv = ImGui::DragInt4(label, values,
    valueOr(API_RO(valueSpeed), 1.0),
    valueOr(API_RO(valueMin), 0.0), valueOr(API_RO(valueMax), 0.0),
    API_RO(format), flags
  );
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, DragIntRange2, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(min)))((int*,API_RW(max)))
((double*,API_RO(speed)))((int*,API_RO(minValue)))((int*,API_RO(maxValue)))
((const char*,API_RO(format)))((const char*,API_RO(formatMax)))
((int*,API_RO(flags))),
"Default values: speed = 1.0, minValue = 0, maxValue = 0, format = '%d', formatMax = nil, flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));
  nullIfEmpty(API_RO(formatMax));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragIntRange2(label, API_RW(min), API_RW(max),
    valueOr(API_RO(speed), 1.0), valueOr(API_RO(minValue), 0),
    valueOr(API_RO(maxValue), 0), API_RO(format), API_RO(formatMax), flags);
});

DEFINE_API(bool, DragFloatRange2, ((ImGui_Context*,ctx))
((const char*,label))((double*,API_RW(min)))((double*,API_RW(max)))
((double*,API_RO(speed)))((double*,API_RO(minValue)))((double*,API_RO(maxValue)))
((const char*,API_RO(format)))((const char*,API_RO(formatMax)))
((int*,API_RO(flags))),
"Default values: speed = 1.0, minValue = 0.0, maxValue = 0.0, format = '%.3f', formatMax = nil, flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, DragDouble, ((ImGui_Context*,ctx))
((const char*,label))((double*,API_RW(value)))((double*,API_RO(valueSpeed)))
((double*,API_RO(valueMin)))((double*,API_RO(valueMax)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragScalar(label, ImGuiDataType_Double,
    API_RW(value), valueOr(API_RO(valueSpeed), 1.0),
    API_RO(valueMin), API_RO(valueMax),
    API_RO(format) ? API_RO(format) : "%.6f", flags
  );
});

DEFINE_API(bool, DragDouble2, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))
((double*,API_RO(valueSpeed)))((double*,API_RO(valueMin)))
((double*,API_RO(valueMax)))((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, DragDouble3, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))((double*,API_RW(value3)))
((double*,API_RO(valueSpeed)))((double*,API_RO(valueMin)))
((double*,API_RO(valueMax)))((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, DragDouble4, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))((double*,API_RW(value3)))
((double*,API_RW(value4)))((double*,API_RO(valueSpeed)))
((double*,API_RO(valueMin)))((double*,API_RO(valueMax)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: valueSpeed = 1.0, valueMin = 0, valueMax = 0, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, DragDoubleN, ((ImGui_Context*,ctx))
((const char*,label))((reaper_array*,values))((double*,API_RO(speed)))
((double*,API_RO(min)))((double*,API_RO(max)))((const char*,API_RO(format)))
((int*,API_RO(flags))),
"Default values: speed = 1.0, min = nil, max = nil, format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragScalarN(label, ImGuiDataType_Double,
    values->data, values->size, valueOr(API_RO(speed), 1.0),
    API_RO(min), API_RO(max), API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, SliderInt, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value)))((int,valueMin))((int,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderInt(label, API_RW(value), valueMin, valueMax,
    API_RO(format) ? API_RO(format) : "%d", flags);
});

DEFINE_API(bool, SliderInt2, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value1)))((int*,API_RW(value2)))
((int,valueMin))((int,valueMax))((const char*,API_RO(format)))
((int*,API_RO(flags))),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  int values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::SliderInt2(label, values, valueMin, valueMax,
    API_RO(format) ? API_RO(format) : "%d", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, SliderInt3, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value1)))((int*,API_RW(value2)))
((int*,API_RW(value3)))((int,valueMin))((int,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, SliderInt4, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(value1)))((int*,API_RW(value2)))
((int*,API_RW(value3)))((int*,API_RW(value4)))((int,valueMin))((int,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
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

DEFINE_API(bool, SliderDouble, ((ImGui_Context*,ctx))
((const char*,label))((double*,API_RW(value)))((double,valueMin))((double,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderScalar(label, ImGuiDataType_Double,
    API_RW(value), &valueMin, &valueMax,
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, SliderDouble2, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))
((double*,valueMin))((double*,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2) };
  const bool rv = ImGui::SliderScalarN(label, ImGuiDataType_Double, values, 2,
    valueMin, valueMax, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1];
  return rv;
});

DEFINE_API(bool, SliderDouble3, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))((double*,API_RW(value3)))
((double*,valueMin))((double*,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3) };
  const bool rv = ImGui::SliderScalarN(label, ImGuiDataType_Double, values, 3,
    valueMin, valueMax, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2];
  return rv;
});

DEFINE_API(bool, SliderDouble4, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value1)))((double*,API_RW(value2)))((double*,API_RW(value3)))
((double*,API_RW(value4)))((double*,valueMin))((double*,valueMax))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  double values[] { *API_RW(value1), *API_RW(value2), *API_RW(value3), *API_RW(value4) };
  const bool rv = ImGui::SliderScalarN(label, ImGuiDataType_Double, values, 4,
    valueMin, valueMax, API_RO(format) ? API_RO(format) : "%.6f", flags);
  *API_RW(value1) = values[0], *API_RW(value2) = values[1],
  *API_RW(value3) = values[2], *API_RW(value4) = values[3];
  return rv;
});

DEFINE_API(bool, SliderDoubleN, ((ImGui_Context*,ctx))
((const char*,label))((reaper_array*,values))
((double,valueMin))((double,valueMax))((const char*,API_RO(format)))
((int*,API_RO(flags))),
"Default values: format = '%.6f', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderScalarN(label, ImGuiDataType_Double,
    values->data, values->size, &valueMin, &valueMax,
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});

DEFINE_API(bool, VSliderInt, ((ImGui_Context*,ctx))
((const char*,label))((double,width))((double,height))((int*,API_RW(value)))
((int,valueMin))((int,valueMax))((const char*,API_RO(format)))
((int*,API_RO(flags))),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::VSliderInt(label, ImVec2(width, height), API_RW(value),
    valueMin, valueMax, API_RO(format) ? API_RO(format) : "%d", flags);
});

DEFINE_API(bool, VSliderDouble, ((ImGui_Context*,ctx))
((const char*,label))((double,width))((double,height))((double*,API_RW(value)))
((double,valueMin))((double,valueMax))((const char*,API_RO(format)))
((int*,API_RO(flags))),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::VSliderScalar(label, ImVec2(width, height),
    ImGuiDataType_Double, API_RW(value), &valueMin, &valueMax,
    API_RO(format) ? API_RO(format) : "%.6f", flags);
});
