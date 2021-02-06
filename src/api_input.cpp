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
DEFINE_API(bool, InputText, ((Window*,window))
((const char*,label))((char*,bufOutNeedBig))((int,bufOutNeedBig_sz))
((int*,flagsInOptional)),
"",
{
  USE_WINDOW(window, false);

  ImGuiInputTextFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputText(label, bufOutNeedBig, bufOutNeedBig_sz,
    flags, &inputTextCallback, nullptr);
});
// IMGUI_API bool          InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

DEFINE_API(bool, InputTextWithHint, ((Window*,window))
((const char*,label))((const char*,hint))
((char*,bufOutNeedBig))((int,bufOutNeedBig_sz))
((int*,flagsInOptional)),
"",
{
  USE_WINDOW(window, false);

  ImGuiInputTextFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeInputTextFlags(flags);
  flags |= ImGuiInputTextFlags_CallbackResize;

  return ImGui::InputTextWithHint(label, hint,
    bufOutNeedBig, bufOutNeedBig_sz,
    flags, &inputTextCallback, nullptr);
});

DEFINE_API(bool, InputInt, ((Window*,window))((const char*,label))
((int*,valueInOut))((int*,stepInOptional))((int*,stepFastInOptional))
((int*,flagsInOptional)),
"'step' defaults to 1, 'stepFast' defaults to 100",
{
  USE_WINDOW(window, false);

  ImGuiInputTextFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputInt(label, valueInOut,
    VALUE_OR(stepInOptional, 1),
    VALUE_OR(stepFastInOptional, 100),
    flags);
});

DEFINE_API(bool, InputDouble, ((Window*,window))((const char*,label))
((double*,valueInOut))((double*,stepInOptional))((double*,stepFastInOptional))
((const char*,formatInOptional))((int*,flagsInOptional)),
"'step' defaults to 1, 'stepFast' defaults to 100",
{
  USE_WINDOW(window, false);

  ImGuiInputTextFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputDouble(label, valueInOut,
    VALUE_OR(stepInOptional, 1),
    VALUE_OR(stepFastInOptional, 100),
    NULL_IF_EMPTY(formatInOptional),
    flags);
});

// IMGUI_API bool          InputScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);
// IMGUI_API bool          InputScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);

DEFINE_API(bool, InputDoubleN, ((Window*,window))((const char*,label))
((reaper_array*,values))((double*,stepInOptional))((double*,stepFastInOptional))
((const char*,formatInOptional))((int*,flagsInOptional)),
"",
{
  USE_WINDOW(window, false);

  ImGuiInputTextFlags flags { VALUE_OR(flagsInOptional, 0) };
  sanitizeInputTextFlags(flags);

  return ImGui::InputScalarN(label, ImGuiDataType_Double,
    values->data, values->size,
    stepInOptional, stepFastInOptional,
    NULL_IF_EMPTY(formatInOptional),
    flags);
});
