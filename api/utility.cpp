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

#include <version.hpp>

#include "drawlist.hpp"
#include "helper.hpp"
#include "listclipper.hpp"
#include "textfilter.hpp"
#include "viewport.hpp"

#include "../src/color.hpp"
#include "../src/font.hpp"
#include "../src/function.hpp"
#include "../src/image.hpp"
#include "../src/platform.hpp"

#include <climits>
#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h> // WDL_DIRCHAR_STR

API_SECTION("Utility");

API_FUNC(0_7, void, GetVersion,
(W<char*>,imgui_version) (WS<int>,imgui_version_sz)
(W<int*>,imgui_version_num)
(W<char*>,reaimgui_version) (WS<int>,reaimgui_version_sz),
"")
{
  if(imgui_version)
    snprintf(imgui_version, imgui_version_sz, "%s", IMGUI_VERSION);
  if(imgui_version_num)
    *imgui_version_num = IMGUI_VERSION_NUM;
  if(reaimgui_version)
    snprintf(reaimgui_version, reaimgui_version_sz, "%s", REAIMGUI_VERSION);
}

#define RESOURCE_ISVALID(klass)          \
  if(!strcmp(type, "ImGui_" #klass "*")) \
    return Resource::isValid(static_cast<klass *>(pointer))
#define RESOURCEPROXY_ISVALID(klass)     \
  if(!strcmp(type, "ImGui_" #klass "*")) \
    return klass::isValid(static_cast<klass *>(pointer));

API_FUNC(0_3, bool, ValidatePtr, (void*,pointer) (const char*,type),
R"(Return whether the given pointer is a valid instance of one of the following
types (indentation represents inheritance):

- ImGui_Context*
- ImGui_DrawList*
- ImGui_DrawListSplitter*
- ImGui_Font*
- ImGui_Function*
- ImGui_Image*
  - ImGui_ImageSet*
- ImGui_ListClipper*
- ImGui_TextFilter*
- ImGui_Viewport*)")
{

  RESOURCE_ISVALID(Context);
  RESOURCE_ISVALID(DrawListSplitter);
  RESOURCE_ISVALID(Font);
  RESOURCE_ISVALID(Function);
  RESOURCE_ISVALID(Image);
  RESOURCE_ISVALID(ImageSet);
  RESOURCE_ISVALID(ListClipper);
  RESOURCE_ISVALID(TextFilter);

  RESOURCEPROXY_ISVALID(DrawListProxy);
  RESOURCEPROXY_ISVALID(ViewportProxy);

  return false;
}

#undef RESOURCE_ISVALID
#undef RESOURCEPROXY_ISVALID

API_FUNC(0_9, const char*, GetBuiltinPath, API_NO_ARGS,
"Returns the path to the directory containing imgui.lua, imgui.py and gfx2imgui.lua.")
{
  static const std::string output {
    std::string {GetResourcePath()} +
    WDL_DIRCHAR_STR "Scripts" WDL_DIRCHAR_STR "ReaTeam Extensions"
    WDL_DIRCHAR_STR "API"
  };

  return output.c_str();
}

API_FUNC(0_1, void, ProgressBar, (Context*,ctx)
(double,fraction)
(RO<double*>,size_arg_w,-FLT_MIN) (RO<double*>,size_arg_h,0.0)
(RO<const char*>,overlay),
R"(Fraction < 0.0 displays an indeterminate progress bar animation since v0.9.1.
The value must be animated along with time, for example `-1.0 * ImGui.GetTime()`.)")
{
  FRAME_GUARD;
  nullIfEmpty(overlay);
  const ImVec2 size(API_GET(size_arg_w), API_GET(size_arg_h));
  ImGui::ProgressBar(fraction, size, overlay);
}

API_FUNC(0_5_1, void, PointConvertNative, (Context*,ctx)
(RW<double*>,x) (RW<double*>,y) (RO<bool*>,to_native,false),
R"(Convert a position from the current platform's native coordinate position
system to ReaImGui global coordinates (or vice versa).

This effectively flips the Y coordinate on macOS and applies HiDPI scaling on
Windows and Linux.)")
{
  FRAME_GUARD; // scalePosition uses the active context and its monitor list
  assertValid(x);
  assertValid(y);

  ImVec2 point;
  point.x = *x;
  point.y = *y;
  Platform::scalePosition(&point, API_GET(to_native));
  *x = point.x;
  *y = point.y;
}

#define NUMERIC_LIMITS(vernum, name, type, minVal, maxVal) \
  API_FUNC(vernum, void, NumericLimits_##name,             \
    (W<type*>,min) (W<type*>,max),                         \
  "Returns " #minVal " and " #maxVal " for this system.")  \
  {                                                        \
    if(min) *min = minVal;                                 \
    if(max) *max = maxVal;                                 \
  }

NUMERIC_LIMITS(0_8_4, Double, double, DBL_MIN, DBL_MAX);
NUMERIC_LIMITS(0_4,   Float,  double, FLT_MIN, FLT_MAX);
NUMERIC_LIMITS(0_8_4, Int,    int,    INT_MIN, INT_MAX);

API_SUBSECTION("ID Stack/Scope",
R"(Read the [FAQ](https://dearimgui.com/faq) for more details about how IDs are
handled in dear imgui.

- Those questions are answered and impacted by understanding of the ID stack system:
  - "Q: Why is my widget not reacting when I click on it?"
  - "Q: How can I have widgets with an empty label?"
  - "Q: How can I have multiple widgets with the same label?"
- Short version: ID are hashes of the entire ID stack. If you are creating widgets
  in a loop you most likely want to push a unique identifier (e.g. object pointer,
  loop index) to uniquely differentiate them.
- You can also use the "Label##foobar" syntax within widget label to distinguish
  them from each others.
- We use the "label"/"name" terminology to denote a string that will be
  displayed + used as an ID, whereas "str_id" denote a string that is only used
  as an ID and not normally displayed.)");

API_FUNC(0_1, void, PushID, (Context*,ctx)
(const char*,str_id),
"Push string into the ID stack.")
{
  FRAME_GUARD;
  ImGui::PushID(str_id);
}

API_FUNC(0_1, void, PopID, (Context*,ctx),
"Pop from the ID stack.")
{
  FRAME_GUARD;
  ImGui::PopID();
}

API_SUBSECTION("Color Conversion");

API_FUNC(0_7, void, ColorConvertU32ToDouble4,
(int,rgba)
(W<double*>,r) (W<double*>,g) (W<double*>,b) (W<double*>,a),
"Unpack a 32-bit integer (0xRRGGBBAA) into separate RGBA values (0..1).")
{
  ImVec4 color {Color {static_cast<uint32_t>(rgba)}};
  if(r) *r = color.x;
  if(g) *g = color.y;
  if(b) *b = color.z;
  if(a) *a = color.w;
}

API_FUNC(0_7, int, ColorConvertDouble4ToU32,
(double,r) (double,g) (double,b) (double,a),
"Pack 0..1 RGBA values into a 32-bit integer (0xRRGGBBAA).")
{
  return Color {ImVec4(r, g, b, a)}.pack(true);
}

API_FUNC(0_7, void, ColorConvertHSVtoRGB,
(double,h) (double,s) (double,v)
(W<double*>,r) (W<double*>,g) (W<double*>,b),
"Convert HSV values (0..1) into RGB (0..1).")
{
  float rgb[3];
  ImGui::ColorConvertHSVtoRGB(h, s, v, rgb[0], rgb[1], rgb[2]);
  if(r) *r = rgb[0];
  if(g) *g = rgb[1];
  if(b) *b = rgb[2];
}

API_FUNC(0_7, void, ColorConvertRGBtoHSV,
(double,r) (double,g) (double,b)
(W<double*>,h) (W<double*>,s) (W<double*>,v),
"Convert RGB values (0..1) into HSV (0..1).")
{
  float hsv[3];
  ImGui::ColorConvertRGBtoHSV(r, g, b, hsv[0], hsv[1], hsv[2]);
  if(h) *h = hsv[0];
  if(s) *s = hsv[1];
  if(v) *v = hsv[2];
}

API_FUNC(0_3, int, ColorConvertNative,
(int,rgb),
R"(Convert a native color coming from REAPER or 0xRRGGBB to native.
This swaps the red and blue channels on Windows.)")
{
  return Color::convertNative(rgb);
}

API_SUBSECTION("Logging/Capture",
R"(All text output from the interface can be captured into tty/file/clipboard.
By default, tree nodes are automatically opened during logging.)");

API_FUNC(0_1, void, LogToTTY, (Context*,ctx)
(RO<int*>,auto_open_depth,-1),
"Start logging all text output from the interface to the TTY (stdout).")
{
  FRAME_GUARD;
  ImGui::LogToTTY(API_GET(auto_open_depth));
}

API_FUNC(0_1, void, LogToFile, (Context*,ctx)
(RO<int*>,auto_open_depth,-1) (RO<const char*>,filename),
R"(Start logging all text output from the interface to a file.
The data is saved to $resource_path/imgui_log.txt if filename is nil.)")
{
  FRAME_GUARD;
  nullIfEmpty(filename);
  ImGui::LogToFile(API_GET(auto_open_depth), filename);
}

API_FUNC(0_1, void, LogToClipboard, (Context*,ctx)
(RO<int*>,auto_open_depth,-1),
R"(Start logging all text output from the interface to the OS clipboard.
See also SetClipboardText.)")
{
  FRAME_GUARD;
  ImGui::LogToClipboard(API_GET(auto_open_depth));
}

API_FUNC(0_1, void, LogFinish, (Context*,ctx),
"Stop logging (close file, etc.)")
{
  FRAME_GUARD;
  ImGui::LogFinish();
}

API_FUNC(0_1, void, LogText, (Context*,ctx)
(const char*,text),
"Pass text data straight to log (without being displayed)")
{
  FRAME_GUARD;
  ImGui::LogText("%s", text);
}

API_SUBSECTION("Clipboard");

API_FUNC(0_1, const char*, GetClipboardText, (Context*,ctx),
"")
{
  assertValid(ctx);
  ctx->setCurrent();
  return ImGui::GetClipboardText();
}

API_FUNC(0_1, void, SetClipboardText, (Context*,ctx)
(const char*,text),
R"(See also the LogToClipboard function to capture GUI into clipboard,
or easily output text data to the clipboard.)")
{
  assertValid(ctx);
  ctx->setCurrent();
  return ImGui::SetClipboardText(text);
}

API_SUBSECTION("Conditions",
  "Used for SetNextWindow*(), SetWindow*() and SetNextItem*() functions.");
API_ENUM(0_1, ImGui, Cond_Always,
  "No condition (always set the variable).");
API_ENUM(0_1, ImGui, Cond_Once,
  "Set the variable once per runtime session (only the first call will succeed).");
API_ENUM(0_1, ImGui, Cond_FirstUseEver,
R"(Set the variable if the object/window has no persistently saved data
   (no entry in .ini file).)");
API_ENUM(0_1, ImGui, Cond_Appearing,
R"(Set the variable if the object/window is appearing after being
   hidden/inactive (or the first time).)");
