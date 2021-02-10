#include "api_helper.hpp"

  // Drag and Drop
DEFINE_API(bool, BeginDragDropSource, ((ImGui_Context*,ctx))((int*,flagsInOptional)),
R"(Call when the current item is active. If this return true, you can call SetDragDropPayload() + EndDragDropSource().

If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip as replacement).)",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::BeginDragDropSource(valueOr(flagsInOptional, 0));
});

DEFINE_API(bool, SetDragDropPayload, ((ImGui_Context*,ctx))
((const char*,type))((const char*,data))((int*,condInOptional)),
"type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui.",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(data);

  return ImGui::SetDragDropPayload(type, data, data ? strlen(data) : 0,
    valueOr(condInOptional, 0));
});

DEFINE_API(void, EndDragDropSource, ((ImGui_Context*,ctx)),
"only call EndDragDropSource() if BeginDragDropSource() returns true!",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndDragDropSource();
});

    // IMGUI_API bool                  BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
    // IMGUI_API const ImGuiPayload*   AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags = 0);          // accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
    // IMGUI_API void                  EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
    // IMGUI_API const ImGuiPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. may return NULL. use ImGuiPayload::IsDataType() to test for the payload type.
