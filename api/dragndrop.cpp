/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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

#include "../src/color.hpp"

#include <cassert>
#include <cstring> // strlen

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

API_FUNC(0_1, bool, BeginDragDropSource, (Context*,ctx)
(RO<int*>,flags,ImGuiDragDropFlags_None),
R"(Call after submitting an item which may be dragged. when this return true,
you can call SetDragDropPayload() + EndDragDropSource()

If you stop calling BeginDragDropSource() the payload is preserved however
it won't have a preview tooltip (we currently display a fallback "..." tooltip
as replacement).)")
{
  FRAME_GUARD;
  return ImGui::BeginDragDropSource(API_GET(flags));
}

API_FUNC(0_1, bool, SetDragDropPayload, (Context*,ctx)
(const char*,type) (const char*,data) (RO<int*>,cond,ImGuiCond_Always),
R"(The type is a user defined string of maximum 32 characters.
Strings starting with '_' are reserved for dear imgui internal types.
Data is copied and held by imgui.)")
{
  FRAME_GUARD;
  nullIfEmpty(data);

  if(!isUserType(type))
    return false;

  size_t dataSize {};
  if(data) {
    dataSize = strlen(data);
    if(!dataSize)
      data = nullptr; // imgui asserts if data != null && size == 0
  }

  return ImGui::SetDragDropPayload(type, data, dataSize, API_GET(cond));
}

API_FUNC(0_1, void, EndDragDropSource, (Context*,ctx),
"Only call EndDragDropSource() if BeginDragDropSource returns true!")
{
  FRAME_GUARD;
  ImGui::EndDragDropSource();
}

API_FUNC(0_1, bool, BeginDragDropTarget, (Context*,ctx),
R"(Call after submitting an item that may receive a payload.
If this returns true, you can call AcceptDragDropPayload + EndDragDropTarget.)")
{
  FRAME_GUARD;
  return ImGui::BeginDragDropTarget();
}

API_FUNC(0_1, bool, AcceptDragDropPayload, (Context*,ctx)
(const char*,type)
(WB<char*>,payload) (WBS<int>,payload_sz)
(RO<int*>,flags,ImGuiDragDropFlags_None),
R"(Accept contents of a given type. If DragDropFlags_AcceptBeforeDelivery is set
you can peek into the payload before the mouse button is released.)")
{
  FRAME_GUARD;

  if(!isUserType(type))
    return false;

  const ImGuiPayload *impayload {ImGui::AcceptDragDropPayload(type, API_GET(flags))};
  if(!impayload)
    return false;

  if(payload && impayload->Data)
    copyToBigBuf(payload, payload_sz, impayload->Data, impayload->DataSize);

  return true;
}

static bool AcceptDragDropPayloadColor(int *color, bool alpha,
  ImGuiDragDropFlags flags)
{
  assertValid(color);

  const char *type {alpha ? IMGUI_PAYLOAD_TYPE_COLOR_4F : IMGUI_PAYLOAD_TYPE_COLOR_3F};
  const ImGuiPayload *payload {ImGui::AcceptDragDropPayload(type, flags)};

  if(!payload)
    return false;

  const size_t size {sizeof(float) * (alpha ? 4 : 3)};
  assert(static_cast<size_t>(payload->DataSize) == size);

  float buf[4];
  memcpy(buf, payload->Data, size);
  *color = Color{buf, alpha}.pack(alpha);

  return true;
}

API_FUNC(0_1, bool, AcceptDragDropPayloadRGB, (Context*,ctx)
(W<int*>,rgb) (RO<int*>,flags,ImGuiDragDropFlags_None),
"Accept a RGB color. See AcceptDragDropPayload.")
{
  FRAME_GUARD;
  return AcceptDragDropPayloadColor(rgb, false, API_GET(flags));
}

API_FUNC(0_1, bool, AcceptDragDropPayloadRGBA, (Context*,ctx)
(W<int*>,rgba) (RO<int*>,flags,ImGuiDragDropFlags_None),
"Accept a RGBA color. See AcceptDragDropPayload.")
{
  FRAME_GUARD;
  return AcceptDragDropPayloadColor(rgba, true, API_GET(flags));
}

API_FUNC(0_1, bool, AcceptDragDropPayloadFiles, (Context*,ctx)
(W<int*>,count) (RO<int*>,flags,ImGuiDragDropFlags_None),
R"(Accept a list of dropped files. See AcceptDragDropPayload and GetDragDropPayloadFile.)")
{
  FRAME_GUARD;
  assertValid(count);

  const ImGuiPayload *payload {
    ImGui::AcceptDragDropPayload(REAIMGUI_PAYLOAD_TYPE_FILES, API_GET(flags))
  };

  if(payload)
    *count = ctx->draggedFiles().size();

  return payload;
}

