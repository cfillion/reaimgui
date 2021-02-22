#include "api_helper.hpp"

DEFINE_API(bool, BeginTabBar, (ImGui_Context*,ctx)
(const char*,str_id)(int*,API_RO(flags)),
R"(Create and append into a TabBar.

Default values: flags = ImGui_TabBarFlags_None)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::BeginTabBar(str_id, valueOr(API_RO(flags), ImGuiTabBarFlags_None));
});

DEFINE_API(void, EndTabBar, (ImGui_Context*,ctx),
"Only call EndTabBar() if BeginTabBar() returns true!",
{
  Context::check(ctx)->enterFrame();
  ImGui::EndTabBar();
});

DEFINE_API(bool, BeginTabItem, (ImGui_Context*,ctx)
(const char*,label)(bool*,API_RWO(open))(int*,API_RO(flags)),
R"(Create a Tab. Returns true if the Tab is selected.

Default values: flags = ImGui_TabItemFlags_None
'open' is read/write.)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::BeginTabItem(label, API_RWO(open),
    valueOr(API_RO(flags), ImGuiTabItemFlags_None));
});

DEFINE_API(void, EndTabItem, (ImGui_Context*,ctx),
"Only call EndTabItem() if BeginTabItem() returns true!",
{
  Context::check(ctx)->enterFrame();
  ImGui::EndTabItem();
});

DEFINE_API(bool, TabItemButton, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RO(flags)),
R"(Create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.

Default values: flags = ImGui_TabItemFlags_None)",
{
  Context::check(ctx)->enterFrame();
  return ImGui::TabItemButton(label,
    valueOr(API_RO(flags), ImGuiTabItemFlags_None));
});

// IMGUI_API void          SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.
