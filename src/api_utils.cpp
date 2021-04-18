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

DEFINE_API(void, LogToTTY, (ImGui_Context*,ctx)
(int*,API_RO(auto_open_depth)),
R"(Start logging all text output from the interface to the TTY (stdout). By default, tree nodes are automatically opened during logging.

Default values: auto_open_depth = -1)",
{
  FRAME_GUARD;
  ImGui::LogToTTY(valueOr(API_RO(auto_open_depth), -1));
});

DEFINE_API(void, LogToFile, (ImGui_Context*,ctx)
(int*,API_RO(auto_open_depth))(const char*,API_RO(filename)),
R"(Start logging all text output from the interface to a file. By default, tree nodes are automatically opened during logging. The data is saved to $resource_path/imgui_log.txt if filename is nil.

Default values: auto_open_depth = -1, filename = nil)",
{
  FRAME_GUARD;
  nullIfEmpty(API_RO(filename));
  ImGui::LogToFile(valueOr(API_RO(auto_open_depth), -1), API_RO(filename));
});

DEFINE_API(void, LogToClipboard, (ImGui_Context*,ctx)
(int*,API_RO(auto_open_depth)),
R"(Start logging all text output from the interface to the OS clipboard. By default, tree nodes are automatically opened during logging. See also ImGui_SetClipboardText.

Default values: auto_open_depth = -1)",
{
  FRAME_GUARD;
  ImGui::LogToClipboard(valueOr(API_RO(auto_open_depth), -1));
});

DEFINE_API(void, LogFinish, (ImGui_Context*,ctx),
"Stop logging (close file, etc.)",
{
  FRAME_GUARD;
  ImGui::LogFinish();
});

DEFINE_API(void, LogText, (ImGui_Context*,ctx)
(const char*,text),
"Pass text data straight to log (without being displayed)",
{
  FRAME_GUARD;
  ImGui::LogText("%s", text);
});

DEFINE_API(const char*, GetClipboardText, (ImGui_Context*,ctx),
"See ImGui_SetClipboardText.",
{
  assertValid(ctx);
  ctx->setCurrent();
  return ImGui::GetClipboardText();
});

DEFINE_API(void, SetClipboardText, (ImGui_Context*,ctx)
(const char*,text),
"See also the ImGui_LogToClipboard function to capture GUI into clipboard, or easily output text data to the clipboard.",
{
  assertValid(ctx);
  ctx->setCurrent();
  return ImGui::SetClipboardText(text);
});
