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

#include "api_helper.hpp"

#include "color.hpp"

#include <imgui/imgui_internal.h> // internal ImGuiInputTextFlags
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <reaper_plugin_functions.h> // realloc_cmd_ptr
#include <reaper_plugin_secrets.h>   // reaper_array
#include <vector>

static void copyToBuffer(const std::string &value, char *buf, const size_t bufSize)
{
  int newSize {};
  if(value.size() >= bufSize && realloc_cmd_ptr(&buf, &newSize, value.size())) {
    // the buffer is no longer null-terminated after using realloc_cmd_ptr!
    std::memcpy(buf, value.c_str(), newSize);
  }
  else {
    const size_t limit { std::min(bufSize - 1, value.size()) };
    std::memcpy(buf, value.c_str(), limit);
    buf[limit] = '\0';
  }
}

class InputTextFlags {
public:
  InputTextFlags(int *flags)
    : m_flags { valueOr(flags, ImGuiInputTextFlags_None) }
  {
    m_flags &= ~(
      // don't expose these to users
      ImGuiInputTextFlags_CallbackCompletion |
      ImGuiInputTextFlags_CallbackHistory    |
      ImGuiInputTextFlags_CallbackAlways     |
      ImGuiInputTextFlags_CallbackCharFilter |
      ImGuiInputTextFlags_CallbackEdit       |
      ImGuiInputTextFlags_CallbackResize     |

      // reserved for ImGui's internal use
      ImGuiInputTextFlags_Multiline | ImGuiInputTextFlags_NoMarkEdited
    );
  }

  operator ImGuiInputTextFlags() const { return m_flags; }

private:
  ImGuiInputTextFlags m_flags;
};

