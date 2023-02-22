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

#include "color.hpp"

#include <cassert>
#include <cstring> // strlen
#include <reaper_plugin_functions.h> // realloc_cmd_ptr

API_SECTION("Drag & Drop",
R"(On source items, call BeginDragDropSource(),
if it returns true also call SetDragDropPayload() + EndDragDropSource().

On target candidates, call BeginDragDropTarget(),
if it returns true also call AcceptDragDropPayload() + EndDragDropTarget().

An item can be both a drag source and a drop target.)");

static bool isUserType(const char *type)
{
  // types starting with '_' are reserved for ImGui use
  // (and their payloads are likely not strings either)
  return type && *type && type[0] != '_';
}

DEFINE_API(bool, BeginDragDropSource, (ImGui_Context*,ctx)
(int*,API_RO(flags),ImGuiDragDropFlags_None),
R"(Call after submitting an item which may be dragged. when this return true,
you can call SetDragDropPayload() + EndDragDropSource()

If you stop calling BeginDragDropSource() the payload is preserved however
it won't have a preview tooltip (we currently display a fallback "..." tooltip
as replacement).)")
{
  FRAME_GUARD;
  return ImGui::BeginDragDropSource(API_RO_GET(flags));
}

DEFINE_API(bool, SetDragDropPayload, (ImGui_Context*,ctx)
(const char*,type)(const char*,data)(int*,API_RO(cond),ImGuiCond_Always),
R"(The type is a user defined string of maximum 32 characters.
Strings starting with '_' are reserved for dear imgui internal types.
Data is copied and held by imgui.)")
{
  FRAME_GUARD;
  nullIfEmpty(data);

  if(!isUserType(type))
    return false;

  return ImGui::SetDragDropPayload(
    type, data, data ? strlen(data) : 0, API_RO_GET(cond));
}

DEFINE_API(void, EndDragDropSource, (ImGui_Context*,ctx),
"Only call EndDragDropSource() if BeginDragDropSource returns true!")
{
  FRAME_GUARD;
  ImGui::EndDragDropSource();
}

DEFINE_API(bool, BeginDragDropTarget, (ImGui_Context*,ctx),
R"(Call after submitting an item that may receive a payload.
If this returns true, you can call AcceptDragDropPayload + EndDragDropTarget.)")
{
  FRAME_GUARD;
  return ImGui::BeginDragDropTarget();
}

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
(int*,API_RO(flags),ImGuiDragDropFlags_None),
R"(Accept contents of a given type. If DragDropFlags_AcceptBeforeDelivery is set
you can peek into the payload before the mouse button is released.)")
{
  FRAME_GUARD;

  if(!isUserType(type))
    return false;

  const ImGuiDragDropFlags flags { API_RO_GET(flags) };
  const ImGuiPayload *payload { ImGui::AcceptDragDropPayload(type, flags) };

  if(payload)
    copyPayload(payload, &API_WBIG(payload), API_WBIG_SZ(payload));

  return payload;
}

static bool AcceptDragDropPayloadColor(int *color, bool alpha,
  ImGuiDragDropFlags flags)
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
(int*,API_W(rgb))(int*,API_RO(flags),ImGuiDragDropFlags_None),
"Accept a RGB color. See AcceptDragDropPayload.")
{
  FRAME_GUARD;
  return AcceptDragDropPayloadColor(API_W(rgb), false, API_RO_GET(flags));
}

DEFINE_API(bool, AcceptDragDropPayloadRGBA, (ImGui_Context*,ctx)
(int*,API_W(rgba))(int*,API_RO(flags),ImGuiDragDropFlags_None),
"Accept a RGBA color. See AcceptDragDropPayload.")
{
  FRAME_GUARD;
  return AcceptDragDropPayloadColor(API_W(rgba), true, API_RO_GET(flags));
}

