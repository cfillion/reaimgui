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

#include "callback.hpp"
#include "flags.hpp"

#include "../src/api_eel.hpp"
#include "../src/function.hpp"

#include <imgui/misc/cpp/imgui_stdlib.h>
#include <reaper_plugin_functions.h> // realloc_cmd_ptr
#include <reaper_plugin_secrets.h>   // reaper_array
#include <vector>

API_SECTION("Text & Scalar Input");

using InputTextCallback = Callback<ImGuiInputTextCallbackData>;

class InputTextFlags : public Flags<ImGuiInputTextFlags> {
public:
  InputTextFlags(int flags) : Flags { flags }
  {
    *this &= ~(
      // don't expose these to users
      ImGuiInputTextFlags_CallbackResize |

      // reserved for ImGui's internal use
      ImGuiInputTextFlags_Multiline | ImGuiInputTextFlags_NoMarkEdited
    );
  }
};

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

static int runCallback(ImGuiInputTextCallbackData *data)
{
  InputTextCallback::invoke(data);
  return 0;
}

static ImGuiInputTextCallback useCallback(Function *func)
{
  if(!func)
    return nullptr;

  assertValid(func);
  func->keepAlive();
  return &runCallback;
}

#define CALLBACK_ARGS useCallback(API_RO(callback)), API_RO(callback)

DEFINE_API(bool, InputText, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags),ImGuiInputTextFlags_None)
(ImGui_Function*,API_RO(callback)),
"")
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const InputTextFlags flags { API_RO_GET(flags) };

  // The output buffer is updated only when true is returned.
  // This differs from upstream Dear ImGui when InputTextFlags_EnterReturnsTrue
  // is used. However it makes the behavior consistent with the scalar input
  // functions (eg. InputDouble). https://github.com/ocornut/imgui/issues/3946
  if(ImGui::InputText(label, &value, flags, CALLBACK_ARGS)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
}

DEFINE_API(bool, InputTextMultiline, (ImGui_Context*,ctx)
(const char*,label)(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(double*,API_RO(size_w),0.0)(double*,API_RO(size_h),0.0)
(int*,API_RO(flags),ImGuiInputTextFlags_None)
(ImGui_Function*,API_RO(callback)),
"")
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const ImVec2 size(API_RO_GET(size_w), API_RO_GET(size_h));
  const InputTextFlags flags { API_RO_GET(flags) };

  if(ImGui::InputTextMultiline(label, &value, size, flags, CALLBACK_ARGS)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
}

DEFINE_API(bool, InputTextWithHint, (ImGui_Context*,ctx)
(const char*,label)(const char*,hint)
(char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf))
(int*,API_RO(flags),ImGuiInputTextFlags_None)
(ImGui_Function*,API_RO(callback)),
"")
{
  FRAME_GUARD;
  assertValid(API_RWBIG(buf));

  std::string value { API_RWBIG(buf) };
  const InputTextFlags flags { API_RO_GET(flags) };

  if(ImGui::InputTextWithHint(label, hint, &value, flags, CALLBACK_ARGS)) {
    copyToBuffer(value, API_RWBIG(buf), API_RWBIG_SZ(buf));
    return true;
  }
  return false;
}

DEFINE_API(bool, InputInt, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v))(int*,API_RO(step),1)(int*,API_RO(step_fast),100)
(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;

  const InputTextFlags flags { API_RO_GET(flags) };
  return ImGui::InputInt(label, API_RW(v),
    API_RO_GET(step), API_RO_GET(step_fast), flags);
}

DEFINE_API(bool, InputInt2, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;

  ReadWriteArray<int, int, 2> values { API_RW(v1), API_RW(v2) };
  const InputTextFlags flags { API_RO_GET(flags) };

  if(ImGui::InputInt2(label, values.data(), flags))
    return values.commit();
  else
    return false;
}

DEFINE_API(bool, InputInt3, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RW(v3))
(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;

  ReadWriteArray<int, int, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  const InputTextFlags flags { API_RO_GET(flags) };

  if(ImGui::InputInt3(label, values.data(), flags))
    return values.commit();
  else
    return false;
}

