/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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

API_SECTION("Drag & Slider",
R"(We use the same sets of flags for Drag*() and Slider*() functions as the
features are the same and it makes it easier to swap them.

CTRL+Click on any drag box or slider to turn them into an input box.
Manually input values aren't clamped by default and can go off-bounds.
Use SliderFlags_AlwaysClamp to always clamp.

Adjust format string to decorate the value with a prefix, a suffix, or adapt the
editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs;
"Biscuit: %.0f" -> Biscuit: 1; etc.

Format string may also be set to nil or use the default format ("%f" or "%d").)");

class SliderFlags : public Flags<ImGuiSliderFlags> {
public:
  SliderFlags(int flags) : Flags { flags }
  {
    // dear imgui will assert if these bits are set
    *this &= ~ImGuiSliderFlags_InvalidMask_;
  }
};

API_SUBSECTION("Drag Sliders",
R"(Speed are per-pixel of mouse movement (v_speed=0.2: mouse needs to move by 5
pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed
is Max(v_speed, minimum_step_at_given_precision).

Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual
input can override those limits if SliderFlags_AlwaysClamp is not used.
Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with
v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.)");

API_FUNC(0_1, bool, DragInt, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v))(double*,API_RO(v_speed),1.0)
(int*,API_RO(v_min),0)(int*,API_RO(v_max),0)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  return ImGui::DragInt(label, API_RW(v), API_RO_GET(v_speed),
    API_RO_GET(v_min), API_RO_GET(v_max), API_RO_GET(format),
    SliderFlags { API_RO_GET(flags) });
}

API_FUNC(0_1, bool, DragInt2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(double*,API_RO(v_speed),1.0)
(int*,API_RO(v_min),0)(int*,API_RO(v_max),0)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };

  if(ImGui::DragInt2(label, values.data(), API_RO_GET(v_speed),
      API_RO_GET(v_min), API_RO_GET(v_max), API_RO_GET(format),
      SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragInt3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(double*,API_RO(v_speed),1.0)
(int*,API_RO(v_min),0)(int*,API_RO(v_max),0)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };

  if(ImGui::DragInt3(label, values.data(), API_RO_GET(v_speed),
      API_RO_GET(v_min), API_RO_GET(v_max), API_RO_GET(format),
      SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragInt4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(int*,API_RW(v4))(double*,API_RO(v_speed),1.0)
(int*,API_RO(v_min),0)(int*,API_RO(v_max),0)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };

  if(ImGui::DragInt4(label, values.data(), API_RO_GET(v_speed),
      API_RO_GET(v_min), API_RO_GET(v_max), API_RO_GET(format),
      SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragIntRange2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v_current_min))(int*,API_RW(v_current_max))
(double*,API_RO(v_speed),1.0)(int*,API_RO(v_min),0)(int*,API_RO(v_max),0)
(const char*,API_RO(format),"%d")(const char*,API_RO(format_max))
(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));
  nullIfEmpty(API_RO(format_max));

  ReadWriteArray<int, int, 2> values
    { API_RW(v_current_min), API_RW(v_current_max) };

  if(ImGui::DragIntRange2(label, &values[0], &values[1],
      API_RO_GET(v_speed), API_RO_GET(v_min), API_RO_GET(v_max),
      API_RO_GET(format), API_RO(format_max),
      SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragFloatRange2, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v_current_min))(double*,API_RW(v_current_max))
(double*,API_RO(v_speed),1.0)(double*,API_RO(v_min),0.0)(double*,API_RO(v_max),0.0)
(const char*,API_RO(format),"%.3f")(const char*,API_RO(format_max))
(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));
  nullIfEmpty(API_RO(format_max));

  ReadWriteArray<double, float, 2> values
    { API_RW(v_current_min), API_RW(v_current_max) };

  if(ImGui::DragFloatRange2(label, &values[0], &values[1],
      API_RO_GET(v_speed), API_RO_GET(v_min), API_RO_GET(v_max),
      API_RO_GET(format), API_RO(format_max),
      SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDouble, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v))(double*,API_RO(v_speed),1.0)
(double*,API_RO(v_min),0.0)(double*,API_RO(v_max),0.0)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  const double v_min { API_RO_GET(v_min) }, v_max { API_RO_GET(v_max) };
  return ImGui::DragScalar(label, ImGuiDataType_Double,
    API_RW(v), API_RO_GET(v_speed), &v_min, &v_max,
    API_RO_GET(format), SliderFlags { API_RO_GET(flags) });
}

static bool dragDoubleN(const char *label, double *data, const size_t size,
  const double v_speed, const double v_min, const double v_max,
  const char *format, const SliderFlags flags)
{
  return ImGui::DragScalarN(label, ImGuiDataType_Double, data, size,
    v_speed, &v_min, &v_max, format, flags);
}

API_FUNC(0_1, bool, DragDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))
(double*,API_RO(v_speed),1.0)(double*,API_RO(v_min),0.0)(double*,API_RO(v_max),0.0)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };

  if(dragDoubleN(label, values.data(), values.size(),
      API_RO_GET(v_speed), API_RO_GET(v_min), API_RO_GET(v_max),
      API_RO_GET(format), API_RO_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RO(v_speed),1.0)(double*,API_RO(v_min),0.0)(double*,API_RO(v_max),0.0)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };

  if(dragDoubleN(label, values.data(), values.size(),
      API_RO_GET(v_speed), API_RO_GET(v_min), API_RO_GET(v_max),
      API_RO_GET(format), API_RO_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RW(v4))(double*,API_RO(v_speed),1.0)
(double*,API_RO(v_min),0.0)(double*,API_RO(v_max),0.0)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };

  if(dragDoubleN(label, values.data(), values.size(),
      API_RO_GET(v_speed), API_RO_GET(v_min), API_RO_GET(v_max),
      API_RO_GET(format), API_RO_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDoubleN, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)
(double*,API_RO(speed),1.0)(double*,API_RO(min),0.0)(double*,API_RO(max),0.0)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  return dragDoubleN(label, values->data, values->size,
    API_RO_GET(speed), API_RO_GET(min), API_RO_GET(max),
    API_RO_GET(format), API_RO_GET(flags));
}

