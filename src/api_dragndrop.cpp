#include "api_helper.hpp"

  // Drag and Drop
DEFINE_API(bool, BeginDragDropSource, ((Window*,window))((int*,flagsInOptional)),
R"(Call when the current item is active. If this return true, you can call SetDragDropPayload() + EndDragDropSource().

If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip as replacement).)",
{
  USE_WINDOW(window, false);
  return ImGui::BeginDragDropSource(valueOr(flagsInOptional, 0));
});

DEFINE_API(bool, SetDragDropPayload, ((Window*,window))
((const char*,type))((const char*,data))((int*,condInOptional)),
"type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui.",
{
  USE_WINDOW(window, false);
  nullIfEmpty(data);

  return ImGui::SetDragDropPayload(type, data, data ? strlen(data) : 0,
    valueOr(condInOptional, 0));
});

DEFINE_API(void, EndDragDropSource, ((Window*,window)),
"only call EndDragDropSource() if BeginDragDropSource() returns true!",
{
  USE_WINDOW(window);
  ImGui::EndDragDropSource();
});

    // IMGUI_API bool                  BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
    // IMGUI_API const ImGuiPayload*   AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags = 0);          // accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
    // IMGUI_API void                  EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
    // IMGUI_API const ImGuiPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. may return NULL. use ImGuiPayload::IsDataType() to test for the payload type.