DEFINE_API(bool, InputInt4, (ImGui_Context*,ctx)(const char*,label)
(int*,API_RW(v1))(int*,API_RW(v2))(int*,API_RW(v3))
(int*,API_RW(v4))(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;

  ReadWriteArray<int, int, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  const InputTextFlags flags { API_RO_GET(flags) };

  if(ImGui::InputInt4(label, values.data(), flags))
    return values.commit();
  else
    return false;
}

DEFINE_API(bool, InputDouble, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v))(double*,API_RO(step),0.0)(double*,API_RO(step_fast),0.0)
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  const InputTextFlags flags { API_RO_GET(flags) };

  return ImGui::InputDouble(label, API_RW(v),
    API_RO_GET(step), API_RO_GET(step_fast), API_RO_GET(format), flags);
}

static bool inputDoubleN(const char *label, double *data, const size_t size,
  const char *format, const InputTextFlags flags)
{
  return ImGui::InputScalarN(label, ImGuiDataType_Double, data, size,
    nullptr, nullptr, format, flags);
}

DEFINE_API(bool, InputDouble2, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 2> values { API_RW(v1), API_RW(v2) };
  const InputTextFlags flags { API_RO_GET(flags) };

  if(inputDoubleN(label, values.data(), values.size(), API_RO_GET(format), flags))
    return values.commit();
  else
    return false;
}

DEFINE_API(bool, InputDouble3, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 3> values { API_RW(v1), API_RW(v2), API_RW(v3) };
  const InputTextFlags flags { API_RO_GET(flags) };

  if(inputDoubleN(label, values.data(), values.size(), API_RO_GET(format), flags))
    return values.commit();
  else
    return false;
}

DEFINE_API(bool, InputDouble4, (ImGui_Context*,ctx)(const char*,label)
(double*,API_RW(v1))(double*,API_RW(v2))(double*,API_RW(v3))(double*,API_RW(v4))
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(format));

  ReadWriteArray<double, double, 4> values
    { API_RW(v1), API_RW(v2), API_RW(v3), API_RW(v4) };
  const InputTextFlags flags { API_RO_GET(flags) };

  if(inputDoubleN(label, values.data(), values.size(), API_RO_GET(format), flags))
    return values.commit();
  else
    return false;
}

DEFINE_API(bool, InputDoubleN, (ImGui_Context*,ctx)(const char*,label)
(reaper_array*,values)(double*,API_RO(step))(double*,API_RO(step_fast))
(const char*,API_RO(format),"%.3f")(int*,API_RO(flags),ImGuiInputTextFlags_None),
"")
{
  FRAME_GUARD;
  assertValid(values);
  nullIfEmpty(API_RO(format));

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size, API_RO(step), API_RO(step_fast),
    API_RO_GET(format), InputTextFlags { API_RO_GET(flags) });
}

API_SUBSECTION("Flags",
R"(Most of these are only useful for InputText*() and not for InputDoubleX,
InputIntX etc.

(Those are per-item flags. There are shared flags in SetConfigVar:
ConfigVar_InputTextCursorBlink and ConfigVar_InputTextEnterKeepActive.))");

DEFINE_ENUM(ImGui, InputTextFlags_None,             "");
DEFINE_ENUM(ImGui, InputTextFlags_CharsDecimal,     "Allow 0123456789.+-*/.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsHexadecimal, "Allow 0123456789ABCDEFabcdef.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsUppercase,   "Turn a..z into A..Z.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsNoBlank,     "Filter out spaces, tabs.");
DEFINE_ENUM(ImGui, InputTextFlags_AutoSelectAll,
  "Select entire text when first taking mouse focus.");
DEFINE_ENUM(ImGui, InputTextFlags_EnterReturnsTrue,
R"(Return 'true' when Enter is pressed (as opposed to every time the value was
   modified). Consider looking at the IsItemDeactivatedAfterEdit function.)");
DEFINE_ENUM(ImGui, InputTextFlags_CallbackCompletion,
  "Callback on pressing TAB (for completion handling).");
