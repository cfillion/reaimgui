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

#include "resource.hpp"

#include <reaper_plugin_functions.h>

class ListClipper : public Resource {
public:
  static ImGuiListClipper *use(ListClipper *);

  ListClipper(Context *ctx) : m_ctx { ctx } {}
  ~ListClipper() { m_imlc.ItemsCount = -1; } // ensure the assert doesn't trip

protected:
  void heartbeat() override { delete this; }

private:
  Context *m_ctx;
  ImGuiListClipper m_imlc;
};

ImGuiListClipper *ListClipper::use(ListClipper *lc)
{
  if(Resource::exists(lc) && Resource::exists(lc->m_ctx)) {
    lc->m_ctx->enterFrame();
    return &lc->m_imlc;
  }

  char message[255];
  snprintf(message, sizeof(message), "expected a valid ImGui_ListClipper*, got %p", lc);
  throw reascript_error { message };
}

using ImGui_ListClipper = ListClipper;

DEFINE_API(ImGui_ListClipper*, CreateListClipper, (ImGui_Context*,ctx),
R"(Helper: Manually clip large list of items.
If you are submitting lots of evenly spaced items and you have a random access to the list, you can perform coarse
clipping based on visibility to save yourself from processing those items at all.
The clipper calculates the range of visible items and advance the cursor to compensate for the non-visible items we have skipped.
(Dear ImGui already clip items based on their bounds but it needs to measure text size to do so, whereas manual coarse clipping before submission makes this cost and your own data fetching/submission cost almost null)

Usage:
  local clipper = reaper.ImGui_CreateListClipper(ctx)
  reaper.ImGui_ListClipper_Begin(clipper, 1000) -- We have 1000 elements, evenly spaced
  while reaper.ImGui_ListClipper_Step(clipper) do
    local display_start = reaper.ImGui_ListClipper_GetDisplayStart(clipper)
    local display_end   = reaper.ImGui_ListClipper_GetDisplayEnd(clipper)
    for row = display_start, display_end - 1 do
      reaper.ImGui_Text(ctx, ("line number %d"):format(i))
    end

Generally what happens is:
- Clipper lets you process the first element (DisplayStart = 0, DisplayEnd = 1) regardless of it being visible or not.
- User code submit one element.
- Clipper can measure the height of the first element
- Clipper calculate the actual range of elements to display based on the current clipping rectangle, position the cursor before the first visible element.
- User code submit visible elements.

The returned clipper object is tied to the context and valid until the next timer tick. See ImGui_ListClipper_Begin.)",
{
  assertValid(ctx);
  return new ListClipper { ctx };
});

DEFINE_API(void, ListClipper_Begin, (ImGui_ListClipper*,clipper)
(int,items_count)(double*,API_RO(items_height)),
R"(items_count: Use INT_MAX if you don't know how many items you have (in which case the cursor won't be advanced in the final step)
items_height: Use -1.0f to be calculated automatically on first step. Otherwise pass in the distance between your items, typically GetTextLineHeightWithSpacing() or GetFrameHeightWithSpacing().

Default values: items_height = -1.0)",
{
  ListClipper::use(clipper)
    ->Begin(items_count, valueOr(API_RO(items_height), -1.0));
});

DEFINE_API(bool, ListClipper_Step, (ImGui_ListClipper*,clipper),
"Call until it returns false. The DisplayStart/DisplayEnd fields will be set and you can process/draw those items.",
{
  return ListClipper::use(clipper)->Step();
});

DEFINE_API(void, ListClipper_End, (ImGui_ListClipper*,clipper),
"Automatically called on the last call of Step() that returns false. See ImGui_ListClipper_Step.",
{
  ListClipper::use(clipper)->End();
});

DEFINE_API(int, ListClipper_GetDisplayStart, (ImGui_ListClipper*,clipper),
"",
{
  return ListClipper::use(clipper)->DisplayStart;
});

DEFINE_API(int, ListClipper_GetDisplayEnd, (ImGui_ListClipper*,clipper),
"",
{
  return ListClipper::use(clipper)->DisplayEnd;
});
