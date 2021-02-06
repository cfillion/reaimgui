#include "api_helper.hpp"

#include <imgui/imgui.h>

// TODO flag
DEFINE_API(bool, BeginMenuBar, ((Window*, window)),
R"(Append to menu-bar of current window (requires ImGuiWindowFlags_MenuBar flag set on parent window). See ImGui_EndMenuBar.)",
{
  USE_WINDOW(window, false);
  return ImGui::BeginMenuBar();
});

DEFINE_API(void, EndMenuBar, ((Window*, window)),
R"(Only call EndMenuBar() if BeginMenuBar() returns true! See ImGui_BeginMenuBar.)",
{
  USE_WINDOW(window);
  ImGui::EndMenuBar();
});

DEFINE_API(bool, BeginMainMenuBar, ((Window*, window)),
R"(Create a menu bar at the top of the screen and append to it.)",
{
  USE_WINDOW(window, false);
  return ImGui::BeginMainMenuBar();
});

DEFINE_API(void, EndMainMenuBar, ((Window*, window)),
R"(Only call EndMainMenuBar() if BeginMainMenuBar() returns true! See ImGui_BeginMainMenuBar.)",
{
  USE_WINDOW(window);
  ImGui::EndMainMenuBar();
});

DEFINE_API(bool, BeginMenu, ((Window*, window))
((const char*, label))((bool*, enabledInOptional)),
R"(Create a sub-menu entry. only call EndMenu() if this returns true! See ImGui_EndMenu.

'enabled' is true by default.)",
{
  USE_WINDOW(window, false);
  return ImGui::BeginMenu(label, VALUE_OR(enabledInOptional, true));
});

DEFINE_API(void, EndMenu, ((Window*, window)),
R"(Only call EndMenu() if BeginMenu() returns true! See ImGui_BegiMenu.)",
{
  USE_WINDOW(window);
  ImGui::EndMenu();
});

DEFINE_API(bool, MenuItem, ((Window*, window))
((const char*, label))((const char*, shortcutInOptional))
((bool*, selectedInOptional))((bool*, enabledInOptional)),
R"(Return true when activated. Shortcuts are displayed for convenience but not processed by ImGui at the moment. Toggle state is written to 'selected' when provided.

'enabled' is true by default.)",
{
  USE_WINDOW(window, false);
  return ImGui::MenuItem(label, shortcutInOptional, selectedInOptional,
    VALUE_OR(enabledInOptional, true));
});
