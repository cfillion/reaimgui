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
  return ImGui::BeginDragDropSource(valueOr(API_RO(flags), ImGuiDragDropFlags_None));
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

static void copyPayload(const ImGuiPayload *payload, char **reabuf, const int reabuf_sz)
{
  assertValid(*reabuf);

  int newSize {};
  if(payload->DataSize > reabuf_sz &&
      realloc_cmd_ptr(reabuf, &newSize, payload->DataSize)) {
    // output buffer is no longer null terminated!
    std::memcpy(*reabuf, payload->Data, newSize);
  }
  else {
    const int limit { std::min(reabuf_sz - 1, payload->DataSize) };
    std::memcpy(*reabuf, payload->Data, limit);
    (*reabuf)[limit] = '\0';
  }
}

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

  if(payload)
    copyPayload(payload, &API_WBIG(payload), API_WBIG_SZ(payload));

  return payload;
});

static bool AcceptDragDropPayloadColor(int *color, bool alpha, ImGuiDragDropFlags flags)
{
  assertValid(color);

  const char *type { alpha ? IMGUI_PAYLOAD_TYPE_COLOR_4F : IMGUI_PAYLOAD_TYPE_COLOR_3F };
  const ImGuiPayload *payload { ImGui::AcceptDragDropPayload(type, flags) };

  if(!payload)
    return false;

  const size_t size { sizeof(float) * (alpha ? 4 : 3) };
  assert(static_cast<size_t>(payload->DataSize) == size);

  float buf[4];
  memcpy(buf, payload->Data, size);
  *color = Color{buf, alpha}.pack(alpha);

  return true;
}

DEFINE_API(bool, AcceptDragDropPayloadRGB, (ImGui_Context*,ctx)
(int*,API_W(rgb))(int*,API_RO(flags)),
R"(Accept contents of a RGB color. See ImGui_AcceptDragDropPayload.

Default values: flags = ImGui_DragDropFlags_None)",
{
  FRAME_GUARD;
  const ImGuiDragDropFlags flags { valueOr(API_RO(flags), ImGuiDragDropFlags_None) };
  return AcceptDragDropPayloadColor(API_W(rgb), false, flags);
});

DEFINE_API(bool, AcceptDragDropPayloadRGBA, (ImGui_Context*,ctx)
(int*,API_W(rgba))(int*,API_RO(flags)),
R"(Accept contents of a RGBA color. See ImGui_AcceptDragDropPayload.

Default values: flags = ImGui_DragDropFlags_None)",
{
  FRAME_GUARD;
  const ImGuiDragDropFlags flags { valueOr(API_RO(flags), ImGuiDragDropFlags_None) };
  return AcceptDragDropPayloadColor(API_W(rgba), true, flags);
});

DEFINE_API(bool, AcceptDragDropPayloadFiles, (ImGui_Context*,ctx)
(int*,API_W(count))(int*,API_RO(flags)),
R"(Accept contents of a RGBA color. See ImGui_AcceptDragDropPayload.

Default values: flags = ImGui_DragDropFlags_None)",
{
  FRAME_GUARD;
  assertValid(API_W(count));

  const ImGuiDragDropFlags flags { valueOr(API_RO(flags), ImGuiDragDropFlags_None) };
  const ImGuiPayload *payload { ImGui::AcceptDragDropPayload(REAIMGUI_PAYLOAD_TYPE_FILES, flags) };

  if(payload)
    *API_W(count) = ctx->draggedFiles().size();

  return payload;
});

DEFINE_API(void, EndDragDropTarget, (ImGui_Context*,ctx),
"Only call EndDragDropTarget() if BeginDragDropTarget() returns true!",
{
  FRAME_GUARD;
  ImGui::EndDragDropTarget();
});

DEFINE_API(bool, GetDragDropPayload, (ImGui_Context*,ctx)
(char*,API_W(type))(int,API_W_SZ(type))
(char*,API_WBIG(payload))(int,API_WBIG_SZ(payload))
(bool*,API_W(is_preview))(bool*,API_W(is_delivery)),
"Peek directly into the current payload from anywhere.",
{
  FRAME_GUARD;

  const ImGuiPayload *payload { ImGui::GetDragDropPayload() };
  if(!payload || payload->DataFrameCount == -1 || !isUserType(payload->DataType))
    return false;

  if(API_W(type))
    snprintf(API_W(type), API_W_SZ(type), "%s", payload->DataType);
  copyPayload(payload, &API_WBIG(payload), API_WBIG_SZ(payload));
  if(API_W(is_preview))  *API_W(is_preview)  = payload->Preview;
  if(API_W(is_delivery)) *API_W(is_delivery) = payload->Delivery;

  return true;
});

DEFINE_API(bool, GetDragDropPayloadFile, (ImGui_Context*,ctx)
(int,index)(char*,API_W(filename))(int,API_W_SZ(filename)),
"",
{
  FRAME_GUARD;

  const auto &files { ctx->draggedFiles() };

  const ImGuiPayload *payload { ImGui::GetDragDropPayload() };
  if(!payload || !payload->IsDataType(REAIMGUI_PAYLOAD_TYPE_FILES))
    return false;
  else if(static_cast<size_t>(index) >= files.size())
    return false;

  snprintf(API_W(filename), API_W_SZ(filename), "%s", files[index].c_str());

  return true;
});
