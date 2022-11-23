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

API_SECTION("Menu");

DEFINE_API(bool, BeginMenuBar, (ImGui_Context*,ctx),
R"(Append to menu-bar of current window (requires WindowFlags_MenuBar flag set
on parent window). See EndMenuBar.)",
{
  FRAME_GUARD;
  return ImGui::BeginMenuBar();
});

DEFINE_API(void, EndMenuBar, (ImGui_Context*,ctx),
"Only call EndMenuBar if BeginMenuBar returns true!",
{
  FRAME_GUARD;
  ImGui::EndMenuBar();
});

DEFINE_API(bool, BeginMenu, (ImGui_Context*,ctx)
(const char*,label)(bool*,API_RO(enabled),true),
"Create a sub-menu entry. only call EndMenu if this returns true!",
{
  FRAME_GUARD;
  return ImGui::BeginMenu(label, API_RO_GET(enabled));
});

DEFINE_API(void, EndMenu, (ImGui_Context*,ctx),
R"(Only call EndMenu() if BeginMenu returns true!)",
{
  FRAME_GUARD;
  ImGui::EndMenu();
});

DEFINE_API(bool, MenuItem, (ImGui_Context*,ctx)
(const char*,label)(const char*,API_RO(shortcut))
(bool*,API_RWO(p_selected))(bool*,API_RO(enabled),true),
R"(Return true when activated. Shortcuts are displayed for convenience but not
processed by ImGui at the moment. Toggle state is written to 'selected' when
provided.)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(shortcut));

  return ImGui::MenuItem(label, API_RO(shortcut), API_RWO(p_selected),
    API_RO_GET(enabled));
});