DEFINE_API(bool, AcceptDragDropPayloadFiles, (ImGui_Context*,ctx)
(int*,API_W(count))(int*,API_RO(flags),ImGuiDragDropFlags_None),
R"(Accept a list of dropped files. See AcceptDragDropPayload and GetDragDropPayloadFile.)")
{
  FRAME_GUARD;
  assertValid(API_W(count));

  const ImGuiPayload *payload {
    ImGui::AcceptDragDropPayload(REAIMGUI_PAYLOAD_TYPE_FILES, API_RO_GET(flags))
  };

  if(payload)
    *API_W(count) = ctx->draggedFiles().size();

  return payload;
}

DEFINE_API(void, EndDragDropTarget, (ImGui_Context*,ctx),
"Only call EndDragDropTarget() if BeginDragDropTarget returns true!")
{
  FRAME_GUARD;
  ImGui::EndDragDropTarget();
}

DEFINE_API(bool, GetDragDropPayload, (ImGui_Context*,ctx)
(char*,API_W(type))(int,API_W_SZ(type))
(char*,API_WBIG(payload))(int,API_WBIG_SZ(payload))
(bool*,API_W(is_preview))(bool*,API_W(is_delivery)),
"Peek directly into the current payload from anywhere.")
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
}

DEFINE_API(bool, GetDragDropPayloadFile, (ImGui_Context*,ctx)
(int,index)(char*,API_W(filename))(int,API_W_SZ(filename)),
R"(Get a filename from the list of dropped files.
Returns false if index is out of bounds.)")
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
}

DEFINE_SECTION(flags, ROOT_SECTION, "Flags");
DEFINE_ENUM(ImGui, DragDropFlags_None, "");
API_SECTION_P(flags, "Source", "For BeginDragDropSource");
DEFINE_ENUM(ImGui, DragDropFlags_SourceNoPreviewTooltip,
R"(By default, a successful call to BeginDragDropSource opens a tooltip so you
   can display a preview or description of the source contents.
   This flag disables this behavior.)");
DEFINE_ENUM(ImGui, DragDropFlags_SourceNoDisableHover,
R"(By default, when dragging we clear data so that IsItemHovered will return
   false, to avoid subsequent user code submitting tooltips. This flag disables
   this behavior so you can still call IsItemHovered on the source item.)");
DEFINE_ENUM(ImGui, DragDropFlags_SourceNoHoldToOpenOthers,
R"(Disable the behavior that allows to open tree nodes and collapsing header by
   holding over them while dragging a source item.)");
DEFINE_ENUM(ImGui, DragDropFlags_SourceAllowNullID,
R"(Allow items such as Text, Image that have no unique identifier to be used as
   drag source, by manufacturing a temporary identifier based on their
   window-relative position. This is extremely unusual within the dear imgui
   ecosystem and so we made it explicit.)");
DEFINE_ENUM(ImGui, DragDropFlags_SourceExtern,
R"(External source (from outside of dear imgui), won't attempt to read current
   item/window info. Will always return true.
   Only one Extern source can be active simultaneously.)");
DEFINE_ENUM(ImGui, DragDropFlags_SourceAutoExpirePayload,
R"(Automatically expire the payload if the source cease to be submitted
   (otherwise payloads are persisting while being dragged).)");
API_SECTION_P(flags, "Payload", "For AcceptDragDropPayload");
DEFINE_ENUM(ImGui, DragDropFlags_AcceptBeforeDelivery,
R"(AcceptDragDropPayload will returns true even before the mouse button is
   released. You can then check GetDragDropPayload/is_delivery to test if the
   payload needs to be delivered.)");
DEFINE_ENUM(ImGui, DragDropFlags_AcceptNoDrawDefaultRect,
  "Do not draw the default highlight rectangle when hovering over target.");
DEFINE_ENUM(ImGui, DragDropFlags_AcceptNoPreviewTooltip,
  "Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.");
DEFINE_ENUM(ImGui, DragDropFlags_AcceptPeekOnly,
R"(For peeking ahead and inspecting the payload before delivery.
   Equivalent to DragDropFlags_AcceptBeforeDelivery |
   DragDropFlags_AcceptNoDrawDefaultRect.)");
