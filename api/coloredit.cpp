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

#include "../src/color.hpp"

API_SECTION("Color Edit",
R"(Tip: the ColorEdit* functions have a little color square that can be
left-clicked to open a picker, and right-clicked to open an option menu.)");

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

API_FUNC(0_1, bool, ColorEdit4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgba))
(int*,API_RO(flags),ImGuiColorEditFlags_None),
R"(Color is in 0xRRGGBBAA or, if ColorEditFlags_NoAlpha is set, 0xXXRRGGBB
(XX is ignored and will not be modified).)")
{
  FRAME_GUARD;
  assertValid(API_RW(col_rgba));

  ImGuiColorEditFlags flags { API_RO_GET(flags) };
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
}

API_FUNC(0_1, bool, ColorEdit3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgb))
(int*,API_RO(flags),ImGuiColorEditFlags_None),
"Color is in 0xXXRRGGBB. XX is ignored and will not be modified.")
{
  // Edit4 will take care of starting the frame and validating col_rgb
  ImGuiColorEditFlags flags { API_RO_GET(flags) };
  flags |= ImGuiColorEditFlags_NoAlpha;
  return ColorEdit4::impl(ctx, label, API_RW(col_rgb), &flags);
}

API_FUNC(0_1, bool, ColorPicker4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgba))
(int*,API_RO(flags),ImGuiColorEditFlags_None)(int*,API_RO(ref_col)),
"")
{
  FRAME_GUARD;
  assertValid(API_RW(col_rgba));

  ImGuiColorEditFlags flags { API_RO_GET(flags) };
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
}

API_FUNC(0_1, bool, ColorPicker3, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgb))
(int*,API_RO(flags),ImGuiColorEditFlags_None),
R"(Color is in 0xXXRRGGBB. XX is ignored and will not be modified.)")
{
  // Picker4 will take care of starting the frame and validating col_rgb
  ImGuiColorEditFlags flags { API_RO_GET(flags) };
  flags |= ImGuiColorEditFlags_NoAlpha;
  return ColorPicker4::impl(ctx, label, API_RW(col_rgb), &flags, nullptr);
}

API_FUNC(0_1, bool, ColorButton, (ImGui_Context*,ctx)
(const char*,desc_id)(int,col_rgba)(int*,API_RO(flags),ImGuiColorEditFlags_None)
(double*,API_RO(size_w),0.0)(double*,API_RO(size_h),0.0),
R"(Display a color square/button, hover for details, return true when pressed.
Color is in 0xRRGGBBAA or, if ColorEditFlags_NoAlpha is set, 0xRRGGBB.)")
{
  FRAME_GUARD;

  ImGuiColorEditFlags flags { API_RO_GET(flags) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  const Color col(col_rgba, alpha);
  const ImVec2 size(API_RO_GET(size_w), API_RO_GET(size_h));

  return ImGui::ColorButton(desc_id, col, flags, size);
}

API_FUNC(0_1, void, SetColorEditOptions, (ImGui_Context*,ctx)
(int,flags),
R"(Picker type, etc. User will be able to change many settings, unless you pass
the _NoOptions flag to your calls.)")
{
  FRAME_GUARD;
  sanitizeColorEditFlags(flags);
  ImGui::SetColorEditOptions(flags);
}

API_SECTION_DEF(colorFlags, ROOT_SECTION, "Flags");
API_ENUM(0_1, ImGui, ColorEditFlags_None, "");
API_ENUM(0_1, ImGui, ColorEditFlags_NoAlpha,
R"(ColorEdit, ColorPicker, ColorButton: ignore Alpha component
  (will only read 3 components from the input pointer).)");
API_ENUM(0_1, ImGui, ColorEditFlags_NoPicker,
  "ColorEdit: disable picker when clicking on color square.");
API_ENUM(0_1, ImGui, ColorEditFlags_NoOptions,
  "ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.");
API_ENUM(0_1, ImGui, ColorEditFlags_NoSmallPreview,
R"(ColorEdit, ColorPicker: disable color square preview next to the inputs.
   (e.g. to show only the inputs).)");
API_ENUM(0_1, ImGui, ColorEditFlags_NoInputs,
R"(ColorEdit, ColorPicker: disable inputs sliders/text widgets
   (e.g. to show only the small preview color square).)");
API_ENUM(0_1, ImGui, ColorEditFlags_NoTooltip,
  "ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.");
API_ENUM(0_1, ImGui, ColorEditFlags_NoLabel,
R"(ColorEdit, ColorPicker: disable display of inline text label
   (the label is still forwarded to the tooltip and picker).)");
API_ENUM(0_1, ImGui, ColorEditFlags_NoSidePreview,
R"(ColorPicker: disable bigger color preview on right side of the picker,
   use small color square preview instead.)");
API_ENUM(0_1, ImGui, ColorEditFlags_NoDragDrop,
  "ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.");
API_ENUM(0_1, ImGui, ColorEditFlags_NoBorder,
  "ColorButton: disable border (which is enforced by default).");
API_SECTION_P(colorFlags, "User Options", "(right-click on widget to change some of them)");
API_ENUM(0_1, ImGui, ColorEditFlags_AlphaBar,
  "ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.");
API_ENUM(0_1, ImGui, ColorEditFlags_AlphaPreview,
R"(ColorEdit, ColorPicker, ColorButton: display preview as a transparent color
   over a checkerboard, instead of opaque.)");
API_ENUM(0_1, ImGui, ColorEditFlags_AlphaPreviewHalf,
R"(ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard,
   instead of opaque.)");
// API_ENUM(ImGui, ColorEditFlags_HDR,
// R"((WIP) ColorEdit: Currently only disable 0.0..1.0 limits in RGBA edition
//    (note: you probably want to use ImGuiColorEditFlags_Float flag as well).)");
API_ENUM(0_1, ImGui, ColorEditFlags_DisplayRGB,
R"(ColorEdit: override _display_ type to RGB. ColorPicker:
   select any combination using one or more of RGB/HSV/Hex.)");
API_ENUM(0_1, ImGui, ColorEditFlags_DisplayHSV,
R"(ColorEdit: override _display_ type to HSV. ColorPicker:
   select any combination using one or more of RGB/HSV/Hex.)");
API_ENUM(0_1, ImGui, ColorEditFlags_DisplayHex,
R"(ColorEdit: override _display_ type to Hex. ColorPicker:
   select any combination using one or more of RGB/HSV/Hex.)");
API_ENUM(0_1, ImGui, ColorEditFlags_Uint8,
  "ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.");
API_ENUM(0_1, ImGui, ColorEditFlags_Float,
R"(ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0..1.0
   floats instead of 0..255 integers. No round-trip of value via integers.)");
API_ENUM(0_1, ImGui, ColorEditFlags_PickerHueBar,
  "ColorPicker: bar for Hue, rectangle for Sat/Value.");
API_ENUM(0_1, ImGui, ColorEditFlags_PickerHueWheel,
  "ColorPicker: wheel for Hue, triangle for Sat/Value.");
API_ENUM(0_1, ImGui, ColorEditFlags_InputRGB,
  "ColorEdit, ColorPicker: input and output data in RGB format.");
API_ENUM(0_1, ImGui, ColorEditFlags_InputHSV,
  "ColorEdit, ColorPicker: input and output data in HSV format.");
