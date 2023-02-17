/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

#include "color.hpp"
#include "drawlist.hpp"
#include "font.hpp"
#include "image.hpp"
#include "listclipper.hpp"
#include "platform.hpp"
#include "resource_proxy.hpp"
#include "textfilter.hpp"
#include "version.hpp"

#include <climits>

API_SECTION("Utility");

DEFINE_API(void, GetVersion,
(char*,API_W(imgui_version))(int,API_W_SZ(imgui_version))
(int*,API_W(imgui_version_num))
(char*,API_W(reaimgui_version))(int,API_W_SZ(reaimgui_version)),
"",
{
  if(API_W(imgui_version))
    snprintf(API_W(imgui_version), API_W_SZ(imgui_version), "%s", IMGUI_VERSION);
  if(API_W(imgui_version_num))
    *API_W(imgui_version_num) = IMGUI_VERSION_NUM;
  if(API_W(reaimgui_version))
    snprintf(API_W(reaimgui_version), API_W_SZ(reaimgui_version), "%s", REAIMGUI_VERSION);
});

#define RESOURCE_ISVALID(klass)          \
  if(!strcmp(type, "ImGui_" #klass "*")) \
    return Resource::isValid(static_cast<klass *>(pointer))
#define RESOURCEPROXY_ISVALID(klass)     \
  if(!strcmp(type, "ImGui_" #klass "*")) \
    return klass.decode<Context>(pointer, &proxyKey);

DEFINE_API(bool, ValidatePtr, (void*,pointer)(const char*,type),
R"(Return whether the pointer of the specified type is valid.

Supported types are:

- ImGui_Context*
- ImGui_DrawList*
- ImGui_DrawListSplitter*
- ImGui_Font*
- ImGui_Image*
- ImGui_ImageSet*
- ImGui_ListClipper*
- ImGui_TextFilter*
- ImGui_Viewport*)",
{

  RESOURCE_ISVALID(Context);
  RESOURCE_ISVALID(DrawListSplitter);
  RESOURCE_ISVALID(Font);
  RESOURCE_ISVALID(Image);
  RESOURCE_ISVALID(ImageSet);
  RESOURCE_ISVALID(ListClipper);
  RESOURCE_ISVALID(TextFilter);

  ResourceProxy::Key proxyKey;
  RESOURCEPROXY_ISVALID(DrawList);
  RESOURCEPROXY_ISVALID(Viewport);

  return false;
});

#undef RESOURCE_ISVALID
#undef RESOURCEPROXY_ISVALID

DEFINE_API(void, ProgressBar, (ImGui_Context*,ctx)
(double,fraction)
(double*,API_RO(size_arg_w),-FLT_MIN)(double*,API_RO(size_arg_h),0.0)
(const char*,API_RO(overlay)),
"",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(overlay));
  const ImVec2 size(API_RO_GET(size_arg_w), API_RO_GET(size_arg_h));
  ImGui::ProgressBar(fraction, size, API_RO(overlay));
});

DEFINE_API(void, PointConvertNative, (ImGui_Context*,ctx)
(double*,API_RW(x))(double*,API_RW(y))(bool*,API_RO(to_native),false),
R"(Convert a position from the current platform's native coordinate position
system to ReaImGui global coordinates (or vice versa).

This effectively flips the Y coordinate on macOS and applies HiDPI scaling on
Windows and Linux.)",
{
  FRAME_GUARD; // scalePosition uses the active context and its monitor list
  assertValid(API_RW(x));
  assertValid(API_RW(y));

  ImVec2 point;
  point.x = *API_RW(x);
  point.y = *API_RW(y);
  Platform::scalePosition(&point, API_RO_GET(to_native));
  *API_RW(x) = point.x;
  *API_RW(y) = point.y;
});

#define NUMERIC_LIMITS(name, type, minVal, maxVal)                             \
  DEFINE_API(void, NumericLimits_##name, (type*,API_W(min))(type*,API_W(max)), \
  "Returns " #minVal " and " #maxVal " for this system.",                      \
  {                                                                            \
    assertValid(API_W(min));                                                   \
    assertValid(API_W(max));                                                   \
    *API_W(min) = minVal;                                                      \
    *API_W(max) = maxVal;                                                      \
  })

NUMERIC_LIMITS(Double, double, DBL_MIN, DBL_MAX);
NUMERIC_LIMITS(Float,  double, FLT_MIN, FLT_MAX);
NUMERIC_LIMITS(Int,    int,    INT_MIN, INT_MAX);

