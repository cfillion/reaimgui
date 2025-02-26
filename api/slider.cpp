/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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
  SliderFlags(int flags) : Flags {flags}
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

API_FUNC(0_1, bool, DragInt, (Context*,ctx)
(const char*,label) (RW<int*>,v) (RO<double*>,v_speed,1.0)
(RO<int*>,v_min,0) (RO<int*>,v_max,0)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  return ImGui::DragInt(label, v, API_GET(v_speed),
    API_GET(v_min), API_GET(v_max), API_GET(format),
    SliderFlags {API_GET(flags)});
}

API_FUNC(0_1, bool, DragInt2, (Context*,ctx)
(const char*,label) (RW<int*>,v1) (RW<int*>,v2)
(RO<double*>,v_speed,1.0)
(RO<int*>,v_min,0) (RO<int*>,v_max,0)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<int, int, 2> values {v1, v2};

  if(ImGui::DragInt2(label, values.data(), API_GET(v_speed),
      API_GET(v_min), API_GET(v_max), API_GET(format),
      SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragInt3, (Context*,ctx)
(const char*,label) (RW<int*>,v1) (RW<int*>,v2)
(RW<int*>,v3) (RO<double*>,v_speed,1.0)
(RO<int*>,v_min,0) (RO<int*>,v_max,0)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<int, int, 3> values {v1, v2, v3};

  if(ImGui::DragInt3(label, values.data(), API_GET(v_speed),
      API_GET(v_min), API_GET(v_max), API_GET(format),
      SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragInt4, (Context*,ctx)
(const char*,label) (RW<int*>,v1) (RW<int*>,v2)
(RW<int*>,v3) (RW<int*>,v4) (RO<double*>,v_speed,1.0)
(RO<int*>,v_min,0) (RO<int*>,v_max,0)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<int, int, 4> values {v1, v2, v3, v4};

  if(ImGui::DragInt4(label, values.data(), API_GET(v_speed),
      API_GET(v_min), API_GET(v_max), API_GET(format),
      SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragIntRange2, (Context*,ctx)
(const char*,label) (RW<int*>,v_current_min) (RW<int*>,v_current_max)
(RO<double*>,v_speed,1.0) (RO<int*>,v_min,0) (RO<int*>,v_max,0)
(RO<const char*>,format,"%d") (RO<const char*>,format_max)
(RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);
  nullIfEmpty(format_max);

  ReadWriteArray<int, int, 2> values {v_current_min, v_current_max};

  if(ImGui::DragIntRange2(label, &values[0], &values[1],
      API_GET(v_speed), API_GET(v_min), API_GET(v_max),
      API_GET(format), format_max, SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragFloatRange2, (Context*,ctx)
(const char*,label) (RW<double*>,v_current_min) (RW<double*>,v_current_max)
(RO<double*>,v_speed,1.0) (RO<double*>,v_min,0.0) (RO<double*>,v_max,0.0)
(RO<const char*>,format,"%.3f") (RO<const char*>,format_max)
(RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);
  nullIfEmpty(format_max);

  ReadWriteArray<double, float, 2> values {v_current_min, v_current_max};

  if(ImGui::DragFloatRange2(label, &values[0], &values[1],
      API_GET(v_speed), API_GET(v_min), API_GET(v_max),
      API_GET(format), format_max, SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDouble, (Context*,ctx)
(const char*,label) (RW<double*>,v) (RO<double*>,v_speed,1.0)
(RO<double*>,v_min,0.0) (RO<double*>,v_max,0.0)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  return ImGui::DragScalar(label, ImGuiDataType_Double,
    v, API_GET(v_speed), &API_GET(v_min), &API_GET(v_max),
    API_GET(format), SliderFlags {API_GET(flags)});
}

static bool dragDoubleN(const char *label, double *data, const size_t size,
  const double v_speed, const double v_min, const double v_max,
  const char *format, const SliderFlags flags)
{
  return ImGui::DragScalarN(label, ImGuiDataType_Double, data, size,
    v_speed, &v_min, &v_max, format, flags);
}

API_FUNC(0_1, bool, DragDouble2, (Context*,ctx) (const char*,label)
(RW<double*>,v1) (RW<double*>,v2)
(RO<double*>,v_speed,1.0) (RO<double*>,v_min,0.0) (RO<double*>,v_max,0.0)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<double, double, 2> values {v1, v2};

  if(dragDoubleN(label, values.data(), values.size(),
      API_GET(v_speed), API_GET(v_min), API_GET(v_max),
      API_GET(format), API_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDouble3, (Context*,ctx) (const char*,label)
(RW<double*>,v1) (RW<double*>,v2) (RW<double*>,v3)
(RO<double*>,v_speed,1.0) (RO<double*>,v_min,0.0) (RO<double*>,v_max,0.0)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<double, double, 3> values {v1, v2, v3};

  if(dragDoubleN(label, values.data(), values.size(),
      API_GET(v_speed), API_GET(v_min), API_GET(v_max),
      API_GET(format), API_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDouble4, (Context*,ctx) (const char*,label)
(RW<double*>,v1) (RW<double*>,v2) (RW<double*>,v3)
(RW<double*>,v4) (RO<double*>,v_speed,1.0)
(RO<double*>,v_min,0.0) (RO<double*>,v_max,0.0)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<double, double, 4> values {v1, v2, v3, v4};

  if(dragDoubleN(label, values.data(), values.size(),
      API_GET(v_speed), API_GET(v_min), API_GET(v_max),
      API_GET(format), API_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, DragDoubleN, (Context*,ctx)
(const char*,label) (reaper_array*,values)
(RO<double*>,speed,1.0) (RO<double*>,min,0.0) (RO<double*>,max,0.0)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(format);

  return dragDoubleN(label, values->data, values->size,
    API_GET(speed), API_GET(min), API_GET(max),
    API_GET(format), API_GET(flags));
}

API_SUBSECTION("Regular Sliders");

API_FUNC(0_1, bool, SliderInt, (Context*,ctx)
(const char*,label) (RW<int*>,v) (int,v_min) (int,v_max)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  return ImGui::SliderInt(label, v, v_min, v_max,
    API_GET(format), SliderFlags {API_GET(flags)});
}

API_FUNC(0_1, bool, SliderInt2, (Context*,ctx)
(const char*,label) (RW<int*>,v1) (RW<int*>,v2) (int,v_min) (int,v_max)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<int, int, 2> values {v1, v2};

  if(ImGui::SliderInt2(label, values.data(), v_min, v_max,
      API_GET(format), SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderInt3, (Context*,ctx)
(const char*,label) (RW<int*>,v1) (RW<int*>,v2)
(RW<int*>,v3) (int,v_min) (int,v_max)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<int, int, 3> values {v1, v2, v3};

  if(ImGui::SliderInt3(label, values.data(), v_min, v_max,
      API_GET(format), SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderInt4, (Context*,ctx)
(const char*,label) (RW<int*>,v1) (RW<int*>,v2)
(RW<int*>,v3) (RW<int*>,v4) (int,v_min) (int,v_max)
(RO<const char*>,format,"%d") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<int, int, 4> values {v1, v2, v3, v4};

  if(ImGui::SliderInt4(label, values.data(), v_min, v_max,
      API_GET(format), SliderFlags {API_GET(flags)}))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDouble, (Context*,ctx)
(const char*,label) (RW<double*>,v) (double,v_min) (double,v_max)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  return ImGui::SliderScalar(label, ImGuiDataType_Double, v,
    &v_min, &v_max, API_GET(format), SliderFlags {API_GET(flags)});
}

static bool sliderDoubleN(const char *label, double *data, const size_t size,
  const double v_min, const double v_max, const char *format,
  const SliderFlags flags)
{
  return ImGui::SliderScalarN(label, ImGuiDataType_Double, data, size,
    &v_min, &v_max, format, flags);
}

API_FUNC(0_1, bool, SliderDouble2, (Context*,ctx) (const char*,label)
(RW<double*>,v1) (RW<double*>,v2)
(double,v_min) (double,v_max)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<double, double, 2> values {v1, v2};

  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_GET(format), API_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDouble3, (Context*,ctx) (const char*,label)
(RW<double*>,v1) (RW<double*>,v2) (RW<double*>,v3)
(double,v_min) (double,v_max)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<double, double, 3> values {v1, v2, v3};

  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_GET(format), API_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDouble4, (Context*,ctx) (const char*,label)
(RW<double*>,v1) (RW<double*>,v2) (RW<double*>,v3)
(RW<double*>,v4) (double,v_min) (double,v_max)
(RO<const char*>,format,"%.3f") (RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  ReadWriteArray<double, double, 4> values {v1, v2, v3, v4};

  if(sliderDoubleN(label, values.data(), values.size(),
      v_min, v_max, API_GET(format), API_GET(flags)))
    return values.commit();
  else
    return false;
}

API_FUNC(0_1, bool, SliderDoubleN, (Context*,ctx)
(const char*,label) (reaper_array*,values)
(double,v_min) (double,v_max) (RO<const char*>,format,"%.3f")
(RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(format);

  return sliderDoubleN(label, values->data, values->size,
    v_min, v_max, API_GET(format), API_GET(flags));
}

API_FUNC(0_1, bool, SliderAngle, (Context*,ctx)
(const char*,label) (RW<double*>,v_rad)
(RO<double*>,v_degrees_min,-360.0) (RO<double*>,v_degrees_max,+360.0)
(RO<const char*>,format,"%.0f deg")
(RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  assertValid(v_rad);
  nullIfEmpty(format);

  float rad = *v_rad;

  if(ImGui::SliderAngle(label, &rad,
      API_GET(v_degrees_min), API_GET(v_degrees_max),
      API_GET(format), SliderFlags {API_GET(flags)})) {
    *v_rad = rad;
    return true;
  }
  return false;
}

API_FUNC(0_1, bool, VSliderInt, (Context*,ctx)
(const char*,label) (double,size_w) (double,size_h) (RW<int*>,v)
(int,v_min) (int,v_max) (RO<const char*>,format,"%d")
(RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  return ImGui::VSliderInt(label, ImVec2(size_w, size_h), v,
    v_min, v_max, API_GET(format), SliderFlags {API_GET(flags)});
}

API_FUNC(0_1, bool, VSliderDouble, (Context*,ctx)
(const char*,label) (double,size_w) (double,size_h) (RW<double*>,v)
(double,v_min) (double,v_max) (RO<const char*>,format,"%.3f")
(RO<int*>,flags,ImGuiSliderFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(format);

  return ImGui::VSliderScalar(label, ImVec2(size_w, size_h),
    ImGuiDataType_Double, v, &v_min, &v_max,
    API_GET(format), SliderFlags {API_GET(flags)});
}

API_SUBSECTION("Flags",
R"(For DragDouble, DragInt, SliderDouble, SliderInt etc. (Those are per-item
flags. There are shared flags in SetConfigVar: ConfigVar_DragClickToInputText)");
API_ENUM(0_1, ImGui, SliderFlags_None, "");
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
API_ENUM(0_9_2, ImGui, SliderFlags_WrapAround,
R"(Enable wrapping around from max to min and from min to max
   (only supported by DragXXX() functions for now).)");
