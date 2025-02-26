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

#include "../src/color.hpp"

API_SECTION("Text");

API_FUNC(0_1, void, Text, (Context*,ctx)
(const char*,text),
"")
{
  FRAME_GUARD;
  ImGui::TextUnformatted(text);
}

API_FUNC(0_1, void, TextColored, (Context*,ctx)
(int,col_rgba) (const char*,text),
"Shortcut for PushStyleColor(Col_Text, color); Text(text); PopStyleColor();")
{
  FRAME_GUARD;

  ImVec4 color {Color(col_rgba)};
  ImGui::PushStyleColor(ImGuiCol_Text, color);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
}

API_FUNC(0_1, void, TextDisabled, (Context*,ctx)
(const char*,text),
"")
{
  FRAME_GUARD;
  const ImGuiStyle &style {ImGui::GetStyle()};
  ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
}

API_FUNC(0_1, void, TextWrapped, (Context*,ctx)
(const char*,text),
R"(Shortcut for PushTextWrapPos(0.0); Text(text); PopTextWrapPos();.
Note that this won't work on an auto-resizing window if there's no other
widgets to extend the window width, yoy may need to set a size using
SetNextWindowSize.)")
{
  FRAME_GUARD;
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextUnformatted(text);
  ImGui::PopTextWrapPos();
}

API_FUNC(0_1, void, LabelText, (Context*,ctx)
(const char*,label) (const char*,text),
"Display text+label aligned the same way as value+label widgets")
{
  FRAME_GUARD;
  ImGui::LabelText(label, "%s", text);
}

API_FUNC(0_1, void, Bullet, (Context*,ctx),
R"(Draw a small circle + keep the cursor on the same line.
Advance cursor x position by GetTreeNodeToLabelSpacing,
same distance that TreeNode uses.)")
{
  FRAME_GUARD;
  ImGui::Bullet();
}

API_FUNC(0_1, void, BulletText, (Context*,ctx)
(const char*,text),
"Shortcut for Bullet + Text.")
{
  FRAME_GUARD;
  ImGui::Bullet();
  ImGui::TextUnformatted(text);
}

API_FUNC(0_1, void, PushTextWrapPos, (Context*,ctx)
(RO<double*>,wrap_local_pos_x,0.0),
R"(Push word-wrapping position for Text*() commands.

-  < 0.0: no wrapping
-  = 0.0: wrap to end of window (or column)
- \> 0.0: wrap at 'wrap_pos_x' position in window local space.)")
{
  FRAME_GUARD;
  ImGui::PushTextWrapPos(API_GET(wrap_local_pos_x));
}

API_FUNC(0_1, void, PopTextWrapPos, (Context*,ctx),
"")
{
  FRAME_GUARD;
  ImGui::PopTextWrapPos();
}

API_FUNC(0_1, void, AlignTextToFramePadding, (Context*,ctx),
R"(Vertically align upcoming text baseline to StyleVar_FramePadding.y so that it
will align properly to regularly framed items (call if you have text on a line
before a framed item).)")
{
  FRAME_GUARD;
  ImGui::AlignTextToFramePadding();
}

API_FUNC(0_1, double, GetTextLineHeight, (Context*,ctx),
"Same as GetFontSize")
{
  FRAME_GUARD;
  return ImGui::GetTextLineHeight();
}

API_FUNC(0_1, double, GetTextLineHeightWithSpacing, (Context*,ctx),
R"(GetFontSize + StyleVar_ItemSpacing.y
(distance in pixels between 2 consecutive lines of text).)")
{
  FRAME_GUARD;
  return ImGui::GetTextLineHeightWithSpacing();
}

API_FUNC(0_1, double, GetFrameHeight, (Context*,ctx),
"GetFontSize + StyleVar_FramePadding.y * 2")
{
  FRAME_GUARD;
  return ImGui::GetFrameHeight();
}

API_FUNC(0_1, double, GetFrameHeightWithSpacing, (Context*,ctx),
R"(GetFontSize + StyleVar_FramePadding.y * 2 + StyleVar_ItemSpacing.y
(distance in pixels between 2 consecutive lines of framed widgets).)")
{
  FRAME_GUARD;
  return ImGui::GetFrameHeightWithSpacing();
}

API_FUNC(0_1, void, CalcTextSize, (Context*,ctx)
(const char*,text) (W<double*>,w) (W<double*>,h)
(RO<bool*>,hide_text_after_double_hash,false) (RO<double*>,wrap_width,-1.0),
"")
{
  FRAME_GUARD;
  const ImVec2 &size {
    ImGui::CalcTextSize(text, nullptr,
      API_GET(hide_text_after_double_hash), API_GET(wrap_width))
  };
  if(w) *w = size.x;
  if(h) *h = size.y;
}

API_FUNC(0_7, void, DebugTextEncoding, (Context*,ctx)
(const char*,text),
R"(Helper tool to diagnose between text encoding issues and font loading issues.
Pass your UTF-8 string and verify that there are correct.)")
{
  FRAME_GUARD;
  ImGui::DebugTextEncoding(text);
}
