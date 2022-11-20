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

#include "flags.hpp"

#include <reaper_plugin_secrets.h> // reaper_array

API_SECTION("Drag & Slider");

DEFINE_API(bool, DragInt, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v))(double*,API_RO(v_speed))
(int*,API_RO(v_min))(int*,API_RO(v_max))
(const char*,API_RO(format))(int*,API_RO(flags)),
R"(- CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use ImGui_SliderFlags_AlwaysClamp to always clamp.
- Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
- Format string may also be set to NULL or use the default format ("%f" or "%d").
- Speed are per-pixel of mouse movement (v_speed=0.2: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
- Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits if ImGui_SliderFlags_AlwaysClamp is not used.
- Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
- We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.

Default values: v_speed = 1.0, v_min = 0, v_max = 0, format = '%d', flags = ImGui_SliderFlags_None)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  SliderFlags flags { API_RO(flags) };
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

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<int, int, 2> values
    { API_RW(v_current_min), API_RW(v_current_max) };
  SliderFlags flags { API_RO(flags) };

  if(ImGui::DragIntRange2(label, &values[0], &values[1],
      valueOr(API_RO(v_speed), 1.0), valueOr(API_RO(v_min), 0),
      valueOr(API_RO(v_max), 0), API_RO(format), API_RO(format_max), flags))
    return values.commit();
  else
    return false;
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

  ReadWriteArray<double, float, 2> values
    { API_RW(v_current_min), API_RW(v_current_max) };
  SliderFlags flags { API_RO(flags) };

  if(ImGui::DragFloatRange2(label, &values[0], &values[1],
      valueOr(API_RO(v_speed), 1.f), valueOr(API_RO(v_min), 0.f),
      valueOr(API_RO(v_max), 0.f), API_RO(format) ? API_RO(format) : "%.3f",
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

  SliderFlags flags { API_RO(flags) };
  return ImGui::DragScalar(label, ImGuiDataType_Double,
    API_RW(v), valueOr(API_RO(v_speed), 1.0),
    API_RO(v_min), API_RO(v_max),
    API_RO(format) ? API_RO(format) : "%.3f", flags
  );
});

static bool dragDoubleN(const char *label, double *data, const size_t size,
  double *v_speed, double *v_min, double *v_max, const char *format,
  const ImGuiSliderFlags flags)
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

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  SliderFlags flags { API_RO(flags) };

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

  SliderFlags flags { API_RO(flags) };
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

  SliderFlags flags { API_RO(flags) };
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

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  SliderFlags flags { API_RO(flags) };

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

  SliderFlags flags { API_RO(flags) };
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

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  SliderFlags flags { API_RO(flags) };

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

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  SliderFlags flags { API_RO(flags) };

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

  SliderFlags flags { API_RO(flags) };
  return sliderDoubleN(label, values->data, values->size,
    v_min, v_max, API_RO(format), flags);
});

DEFINE_API(bool, SliderAngle, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v_rad))(double*,API_RO(v_degrees_min))
(double*,API_RO(v_degrees_max))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: v_degrees_min = -360.0, v_degrees_max = +360.0, format = '%.0f deg', flags = ImGui_SliderFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RW(v_rad));
  nullIfEmpty(API_RO(format));

  float rad = *API_RW(v_rad);
  SliderFlags flags { API_RO(flags) };

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

  SliderFlags flags { API_RO(flags) };
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

  SliderFlags flags { API_RO(flags) };
  return ImGui::VSliderScalar(label, ImVec2(size_w, size_h),
    ImGuiDataType_Double, API_RW(v), &v_min, &v_max,
    API_RO(format) ? API_RO(format) : "%.3f", flags);
});

// ImGuiSliderFlags
DEFINE_ENUM(ImGui, SliderFlags_None,            "For ImGui_DragDouble, ImGui_DragInt, ImGui_SliderDouble, ImGui_SliderInt etc. (Those are per-item flags. There are shared flags in ImGui_SetConfigVar: ImGui_ConfigVar_DragClickToInputText)");
DEFINE_ENUM(ImGui, SliderFlags_AlwaysClamp,     "Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.");
DEFINE_ENUM(ImGui, SliderFlags_Logarithmic,     "Make the widget logarithmic (linear otherwise). Consider using ImGui_SliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.");
DEFINE_ENUM(ImGui, SliderFlags_NoRoundToFormat, "Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits).");
DEFINE_ENUM(ImGui, SliderFlags_NoInput,         "Disable CTRL+Click or Enter key allowing to input text directly into the widget.");

