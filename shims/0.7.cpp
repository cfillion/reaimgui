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

#include "shims.hpp"

SHIM("0.7",
  (int, ModFlags_None)
  (int, ModFlags_Ctrl)
  (int, ModFlags_Shift)
  (int, ModFlags_Alt)
  (int, ModFlags_Super)

  (void, SetNextFrameWantCaptureKeyboard, Context*, bool)

  (void, ColorConvertHSVtoRGB, double, double, double, W<double*>, W<double*>, W<double*>)
  (void, ColorConvertRGBtoHSV, double, double, double, W<double*>, W<double*>, W<double*>)
  (int,  ColorConvertDouble4ToU32, double, double, double, double)

  (double, GetConfigVar, Context*, int)
  (void,   SetConfigVar, Context*, int, int)
  (int,    ConfigVar_Flags)

  (bool, Combo,   Context*, const char*, RW<int*>, const char*, WS<int>, RO<int*>)
  (bool, ListBox, Context*, const char*, RW<int*>, const char*, WS<int>, RO<int*>)

  (void, GetVersion, W<char*>, WS<int>, W<int*>, W<char*>, WS<int>)
);

// KeyModFlags to ModFlags rename
SHIM_ALIAS(0_1, KeyModFlags_None,  ModFlags_None)
SHIM_ALIAS(0_1, KeyModFlags_Ctrl,  ModFlags_Ctrl)
SHIM_ALIAS(0_1, KeyModFlags_Shift, ModFlags_Shift)
SHIM_ALIAS(0_1, KeyModFlags_Alt,   ModFlags_Alt)
SHIM_ALIAS(0_1, KeyModFlags_Super, ModFlags_Super)

// Capture*FromApp to SetNextFrameWantCapture* rename
SHIM_FUNC(0_1, void, CaptureKeyboardFromApp,
  (Context*,ctx) (RO<bool*>,want_capture_keyboard,true))
{
  api.SetNextFrameWantCaptureKeyboard(ctx, API_GET(want_capture_keyboard));
}

// non-vanilla HSVtoRGB/RGBtoHSV packing and optional alpha parameter
static int shimColorConv(decltype(api.ColorConvertHSVtoRGB) convFunc,
  double x, double y, double z, double *a,
  double *out1, double *out2, double *out3)
{
  convFunc(x, y, z, &x, &y, &z);
  if(out1) *out1 = x;
  if(out2) *out2 = y;
  if(out3) *out3 = z;
  return api.ColorConvertDouble4ToU32(x, y, z, a ? *a : 1.0) >> (a ? 0 : 8);
}

SHIM_FUNC(0_1, int, ColorConvertHSVtoRGB,
  (double,h) (double,s) (double,v) (RO<double*>,a)
  (W<double*>,r) (W<double*>,g) (W<double*>,b))
{
  return shimColorConv(api.ColorConvertHSVtoRGB, h, s, v, a, r, g, b);
}

SHIM_FUNC(0_1, int, ColorConvertRGBtoHSV,
  (double,r) (double,g) (double,b) (RO<double*>,a)
  (W<double*>,h) (W<double*>,s) (W<double*>,v))
{
  return shimColorConv(api.ColorConvertRGBtoHSV, r, g, b, a, h, s, v);
}

// ConfigVar API
SHIM_FUNC(0_1, int, GetConfigFlags, (Context*,ctx))
{
  return api.GetConfigVar(ctx, api.ConfigVar_Flags());
}

SHIM_FUNC(0_1, void, SetConfigFlags, (Context*,ctx) (int,flags))
{
  api.SetConfigVar(ctx, api.ConfigVar_Flags(), flags);
}

// null-terminated combo and list box items
static int convertItemSeparator(char *items)
{
  const size_t size {strlen(items)};
  for(char *c {items + size - 1}; c >= items; --c) {
    if(*c == '\x1f') // ASCII Unit Separator
      *c = '\0';
  }
  return size + 1;
}

SHIM_FUNC(0_1, bool, Combo, (Context*,ctx)
  (const char*,label) (RW<int*>,current_item) (char*,items)
  (RO<int*>,popup_max_height_in_items))
{
  const int size {convertItemSeparator(items)};
  return api.Combo(ctx, label, current_item, items, size, popup_max_height_in_items);
}

SHIM_FUNC(0_1, bool, ListBox, (Context*,ctx) (const char*,label)
  (RW<int*>,current_item) (char*,items) (RO<int*>,height_in_items))
{
  const int size {convertItemSeparator(items)};
  return api.ListBox(ctx, label, current_item, items, size, height_in_items);
}

// Addition of IMGUI_VERSION_NUM to GetVersion
SHIM_FUNC(0_1, void, GetVersion,
  (W<char*>,imgui_version) (WS<int>,imgui_version_sz)
  (W<char*>,reaimgui_version) (WS<int>,reaimgui_version_sz))
{
  api.GetVersion(imgui_version, imgui_version_sz, nullptr,
    reaimgui_version, reaimgui_version_sz);
}