API_SUBSECTION("Regular Sliders");

API_FUNC(0_1, bool, SliderInt, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v))(int,v_min)(int,v_max)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  return ImGui::SliderInt(label, API_RW(v), v_min, v_max,
    API_RO_GET(format), SliderFlags { API_RO_GET(flags) });
}

API_FUNC(0_1, bool, SliderInt2, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))(int,v_min)(int,v_max)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };

  if(ImGui::SliderInt2(label, values.data(), v_min, v_max,
      API_RO_GET(format), SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderInt3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(int,v_min)(int,v_max)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };

  if(ImGui::SliderInt3(label, values.data(), v_min, v_max,
      API_RO_GET(format), SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderInt4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v1))(int*,API_RW(v2))
(int*,API_RW(v3))(int*,API_RW(v4))(int,v_min)(int,v_max)
(const char*,API_RO(format),"%d")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };

  if(ImGui::SliderInt4(label, values.data(), v_min, v_max,
      API_RO_GET(format), SliderFlags { API_RO_GET(flags) }))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDouble, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v))(double,v_min)(double,v_max)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  return ImGui::SliderScalar(label, ImGuiDataType_Double, API_RW(v),
    &v_min, &v_max, API_RO_GET(format), SliderFlags { API_RO_GET(flags) });
}

static bool sliderDoubleN(const char *label, double *data, const size_t size,
  const double v_min, const double v_max, const char *format,
  const SliderFlags flags)
{
  return ImGui::SliderScalarN(label, ImGuiDataType_Double, data, size,
    &v_min, &v_max, format, flags);
}

API_FUNC(0_1, bool, SliderDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))
(double,v_min)(double,v_max)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };

  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_RO_GET(format), API_RO_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double,v_min)(double,v_max)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };

  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_RO_GET(format), API_RO_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RW(v4))(double,v_min)(double,v_max)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };

  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_RO_GET(format), API_RO_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDoubleN, (ImGui_Context*,ctx)
(const char*,label)(reaper_array*,values)
(double,v_min)(double,v_max)(const char*,API_RO(format),"%.3f")
(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  return sliderDoubleN(label, values->data, values->size,
    v_min, v_max, API_RO_GET(format), API_RO_GET(flags));
}

API_FUNC(0_1, bool, SliderAngle, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RW(v_rad))
(double*,API_RO(v_degrees_min),-360.0)(double*,API_RO(v_degrees_max),+360.0)
(const char*,API_RO(format),"%.0f deg")
(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  assertValid(API_RW(v_rad));
  nullIfEmpty(API_RO(format));

  float rad = *API_RW(v_rad);

  if(ImGui::SliderAngle(label, &rad,
      API_RO_GET(v_degrees_min), API_RO_GET(v_degrees_max),
      API_RO_GET(format), SliderFlags { API_RO_GET(flags) })) {
    *API_RW(v_rad) = rad;
    return true;
  }
  return false;
}

API_FUNC(0_1, bool, VSliderInt, (ImGui_Context*,ctx)
(const char*,label)(double,size_w)(double,size_h)(int*,API_RW(v))
(int,v_min)(int,v_max)(const char*,API_RO(format),"%d")
(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  return ImGui::VSliderInt(label, ImVec2(size_w, size_h), API_RW(v),
    v_min, v_max, API_RO_GET(format), SliderFlags { API_RO_GET(flags) });
}

API_FUNC(0_1, bool, VSliderDouble, (ImGui_Context*,ctx)
(const char*,label)(double,size_w)(double,size_h)(double*,API_RW(v))
(double,v_min)(double,v_max)(const char*,API_RO(format),"%.3f")
(int*,API_RO(flags),ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  return ImGui::VSliderScalar(label, ImVec2(size_w, size_h),
    ImGuiDataType_Double, API_RW(v), &v_min, &v_max,
    API_RO_GET(format), SliderFlags { API_RO_GET(flags) });
}

API_SUBSECTION("Flags",
R"(For DragDouble, DragInt, SliderDouble, SliderInt etc. (Those are per-item
flags. There are shared flags in SetConfigVar: ConfigVar_DragClickToInputText)");
API_ENUM(0_1, ImGui, SliderFlags_None,            "");
API_ENUM(0_1, ImGui, SliderFlags_AlwaysClamp,
R"(Clamp value to min/max bounds when input manually with CTRL+Click.
   By default CTRL+Click allows going out of bounds.)");
API_ENUM(0_1, ImGui, SliderFlags_Logarithmic,
R"(Make the widget logarithmic (linear otherwise).
   Consider using SliderFlags_NoRoundToFormat with this if using a format-string
   with small amount of digits.)");
API_ENUM(0_1, ImGui, SliderFlags_NoRoundToFormat,
R"(Disable rounding underlying value to match precision of the display format
   string (e.g. %.3f values are rounded to those 3 digits).)");
API_ENUM(0_1, ImGui, SliderFlags_NoInput,
  "Disable CTRL+Click or Enter key allowing to input text directly into the widget.");
