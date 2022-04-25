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

// expose most settings from ImGuiIO
enum ConfigVar {
  ReaImGuiConfigVar_Flags,

  ReaImGuiConfigVar_MouseDoubleClickTime,
  ReaImGuiConfigVar_MouseDoubleClickMaxDist,
  ReaImGuiConfigVar_MouseDragThreshold,
  // ReaImGuiConfigVar_KeyRepeatDelay,
  // ReaImGuiConfigVar_KeyRepeatRate,

  // ReaImGuiConfigVar_FontGlobalScale,
  // ReaImGuiConfigVar_FontAllowUserScaling,

  ReaImGuiConfigVar_DockingNoSplit,
  ReaImGuiConfigVar_DockingWithShift,
  // ReaImGuiConfigVar_DockingAlwaysTabBar,
  // ReaImGuiConfigVar_DockingTransparentPayload,

  // ReaImGuiConfigVar_ViewportsNoAutoMerge,
  // ReaImGuiConfigVar_ViewportsNoTaskBarIcon,
  ReaImGuiConfigVar_ViewportsNoDecoration,
  // ReaImGuiConfigVar_ViewportsNoDefaultParent,

  ReaImGuiConfigVar_MacOSXBehaviors,
  ReaImGuiConfigVar_InputTrickleEventQueue,
  ReaImGuiConfigVar_InputTextCursorBlink,
  ReaImGuiConfigVar_DragClickToInputText,
  ReaImGuiConfigVar_WindowsResizeFromEdges,
  ReaImGuiConfigVar_WindowsMoveFromTitleBarOnly,
};

DEFINE_API(double, GetConfigVar, (ImGui_Context*,ctx)
(int,var_idx),
"See ImGui_SetConfigVar, ImGui_ConfigVar_*.",
{
  assertValid(ctx);
  const ImGuiIO &io { ctx->IO() };

  switch(static_cast<ConfigVar>(var_idx)) {
  case ReaImGuiConfigVar_Flags:
    return ctx->userConfigFlags();
  case ReaImGuiConfigVar_MouseDoubleClickTime:
    return io.MouseDoubleClickTime;
  case ReaImGuiConfigVar_MouseDoubleClickMaxDist:
    return io.MouseDoubleClickMaxDist;
  case ReaImGuiConfigVar_MouseDragThreshold:
    return io.MouseDragThreshold;
  case ReaImGuiConfigVar_DockingNoSplit:
    return io.ConfigDockingNoSplit;
  case ReaImGuiConfigVar_DockingWithShift:
    return io.ConfigDockingWithShift;
  case ReaImGuiConfigVar_ViewportsNoDecoration:
    return io.ConfigViewportsNoDecoration;
  case ReaImGuiConfigVar_MacOSXBehaviors:
    return io.ConfigMacOSXBehaviors;
  case ReaImGuiConfigVar_InputTrickleEventQueue:
    return io.ConfigInputTrickleEventQueue;
  case ReaImGuiConfigVar_InputTextCursorBlink:
    return io.ConfigInputTextCursorBlink;
  case ReaImGuiConfigVar_DragClickToInputText:
    return io.ConfigDragClickToInputText;
  case ReaImGuiConfigVar_WindowsResizeFromEdges:
    return io.ConfigWindowsResizeFromEdges;
  case ReaImGuiConfigVar_WindowsMoveFromTitleBarOnly:
    return io.ConfigWindowsMoveFromTitleBarOnly;
  }

  throw reascript_error { "unknown config variable" };
});

