#include "api_helper.hpp"

// Tooltips
// - Tooltip are windows following the mouse. They do not take focus away.

DEFINE_API(void, BeginTooltip, ((Window*,window)),
"begin/append a tooltip window. to create full-featured tooltip (with any kind of items).",
{
  USE_WINDOW(window);
  ImGui::BeginTooltip();
});

DEFINE_API(void, EndTooltip, ((Window*,window)),
"",
{
  USE_WINDOW(window);
  ImGui::EndTooltip();
});

DEFINE_API(void, SetTooltip, ((Window*,window))((const char*,text)),
"Set a text-only tooltip, typically use with ImGui_IsItemHovered(). override any previous call to SetTooltip().",
{
  USE_WINDOW(window);
  ImGui::SetTooltip("%s", text);
});
