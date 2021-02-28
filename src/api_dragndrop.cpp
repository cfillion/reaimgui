#include "api_helper.hpp"

#include <cassert>
#include <reaper_plugin_functions.h> // realloc_cmd_ptr

static bool isUserType(const char *type)
{
  // types starting with '_' are reserved for ImGui use
  // (and their payloads are likely not strings either)
  return type && *type && type[0] != '_';
}

// Drag and Drop
DEFINE_API(bool, BeginDragDropSource, (ImGui_Context*,ctx)(int*,API_RO(flags)),
R"(Call when the current item is active. If this return true, you can call SetDragDropPayload() + EndDragDropSource().

If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip as replacement).

Default values: flags = ImGui_DragDropFlags_None)",
{
  FRAME_GUARD;
  return ImGui::BeginDragDropSource(valueOr(API_RO(flags), 0));
});

DEFINE_API(bool, SetDragDropPayload, (ImGui_Context*,ctx)
(const char*,type)(const char*,data)(int*,API_RO(cond)),
R"(type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui.

Default values: cond = ImGui_Cond_Always)",
{
  FRAME_GUARD;
  nullIfEmpty(data);

  if(!isUserType(type))
    return false;

  return ImGui::SetDragDropPayload(type, data, data ? strlen(data) : 0,
    valueOr(API_RO(cond), ImGuiCond_Always));
});

DEFINE_API(void, EndDragDropSource, (ImGui_Context*,ctx),
"Only call EndDragDropSource() if BeginDragDropSource() returns true!",
{
  FRAME_GUARD;
  ImGui::EndDragDropSource();
});

DEFINE_API(bool, BeginDragDropTarget, (ImGui_Context*,ctx),
"Call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()",
{
  FRAME_GUARD;
  return ImGui::BeginDragDropTarget();
});

DEFINE_API(bool, AcceptDragDropPayload, (ImGui_Context*,ctx)
(const char*,type)
(char*,API_WBIG(payload))(int,API_WBIG_SZ(payload))
(int*,API_RO(flags)),
R"(Accept contents of a given type. If ImGui_DragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.

Default values: flags = ImGui_DragDropFlags_None)",
{
  FRAME_GUARD;

  if(!isUserType(type))
    return false;

  const ImGuiDragDropFlags flags { valueOr(API_RO(flags), ImGuiDragDropFlags_None) };
  const ImGuiPayload *payload { ImGui::AcceptDragDropPayload(type, flags) };

  if(!payload)
    return false;

  int newSize {};
  if(payload->DataSize > API_WBIG_SZ(payload) &&
      realloc_cmd_ptr(&API_WBIG(payload), &newSize, payload->DataSize)) {
    // output buffer is no longer null terminated!
    std::memcpy(API_WBIG(payload), payload->Data, newSize);
  }
  else {
    const int limit { std::min(API_WBIG_SZ(payload) - 1, payload->DataSize) };
    std::memcpy(API_WBIG(payload), payload->Data, limit);
    API_WBIG(payload)[limit] = '\0';
  }

  return true;
});

DEFINE_API(bool, AcceptDragDropPayloadRGB, (ImGui_Context*,ctx)
(int*,API_W(rgb))(int*,API_RO(flags)),
R"(Accept contents of a RGB color. If ImGui_DragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.

Default values: flags = ImGui_DragDropFlags_None)",
{
  FRAME_GUARD;

  const ImGuiDragDropFlags flags { valueOr(API_RO(flags), ImGuiDragDropFlags_None) };
  const ImGuiPayload *payload { ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F, flags) };

  if(!payload)
    return false;

  float rgb[3];
  assert(payload->DataSize == sizeof(rgb));
  memcpy(rgb, payload->Data, sizeof(rgb));
  *API_W(rgb) = Color{rgb, false}.pack(false);

  return true;
});

DEFINE_API(bool, AcceptDragDropPayloadRGBA, (ImGui_Context*,ctx)
(int*,API_W(rgba))(int*,API_RO(flags)),
R"(Accept contents of a RGBA color. If ImGui_DragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.

Default values: flags = ImGui_DragDropFlags_None)",
{
  FRAME_GUARD;

  const ImGuiDragDropFlags flags { valueOr(API_RO(flags), ImGuiDragDropFlags_None) };
  const ImGuiPayload *payload { ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F, flags) };

  if(!payload)
    return false;

  float rgba[4];
  assert(payload->DataSize == sizeof(rgba));
  memcpy(rgba, payload->Data, sizeof(rgba));
  *API_W(rgba) = Color{rgba, true}.pack(true);

  return true;
});

DEFINE_API(void, EndDragDropTarget, (ImGui_Context*,ctx),
"Only call EndDragDropTarget() if BeginDragDropTarget() returns true!",
{
  FRAME_GUARD;
  ImGui::EndDragDropTarget();
});

    // IMGUI_API const ImGuiPayload*   GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. may return NULL. use ImGuiPayload::IsDataType() to test for the payload type.
