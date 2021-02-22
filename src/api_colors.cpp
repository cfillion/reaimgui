#include "api_helper.hpp"

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

// Widgets: Color Editor/Picker
DEFINE_API(bool, ColorEdit, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(rgba))) // the ReaScript analyzer doesn't like unsigned int*
((int*,API_RO(flags))),
R"(Color is in 0xRRGGBBAA or, if ImGui_ColorEditFlags_NoAlpha is set, 0xXXRRGGBB (XX is ignored and will not be modified).

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

Default values: flags = 0)",
{
  ensureContext(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  float col[4];
  Color(*API_RW(rgba), alpha).unpack(col);
  const bool ret { ImGui::ColorEdit4(label, col, flags) };

  // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
  *API_RW(rgba) = Color{col}.pack(alpha, *API_RW(rgba));

  return ret;
});

DEFINE_API(bool, ColorPicker, ((ImGui_Context*,ctx))
((const char*,label))((int*,API_RW(rgba)))
((int*,API_RO(flags)))((int*,API_RO(refCol))),
"Default values: flags = ImGui_ColorEditFlags_None, refCol = nil",
{
  ensureContext(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };

  float col[4], refCol[4];
  Color(*API_RW(rgba), alpha).unpack(col);
  if(API_RO(refCol))
    Color(*API_RO(refCol), alpha).unpack(refCol);

  const bool ret {
    ImGui::ColorPicker4(label, col, flags, API_RO(refCol) ? refCol : nullptr)
  };

  // preserves unused bits from the input integer as-is (eg. REAPER's enable flag)
  *API_RW(rgba) = Color{col}.pack(alpha, *API_RW(rgba));

  return ret;
});

DEFINE_API(bool, ColorButton, ((ImGui_Context*,ctx))
((const char*,desc_id))((int*,API_RW(rgba)))
((int*,API_RO(flags)))((double*,API_RO(width)))((double*,API_RO(height))),
R"(Display a color square/button, hover for details, return true when pressed.

Default values: flags = ImGui_ColorEditFlags_None, width = 0.0, height = 0.0)",
{
  ensureContext(ctx)->enterFrame();

  ImGuiColorEditFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeColorEditFlags(flags);

  const bool alpha { (flags & ImGuiColorEditFlags_NoAlpha) == 0 };
  const ImVec4 col { Color(*API_RW(rgba), alpha) };

  return ImGui::ColorButton(desc_id, col, flags,
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)));
});

DEFINE_API(void, SetColorEditOptions, ((ImGui_Context*,ctx))
((int,flags)),
"Picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.",
{
  ensureContext(ctx)->enterFrame();
  sanitizeColorEditFlags(flags);
  ImGui::SetColorEditOptions(flags);
});
