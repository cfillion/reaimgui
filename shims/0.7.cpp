#include "shims.hpp"

SHIM("0.7",
  (int, ModFlags_None)
  (int, ModFlags_Ctrl)
  (int, ModFlags_Shift)
  (int, ModFlags_Alt)
  (int, ModFlags_Super)

  (void, SetNextFrameWantCaptureKeyboard, ImGui_Context*)

  (void, ColorConvertHSVtoRGB, double, double, double, double*, double*, double*)
  (void, ColorConvertRGBtoHSV, double, double, double, double*, double*, double*)
  (int,  ColorConvertDouble4ToU32, double, double, double, double)

  (double, GetConfigVar, ImGui_Context*, int)
  (void,   SetConfigVar, ImGui_Context*, int, int)
  (int,    ConfigVar_Flags)

  (bool, Combo,   ImGui_Context*, const char*, int*, const char*, int, int*)
  (bool, ListBox, ImGui_Context*, const char*, int*, const char*, int, int*)

  (void, GetVersion, char*, int, int*, char*, int)
);

// KeyModFlags to ModFlags rename
SHIM_ALIAS(0_1, KeyModFlags_None,  ModFlags_None)
SHIM_ALIAS(0_1, KeyModFlags_Ctrl,  ModFlags_Ctrl)
SHIM_ALIAS(0_1, KeyModFlags_Shift, ModFlags_Shift)
SHIM_ALIAS(0_1, KeyModFlags_Alt,   ModFlags_Alt)
SHIM_ALIAS(0_1, KeyModFlags_Super, ModFlags_Super)

// Capture*FromApp to SetNextFrameWantCapture* rename
SHIM_ALIAS(0_1, CaptureKeyboardFromApp, SetNextFrameWantCaptureKeyboard)

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
  (double,h)(double,s)(double,v)(double*,API_RO(a))
  (double*,API_W(r))(double*,API_W(g))(double*,API_W(b)))
{
  return shimColorConv(api.ColorConvertHSVtoRGB,
    h, s, v, API_RO(a), API_W(r), API_W(g), API_W(b));
}

SHIM_FUNC(0_1, int, ColorConvertRGBtoHSV,
  (double,r)(double,g)(double,b)(double*,API_RO(a))
  (double*,API_W(h))(double*,API_W(s))(double*,API_W(v)))
{
  return shimColorConv(api.ColorConvertRGBtoHSV,
    r, g, b, API_RO(a), API_W(h), API_W(s), API_W(v));
}

// ConfigVar API
SHIM_FUNC(0_1, int, GetConfigFlags, (ImGui_Context*,ctx))
{
  return api.GetConfigVar(ctx, api.ConfigVar_Flags());
}

SHIM_FUNC(0_1, void, SetConfigFlags, (ImGui_Context*,ctx)(int,flags))
{
  api.SetConfigVar(ctx, api.ConfigVar_Flags(), flags);
}

// null-terminated combo and list box items
static int convertItemSeparator(char *items)
{
  const size_t size { strlen(items) };
  for(char *c { items + size - 1 }; c >= items; --c) {
    if(*c == '\x1f') // ASCII Unit Separator
      *c = '\0';
  }
  return size + 1;
}

SHIM_FUNC(0_1, bool, Combo, (ImGui_Context*,ctx)
  (const char*,label)(int*,API_RW(current_item))(char*,items)
  (int*,API_RO(popup_max_height_in_items)))
{
  const int size { convertItemSeparator(items) };
  return api.Combo(ctx, label, API_RW(current_item),
    items, size, API_RO(popup_max_height_in_items));
}

SHIM_FUNC(0_1, bool, ListBox, (ImGui_Context*,ctx)(const char*,label)
  (int*,API_RW(current_item))(char*,items)(int*,API_RO(height_in_items)))
{
  const int size { convertItemSeparator(items) };
  return api.ListBox(ctx, label, API_RW(current_item),
      items, size, API_RO(height_in_items));
}

// Addition of IMGUI_VERSION_NUM to GetVersion
SHIM_FUNC(0_1, void, GetVersion,
  (char*,API_W(imgui_version))(int,API_W_SZ(imgui_version))
  (char*,API_W(reaimgui_version))(int,API_W_SZ(reaimgui_version)))
{
  api.GetVersion(API_W(imgui_version), API_W_SZ(imgui_version), nullptr,
    API_W(reaimgui_version), API_W_SZ(reaimgui_version));
}