API_FUNC(0_1, void, EndDragDropTarget, (Context*,ctx),
"Only call EndDragDropTarget() if BeginDragDropTarget returns true!")
{
  FRAME_GUARD;
  ImGui::EndDragDropTarget();
}

API_FUNC(0_1, bool, GetDragDropPayload, (Context*,ctx)
(W<char*>,type) (WS<int>,type_sz)
(WB<char*>,payload) (WBS<int>,payload_sz)
(W<bool*>,is_preview) (W<bool*>,is_delivery),
R"(Peek directly into the current payload from anywhere.
Returns false when drag and drop is finished or inactive.)")
{
  FRAME_GUARD;

  const ImGuiPayload *impayload {ImGui::GetDragDropPayload()};
  if(!impayload || impayload->DataFrameCount == -1 || !isUserType(impayload->DataType))
    return false;

  if(type)
    snprintf(type, type_sz, "%s", impayload->DataType);
  if(payload && impayload->Data)
    copyToBigBuf(payload, payload_sz, impayload->Data, impayload->DataSize);
  if(is_preview)  *is_preview  = impayload->Preview;
  if(is_delivery) *is_delivery = impayload->Delivery;

  return true;
}

API_FUNC(0_1, bool, GetDragDropPayloadFile, (Context*,ctx)
(int,index) (W<char*>,filename) (WS<int>,filename_sz),
R"(Get a filename from the list of dropped files.
Returns false if index is out of bounds.)")
{
  FRAME_GUARD;

  const auto &files {ctx->draggedFiles()};

  const ImGuiPayload *payload {ImGui::GetDragDropPayload()};
  if(!payload || !payload->IsDataType(REAIMGUI_PAYLOAD_TYPE_FILES))
    return false;
  else if(static_cast<size_t>(index) >= files.size())
    return false;

  snprintf(filename, filename_sz, "%s", files[index].c_str());

  return true;
}

API_SECTION_DEF(flags, ROOT_SECTION, "Flags");
API_ENUM(0_1, ImGui, DragDropFlags_None, "");
API_SECTION_P(flags, "Source", "For BeginDragDropSource");
API_ENUM(0_1, ImGui, DragDropFlags_SourceNoPreviewTooltip,
R"(By default, a successful call to BeginDragDropSource opens a tooltip so you
   can display a preview or description of the source contents.
   This flag disables this behavior.)");
API_ENUM(0_1, ImGui, DragDropFlags_SourceNoDisableHover,
R"(By default, when dragging we clear data so that IsItemHovered will return
   false, to avoid subsequent user code submitting tooltips. This flag disables
   this behavior so you can still call IsItemHovered on the source item.)");
API_ENUM(0_1, ImGui, DragDropFlags_SourceNoHoldToOpenOthers,
R"(Disable the behavior that allows to open tree nodes and collapsing header by
   holding over them while dragging a source item.)");
API_ENUM(0_1, ImGui, DragDropFlags_SourceAllowNullID,
R"(Allow items such as Text, Image that have no unique identifier to be used as
   drag source, by manufacturing a temporary identifier based on their
   window-relative position. This is extremely unusual within the dear imgui
   ecosystem and so we made it explicit.)");
API_ENUM(0_1, ImGui, DragDropFlags_SourceExtern,
R"(External source (from outside of dear imgui), won't attempt to read current
   item/window info. Will always return true.
   Only one Extern source can be active simultaneously.)");
API_ENUM(0_9_2, ImGui, DragDropFlags_PayloadAutoExpire,
R"(Automatically expire the payload if the source cease to be submitted
   (otherwise payloads are persisting while being dragged).)");
API_SECTION_P(flags, "Payload", "For AcceptDragDropPayload");
API_ENUM(0_1, ImGui, DragDropFlags_AcceptBeforeDelivery,
R"(AcceptDragDropPayload will returns true even before the mouse button is
   released. You can then check GetDragDropPayload/is_delivery to test if the
   payload needs to be delivered.)");
API_ENUM(0_1, ImGui, DragDropFlags_AcceptNoDrawDefaultRect,
  "Do not draw the default highlight rectangle when hovering over target.");
API_ENUM(0_1, ImGui, DragDropFlags_AcceptNoPreviewTooltip,
  "Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.");
API_ENUM(0_1, ImGui, DragDropFlags_AcceptPeekOnly,
R"(For peeking ahead and inspecting the payload before delivery.
   Equivalent to DragDropFlags_AcceptBeforeDelivery |
   DragDropFlags_AcceptNoDrawDefaultRect.)");
