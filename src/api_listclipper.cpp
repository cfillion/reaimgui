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
  ImGuiListClipper clipper;
  clipper.Begin(1000);         // We have 1000 elements, evenly spaced.
  while (clipper.Step())
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
          ImGui::Text("line number %d", i);

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
