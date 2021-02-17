#include "api_helper.hpp"

#include <imgui/imgui.h>

DEFINE_API(bool, BeginMenuBar, ((ImGui_Context*,ctx)),
R"(Append to menu-bar of current window (requires ImGui_WindowFlags_MenuBar flag set on parent window). See ImGui_EndMenuBar.)",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::BeginMenuBar();
});

DEFINE_API(void, EndMenuBar, ((ImGui_Context*,ctx)),
R"(Only call EndMenuBar() if BeginMenuBar() returns true! See ImGui_BeginMenuBar.)",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndMenuBar();
});

DEFINE_API(bool, BeginMainMenuBar, ((ImGui_Context*,ctx)),
R"(Create a menu bar at the top of the screen and append to it.)",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::BeginMainMenuBar();
});

DEFINE_API(void, EndMainMenuBar, ((ImGui_Context*,ctx)),
R"(Only call EndMainMenuBar() if BeginMainMenuBar() returns true! See ImGui_BeginMainMenuBar.)",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndMainMenuBar();
});

DEFINE_API(bool, BeginMenu, ((ImGui_Context*,ctx))
((const char*, label))((bool*, API_RO(enabled))),
R"(Create a sub-menu entry. only call EndMenu() if this returns true! See ImGui_EndMenu.

'enabled' is true by default.)",
{
  ENTER_CONTEXT(ctx, false);
  return ImGui::BeginMenu(label, valueOr(API_RO(enabled), true));
});

DEFINE_API(void, EndMenu, ((ImGui_Context*,ctx)),
R"(Only call EndMenu() if BeginMenu() returns true! See ImGui_BeginMenu.)",
{
  ENTER_CONTEXT(ctx);
  ImGui::EndMenu();
});

DEFINE_API(bool, MenuItem, ((ImGui_Context*,ctx))
((const char*, label))((const char*, API_RO(shortcut)))
((bool*, API_RWO(selected)))((bool*, API_RO(enabled))),
R"(Return true when activated. Shortcuts are displayed for convenience but not processed by ImGui at the moment. Toggle state is written to 'selected' when provided.

'enabled' is true by default.)",
{
  ENTER_CONTEXT(ctx, false);
  nullIfEmpty(API_RO(shortcut));

  return ImGui::MenuItem(label, API_RO(shortcut), API_RWO(selected),
    valueOr(API_RO(enabled), true));
});
