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

#include "font.hpp"
#include "listclipper.hpp"
#include "resource_proxy.hpp"
#include "version.hpp"

DEFINE_API(void, GetVersion,
(char*,API_W(imgui_version))(int,API_W_SZ(imgui_version))
(char*,API_W(reaimgui_version))(int,API_W_SZ(reaimgui_version)),
"",
{
  if(API_W(imgui_version))
    snprintf(API_W(imgui_version), API_W_SZ(imgui_version), "%s", IMGUI_VERSION);
  if(API_W(reaimgui_version))
    snprintf(API_W(reaimgui_version), API_W_SZ(reaimgui_version), "%s", REAIMGUI_VERSION);
});

DEFINE_API(ImGui_Context*, CreateContext,
(const char*,label)(int*,API_RO(config_flags)),
R"(Create a new ReaImGui context. The context will remain valid as long as it is used in each defer cycle.

The label is used for the tab text when windows are docked in REAPER and also as a unique identifier for storing settings.

Default values: config_flags = ImGui_ConfigFlags_None)",
{
  const int flags { valueOr(API_RO(config_flags), ImGuiConfigFlags_None) };
  return new Context { label, flags };
});

DEFINE_API(void, DestroyContext, (ImGui_Context*,ctx),
R"(Close and free the resources used by a context.)",
{
  assertValid(ctx);
  delete ctx;
});

DEFINE_API(bool, ValidatePtr, (void*,pointer)(const char*,type),
R"(Return whether the pointer of the specified type is valid. Supported types are ImGui_Context*, ImGui_DrawList*, ImGui_ListClipper* and ImGui_Viewport*.)",
{
  ResourceProxy::Key proxyKey;

  if(!strcmp(type, "ImGui_Context*"))
    return Resource::exists(static_cast<Context *>(pointer));
  else if(!strcmp(type, "ImGui_DrawList*"))
    return DrawList.decode<Context>(pointer, &proxyKey);
  else if(!strcmp(type, "ImGui_Font*"))
    return Resource::exists(static_cast<Font *>(pointer));
  else if(!strcmp(type, "ImGui_ListClipper*"))
    return ListClipper::validate(static_cast<ListClipper *>(pointer));
  else if(!strcmp(type, "ImGui_Viewport*"))
    return Viewport.decode<Context>(pointer, &proxyKey);
  else
    return false;
});

DEFINE_API(int, GetConfigFlags, (ImGui_Context*,ctx),
"See ImGui_SetConfigFlags.",
{
  assertValid(ctx);
  return ctx->IO().ConfigFlags;
});

DEFINE_API(void, SetConfigFlags, (ImGui_Context*,ctx)
(int,flags),
"See ImGui_GetConfigFlags, ImGui_ConfigFlags_None.",
{
  assertValid(ctx);
  ctx->IO().ConfigFlags = flags;
});

DEFINE_API(void, ShowMetricsWindow, (ImGui_Context*,ctx)
(bool*,API_RWO(p_open)),
R"(Create Metrics/Debugger window. Display Dear ImGui internals: windows, draw commands, various internal state, etc.

Default values: p_open = nil)",
{
  FRAME_GUARD;
  ImGui::ShowMetricsWindow(API_RWO(p_open));
});

DEFINE_API(double, GetTime, (ImGui_Context*,ctx),
"Get global imgui time. Incremented every frame.",
{
  FRAME_GUARD;
  return ImGui::GetTime();
});

DEFINE_API(double, GetDeltaTime, (ImGui_Context*,ctx),
"Time elapsed since last frame, in seconds.",
{
  FRAME_GUARD;
  return ctx->IO().DeltaTime;
});

DEFINE_API(int, GetFrameCount, (ImGui_Context*,ctx),
"Get global imgui frame count. incremented by 1 every frame.",
{
  FRAME_GUARD;
  return ImGui::GetFrameCount();
});

DEFINE_API(void, AttachFont, (ImGui_Context*,ctx)
(ImGui_Font*,font),
"Enable a font for use in the given context. Fonts must be attached as soon as possible after creating the context or on a new defer cycle.",
{
  assertValid(ctx);
  assertValid(font);

  if(ctx->inFrame())
    throw reascript_error { "cannot modify font texture: a frame has already begun" };

  ctx->fonts().add(font);
});
