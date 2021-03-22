/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
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
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  std::string value { API_RWBIG(buf) };
  if(ImGui::InputText(label, &value, flags, nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

DEFINE_API(bool, InputTextMultiline, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(double*,API_RO(width))(double*,API_RO(height))
(int*,API_RO(flags)),
"Default values: width = 0.0, height = 0.0, flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  std::string value { API_RWBIG(buf) };
  if(ImGui::InputTextMultiline(label, &value,
      ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)),
      valueOr(API_RO(flags), ImGuiInputTextFlags_None),
      nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

DEFINE_API(bool, InputTextWithHint, (ImGui_Context*,ctx)
(const char*,label)(const char*,hint)
(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  std::string value { API_RWBIG(buf) };
  if(ImGui::InputTextWithHint(label, hint, &value, flags, nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

template <typename PtrType, typename ValType, size_t N>
class ReadWriteArray {
public:
  template<typename... Args,
    typename = typename std::enable_if_t<sizeof...(Args) == N>>
  ReadWriteArray(Args&&... args)
    : m_inputs { std::forward<Args>(args)... }
  {
    size_t i { 0 };
    for(const PtrType *ptr : m_inputs)
      m_values[i++] = *ptr;
  }

  size_t size() const { return N; }
  ValType *data() { return m_values.data(); }
  ValType &operator[](const size_t i) { return m_values[i]; }

  bool commit()
  {
    size_t i { 0 };
    for(const ValType value : m_values)
      *m_inputs[i++] = value;
    return true;
  }

private:
  std::array<PtrType*, N> m_inputs;
  std::array<ValType, N> m_values;
};

DEFINE_API(bool, InputInt, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v))(int*,API_RO(step))(int*,API_RO(step_fast))
(int*,API_RO(flags)),
"Default values: step = 1, step_fast = 100, flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputInt(label, API_RW(v),
    valueOr(API_RO(step), 1), valueOr(API_RO(step_fast), 100), flags);
});

DEFINE_API(bool, InputInt2, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };
  if(ImGui::InputInt2(label, values.data(), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputInt3, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RW(v3))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  if(ImGui::InputInt3(label, values.data(), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputInt4, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RW(v3))
(int*,API_RW(v4))(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  if(ImGui::InputInt4(label, values.data(), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDouble, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v))(double*,API_RO(step))(double*,API_RO(step_fast))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: step = 0.0, step_fast = 0.0, format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputDouble(label, API_RW(v),
    valueOr(API_RO(step), 0.0), valueOr(API_RO(step_fast), 0.0),
    API_RO(format) ? API_RO(format) : "%.3f", flags);
});

static bool inputDoubleN(const char *label, double *data, const size_t size,
  const char *format, const ImGuiInputTextFlags flags)
{
  return ImGui::InputScalarN(label, ImGuiDataType_Double, data, size,
    nullptr, nullptr, format ? format : "%.3f", flags);
}

DEFINE_API(bool, InputDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };
  if(inputDoubleN(label, values.data(), values.size(), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  if(inputDoubleN(label, values.data(), values.size(), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RW(v4))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  if(inputDoubleN(label, values.data(), values.size(), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDoubleN, (ImGui_Context*,ctx)(const char*,label)
(reaper_array*,values)(double*,API_RO(step))(double*,API_RO(step_fast))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: step = nil, format = nil, step_fast = nil, format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size, API_RO(step), API_RO(step_fast),
    API_RO(format), flags);
});

static void sanitizeSliderFlags(ImGuiSliderFlags &flags)
{
  // dear imgui will assert if these bits are set
  flags &= ~ImGuiSliderFlags_InvalidMask_;
}

DEFINE_API(bool, DragInt, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v))(double*,API_RO(v_speed))
(int*,API_RO(v_min))(int*,API_RO(v_max))
(const char*,API_RO(format))(int*,API_RO(flags)),
R"(- CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped and can go off-bounds.
- Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
- Format string may also be set to NULL or use the default format ("%f" or "%d").
- Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
- Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits.
- Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
- We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.

Default values: v_speed = 1.0, v_min = 0, v_max = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragInt(label, API_RW(v), valueOr(API_RO(v_speed), 1.0),
    valueOr(API_RO(v_min), 0), valueOr(API_RO(v_max), 0), API_RO(format), flags);
});

DEFINE_API(bool, DragInt2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(double*,API_RO(v_speed))
(int*,API_RO(v_min))(int*,API_RO(v_max))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0, v_max = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };
  if(ImGui::DragInt2(label, values.data(), valueOr(API_RO(v_speed), 1.0),
      valueOr(API_RO(v_min), 0), valueOr(API_RO(v_max), 0), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, DragInt3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(double*,API_RO(v_speed))
(int*,API_RO(v_min))(int*,API_RO(v_max))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0, v_max = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  if(ImGui::DragInt3(label, values.data(), valueOr(API_RO(v_speed), 1.0),
      valueOr(API_RO(v_min), 0), valueOr(API_RO(v_max), 0), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, DragInt4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(int*,API_RW(v4))(double*,API_RO(v_speed))
(int*,API_RO(v_min))(int*,API_RO(v_max))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0, v_max = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  if(ImGui::DragInt4(label, values.data(), valueOr(API_RO(v_speed), 1.0),
      valueOr(API_RO(v_min), 0), valueOr(API_RO(v_max), 0), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, DragIntRange2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v_current_min))(int*,API_RW(v_current_max))
(double*,API_RO(v_speed))(int*,API_RO(v_min))(int*,API_RO(v_max))
(const char*,API_RO(format))(const char*,API_RO(format_max))
(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0, v_max = 0, format = '%d', format_max = nil, flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));
  nullIfEmpty(API_RO(format_max));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragIntRange2(label, API_RW(v_current_min), API_RW(v_current_max),
    valueOr(API_RO(v_speed), 1.0), valueOr(API_RO(v_min), 0),
    valueOr(API_RO(v_max), 0), API_RO(format), API_RO(format_max), flags);
});

DEFINE_API(bool, DragFloatRange2, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v_current_min))(double*,API_RW(v_current_max))
(double*,API_RO(v_speed))(double*,API_RO(v_min))(double*,API_RO(v_max))
(const char*,API_RO(format))(const char*,API_RO(format_max))
(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0.0, v_max = 0.0, format = '%.3f', format_max = nil, flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));
  nullIfEmpty(API_RO(format_max));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  ReadWriteArray<double, float, 2> values
    { API_RW(v_current_min), API_RW(v_current_max) };
  if(ImGui::DragFloatRange2(label, &values[0], &values[1],
      valueOr(API_RO(v_speed), 1.0), valueOr(API_RO(v_min), 0.0),
      valueOr(API_RO(v_max), 0.0), API_RO(format) ? API_RO(format) : "%.3f",
      API_RO(format_max), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, DragDouble, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v))(double*,API_RO(v_speed))
(double*,API_RO(v_min))(double*,API_RO(v_max))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0.0, v_max = 0.0, format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::DragScalar(label, ImGuiDataType_Double,
    API_RW(v), valueOr(API_RO(v_speed), 1.0),
    API_RO(v_min), API_RO(v_max),
    API_RO(format) ? API_RO(format) : "%.3f", flags
  );
});

