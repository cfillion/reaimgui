/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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

#include <variant>

API_SECTION("Context");

API_FUNC(0_5, Context*, CreateContext,
(const char*,label) (RO<int*>,config_flags,ImGuiConfigFlags_None),
R"(Create a new ReaImGui context.
The context will remain valid as long as it is used in each defer cycle.

The label is used for the tab text when windows are docked in REAPER
and also as a unique identifier for storing settings.)")
{
  return new Context {label, API_GET(config_flags)};
}

API_FUNC(0_1, double, GetTime, (Context*,ctx),
"Get global imgui time. Incremented every frame.")
{
  FRAME_GUARD;
  return ImGui::GetTime();
}

API_FUNC(0_1, double, GetDeltaTime, (Context*,ctx),
"Time elapsed since last frame, in seconds.")
{
  FRAME_GUARD;
  return ctx->IO().DeltaTime;
}

API_FUNC(0_1, int, GetFrameCount, (Context*,ctx),
"Get global imgui frame count. incremented by 1 every frame.")
{
  FRAME_GUARD;
  return ImGui::GetFrameCount();
}

API_FUNC(0_8, double, GetFramerate, (Context*,ctx),
R"(Estimate of application framerate (rolling average over 60 frames, based on
GetDeltaTime), in frame per second. Solely for convenience.)")
{
  FRAME_GUARD;
  return ctx->IO().Framerate;
}

API_FUNC(0_8, void, Attach, (Context*,ctx) (Resource*,obj),
R"(Link the object's lifetime to the given context.
Objects can be draw list splitters, fonts, images, list clippers, etc.
Call Detach to let the object be garbage-collected after unuse again.

List clipper objects may only be attached to the context they were created for.

Fonts are (currently) a special case: they must be attached to the context
before usage. Furthermore, fonts may only be attached or detached immediately
after the context is created or before any other function calls modifying the
context per defer cycle. See "limitations" in the font API documentation.)")
{
  assertValid(ctx);
  assertValid(obj);
  ctx->attach(obj);
}

API_FUNC(0_8, void, Detach, (Context*,ctx) (Resource*,obj),
R"(Unlink the object's lifetime. Unattached objects are automatically destroyed
when left unused. You may check whether an object has been destroyed using
ValidatePtr.)")
{
  assertValid(ctx);
  assertValid(obj);
  ctx->detach(obj);
}

API_SUBSECTION("Options");

template<typename... T>
using IOFields = std::variant<T ImGuiIO::*..., T ImGuiStyle::*...>;

// expose most settings from ImGuiIO
// ITEM ORDER MUST MATCH WITH THE API_CONFIGVAR() BELOW!
static constexpr IOFields<bool, float, int> g_configVars[] {
  &ImGuiIO::ConfigFlags,

  &ImGuiIO::MouseDoubleClickTime,
  &ImGuiIO::MouseDoubleClickMaxDist,
  &ImGuiIO::MouseDragThreshold,
  &ImGuiIO::KeyRepeatDelay,
  &ImGuiIO::KeyRepeatRate,

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

  &ImGuiIO::ConfigDebugBeginReturnValueOnce,
  &ImGuiIO::ConfigDebugBeginReturnValueLoop,

  &ImGuiStyle::HoverStationaryDelay,
  &ImGuiStyle::HoverDelayShort,
  &ImGuiStyle::HoverDelayNormal,
  &ImGuiStyle::HoverFlagsForTooltipMouse,
  &ImGuiStyle::HoverFlagsForTooltipNav,
};

