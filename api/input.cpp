/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#include "flags.hpp"

#include <imgui/misc/cpp/imgui_stdlib.h>
#include <reaper_plugin_functions.h> // realloc_cmd_ptr
#include <reaper_plugin_secrets.h>   // reaper_array
#include <vector>

API_SECTION("Text & Scalar Input");

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

DEFINE_API(bool, InputText, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags)),
"Default values: flags = InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const InputTextFlags flags { API_RO(flags) };

  // The output buffer is updated only when true is returned.
  // This differs from upstream Dear ImGui when InputTextFlags_EnterReturnsTrue
  // is used. However it makes the behavior consistent with the scalar input
  // functions (eg. InputDouble). https://github.com/ocornut/imgui/issues/3946
  if(ImGui::InputText(label, &value, flags, nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

DEFINE_API(bool, InputTextMultiline, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(double*,API_RO(size_w))(double*,API_RO(size_h))
(int*,API_RO(flags)),
"Default values: size_w = 0.0, size_h = 0.0, flags = InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const ImVec2 size { valueOr(API_RO(size_w), 0.f),
                      valueOr(API_RO(size_h), 0.f) };
  const InputTextFlags flags { API_RO(flags) };

  if(ImGui::InputTextMultiline(label, &value, size, flags, nullptr, nullptr)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
});

DEFINE_API(bool, InputTextWithHint, (ImGui_Context*,ctx)
(const char*,label)(const char*,hint)
(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags)),
"Default values: flags = InputTextFlags_None",
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
"Default values: step = 1, step_fast = 100, flags = InputTextFlags_None",
{
  FRAME_GUARD;

  const InputTextFlags flags { API_RO(flags) };
  return ImGui::InputInt(label, API_RW(v),
    valueOr(API_RO(step), 1), valueOr(API_RO(step_fast), 100), flags);
});

DEFINE_API(bool, InputInt2, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RO(flags)),
"Default values: flags = InputTextFlags_None",
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
"Default values: flags = InputTextFlags_None",
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
"Default values: flags = InputTextFlags_None",
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
"Default values: step = 0.0, step_fast = 0.0, format = '%.3f', flags = InputTextFlags_None",
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
"Default values: format = '%.3f', flags = InputTextFlags_None",
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
"Default values: format = '%.3f', flags = InputTextFlags_None",
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
"Default values: format = '%.3f', flags = InputTextFlags_None",
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
"Default values: step = nil, format = nil, step_fast = nil, format = '%.3f', flags = InputTextFlags_None",
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  const InputTextFlags flags { API_RO(flags) };

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size, API_RO(step), API_RO(step_fast),
    API_RO(format), flags);
});

API_SUBSECTION("Flags", "Most of these are only useful for InputText*() and not for InputDoubleX, InputIntX etc.");

DEFINE_ENUM(ImGui, InputTextFlags_None,                "Most of the InputTextFlags flags are only useful for InputText and not for InputIntX, InputDouble etc. (Those are per-item flags. There are shared flags in SetConfigVar: ConfigVar_InputTextCursorBlink and ConfigVar_InputTextEnterKeepActive)");
DEFINE_ENUM(ImGui, InputTextFlags_CharsDecimal,        "Allow 0123456789.+-*/.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsHexadecimal,    "Allow 0123456789ABCDEFabcdef.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsUppercase,      "Turn a..z into A..Z.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsNoBlank,        "Filter out spaces, tabs.");
DEFINE_ENUM(ImGui, InputTextFlags_AutoSelectAll,       "Select entire text when first taking mouse focus.");
DEFINE_ENUM(ImGui, InputTextFlags_EnterReturnsTrue,    "Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit function.");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackCompletion,  "Callback on pressing TAB (for completion handling).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackHistory,     "Callback on pressing Up/Down arrows (for history handling).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackAlways,      "Callback on each iteration. User code may query cursor position, modify text buffer.");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackCharFilter,  "Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.");
DEFINE_ENUM(ImGui, InputTextFlags_AllowTabInput,       "Pressing TAB input a '\\t' character into the text field.");
DEFINE_ENUM(ImGui, InputTextFlags_CtrlEnterForNewLine, "In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).");
DEFINE_ENUM(ImGui, InputTextFlags_NoHorizontalScroll,  "Disable following the cursor horizontally.");
DEFINE_ENUM(ImGui, InputTextFlags_AlwaysOverwrite,     "Overwrite mode.");
DEFINE_ENUM(ImGui, InputTextFlags_ReadOnly,            "Read-only mode.");
DEFINE_ENUM(ImGui, InputTextFlags_Password,            "Password mode, display all characters as '*'.");
DEFINE_ENUM(ImGui, InputTextFlags_NoUndoRedo,          "Disable undo/redo. Note that input text owns the text data while active.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsScientific,     "Allow 0123456789.+-*/eE (Scientific notation input).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackResize,      "Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this).");
// DEFINE_ENUM(ImGui, InputTextFlags_CallbackEdit,        "Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active).");
DEFINE_ENUM(ImGui, InputTextFlags_EscapeClearsAll,     "Escape key clears content if not empty, and deactivate otherwise (constrast to default behavior of Escape to revert)");