DEFINE_ENUM(ImGui, InputTextFlags_CallbackHistory,
  "Callback on pressing Up/Down arrows (for history handling).");
DEFINE_ENUM(ImGui, InputTextFlags_CallbackAlways,
  "Callback on each iteration. User code may query cursor position, modify text buffer.");
DEFINE_ENUM(ImGui, InputTextFlags_CallbackCharFilter,
R"(Callback on character inputs to replace or discard them.
   Modify 'EventChar' to replace or 'EventChar = 0' to discard.)");
DEFINE_ENUM(ImGui, InputTextFlags_CallbackEdit,
R"(Callback on any edit (note that InputText() already returns true on edit,
   the callback is useful mainly to manipulate the underlying buffer while
   focus is active).)");
DEFINE_ENUM(ImGui, InputTextFlags_AllowTabInput,
  "Pressing TAB input a '\\t' character into the text field.");
DEFINE_ENUM(ImGui, InputTextFlags_CtrlEnterForNewLine,
R"(In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter
   (default is opposite: unfocus with Ctrl+Enter, add line with Enter).)");
DEFINE_ENUM(ImGui, InputTextFlags_NoHorizontalScroll,
  "Disable following the cursor horizontally.");
DEFINE_ENUM(ImGui, InputTextFlags_AlwaysOverwrite, "Overwrite mode.");
DEFINE_ENUM(ImGui, InputTextFlags_ReadOnly,        "Read-only mode.");
DEFINE_ENUM(ImGui, InputTextFlags_Password,
    "Password mode, display all characters as '*'.");
DEFINE_ENUM(ImGui, InputTextFlags_NoUndoRedo,
    "Disable undo/redo. Note that input text owns the text data while active.");
DEFINE_ENUM(ImGui, InputTextFlags_CharsScientific,
    "Allow 0123456789.+-*/eE (Scientific notation input).");
DEFINE_ENUM(ImGui, InputTextFlags_EscapeClearsAll,
R"(Escape key clears content if not empty, and deactivate otherwise
   (constrast to default behavior of Escape to revert).)");

API_SUBSECTION("InputText Callback",
R"(The functions and variables documented in this section are only available
within the callbacks given to the InputText* functions.
See CreateFunctionFromEEL.

```lua
local reverseAlphabet = reaper.ImGui_CreateFunctionFromEEL([[
  EventChar >= 'a' && EventChar <= 'z' ? EventChar = 'z' - (EventChar - 'a');
]])

local function frame()
  rv, text = reaper.ImGui_InputText(ctx, 'No lowercase', text,
    reaper.ImGui_InputTextFlags_CallbackCharFilter(), reverseAlphabet)
end
```

Variable access table (R = updated for reading,
                       W = writes are applied,
                       - = unmodified):
<table>
<tr>
  <th></th>
  <th>Always</th><th>CharFilter</th><th>Completion</th><th>Edit</th><th>History</th>
</tr>
<tr><th>EventFlag</th>     <td>R/-</td><td>R/-</td><td>R/-</td><td>R/-</td><td>R/-</td></tr>
<tr><th>Flags</th>         <td>R/-</td><td>R/-</td><td>R/-</td><td>R/-</td><td>R/-</td></tr>
<tr><th>EventChar</th>     <td>-/-</td><td>R/W</td><td>-/-</td><td>-/-</td><td>-/-</td></tr>
<tr><th>EventKey</th>      <td>-/-</td><td>-/-</td><td>R/-</td><td>-/-</td><td>R/-</td></tr>
<tr><th>Buf</th>           <td>R/-</td><td>-/-</td><td>R/-</td><td>R/-</td><td>R/-</td></tr>
<tr><th>CursorPos</th>     <td>R/W</td><td>-/-</td><td>R/W</td><td>R/W</td><td>R/W</td></tr>
<tr><th>SelectionStart</th><td>R/W</td><td>-/-</td><td>R/W</td><td>R/W</td><td>R/W</td></tr>
<tr><th>SelectionEnd</th>  <td>R/W</td><td>-/-</td><td>R/W</td><td>R/W</td><td>R/W</td></tr>
</table>

The InputTextCallback_* functions should only be used when EventFlag is one of
InputTextFlags_CallbackAlways/Completion/Edit/History.)");