DEFINE_API(void, SetConfigVar, (ImGui_Context*,ctx)
(int,var_idx)(double,value),
"See ImGui_GetConfigVar, ImGui_ConfigVar_*.",
{
  assertValid(ctx);
  ImGuiIO &io { ctx->IO() };

  switch(static_cast<ConfigVar>(var_idx)) {
  case ReaImGuiConfigVar_Flags:
    ctx->setUserConfigFlags(value);               return;
  case ReaImGuiConfigVar_MouseDoubleClickTime:
    io.MouseDoubleClickTime              = value; return;
  case ReaImGuiConfigVar_MouseDoubleClickMaxDist:
    io.MouseDoubleClickMaxDist           = value; return;
  case ReaImGuiConfigVar_MouseDragThreshold:
    io.MouseDragThreshold                = value; return;
  case ReaImGuiConfigVar_DockingNoSplit:
    io.ConfigDockingNoSplit              = value; return;
  case ReaImGuiConfigVar_DockingWithShift:
    io.ConfigDockingWithShift            = value; return;
  case ReaImGuiConfigVar_ViewportsNoDecoration:
    io.ConfigViewportsNoDecoration       = value; return;
  case ReaImGuiConfigVar_MacOSXBehaviors:
    io.ConfigMacOSXBehaviors             = value; return;
  case ReaImGuiConfigVar_InputTrickleEventQueue:
    io.ConfigInputTrickleEventQueue      = value; return;
  case ReaImGuiConfigVar_InputTextCursorBlink:
    io.ConfigInputTextCursorBlink        = value; return;
  case ReaImGuiConfigVar_DragClickToInputText:
    io.ConfigDragClickToInputText        = value; return;
  case ReaImGuiConfigVar_WindowsResizeFromEdges:
    io.ConfigWindowsResizeFromEdges      = value; return;
  case ReaImGuiConfigVar_WindowsMoveFromTitleBarOnly:
    io.ConfigWindowsMoveFromTitleBarOnly = value; return;
  }

  throw reascript_error { "unknown config variable" };
});

DEFINE_ENUM(ReaImGui, ConfigVar_Flags,                       "ImGui_ConfigFlags_*");
DEFINE_ENUM(ReaImGui, ConfigVar_MouseDoubleClickTime,        "Time for a double-click, in seconds.");
DEFINE_ENUM(ReaImGui, ConfigVar_MouseDoubleClickMaxDist,     "Distance threshold to stay in to validate a double-click, in pixels.");
DEFINE_ENUM(ReaImGui, ConfigVar_MouseDragThreshold,          "Distance threshold before considering we are dragging.");
DEFINE_ENUM(ReaImGui, ConfigVar_DockingNoSplit,              "Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.");
DEFINE_ENUM(ReaImGui, ConfigVar_DockingWithShift,            "Enable docking with holding Shift key (reduce visual noise, allows dropping in wider space)");
DEFINE_ENUM(ReaImGui, ConfigVar_ViewportsNoDecoration,       "Disable default OS window decoration. Enabling decoration can create subsequent issues at OS levels (e.g. minimum window size).");
DEFINE_ENUM(ReaImGui, ConfigVar_MacOSXBehaviors,             "OS X style: Text editing cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End, Double click selects by word instead of selecting whole text, Multi-selection in lists uses Cmd/Super instead of Ctrl.");
DEFINE_ENUM(ReaImGui, ConfigVar_InputTrickleEventQueue,      "Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.");
DEFINE_ENUM(ReaImGui, ConfigVar_InputTextCursorBlink,        "Enable blinking cursor (optional as some users consider it to be distracting).");
DEFINE_ENUM(ReaImGui, ConfigVar_DragClickToInputText,        "Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving). Not desirable on devices without a keyboard.");
DEFINE_ENUM(ReaImGui, ConfigVar_WindowsResizeFromEdges,      "Enable resizing of windows from their edges and from the lower-left corner.");
DEFINE_ENUM(ReaImGui, ConfigVar_WindowsMoveFromTitleBarOnly, "Enable allowing to move windows only when clicking on their title bar. Does not apply to windows without a title bar.");

// ImGuiConfigFlags
DEFINE_ENUM(ImGui, ConfigFlags_None,                 "Flags for ImGui_CreateContext and ImGui_SetConfigVar(ImGui_ConfigVar_Flags()).");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableKeyboard,    "Master keyboard navigation enable flag.");
// DEFINE_ENUM(ImGui, ConfigFlags_NavEnableGamepad,     "Master gamepad navigation enable flag. This is mostly to instruct your imgui backend to fill io.NavInputs[]. Backend also needs to set ImGuiBackendFlags_HasGamepad.");
DEFINE_ENUM(ImGui, ConfigFlags_NavEnableSetMousePos, "Instruct navigation to move the mouse cursor.");
// DEFINE_ENUM(ImGui, ConfigFlags_NavNoCaptureKeyboard, "Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouse,              "Instruct imgui to ignore mouse position/buttons.");
DEFINE_ENUM(ImGui, ConfigFlags_NoMouseCursorChange,  "Instruct backend to not alter mouse cursor shape and visibility.");
DEFINE_ENUM(ImGui, ConfigFlags_DockingEnable,        "[BETA] Enable docking functionality.");

DEFINE_ENUM(ReaImGui, ConfigFlags_NoSavedSettings, "Disable state restoration and persistence for the whole context.");