#define API_CONFIGVAR(vernum, name, doc) \
  API_FUNC(vernum, int, ConfigVar##_##name, API_NO_ARGS, doc) \
    { return __COUNTER__ - baseConfigVar - 1; }

constexpr int baseConfigVar {__COUNTER__};
API_CONFIGVAR(0_7, Flags, "ConfigFlags_*");

API_CONFIGVAR(0_7, MouseDoubleClickTime, "Time for a double-click, in seconds.");
API_CONFIGVAR(0_7, MouseDoubleClickMaxDist,
  "Distance threshold to stay in to validate a double-click, in pixels.");
API_CONFIGVAR(0_7, MouseDragThreshold,
  "Distance threshold before considering we are dragging.");
API_CONFIGVAR(0_7, KeyRepeatDelay,
R"(When holding a key/button, time before it starts repeating, in seconds
   (for buttons in Repeat mode, etc.).)");
API_CONFIGVAR(0_7, KeyRepeatRate,
  "When holding a key/button, rate at which it repeats, in seconds.");

API_CONFIGVAR(0_7, DockingNoSplit,
R"(Simplified docking mode: disable window splitting, so docking is limited to
   merging multiple windows together into tab-bars.)");
API_CONFIGVAR(0_7, DockingWithShift,
R"(Enable docking with holding Shift key
   (reduce visual noise, allows dropping in wider space)");
API_CONFIGVAR(0_7, DockingTransparentPayload,
R"(Make window or viewport transparent when docking and only display docking
   boxes on the target viewport.)");

API_CONFIGVAR(0_7, ViewportsNoDecoration,
R"(Disable default OS window decoration. Enabling decoration can create
   subsequent issues at OS levels (e.g. minimum window size).)");

API_CONFIGVAR(0_7, MacOSXBehaviors,
R"(Enabled by default on macOS. Swap Cmd<>Ctrl keys, OS X style text editing
   cursor movement using Alt instead of Ctrl, Shortcuts using Cmd/Super instead
   of Ctrl, Line/Text Start and End using Cmd+Arrows instead of Home/End,
   Double click selects by word instead of selecting whole text, Multi-selection
   in lists uses Cmd/Super instead of Ctrl.)");
API_CONFIGVAR(0_7, InputTrickleEventQueue,
R"(Enable input queue trickling: some types of events submitted during the same
   frame (e.g. button down + up) will be spread over multiple frames, improving
   interactions with low framerates.

   Warning: when this option is disabled mouse clicks and key presses faster
   than a frame will be lost.
   This affects accessiblity features and some input devices.)");
API_CONFIGVAR(0_7, InputTextCursorBlink,
  "Enable blinking cursor (optional as some users consider it to be distracting).");
API_CONFIGVAR(0_8, InputTextEnterKeepActive,
  "Pressing Enter will keep item active and select contents (single-line only).");
API_CONFIGVAR(0_7, DragClickToInputText,
R"(Enable turning Drag* widgets into text input with a simple mouse
   click-release (without moving). Not desirable on devices without a keyboard.)");
API_CONFIGVAR(0_7, WindowsResizeFromEdges,
  "Enable resizing of windows from their edges and from the lower-left corner.");
API_CONFIGVAR(0_7, WindowsMoveFromTitleBarOnly,
R"(Enable allowing to move windows only when clicking on their title bar.
   Does not apply to windows without a title bar.)");

API_CONFIGVAR(0_8_5, DebugBeginReturnValueOnce,
R"(First-time calls to Begin()/BeginChild() will return false.
**Needs to be set at context startup time** if you don't want to miss windows.)");
API_CONFIGVAR(0_8_5, DebugBeginReturnValueLoop,
R"(Some calls to Begin()/BeginChild() will return false.
Will cycle through window depths then repeat. Suggested use: add
"SetConfigVar(ConfigVar_DebugBeginReturnValueLoop(), GetKeyMods() == Mod_Shift"
in your main loop then occasionally press SHIFT.
Windows should be flickering while running.)");

API_CONFIGVAR(0_9, HoverStationaryDelay,
R"(Delay for IsItemHovered(HoveredFlags_Stationary).
   Time required to consider mouse stationary.)");
API_CONFIGVAR(0_8, HoverDelayShort,
R"(Delay for IsItemHovered(HoveredFlags_DelayShort).
   Usually used along with ConfigVar_HoverStationaryDelay.)");
API_CONFIGVAR(0_8, HoverDelayNormal,
R"(Delay for IsItemHovered(HoveredFlags_DelayNormal).
   Usually used along with ConfigVar_HoverStationaryDelay.)");
API_CONFIGVAR(0_9, HoverFlagsForTooltipMouse,
R"(Default flags when using IsItemHovered(HoveredFlags_ForTooltip) or
   BeginItemTooltip()/SetItemTooltip() while using mouse.)");
API_CONFIGVAR(0_9, HoverFlagsForTooltipNav,
R"(Default flags when using IsItemHovered(HoveredFlags_ForTooltip) or
   BeginItemTooltip()/SetItemTooltip() while using keyboard/gamepad.)");

static_assert(__COUNTER__ - baseConfigVar - 1 == std::size(g_configVars),
  "forgot to API_CONFIGVAR() a config var?");

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

API_FUNC(0_7, double, GetConfigVar, (Context*,ctx)
(int,var_idx),
"")
{
  assertValid(ctx);
  if(static_cast<size_t>(var_idx) >= std::size(g_configVars))
    throw reascript_error {"unknown config variable"};

  return std::visit(overloaded {
    [ctx](auto ImGuiIO::*field) -> double {
      const ImGuiIO &io {ctx->IO()};

      if constexpr(std::is_same_v<decltype(ImGuiIO::ConfigFlags),
                   std::decay_t<decltype(io.*field)>>) {
        if(field == &ImGuiIO::ConfigFlags)
          return ctx->userConfigFlags();
      }

      return io.*field;
    },
    [ctx](auto ImGuiStyle::*field) -> double { return ctx->style().*field; },
  }, g_configVars[var_idx]);
}

API_FUNC(0_7, void, SetConfigVar, (Context*,ctx)
(int,var_idx) (double,value),
"")
{
  assertValid(ctx);
  if(static_cast<size_t>(var_idx) >= std::size(g_configVars))
    throw reascript_error {"unknown config variable"};

  std::visit(overloaded {
    [ctx, value](auto ImGuiIO::*field) {
      ImGuiIO &io {ctx->IO()};

      if constexpr(std::is_same_v<decltype(ImGuiIO::ConfigFlags),
                   std::decay_t<decltype(io.*field)>>) {
        if(field == &ImGuiIO::ConfigFlags) {
          ctx->setUserConfigFlags(value);
          return;
        }
      }

      io.*field = value;
    },
    [ctx, value](auto ImGuiStyle::*field) { ctx->style().*field = value; },
  }, g_configVars[var_idx]);
}

API_SUBSECTION("Flags", "For CreateContext and SetConfigVar(ConfigVar_Flags()).");
API_ENUM(0_1, ImGui, ConfigFlags_None, "");
API_ENUM(0_1, ImGui, ConfigFlags_NavEnableKeyboard,
R"(Master keyboard navigation enable flag.
   Enable full Tabbing + directional arrows + space/enter to activate.)");
// API_ENUM(ImGui, ConfigFlags_NavEnableGamepad,
//"Master gamepad navigation enable flag.");
API_ENUM(0_1, ImGui, ConfigFlags_NavEnableSetMousePos,
  "Instruct navigation to move the mouse cursor.");
API_ENUM(0_8, ImGui, ConfigFlags_NavNoCaptureKeyboard,
R"(Instruct navigation to not capture global keyboard input when
   ConfigFlags_NavEnableKeyboard is set (see SetNextFrameWantCaptureKeyboard).)");
API_ENUM(0_1, ImGui, ConfigFlags_NoMouse,
  "Instruct dear imgui to disable mouse inputs and interactions");
API_ENUM(0_1, ImGui, ConfigFlags_NoMouseCursorChange,
  "Instruct backend to not alter mouse cursor shape and visibility.");
API_ENUM(0_5, ImGui, ConfigFlags_DockingEnable, "Enable docking functionality.");
API_ENUM(0_9_2, ImGui, ConfigFlags_NoKeyboard,
R"(Instruct dear imgui to disable keyboard inputs and interactions.
This is done by ignoring keyboard events and clearing existing states.)");

API_ENUM(0_4, ReaImGui, ConfigFlags_NoSavedSettings,
  "Disable state restoration and persistence for the whole context.");