static bool dragDoubleN(const char *label, double *data, const size_t size,
  double *v_speed, double *v_min, double *v_max, const char *format,
  ImGuiSliderFlags flags)
{
  return ImGui::DragScalarN(label, ImGuiDataType_Double, data, size,
    valueOr(v_speed, 1.0), v_min, v_max, format ? format : "%.3f", flags);
}

DEFINE_API(bool, DragDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))
(double*,API_RO(v_speed))(double*,API_RO(v_min))
(double*,API_RO(v_max))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0.0, v_max = 0.0, format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };
  if(dragDoubleN(label, values.data(), values.size(),
      API_RO(v_speed), API_RO(v_min), API_RO(v_max), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, DragDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RO(v_speed))(double*,API_RO(v_min))
(double*,API_RO(v_max))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0.0, v_max = 0.0, format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  if(dragDoubleN(label, values.data(), values.size(),
      API_RO(v_speed), API_RO(v_min), API_RO(v_max), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, DragDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RW(v4))(double*,API_RO(v_speed))
(double*,API_RO(v_min))(double*,API_RO(v_max))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_speed = 1.0, v_min = 0.0, v_max = 0.0, format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  if(dragDoubleN(label, values.data(), values.size(),
      API_RO(v_speed), API_RO(v_min), API_RO(v_max), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, DragDoubleN, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)(double*,API_RO(speed))
(double*,API_RO(min))(double*,API_RO(max))(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: speed = 1.0, min = nil, max = nil, format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return dragDoubleN(label, values->data, values->size, API_RO(speed),
    API_RO(min), API_RO(max), API_RO(format), flags);
});

DEFINE_API(bool, SliderInt, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v))(int,v_min)(int,v_max)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderInt(label, API_RW(v), v_min, v_max,
    API_RO(format) ? API_RO(format) : "%d", flags);
});

