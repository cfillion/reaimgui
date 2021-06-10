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

#include "settings.hpp"

#include "context.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h>

// static void *Settings_readOpen(
//   ImGuiContext *, ImGuiSettingsHandler *handler, const char *)
// {
//   return handler; // must return non-NULL for readLine to be called
// }
//
// static void Settings_readLine(
//   ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *, const char *line)
// {
//   Settings *settings { static_cast<Settings *>(handler->UserData) };
//   const int flags { ctx->IO.ConfigFlags };
//
//   int x, y;
//   if(sscanf(line, "Pos=%d,%d", &x, &y) == 2)
//     settings->pos = { x, y };
//   else if(!(flags & ReaImGuiConfigFlags_NoRestoreSize) &&
//       sscanf(line, "Size=%d,%d", &x, &y) == 2)
//     settings->size = { x, y };
//   else if(sscanf(line, "Dock=%d", &x) == 1)
//     settings->dock = x;
// }
//
// static void Settings_writeAll(
//   ImGuiContext *, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf)
// {
//   const Settings *settings { static_cast<Settings *>(handler->UserData) };
//   buf->appendf("[%s][%s]\n", handler->TypeName, settings->title.c_str());
//   buf->appendf("Pos=%d,%d\n", settings->pos.x, settings->pos.y);
//   buf->appendf("Size=%d,%d\n", settings->size.x, settings->size.y);
//   buf->appendf("Dock=%d\n", settings->dock);
//   buf->append("\n");
// }

Settings::Settings(const char *name)
  : m_filename { GetResourcePath() }
{
  if(!name[0]) // does not prohibit empty window titles
    throw reascript_error { "context name is required" };

  m_filename += WDL_DIRCHAR_STR "ReaImGui";
  RecursiveCreateDirectory(m_filename.c_str(), 0);

  const size_t pathSize { m_filename.size() };
  m_filename.resize(pathSize +
    (sizeof(ImGuiID) * 2) + strlen(WDL_DIRCHAR_STR ".ini"));
  snprintf(&m_filename[pathSize], (m_filename.size() - pathSize) + 1,
    WDL_DIRCHAR_STR "%0*X.ini",
    static_cast<int>(sizeof(ImGuiID) * 2), ImHashStr(name));
}

// void Settings::install()
// {
//   ImGuiSettingsHandler ini_handler;
//   ini_handler.TypeName = "Context";
//   ini_handler.TypeHash = ImHashStr(ini_handler.TypeName);
//   ini_handler.WriteAllFn = Settings_writeAll;
//   ini_handler.ReadOpenFn = Settings_readOpen;
//   ini_handler.ReadLineFn = Settings_readLine;
//   ini_handler.UserData = this;
//
//   ImGuiContext *imgui { ImGui::GetCurrentContext() };
//   imgui->SettingsHandlers.push_back(ini_handler);
// }

// void Settings::load()
// {
//   update();
//
//   ImGuiIO &io { ImGui::GetIO() };
//   if(io.IniFilename)
//     ImGui::LoadIniSettingsFromDisk(io.IniFilename);
// }

void Settings::update() // called every frame
{
  ImGuiIO &io { ImGui::GetIO() };
  if(io.ConfigFlags & ReaImGuiConfigFlags_NoSavedSettings)
    io.IniFilename = nullptr;
  else
    io.IniFilename = m_filename.c_str();
}
