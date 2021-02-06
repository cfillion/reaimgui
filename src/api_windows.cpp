#include "api_helper.hpp"

#include <imgui/imgui.h>

// Cannot name 'open' as 'openInOutOptional' (and have it listed in the docs as
// a return value) because REAPER would always make it non-null.
DEFINE_API(bool, Begin, ((Window*, window))
((const char*, name))((bool*, openInOptional))((int*, windowFlagsInOptional)),
R"(Push window to the stack and start appending to it. See ImGui_End.

- Passing 'open' shows a window-closing widget in the upper-right corner of the window, which clicking will set the boolean to false when clicked.
- You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times. Some information such as 'flags' or 'open' will only be considered by the first call to Begin().
- Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
  [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
- Note that the bottom of window stack always contains a window called "Debug".)",
{
  USE_WINDOW(window, false);
  ImGuiWindowFlags flags { VALUE_OR(windowFlagsInOptional, 0) };
  flags |= ImGuiWindowFlags_NoSavedSettings;
  return ImGui::Begin(name, openInOptional, flags);
});

DEFINE_API(void, End, ((Window*, window)),
R"(Pop window from the stack. See ImGui_Begin.)",
{
  USE_WINDOW(window);
  ImGui::End();
});
