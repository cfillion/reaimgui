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

#include "helper.hpp"

DEFINE_API(bool, BeginMenuBar, (ImGui_Context*,ctx),
R"(Append to menu-bar of current window (requires ImGui_WindowFlags_MenuBar flag set on parent window). See ImGui_EndMenuBar.)",
{
  FRAME_GUARD;
  return ImGui::BeginMenuBar();
});

DEFINE_API(void, EndMenuBar, (ImGui_Context*,ctx),
"Only call EndMenuBar if ImGui_BeginMenuBar returns true!",
{
  FRAME_GUARD;
  ImGui::EndMenuBar();
});

DEFINE_API(bool, BeginMenu, (ImGui_Context*,ctx)
(const char*,label)(bool*,API_RO(enabled)),
R"(Create a sub-menu entry. only call ImGui_EndMenu if this returns true!

Default values: enabled = true)",
{
  FRAME_GUARD;
  return ImGui::BeginMenu(label, valueOr(API_RO(enabled), true));
});

DEFINE_API(void, EndMenu, (ImGui_Context*,ctx),
R"(Only call EndMenu() if ImGui_BeginMenu returns true!)",
{
  FRAME_GUARD;
  ImGui::EndMenu();
});

DEFINE_API(bool, MenuItem, (ImGui_Context*,ctx)
(const char*,label)(const char*,API_RO(shortcut))
(bool*,API_RW(p_selected))(bool*,API_RO(enabled)),
R"(Return true when activated. Shortcuts are displayed for convenience but not processed by ImGui at the moment. Toggle state is written to 'selected' when provided.

Default values: enabled = true)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(shortcut));

  return ImGui::MenuItem(label, API_RO(shortcut), API_RW(p_selected),
    valueOr(API_RO(enabled), true));
});
