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

DEFINE_API(bool, Button, (ImGui_Context*,ctx)
(const char*,label)(double*,API_RO(size_w))(double*,API_RO(size_h)),
R"(Most widgets return true when the value has been changed or when pressed/selected
You may also use one of the many IsItemXXX functions (e.g. ImGui_IsItemActive, ImGui_IsItemHovered, etc.) to query widget state.

Default values: size_w = 0.0, size_h = 0.0)",
{
  FRAME_GUARD;

  ImVec2 size { valueOr(API_RO(size_w), 0.f), valueOr(API_RO(size_h), 0.f) };
  return ImGui::Button(label, size);
});

DEFINE_API(bool, SmallButton, (ImGui_Context*,ctx)
(const char*,label),
"Button with ImGui_StyleVar_FramePadding=(0,0) to easily embed within text.",
{
  FRAME_GUARD;
  return ImGui::SmallButton(label);
});

DEFINE_API(bool, InvisibleButton, (ImGui_Context*,ctx)
(const char*,str_id)(double,size_w)(double,size_h)(int*,API_RO(flags)),
R"(Flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with ImGui_IsItemActive, ImGui_IsItemHovered, etc.).

Default values: flags = ImGui_ButtonFlags_None)",
{
  FRAME_GUARD;
  return ImGui::InvisibleButton(str_id, ImVec2(size_w, size_h),
    valueOr(API_RO(flags), ImGuiButtonFlags_None));
});

DEFINE_API(bool, ArrowButton, (ImGui_Context*,ctx)
(const char*,str_id)(int,dir),
"Square button with an arrow shape.",
{
  FRAME_GUARD;
  return ImGui::ArrowButton(str_id, dir);
});

DEFINE_API(bool, Checkbox, (ImGui_Context*,ctx)
(const char*,label)(bool*,API_RW(v)),
"",
{
  FRAME_GUARD;
  if(!API_RW(v))
    return false;
  return ImGui::Checkbox(label, API_RW(v));
});

DEFINE_API(bool, CheckboxFlags, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(flags))(int,flags_value),
"",
{
  FRAME_GUARD;
  return ImGui::CheckboxFlags(label, API_RW(flags), flags_value);
});

DEFINE_API(bool, RadioButton, (ImGui_Context*,ctx)
(const char*,label)(bool,active),
R"(Use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; })",
{
  FRAME_GUARD;
  return ImGui::RadioButton(label, active);
});

DEFINE_API(bool, RadioButtonEx, (ImGui_Context*,ctx)
(const char*,label)(int*,API_RW(v))(int,v_button),
"Shortcut to handle RadioButton's example pattern when value is an integer",
{
  FRAME_GUARD;
  return ImGui::RadioButton(label, API_RW(v), v_button);
});

DEFINE_API(void, PushButtonRepeat, (ImGui_Context*,ctx)
(bool,repeat),
"In 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call ImGui_IsItemActive after any ImGui_Button to tell if the button is held in the current frame.",
{
  FRAME_GUARD;
  ImGui::PushButtonRepeat(repeat);
});

DEFINE_API(void, PopButtonRepeat, (ImGui_Context*,ctx),
"See ImGui_PushButtonRepeat",
{
  FRAME_GUARD;
  ImGui::PopButtonRepeat();
});

// ImGuiButtonFlags
DEFINE_ENUM(ImGui, ButtonFlags_None,              "Flags for ImGui_InvisibleButton.");
DEFINE_ENUM(ImGui, ButtonFlags_MouseButtonLeft,   "React on left mouse button (default).");
DEFINE_ENUM(ImGui, ButtonFlags_MouseButtonRight,  "React on right mouse button.");
DEFINE_ENUM(ImGui, ButtonFlags_MouseButtonMiddle, "React on center mouse button.");
