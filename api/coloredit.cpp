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

#include "helper.hpp"

#include "color.hpp"

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

DEFINE_API(bool, ColorEdit4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgba))(int*,API_RO(flags)),
R"(Color is in 0xRRGGBBAA or, if ImGui_ColorEditFlags_NoAlpha is set, 0xXXRRGGBB (XX is ignored and will not be modified).

Tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  FRAME_GUARD;
  assertValid(API_RW(col_rgba));

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), ImGuiColorEditFlags_None) };
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

Tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = ImGui_ColorEditFlags_None)",
{
  // unneeded, only to show Edit3 in the error message instead of Edit4
  FRAME_GUARD;
  assertValid(API_RW(col_rgb));

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), ImGuiColorEditFlags_None) };
  flags |= ImGuiColorEditFlags_NoAlpha;
  return API_ColorEdit4(ctx, label, API_RW(col_rgb), &flags);
});

DEFINE_API(bool, ColorPicker4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgba))(int*,API_RO(flags))(int*,API_RO(ref_col)),
"Default values: flags = ImGui_ColorEditFlags_None, ref_col = nil",
{
  FRAME_GUARD;
  assertValid(API_RW(col_rgba));

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), ImGuiColorEditFlags_None) };
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
  assertValid(API_RW(col_rgb));

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), ImGuiColorEditFlags_None) };
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

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), ImGuiColorEditFlags_None) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  const ImVec4 col { Color(col_rgba, alpha) };
  const ImVec2 size { valueOr(API_RO(size_w), 0.f),
                      valueOr(API_RO(size_h), 0.f) };

  return ImGui::ColorButton(desc_id, col, flags, size);
});

DEFINE_API(void, SetColorEditOptions, (ImGui_Context*,ctx)
(int,flags),
"Picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.",
{
  FRAME_GUARD;
  sanitizeColorEditFlags(flags);
  ImGui::SetColorEditOptions(flags);
});

// ImGuiColorEditFlags
DEFINE_ENUM(ImGui, ColorEditFlags_None,             "Flags for ImGui_ColorEdit3 / ImGui_ColorEdit4 / ImGui_ColorPicker3 / ImGui_ColorPicker4 / ImGui_ColorButton.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoAlpha,          "ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoPicker,         "ColorEdit: disable picker when clicking on color square.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoOptions,        "ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoSmallPreview,   "ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoInputs,         "ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoTooltip,        "ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoLabel,          "ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).");
DEFINE_ENUM(ImGui, ColorEditFlags_NoSidePreview,    "ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoDragDrop,       "ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.");
DEFINE_ENUM(ImGui, ColorEditFlags_NoBorder,         "ColorButton: disable border (which is enforced by default).");
// User Options (right-click on widget to change some of them).
DEFINE_ENUM(ImGui, ColorEditFlags_AlphaBar,         "ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.");
DEFINE_ENUM(ImGui, ColorEditFlags_AlphaPreview,     "ColorEdit, ColorPicker, ColorButton: display preview as a transparent color over a checkerboard, instead of opaque.");
DEFINE_ENUM(ImGui, ColorEditFlags_AlphaPreviewHalf, "ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard, instead of opaque.");
// DEFINE_ENUM(ImGui, ColorEditFlags_HDR,              "(WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use ImGuiColorEditFlags_Float flag as well).");
DEFINE_ENUM(ImGui, ColorEditFlags_DisplayRGB,       "ColorEdit: override _display_ type to RGB. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ImGui, ColorEditFlags_DisplayHSV,       "ColorEdit: override _display_ type to HSV. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ImGui, ColorEditFlags_DisplayHex,       "ColorEdit: override _display_ type to Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.");
DEFINE_ENUM(ImGui, ColorEditFlags_Uint8,            "ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.");
DEFINE_ENUM(ImGui, ColorEditFlags_Float,            "ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.");
DEFINE_ENUM(ImGui, ColorEditFlags_PickerHueBar,     "ColorPicker: bar for Hue, rectangle for Sat/Value.");
DEFINE_ENUM(ImGui, ColorEditFlags_PickerHueWheel,   "ColorPicker: wheel for Hue, triangle for Sat/Value.");
DEFINE_ENUM(ImGui, ColorEditFlags_InputRGB,         "ColorEdit, ColorPicker: input and output data in RGB format.");
DEFINE_ENUM(ImGui, ColorEditFlags_InputHSV,         "ColorEdit, ColorPicker: input and output data in HSV format.");
DEFINE_ENUM(ImGui, ColorEditFlags__OptionsDefault,  "Defaults Options. You can set application defaults using ImGui_SetColorEditOptions. The intent is that you probably don't want to override them in most of your calls. Let the user choose via the option menu and/or call SetColorEditOptions() once during startup.");