DEFINE_EELVAR(int, EventFlag,      "One of InputTextFlags_Callback*");
DEFINE_EELVAR(int, Flags,          "What was passed to InputText()");
DEFINE_EELVAR(int, EventChar,
  "Character input. Replace character with another one, or set to zero to drop.");
DEFINE_EELVAR(int, EventKey,
R"(Key_UpArrow/DownArrow/Tab. Compare against these constants instead of
a hard-coded numerical value.)");
DEFINE_EELVAR(const char*, Buf,    "Current value being edited.");
DEFINE_EELVAR(int, CursorPos,      "");
DEFINE_EELVAR(int, SelectionStart, "Equal to SelectionEnd when no selection.");
DEFINE_EELVAR(int, SelectionEnd,   "");

template<>
void InputTextCallback::storeVars(Function *func)
{
  func->setDouble(EELVar_EventFlag, s_data->EventFlag);
  func->setDouble(EELVar_Flags,     s_data->Flags);

  if(s_data->EventFlag & ImGuiInputTextFlags_CallbackCharFilter)
    func->setDouble(EELVar_EventChar, s_data->EventChar);

  if(s_data->EventFlag & (ImGuiInputTextFlags_CallbackCompletion |
                          ImGuiInputTextFlags_CallbackHistory))
    func->setDouble(EELVar_EventKey, s_data->EventKey);

  if(s_data->EventFlag & (ImGuiInputTextFlags_CallbackAlways     |
                          ImGuiInputTextFlags_CallbackEdit       |
                          ImGuiInputTextFlags_CallbackCompletion |
                          ImGuiInputTextFlags_CallbackHistory)) {
    func->setString(EELVar_Buf,
      { s_data->Buf, static_cast<size_t>(s_data->BufTextLen) });

    func->setDouble(EELVar_CursorPos,      s_data->CursorPos);
    func->setDouble(EELVar_SelectionStart, s_data->SelectionStart);
    func->setDouble(EELVar_SelectionEnd,   s_data->SelectionEnd);
  }
}

template<>
void InputTextCallback::loadVars(const Function *func)
{
  if(s_data->EventFlag & ImGuiInputTextFlags_CallbackCharFilter)
    s_data->EventChar = *func->getDouble(EELVar_EventChar);

  if(s_data->EventFlag & (ImGuiInputTextFlags_CallbackAlways     |
                          ImGuiInputTextFlags_CallbackEdit       |
                          ImGuiInputTextFlags_CallbackCompletion |
                          ImGuiInputTextFlags_CallbackHistory)) {
    s_data->CursorPos      = *func->getDouble(EELVar_CursorPos);
    s_data->SelectionStart = *func->getDouble(EELVar_SelectionStart);
    s_data->SelectionEnd   = *func->getDouble(EELVar_SelectionEnd);
  }
}

DEFINE_EELAPI(void, InputTextCallback_DeleteChars,
(int,pos)(int,bytes_count),
"")
{
  if(InputTextCallback::DataAccess data {})
    data->DeleteChars(pos, bytes_count);
}

DEFINE_EELAPI(void, InputTextCallback_InsertChars,
(int,pos)(std::string_view,new_text),
"")
{
  if(InputTextCallback::DataAccess data {})
    data->InsertChars(pos, &new_text.front(), &*new_text.end());
}

DEFINE_EELAPI(void, InputTextCallback_SelectAll, NO_ARGS, "")
{
  if(InputTextCallback::DataAccess data {})
    data->SelectAll();
}

DEFINE_EELAPI(void, InputTextCallback_ClearSelection, NO_ARGS, "")
{
  if(InputTextCallback::DataAccess data {})
    data->ClearSelection();
}

DEFINE_EELAPI(bool, InputTextCallback_HasSelection, NO_ARGS, "")
{
  if(InputTextCallback::DataAccess data {})
    return data->HasSelection();
  return false;
}
