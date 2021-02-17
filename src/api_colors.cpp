#include "api_helper.hpp"

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

// Widgets: Color Editor/Picker
DEFINE_API(bool, ColorEdit, ((ImGui_Context*,ctx))
((const char*,label))((int*,rgbaInOut)) // the ReaScript analyzer doesn't like unsigned int*
((int*,flagsInOptional)),
R"(Color is in 0xRRGGBBAA or, if ImGui_ColorEditFlags_NoAlpha is set, 0xXXRRGGBB (XX is ignored and will not be modified).

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = 0)",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiColorEditFlags flags { valueOr(flagsInOptional, 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  float col[4];
  Color(*rgbaInOut, alpha).unpack(col);
  const bool ret { ImGui::ColorEdit4(label, col, flags) };

  // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
  *rgbaInOut = Color{col}.pack(alpha, *rgbaInOut);

  return ret;
});

DEFINE_API(bool, ColorPicker, ((ImGui_Context*,ctx))
((const char*,label))((int*,rgbaInOut))
((int*,flagsInOptional))((int*,refColInOptional)),
"Default values: flags = ImGui_ColorEditFlags_None, refCol = nil",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiColorEditFlags flags { valueOr(flagsInOptional, 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };

  float col[4], refCol[4];
  Color(*rgbaInOut, alpha).unpack(col);
  if(refColInOptional)
    Color(*refColInOptional, alpha).unpack(col);

  const bool ret {
    ImGui::ColorPicker4(label, col, flags, refColInOptional ? refCol : nullptr)
  };

  // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
  *rgbaInOut = Color{col}.pack(alpha, *rgbaInOut);

  return ret;
});

DEFINE_API(bool, ColorButton, ((ImGui_Context*,ctx))
((const char*,desc_id))((int*,rgbaInOut))
((int*,flagsInOptional))((double*,widthInOptional))((double*,heightInOptional)),
R"(Display a color square/button, hover for details, return true when pressed.

Default values: flags = ImGui_ColorEditFlags_None, width = 0.0, height = 0.0)",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiColorEditFlags flags { valueOr(flagsInOptional, 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  const ImVec4 col { Color(*rgbaInOut, alpha) };

  return ImGui::ColorButton(desc_id, col, flags,
    ImVec2(valueOr(widthInOptional, 0.0), valueOr(heightInOptional, 0.0)));
});
// void          SetColorEditOptions(ImGuiColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.