DEFINE_API(bool, SliderInt2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int,v_min)(int,v_max)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };
  if(ImGui::SliderInt2(label, values.data(), v_min, v_max,
      API_RO(format) ? API_RO(format) : "%d", flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, SliderInt3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(int,v_min)(int,v_max)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  if(ImGui::SliderInt3(label, values.data(), v_min, v_max,
      API_RO(format) ? API_RO(format) : "%d", flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, SliderInt4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(int*,API_RW(v4))(int,v_min)(int,v_max)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  if(ImGui::SliderInt4(label, values.data(), v_min, v_max,
      API_RO(format) ? API_RO(format) : "%d", flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, SliderDouble, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v))(double,v_min)(double,v_max)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::SliderScalar(label, ImGuiDataType_Double, API_RW(v),
    &v_min, &v_max, API_RO(format) ? API_RO(format) : "%.3f", flags);
});

static bool sliderDoubleN(const char *label, double *data, const size_t size,
  const double v_min, const double v_max, const char *format,
  const ImGuiSliderFlags flags)
{
  return ImGui::SliderScalarN(label, ImGuiDataType_Double, data, size,
    &v_min, &v_max, format ? format : "%.3f", flags);
}

DEFINE_API(bool, SliderDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))
(double,v_min)(double,v_max)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };
  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, SliderDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double,v_min)(double,v_max)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, SliderDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RW(v4))(double,v_min)(double,v_max)
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), ImGuiInputTextFlags_None) };
  sanitizeInputTextFlags(flags);

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, SliderDoubleN, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)
(double,v_min)(double,v_max)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return sliderDoubleN(label, values->data, values->size,
    v_min, v_max, API_RO(format), flags);
});

DEFINE_API(bool, SliderAngle, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v_rad))(double*,API_RO(v_degrees_min))
(double*,API_RO(v_degrees_max))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_degrees_min = -360.0, v_degrees_max = +360.0, format = '%.0f deg', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  float rad = *API_RW(v_rad);
  if(ImGui::SliderAngle(label, &rad,
      valueOr(API_RO(v_degrees_min), -360.0), valueOr(API_RO(v_degrees_max), +360.0),
      API_RO(format) ? API_RO(format) : "%.0f deg", flags)) {
    *API_RW(v_rad) = rad;
    return true;
  }
  return false;
});

DEFINE_API(bool, VSliderInt, (ImGui_Context*,ctx)
(const char*,label)(double,size_w)(double,size_h)(int*,API_RW(v))
(int,v_min)(int,v_max)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%d', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::VSliderInt(label, ImVec2(size_w, size_h), API_RW(v),
    v_min, v_max, API_RO(format) ? API_RO(format) : "%d", flags);
});

DEFINE_API(bool, VSliderDouble, (ImGui_Context*,ctx)
(const char*,label)(double,size_w)(double,size_h)(double*,API_RW(v))
(double,v_min)(double,v_max)(const char*,API_RO(format))
(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ImGuiSliderFlags flags { valueOr(API_RO(flags), ImGuiSliderFlags_None) };
  sanitizeSliderFlags(flags);

  return ImGui::VSliderScalar(label, ImVec2(size_w, size_h),
    ImGuiDataType_Double, API_RW(v), &v_min, &v_max,
    API_RO(format) ? API_RO(format) : "%.3f", flags);
});

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

DEFINE_API(bool, ColorEdit4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgba))(int*,API_RO(flags)),
R"(Color is in 0xRRGGBBAA or, if ImGui_ColorEditFlags_NoAlpha is set, 0xXXRRGGBB (XX is ignored and will not be modified).

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  FRAME_GUARD;

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  float col[4];
  Color(*API_RW(col_rgba), alpha).unpack(col);
  if(ImGui::ColorEdit4(label, col, flags)) {
    // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
    *API_RW(col_rgba) = Color{col}.pack(alpha, *API_RW(col_rgba));
    return true;
  }
  return false;
});

DEFINE_API(bool, ColorEdit3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgb))(int*,API_RO(flags)),
R"(Color is in 0xXXRRGGBB. XX is ignored and will not be modified.

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  // unneeded, only to show Edit3 in the error message instead of Edit4
  FRAME_GUARD;

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  flags |= ImGuiColorEditFlags_NoAlpha;
  return API_ColorEdit4(ctx, label, API_RW(col_rgb), &flags);
});