API_SUBSECTION("ID stack/scope",
R"(Read the [FAQ](https://dearimgui.org/faq) for more details about how IDs are
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

DEFINE_API(void, PushID, (ImGui_Context*,ctx)
(const char*,str_id),
"Push string into the ID stack.",
{
  FRAME_GUARD;
  ImGui::PushID(str_id);
});

DEFINE_API(void, PopID, (ImGui_Context*,ctx),
"Pop from the ID stack.",
{
  FRAME_GUARD;
  ImGui::PopID();
});

API_SUBSECTION("Color Conversion");

DEFINE_API(void, ColorConvertU32ToDouble4,
(int,rgba)
(double*,API_W(r))(double*,API_W(g))(double*,API_W(b))(double*,API_W(a)),
"Unpack a 32-bit integer (0xRRGGBBAA) into separate RGBA values (0..1).",
{
  ImVec4 color { Color { static_cast<uint32_t>(rgba) } };
  if(API_W(r)) *API_W(r) = color.x;
  if(API_W(g)) *API_W(g) = color.y;
  if(API_W(b)) *API_W(b) = color.z;
  if(API_W(a)) *API_W(a) = color.w;
});

DEFINE_API(int, ColorConvertDouble4ToU32,
(double,r)(double,g)(double,b)(double,a),
"Pack 0..1 RGBA values into a 32-bit integer (0xRRGGBBAA).",
{
  return Color { ImVec4(r, g, b, a) }.pack(true);
});

DEFINE_API(void, ColorConvertHSVtoRGB,
(double,h)(double,s)(double,v)
(double*,API_W(r))(double*,API_W(g))(double*,API_W(b)),
"Convert HSV values (0..1) into RGB (0..1).",
{
  float rgb[3];
  ImGui::ColorConvertHSVtoRGB(h, s, v, rgb[0], rgb[1], rgb[2]);
  if(API_W(r)) *API_W(r) = rgb[0];
  if(API_W(g)) *API_W(g) = rgb[1];
  if(API_W(b)) *API_W(b) = rgb[2];
});

DEFINE_API(void, ColorConvertRGBtoHSV,
(double,r)(double,g)(double,b)
(double*,API_W(h))(double*,API_W(s))(double*,API_W(v)),
"Convert RGB values (0..1) into HSV (0..1).",
{
  float hsv[3];
  ImGui::ColorConvertRGBtoHSV(r, g, b, hsv[0], hsv[1], hsv[2]);
  if(API_W(h)) *API_W(h) = hsv[0];
  if(API_W(s)) *API_W(s) = hsv[1];
  if(API_W(v)) *API_W(v) = hsv[2];
});

DEFINE_API(int, ColorConvertNative,
(int,rgb),
R"(Convert a native color coming from REAPER or 0xRRGGBB to native.
This swaps the red and blue channels on Windows.)",
{
  return Color::convertNative(rgb);
});

API_SUBSECTION("Logging/Capture",
R"(All text output from the interface can be captured into tty/file/clipboard.
By default, tree nodes are automatically opened during logging.)");

DEFINE_API(void, LogToTTY, (ImGui_Context*,ctx)
(int*,API_RO(auto_open_depth),-1),
"Start logging all text output from the interface to the TTY (stdout).",
{
  FRAME_GUARD;
  ImGui::LogToTTY(API_RO_GET(auto_open_depth));
});

DEFINE_API(void, LogToFile, (ImGui_Context*,ctx)
(int*,API_RO(auto_open_depth),-1)(const char*,API_RO(filename)),
R"(Start logging all text output from the interface to a file.
The data is saved to $resource_path/imgui_log.txt if filename is nil.)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(filename));
  ImGui::LogToFile(API_RO_GET(auto_open_depth), API_RO(filename));
});

DEFINE_API(void, LogToClipboard, (ImGui_Context*,ctx)
(int*,API_RO(auto_open_depth),-1),
R"(Start logging all text output from the interface to the OS clipboard.
See also SetClipboardText.)",
{
  FRAME_GUARD;
  ImGui::LogToClipboard(API_RO_GET(auto_open_depth));
});

DEFINE_API(void, LogFinish, (ImGui_Context*,ctx),
"Stop logging (close file, etc.)",
{
  FRAME_GUARD;
  ImGui::LogFinish();
});

DEFINE_API(void, LogText, (ImGui_Context*,ctx)
(const char*,text),
"Pass text data straight to log (without being displayed)",
{
  FRAME_GUARD;
  ImGui::LogText("%s", text);
});

API_SUBSECTION("Clipboard");

DEFINE_API(const char*, GetClipboardText, (ImGui_Context*,ctx),
"",
{
  assertValid(ctx);
  ctx->setCurrent();
  return ImGui::GetClipboardText();
});

DEFINE_API(void, SetClipboardText, (ImGui_Context*,ctx)
(const char*,text),
R"(See also the LogToClipboard function to capture GUI into clipboard,
or easily output text data to the clipboard.)",
{
  assertValid(ctx);
  ctx->setCurrent();
  return ImGui::SetClipboardText(text);
});

API_SUBSECTION("Conditions", "Used for many Set*() functions.");
DEFINE_ENUM(ImGui, Cond_Always,
  "No condition (always set the variable).");
DEFINE_ENUM(ImGui, Cond_Once,
  "Set the variable once per runtime session (only the first call will succeed).");
DEFINE_ENUM(ImGui, Cond_FirstUseEver,
R"(Set the variable if the object/window has no persistently saved data
   (no entry in .ini file).)");
DEFINE_ENUM(ImGui, Cond_Appearing,
R"(Set the variable if the object/window is appearing after being
   hidden/inactive (or the first time).)");
