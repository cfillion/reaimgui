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

API_SECTION("Button",
R"(Most widgets return true when the value has been changed or when pressed/selected.

You may also use one of the many IsItem* functions (e.g. IsItemActive,
IsItemHovered, etc.) to query widget state.)");

API_FUNC(0_1, bool, Button, (Context*,ctx)
(const char*,label) (RO<double*>,size_w,0.0) (RO<double*>,size_h,0.0),
"")
{
  FRAME_GUARD;
  return ImGui::Button(label, ImVec2(API_GET(size_w), API_GET(size_h)));
}

API_FUNC(0_1, bool, SmallButton, (Context*,ctx)
(const char*,label),
"Button with StyleVar_FramePadding.y == 0 to easily embed within text.")
{
  FRAME_GUARD;
  return ImGui::SmallButton(label);
}

API_FUNC(0_1, bool, InvisibleButton, (Context*,ctx)
(const char*,str_id) (double,size_w) (double,size_h)
(RO<int*>,flags,ImGuiButtonFlags_None),
R"(Flexible button behavior without the visuals, frequently useful to build
custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.).)")
{
  FRAME_GUARD;
  return ImGui::InvisibleButton(str_id, ImVec2(size_w, size_h), API_GET(flags));
}

API_FUNC(0_1, bool, ArrowButton, (Context*,ctx)
(const char*,str_id) (int,dir),
"Square button with an arrow shape. 'dir' is one of the Dir_* values")
{
  FRAME_GUARD;
  IM_ASSERT(dir >= ImGuiDir_None && dir < ImGuiDir_COUNT);
  return ImGui::ArrowButton(str_id, static_cast<ImGuiDir>(dir));
}

API_FUNC(0_1, bool, Checkbox, (Context*,ctx)
(const char*,label) (RW<bool*>,v),
"")
{
  FRAME_GUARD;
  if(!v)
    return false;
  return ImGui::Checkbox(label, v);
}

API_FUNC(0_1, bool, CheckboxFlags, (Context*,ctx)
(const char*,label) (RW<int*>,flags) (int,flags_value),
"")
{
  FRAME_GUARD;
  return ImGui::CheckboxFlags(label, flags, flags_value);
}

API_FUNC(0_1, bool, RadioButton, (Context*,ctx)
(const char*,label) (bool,active),
R"(Use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; })")
{
  FRAME_GUARD;
  return ImGui::RadioButton(label, active);
}

API_FUNC(0_1, bool, RadioButtonEx, (Context*,ctx)
(const char*,label) (RW<int*>,v) (int,v_button),
"Shortcut to handle RadioButton's example pattern when value is an integer")
{
  FRAME_GUARD;
  return ImGui::RadioButton(label, v, v_button);
}

API_FUNC(0_1, void, PushButtonRepeat, (Context*,ctx)
(bool,repeat),
R"(In 'repeat' mode, Button*() functions return repeated true in a typematic
manner (using ConfigVar_KeyRepeatDelay/ConfigVar_KeyRepeatRate settings).

Note that you can call IsItemActive after any Button to tell if the button is
held in the current frame.)")
{
  FRAME_GUARD;
  ImGui::PushButtonRepeat(repeat);
}

API_FUNC(0_1, void, PopButtonRepeat, (Context*,ctx),
"See PushButtonRepeat")
{
  FRAME_GUARD;
  ImGui::PopButtonRepeat();
}

API_SUBSECTION("Flags", "For InvisibleButton.");
API_ENUM(0_1, ImGui, ButtonFlags_None, "");
API_ENUM(0_1, ImGui, ButtonFlags_MouseButtonLeft,
  "React on left mouse button (default).");
API_ENUM(0_1, ImGui, ButtonFlags_MouseButtonRight,
  "React on right mouse button.");
API_ENUM(0_1, ImGui, ButtonFlags_MouseButtonMiddle,
  "React on center mouse button.");

API_SUBSECTION("Cardinal Directions", "For ArrowButton.");
API_ENUM(0_1, ImGui, Dir_None,  "");
API_ENUM(0_1, ImGui, Dir_Left,  "");
API_ENUM(0_1, ImGui, Dir_Right, "");
API_ENUM(0_1, ImGui, Dir_Up,    "");
API_ENUM(0_1, ImGui, Dir_Down,  "");
