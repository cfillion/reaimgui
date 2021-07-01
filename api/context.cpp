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

DEFINE_API(__LINE__, ImGui_Context*, CreateContext,
(const char*,label)(int*,API_RO(config_flags)),
R"(Create a new ReaImGui context. The context will remain valid as long as it is used in each defer cycle.

The label is used for the tab text when windows are docked in REAPER and also as a unique identifier for storing settings.

Default values: config_flags = ImGui_ConfigFlags_None)",
{
  const int flags { valueOr(API_RO(config_flags), ImGuiConfigFlags_None) };
  return new Context { label, flags };
});

DEFINE_API(__LINE__, void, DestroyContext, (ImGui_Context*,ctx),
R"(Close and free the resources used by a context.)",
{
  assertValid(ctx);
  delete ctx;
});

DEFINE_API(__LINE__, int, GetConfigFlags, (ImGui_Context*,ctx),
"See ImGui_SetConfigFlags, ImGui_ConfigFlags_*.",
{
  assertValid(ctx);
  return ctx->userConfigFlags();
});

DEFINE_API(__LINE__, void, SetConfigFlags, (ImGui_Context*,ctx)
(int,flags),
"See ImGui_GetConfigFlags, ImGui_ConfigFlags_*.",
{
  assertValid(ctx);
  ctx->setUserConfigFlags(flags);
});

DEFINE_API(__LINE__, double, GetTime, (ImGui_Context*,ctx),
"Get global imgui time. Incremented every frame.",
{
  FRAME_GUARD;
  return ImGui::GetTime();
});

DEFINE_API(__LINE__, double, GetDeltaTime, (ImGui_Context*,ctx),
"Time elapsed since last frame, in seconds.",
{
  FRAME_GUARD;
  return ctx->IO().DeltaTime;
});

DEFINE_API(__LINE__, int, GetFrameCount, (ImGui_Context*,ctx),
"Get global imgui frame count. incremented by 1 every frame.",
{
  FRAME_GUARD;
  return ImGui::GetFrameCount();
});

// ImGuiConfigFlags
DEFINE_ENUM(ImGui, ConfigFlags_None,                 "Flags for ImGui_SetConfigFlags.");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableKeyboard,    "Master keyboard navigation enable flag.");
// DEFINE_ENUM(ImGui, ConfigFlags_NavEnableGamepad,     "Master gamepad navigation enable flag. This is mostly to instruct your imgui backend to fill io.NavInputs[]. Backend also needs to set ImGuiBackendFlags_HasGamepad.");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableSetMousePos, "Instruct navigation to move the mouse cursor.");
// DEFINE_ENUM(ImGui, ConfigFlags_NavNoCaptureKeyboard, "Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouse,              "Instruct imgui to ignore mouse position/buttons.");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouseCursorChange,  "Instruct backend to not alter mouse cursor shape and visibility.");
DEFINE_ENUM(ImGui, ConfigFlags_DockingEnable,        "[BETA] Enable docking functionality.");

DEFINE_ENUM(ReaImGui, ConfigFlags_NoSavedSettings, "Disable state restoration and persistence for the whole context.");