DEFINE_API(bool, InputText, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const InputTextFlags flags { API_RO(flags) };

  if(ImGui::InputText(label, &value, flags, nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

DEFINE_API(bool, InputTextMultiline, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(double*,API_RO(width))(double*,API_RO(height))
(int*,API_RO(flags)),
"Default values: width = 0.0, height = 0.0, flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const InputTextFlags flags { API_RO(flags) };

  if(ImGui::InputTextMultiline(label, &value,
      ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)),
      flags, nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

DEFINE_API(bool, InputTextWithHint, (ImGui_Context*,ctx)
(const char*,label)(const char*,hint)
(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const InputTextFlags flags { API_RO(flags) };

  if(ImGui::InputTextWithHint(label, hint, &value, flags, nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

DEFINE_API(bool, InputInt, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v))(int*,API_RO(step))(int*,API_RO(step_fast))
(int*,API_RO(flags)),
"Default values: step = 1, step_fast = 100, flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  const InputTextFlags flags { API_RO(flags) };
  return ImGui::InputInt(label, API_RW(v),
    valueOr(API_RO(step), 1), valueOr(API_RO(step_fast), 100), flags);
});

DEFINE_API(bool, InputInt2, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };
  const InputTextFlags flags { API_RO(flags) };

  if(ImGui::InputInt2(label, values.data(), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputInt3, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RW(v3))
(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  const InputTextFlags flags { API_RO(flags) };

  if(ImGui::InputInt3(label, values.data(), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputInt4, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RW(v3))
(int*,API_RW(v4))(int*,API_RO(flags)),
"Default values: flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  const InputTextFlags flags { API_RO(flags) };

  if(ImGui::InputInt4(label, values.data(), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDouble, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v))(double*,API_RO(step))(double*,API_RO(step_fast))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: step = 0.0, step_fast = 0.0, format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  const InputTextFlags flags { API_RO(flags) };

  return ImGui::InputDouble(label, API_RW(v),
    valueOr(API_RO(step), 0.0), valueOr(API_RO(step_fast), 0.0),
    API_RO(format) ? API_RO(format) : "%.3f", flags);
});

static bool inputDoubleN(const char *label, double *data, const size_t size,
  const char *format, const ImGuiInputTextFlags flags)
{
  return ImGui::InputScalarN(label, ImGuiDataType_Double, data, size,
    nullptr, nullptr, format ? format : "%.3f", flags);
}

DEFINE_API(bool, InputDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };
  const InputTextFlags flags { API_RO(flags) };

  if(inputDoubleN(label, values.data(), values.size(), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  const InputTextFlags flags { API_RO(flags) };

  if(inputDoubleN(label, values.data(), values.size(), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(double*,API_RW(v4))(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  const InputTextFlags flags { API_RO(flags) };

  if(inputDoubleN(label, values.data(), values.size(), API_RO(format), flags))
    return values.commit();
  else
    return false;
});

DEFINE_API(bool, InputDoubleN, (ImGui_Context*,ctx)(const char*,label)
(reaper_array*,values)(double*,API_RO(step))(double*,API_RO(step_fast))
(const char*,API_RO(format))(int*,API_RO(flags)),
"Default values: step = nil, format = nil, step_fast = nil, format = '%.3f', flags = ImGui_InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  const InputTextFlags flags { API_RO(flags) };

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size, API_RO(step), API_RO(step_fast),
    API_RO(format), flags);
});

static void sanitizeColorEditFlags(ImGuiColorEditFlags &flags)
{
  flags &= ~ImGuiColorEditFlags_HDR; // enforce 0.0..1.0 limits
}

DEFINE_API(bool, ColorEdit4, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(col_rgba))(int*,API_RO(flags)),
R"(Color is in 0xRRGGBBAA or, if ImGui_ColorEditFlags_NoAlpha is set, 0xXXRRGGBB (XX is ignored and will not be modified).

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

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

tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.

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

  return ImGui::ColorButton(desc_id, col, flags,
    ImVec2(valueOr(API_RO(size_w), 0.0), valueOr(API_RO(size_h), 0.0)));
});

DEFINE_API(void, SetColorEditOptions, (ImGui_Context*,ctx)
(int,flags),
"Picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.",
{
  FRAME_GUARD;
  sanitizeColorEditFlags(flags);
  ImGui::SetColorEditOptions(flags);
});

// Allowing ReaScripts to input a null-separated string would be unsafe.
// REAPER's buf, buf_sz mechanism does not handle strings containing null
// bytes, so the user would have to specify the size manually. This would
// enable reading from arbitrary memory locations.
static std::vector<const char *> splitString(char *list)
{
  constexpr char ITEM_SEP { '\x1f' }; // ASCII Unit Separator
  const auto len { strlen(list) };

  if(len < 1 || list[len - 1] != ITEM_SEP || list[len] != '\0')
    throw reascript_error { "items are not terminated with \\31 (unit separator)" };

  std::vector<const char *> strings;

  do {
    strings.push_back(list);
    while(*list != ITEM_SEP) ++list;
    *list = '\0';
  } while(*++list);

  return strings;
}

DEFINE_API(bool, BeginCombo, (ImGui_Context*,ctx)(const char*,label)
(const char*,preview_value)(int*,API_RO(flags)),
R"(The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.

Default values: flags = ImGui_ComboFlags_None)",
{
  FRAME_GUARD;

  return ImGui::BeginCombo(label, preview_value,
    valueOr(API_RO(flags), ImGuiComboFlags_None));
});

DEFINE_API(void, EndCombo, (ImGui_Context*,ctx),
"Only call EndCombo() if BeginCombo() returns true!",
{
  FRAME_GUARD;
  ImGui::EndCombo();
});

DEFINE_API(bool, Combo, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(current_item))(char*,items)
(int*,API_RO(popup_max_height_in_items)),
R"(Helper over BeginCombo()/EndCombo() for convenience purpose. Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

Default values: popup_max_height_in_items = -1)",
{
  FRAME_GUARD;

  const auto &strings { splitString(items) };
  return ImGui::Combo(label, API_RW(current_item), strings.data(), strings.size(),
    valueOr(API_RO(popup_max_height_in_items), -1));
});

// Widgets: List Boxes
DEFINE_API(bool, ListBox, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(current_item))(char*,items)(int*,API_RO(height_in_items)),
R"(This is an helper over BeginListBox()/EndListBox() for convenience purpose. This is analoguous to how Combos are created.

Use \31 (ASCII Unit Separator) to separate items within the string and to terminate it.

Default values: height_in_items = -1)",
{
  FRAME_GUARD;

  const auto &strings { splitString(items) };
  return ImGui::ListBox(label, API_RW(current_item), strings.data(), strings.size(),
    valueOr(API_RO(height_in_items), -1));
});

DEFINE_API(bool, BeginListBox, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RO(size_w))(double*,API_RO(size_h)),
R"(Open a framed scrolling region.  This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.

The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() or any items.

- Choose frame width:   width  > 0.0: custom  /  width  < 0.0 or -FLT_MIN: right-align   /  width  = 0.0 (default): use current ItemWidth
- Choose frame height:  height > 0.0: custom  /  height < 0.0 or -FLT_MIN: bottom-align  /  height = 0.0 (default): arbitrary default height which can fit ~7 items

Default values: size_w = 0.0, size_h = 0.0

See ImGui_EndListBox.)",
{
  FRAME_GUARD;

  return ImGui::BeginListBox(label,
    ImVec2(valueOr(API_RO(size_w), 0.0), valueOr(API_RO(size_h), 0.0)));
});

DEFINE_API(void, EndListBox, (ImGui_Context*,ctx),
R"(Only call EndListBox() if BeginListBox() returned true!

See ImGui_BeginListBox.)",
{
  FRAME_GUARD;
  ImGui::EndListBox();
});
