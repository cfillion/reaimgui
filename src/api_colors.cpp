#include "api_helper.hpp"

#include "colors.hpp"

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

// Widgets: Color Editor/Picker
DEFINE_API(bool, ColorEdit, ((Window*,window))
((const char*,label))((int*,colorInOut)) // the ReaScript analyzer doesn't like unsigned int*
((int*,flagsInOptional)),
"tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.",
{
  USE_WINDOW(window, false);

  ImGuiColorEditFlags flags { valueOr(flagsInOptional, 0) };
  sanitizeColorEditFlags(flags);

  float col[4];
  float *a { flags & ImGuiColorEditFlags_NoAlpha ? nullptr : &col[3] };
  Color::unpack(*colorInOut, col[0], col[1], col[2], a);

  const bool ret { ImGui::ColorEdit4(label, col, flags) };

  // preserve unused bits from the input integer as-is (eg. REAPER's enable flag)
  const unsigned int newColor { Color::pack(col[0], col[1], col[2], a) };
  const unsigned int mask { a ? 0xFFFFFFFF : 0xFFFFFF };
  *colorInOut = (*colorInOut & ~mask) | newColor;

  return ret;
});
// bool          ColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
// bool          ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
// bool          ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.
// void          SetColorEditOptions(ImGuiColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.
