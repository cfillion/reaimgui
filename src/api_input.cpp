#include "api_helper.hpp"

static int inputTextCallback(ImGuiInputTextCallbackData *data)
{
  if(data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    realloc_cmd_ptr(&data->Buf, &data->BufSize, data->BufSize);

  return 0;
}

static void sanitizeInputTextFlags(ImGuiInputTextFlags &flags)
{
  flags &= ~(
    // don't expose these to users
    ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory |
    ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackCharFilter |
    ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackResize |

    // reserved for ImGui's internal use
    ImGuiInputTextFlags_Multiline | ImGuiInputTextFlags_NoMarkEdited
  );
}

// Widgets: Input with Keyboard
DEFINE_API(bool, InputText, ((ImGui_Context*,ctx))
((const char*,label))((char*,API_WBIG(buf)))((int,API_WBIG_SZ(buf)))
((int*,API_RO(flags))),
"",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputText(label, API_WBIG(buf), API_WBIG_SZ(buf),
    flags, &inputTextCallback, nullptr);
});

DEFINE_API(bool, InputTextMultiline, ((ImGui_Context*,ctx))
((const char*,label))((char*,API_WBIG(buf)))((int,API_WBIG_SZ(buf)))
((double*,API_RO(width)))((double*,API_RO(height)))
((int*,API_RO(flags))),
"Default values: width = 0, height = 0, flags = ImGui_InputTextFlags_None",
{
  ENTER_CONTEXT(ctx, false)

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputTextMultiline(label, API_WBIG(buf), API_WBIG_SZ(buf),
    ImVec2(valueOr(API_RO(width), 0.0), valueOr(API_RO(height), 0.0)),
    valueOr(API_RO(flags), ImGuiInputTextFlags_None),
    &inputTextCallback, nullptr);
});

// IMGUI_API bool          InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

DEFINE_API(bool, InputTextWithHint, ((ImGui_Context*,ctx))
((const char*,label))((const char*,hint))
((char*,API_WBIG(buf)))((int,API_WBIG_SZ(buf)))
((int*,API_RO(flags))),
"",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputTextWithHint(label, hint, API_WBIG(buf), API_WBIG_SZ(buf),
    flags, &inputTextCallback, nullptr);
});

DEFINE_API(bool, InputInt, ((ImGui_Context*,ctx))((const char*,label))
((int*,API_RW(value)))((int*,API_RO(step)))((int*,API_RO(stepFast)))
((int*,API_RO(flags))),
"'step' defaults to 1, 'stepFast' defaults to 100",
{
  ENTER_CONTEXT(ctx, false);

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputInt(label, API_RW(value),
    valueOr(API_RO(step), 1), valueOr(API_RO(stepFast), 100), flags);
});

DEFINE_API(bool, InputDouble, ((ImGui_Context*,ctx))((const char*,label))
((double*,API_RW(value)))((double*,API_RO(step)))((double*,API_RO(stepFast)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"'step' defaults to 1, 'stepFast' defaults to 100",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputDouble(label, API_RW(value),
    valueOr(API_RO(step), 1.0), valueOr(API_RO(stepFast), 100.0),
    API_RO(format), flags);
});

// IMGUI_API bool          InputScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);
// IMGUI_API bool          InputScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);

DEFINE_API(bool, InputDoubleN, ((ImGui_Context*,ctx))((const char*,label))
((reaper_array*,values))((double*,API_RO(step)))((double*,API_RO(stepFast)))
((const char*,API_RO(format)))((int*,API_RO(flags))),
"",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(format));

  ImGuiInputTextFlags flags { valueOr(API_RO(flags), 0) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size, API_RO(step), API_RO(stepFast),
    API_RO(format), flags);
});
