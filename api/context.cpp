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
#include "variant.hpp"

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
R"(Free the resources used by a context. Calling this function is usually not required as all ReaImGui objects are automatically garbage-collected when unused.)",
{
  assertValid(ctx);
  delete ctx;
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

DEFINE_API(double, GetFramerate, (ImGui_Context*,ctx),
"Estimate of application framerate (rolling average over 60 frames, based on ImGui_GetDeltaTime), in frame per second. Solely for convenience.",
{
  FRAME_GUARD;
  return ctx->IO().Framerate;
});

// expose most settings from ImGuiIO
// ITEM ORDER MUST MATCH WITH THE DEFINE_CONFIGVAR() BELOW!
static const std::variant<
  bool  ImGuiIO::*,
  float ImGuiIO::*,
  int   ImGuiIO::*
> g_configVars[] {
  &ImGuiIO::ConfigFlags,

  &ImGuiIO::MouseDoubleClickTime,
  &ImGuiIO::MouseDoubleClickMaxDist,
  &ImGuiIO::MouseDragThreshold,
  &ImGuiIO::KeyRepeatDelay,
  &ImGuiIO::KeyRepeatRate,
  &ImGuiIO::HoverDelayNormal,
  &ImGuiIO::HoverDelayShort,

  // &ImGuiIO::FontGlobalScale,
  // &ImGuiIO::FontAllowUserScaling,

  &ImGuiIO::ConfigDockingNoSplit,
  &ImGuiIO::ConfigDockingWithShift,
  // &ImGuiIO::ConfigDockingAlwaysTabBar,
  &ImGuiIO::ConfigDockingTransparentPayload,

  // &ImGuiIO::ConfigViewportsNoAutoMerge,
  // &ImGuiIO::ConfigViewportsNoTaskBarIcon,
  &ImGuiIO::ConfigViewportsNoDecoration,
  // &ImGuiIO::ConfigViewportsNoDefaultParent,

  &ImGuiIO::ConfigMacOSXBehaviors,
  &ImGuiIO::ConfigInputTrickleEventQueue,
  &ImGuiIO::ConfigInputTextCursorBlink,
  &ImGuiIO::ConfigInputTextEnterKeepActive,
  &ImGuiIO::ConfigDragClickToInputText,
  &ImGuiIO::ConfigWindowsResizeFromEdges,
  &ImGuiIO::ConfigWindowsMoveFromTitleBarOnly,
};

#define DEFINE_CONFIGVAR(name, doc) \
  DEFINE_API(int, ConfigVar##_##name, NO_ARGS, doc, \
    { return __COUNTER__ - baseConfigVar - 1; })

constexpr int baseConfigVar { __COUNTER__ };
DEFINE_CONFIGVAR(Flags,                       "ImGui_ConfigFlags_*");

DEFINE_CONFIGVAR(MouseDoubleClickTime,        "Time for a double-click, in seconds.");
DEFINE_CONFIGVAR(MouseDoubleClickMaxDist,     "Distance threshold to stay in to validate a double-click, in pixels.");
DEFINE_CONFIGVAR(MouseDragThreshold,          "Distance threshold before considering we are dragging.");
DEFINE_CONFIGVAR(KeyRepeatDelay,              "When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).");
DEFINE_CONFIGVAR(KeyRepeatRate,               "When holding a key/button, rate at which it repeats, in seconds.");
DEFINE_CONFIGVAR(HoverDelayNormal,            "Delay on hovering before ImGui_IsItemHovered(ImGui_HoveredFlags_DelayNormal) returns true.");
DEFINE_CONFIGVAR(HoverDelayShort,             "Delay on hovering before ImGui_IsItemHovered(ImGui_HoveredFlags_DelayShort) returns true.");

DEFINE_CONFIGVAR(DockingNoSplit,              "Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.");
DEFINE_CONFIGVAR(DockingWithShift,            "Enable docking with holding Shift key (reduce visual noise, allows dropping in wider space)");
DEFINE_CONFIGVAR(DockingTransparentPayload,   "Make window or viewport transparent when docking and only display docking boxes on the target viewport.");

DEFINE_CONFIGVAR(ViewportsNoDecoration,       "Disable default OS window decoration. Enabling decoration can create subsequent issues at OS levels (e.g. minimum window size).");

DEFINE_CONFIGVAR(MacOSXBehaviors,             "OS X style: Text editing cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End, Double click selects by word instead of selecting whole text, Multi-selection in lists uses Cmd/Super instead of Ctrl.");
DEFINE_CONFIGVAR(InputTrickleEventQueue,      "Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.");
DEFINE_CONFIGVAR(InputTextCursorBlink,        "Enable blinking cursor (optional as some users consider it to be distracting).");
DEFINE_CONFIGVAR(InputTextEnterKeepActive,    "Pressing Enter will keep item active and select contents (single-line only).");
DEFINE_CONFIGVAR(DragClickToInputText,        "Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving). Not desirable on devices without a keyboard.");
DEFINE_CONFIGVAR(WindowsResizeFromEdges,      "Enable resizing of windows from their edges and from the lower-left corner.");
DEFINE_CONFIGVAR(WindowsMoveFromTitleBarOnly, "Enable allowing to move windows only when clicking on their title bar. Does not apply to windows without a title bar.");

static_assert(__COUNTER__ - baseConfigVar - 1 == std::size(g_configVars),
  "forgot to DEFINE_CONFIGVAR() a config var?");

DEFINE_API(double, GetConfigVar, (ImGui_Context*,ctx)
(int,var_idx),
"See ImGui_SetConfigVar, ImGui_ConfigVar_*.",
{
  assertValid(ctx);
  if(static_cast<size_t>(var_idx) >= std::size(g_configVars))
    throw reascript_error { "unknown config variable" };

  return std::visit([ctx](auto ImGuiIO::*field) -> double {
    const ImGuiIO &io { ctx->IO() };

    if constexpr (std::is_same_v<decltype(ImGuiIO::ConfigFlags),
                                 std::decay_t<decltype(io.*field)>>) {
      if(field == &ImGuiIO::ConfigFlags)
        return ctx->userConfigFlags();
    }

    return io.*field;
  }, g_configVars[var_idx]);
});

DEFINE_API(void, SetConfigVar, (ImGui_Context*,ctx)
(int,var_idx)(double,value),
"See ImGui_GetConfigVar, ImGui_ConfigVar_*.",
{
  assertValid(ctx);
  if(static_cast<size_t>(var_idx) >= std::size(g_configVars))
    throw reascript_error { "unknown config variable" };

  std::visit([ctx, value](auto ImGuiIO::*field) {
    ImGuiIO &io { ctx->IO() };

    if constexpr (std::is_same_v<decltype(ImGuiIO::ConfigFlags),
                                 std::decay_t<decltype(io.*field)>>) {
      if(field == &ImGuiIO::ConfigFlags) {
        ctx->setUserConfigFlags(value);
        return;
      }
    }

    io.*field = value;
  }, g_configVars[var_idx]);
});

// ImGuiConfigFlags
DEFINE_ENUM(ImGui, ConfigFlags_None,                 "Flags for ImGui_CreateContext and ImGui_SetConfigVar(ImGui_ConfigVar_Flags()).");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableKeyboard,    "Master keyboard navigation enable flag.");
// DEFINE_ENUM(ImGui, ConfigFlags_NavEnableGamepad,     "Master gamepad navigation enable flag.");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableSetMousePos, "Instruct navigation to move the mouse cursor.");
DEFINE_ENUM(ImGui, ConfigFlags_NavNoCaptureKeyboard, "Instruct navigation to not capture global keyboard input when ImGui_ConfigFlags_NavEnableKeyboard is set (see ImGui_SetNextFrameWantCaptureKeyboard).");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouse,              "Instruct imgui to ignore mouse position/buttons.");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouseCursorChange,  "Instruct backend to not alter mouse cursor shape and visibility.");
DEFINE_ENUM(ImGui, ConfigFlags_DockingEnable,        "[BETA] Enable docking functionality.");

DEFINE_ENUM(ReaImGui, ConfigFlags_NoSavedSettings, "Disable state restoration and persistence for the whole context.");