DEFINE_API(bool, ColorPicker4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgba))(int*,API_RO(flags))(int*,API_RO(ref_col)),
"Default values: flags = ImGui_ColorEditFlags_None, ref_col = nil",
{
  FRAME_GUARD;

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };

  float col[4], refCol[4];
  Color(*API_RW(col_rgba), alpha).unpack(col);
  if(API_RO(ref_col))
    Color(*API_RO(ref_col), alpha).unpack(refCol);

  if(ImGui::ColorPicker4(label, col, flags, API_RO(ref_col) ? refCol : nullptr)) {
    // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
    *API_RW(col_rgba) = Color{col}.pack(alpha, *API_RW(col_rgba));
    return true;
  }
  return false;
});

DEFINE_API(bool, ColorPicker3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgb))(int*,API_RO(flags)),
R"(Color is in 0xXXRRGGBB. XX is ignored and will not be modified.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  // unneeded, only to show Picker3 in the error message instead of Picker4
  FRAME_GUARD;

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  flags |= ImGuiColorEditFlags_NoAlpha;
  return API_ColorPicker4(ctx, label, API_RW(col_rgb), &flags, nullptr);
});

DEFINE_API(bool, ColorButton, (ImGui_Context*,ctx)
(const char*,desc_id)(int,col_rgba)(int*,API_RO(flags))
(double*,API_RO(size_w))(double*,API_RO(size_h)),
R"(Display a color square/button, hover for details, return true when pressed.

Default values: flags = ImGui_ColorEditFlags_None, size_w = 0.0, size_h = 0.0)",
{
  FRAME_GUARD;

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  const ImVec4 col { Color(col_rgba, alpha) };

  return ImGui::ColorButton(desc_id, col, flags,
    ImVec2(valueOr(API_RO(size_w), 0.0), valueOr(API_RO(size_h), 0.0)));
});

DEFINE_API(void, SetColorEditOptions, (ImGui_Context*,ctx)
(int,flags),
"Picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.",
{
  FRAME_GUARD;
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
(const char*,preview_value)(int*,API_RO(flags)),
R"(The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.

Default values: flags = ImGui_ComboFlags_None)",
{
  FRAME_GUARD;

  return ImGui::BeginCombo(label, preview_value,
    valueOr(API_RO(flags), ImGuiComboFlags_None));
});

DEFINE_API(void, EndCombo, (ImGui_Context*,ctx),
"Only call EndCombo() if BeginCombo() returns true!",
{
  FRAME_GUARD;
  ImGui::EndCombo();
});

DEFINE_API(bool, Combo, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(current_item))(char*,items)
(int*,API_RO(popup_max_height_in_items)),
R"(Helper over BeginCombo()/EndCombo() for convenience purpose. Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

Default values: popup_max_height_in_items = -1)",
{
  FRAME_GUARD;

  const auto &strings { splitString(items) };
  return ImGui::Combo(label, API_RW(current_item), strings.data(), strings.size(),
    valueOr(API_RO(popup_max_height_in_items), -1));
});

// Widgets: List Boxes
DEFINE_API(bool, ListBox, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(current_item))(char*,items)(int*,API_RO(height_in_items)),
R"(This is an helper over BeginListBox()/EndListBox() for convenience purpose. This is analoguous to how Combos are created.

Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

Default values: height_in_items = -1)",
{
  FRAME_GUARD;

  const auto &strings { splitString(items) };
  return ImGui::ListBox(label, API_RW(current_item), strings.data(), strings.size(),
    valueOr(API_RO(height_in_items), -1));
});

DEFINE_API(bool, BeginListBox, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RO(size_w))(double*,API_RO(size_h)),
R"(Open a framed scrolling region.  This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.

The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() or any items.

- Choose frame width:   width  > 0.0: custom  /  width  < 0.0 or -FLT_MIN: right-align   /  width  = 0.0 (default): use current ItemWidth
- Choose frame height:  height > 0.0: custom  /  height < 0.0 or -FLT_MIN: bottom-align  /  height = 0.0 (default): arbitrary default height which can fit ~7 items

Default values: size_w = 0.0, size_h = 0.0

See ImGui_EndListBox.)",
{
  FRAME_GUARD;

  return ImGui::BeginListBox(label,
    ImVec2(valueOr(API_RO(size_w), 0.0), valueOr(API_RO(size_h), 0.0)));
});

DEFINE_API(void, EndListBox, (ImGui_Context*,ctx),
R"(Only call EndListBox() if BeginListBox() returned true!

See ImGui_BeginListBox.)",
{
  FRAME_GUARD;
  ImGui::EndListBox();
});
