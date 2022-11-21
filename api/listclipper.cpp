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

#include "listclipper.hpp"

#include <imgui/imgui_internal.h>
#include <reaper_plugin_functions.h>

ListClipper::ListClipper(Context *ctx) : m_ctx { ctx } {}

ListClipper::~ListClipper()
{
  // do ~ImGuiListClipper's work to allow out-of-order destruction

  if(m_imlc.TempData && Resource::isValid(m_ctx)) {
    ImGuiContext *ctx { m_ctx->imgui() };
    --ctx->ClipperTempDataStacked;

    bool shift { false };
    for(int i { 0 }; i < ctx->ClipperTempDataStacked; ++i) {
      ImGuiListClipperData &data { ctx->ClipperTempData[i] };
      if(data.ListClipper == &m_imlc)
        shift = true;
      else if(shift) {
        // i > 0 because shift can only be true from the second iteration
        ImGuiListClipperData &prevData { ctx->ClipperTempData[i - 1] };
        prevData.ListClipper->TempData = &data;
        prevData = data;
      }
    }
  }

  m_imlc.TempData = nullptr;
}

bool ListClipper::isValid() const
{
  return Resource::isValid(m_ctx);
}

ImGuiListClipper *ListClipper::operator->()
{
  assertValid(this);
  assertFrame(m_ctx);
  keepAlive();
  return &m_imlc;
}

API_SECTION("List Clipper", R"(Helper to manually clip large list of items.

If you have lots evenly spaced items and you have random access to the list, you can perform coarse clipping based on visibility to only submit items that are in view.
The clipper calculates the range of visible items and advance the cursor to compensate for the non-visible items we have skipped.
(Dear ImGui already clip items based on their bounds but: it needs to first layout the item to do so, and generally fetching/submitting your own data incurs additional cost. Coarse clipping using ImGui_ListClipper allows you to easily scale using lists with tens of thousands of items without a problem)

Usage:

    if not reaper.ImGui_ValidatePtr(clipper, 'ImGui_ListClipper*') then
      clipper = reaper.ImGui_CreateListClipper(ctx)
    end
    reaper.ImGui_ListClipper_Begin(clipper, 1000) -- We have 1000 elements, evenly spaced
    while reaper.ImGui_ListClipper_Step(clipper) do
      local display_start, display_end = reaper.ImGui_ListClipper_GetDisplayRange(clipper)
      for row = display_start, display_end - 1 do
        reaper.ImGui_Text(ctx, ("line number %d"):format(i))
      end
    end

Generally what happens is:
- Clipper lets you process the first element (DisplayStart = 0, DisplayEnd = 1) regardless of it being visible or not.
- User code submit that one element.
- Clipper can measure the height of the first element
- Clipper calculate the actual range of elements to display based on the current clipping rectangle, position the cursor before the first visible element.
- User code submit visible elements.
- The clipper also handles various subtleties related to keyboard/gamepad navigation, wrapping etc.)");

DEFINE_API(ImGui_ListClipper*, CreateListClipper, (ImGui_Context*,ctx),
"The returned clipper object is only valid for the given context and is valid as long as it is used in each defer cycle. See ListClipper_Begin.)",
{
  assertValid(ctx);
  return new ListClipper { ctx };
});

DEFINE_API(void, ListClipper_Begin, (ImGui_ListClipper*,clipper)
(int,items_count)(double*,API_RO(items_height)),
R"(items_count: Use INT_MAX if you don't know how many items you have (in which case the cursor won't be advanced in the final step)
items_height: Use -1.0 to be calculated automatically on first step. Otherwise pass in the distance between your items, typically GetTextLineHeightWithSpacing or GetFrameHeightWithSpacing.

Default values: items_height = -1.0)",
{
  (*clipper)->Begin(items_count, valueOr(API_RO(items_height), -1.0));
});

DEFINE_API(bool, ListClipper_Step, (ImGui_ListClipper*,clipper),
"Call until it returns false. The display_start/display_end fields from ListClipper_GetDisplayRange will be set and you can process/draw those items.",
{
  return (*clipper)->Step();
});

DEFINE_API(void, ListClipper_End, (ImGui_ListClipper*,clipper),
"Automatically called on the last call of ListClipper_Step that returns false.",
{
  (*clipper)->End();
});

DEFINE_API(void, ListClipper_GetDisplayRange, (ImGui_ListClipper*,clipper)
(int*,API_W(display_start))(int*,API_W(display_end)),
"",
{
  ImGuiListClipper *imclipper { (*clipper).operator->() };
  if(API_W(display_start)) *API_W(display_start) = imclipper->DisplayStart;
  if(API_W(display_end))   *API_W(display_end)   = imclipper->DisplayEnd;
});

DEFINE_API(void, ListClipper_ForceDisplayRangeByIndices, (ImGui_ListClipper*,clipper)
(int,item_min)(int,item_max),
R"(Call ListClipper_ForceDisplayRangeByIndices before first call to ListClipper_Step if you need a range of items to be displayed regardless of visibility.

item_max is exclusive e.g. use (42, 42+1) to make item 42 always visible BUT due to alignment/padding of certain items it is likely that an extra item may be included on either end of the display range.)",
{
  (*clipper)->ForceDisplayRangeByIndices(item_min, item_max);
});
