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

#include "color.hpp"

DEFINE_API(void, Text, (ImGui_Context*,ctx)
(const char*,text),
"",
{
  FRAME_GUARD;
  ImGui::TextUnformatted(text);
});

DEFINE_API(void, TextColored, (ImGui_Context*,ctx)
(int,col_rgba)(const char*,text),
"Shortcut for ImGui_PushStyleColor(ImGui_Col_Text, color); ImGui_Text(text); ImGui_PopStyleColor();",
{
  FRAME_GUARD;

  ImVec4 color { Color(col_rgba) };
  ImGui::PushStyleColor(ImGuiCol_Text, color);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextDisabled, (ImGui_Context*,ctx)
(const char*,text),
"",
{
  FRAME_GUARD;
  const ImGuiStyle &style { ImGui::GetStyle() };
  ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
});

DEFINE_API(void, TextWrapped, (ImGui_Context*,ctx)
(const char*,text),
"Shortcut for ImGui_PushTextWrapPos(0.0); ImGui_Text(text); ImGui_PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using ImGui_SetNextWindowSize.",
{
  FRAME_GUARD;
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextUnformatted(text);
  ImGui::PopTextWrapPos();
});

DEFINE_API(void, LabelText, (ImGui_Context*,ctx)
(const char*,label)(const char*,text),
"Display text+label aligned the same way as value+label widgets",
{
  FRAME_GUARD;
  ImGui::LabelText(label, "%s", text);
});

DEFINE_API(void, Bullet, (ImGui_Context*,ctx),
"Draw a small circle + keep the cursor on the same line. Advance cursor x position by ImGui_GetTreeNodeToLabelSpacing, same distance that ImGui_TreeNode uses.",
{
  FRAME_GUARD;
  ImGui::Bullet();
});

DEFINE_API(void, BulletText, (ImGui_Context*,ctx)
(const char*,text),
"Shortcut for ImGui_Bullet + ImGui_Text.",
{
  FRAME_GUARD;
  ImGui::Bullet();
  ImGui::TextUnformatted(text);
});

DEFINE_API(void, PushTextWrapPos, (ImGui_Context*,ctx)
(double*,API_RO(wrap_local_pos_x)),
R"(Push word-wrapping position for Text*() commands. < 0.0: no wrapping; 0.0: wrap to end of window (or column); > 0.0: wrap at 'wrap_pos_x' position in window local space.

Default values: wrap_local_pos_x = 0.0)",
{
  FRAME_GUARD;
  ImGui::PushTextWrapPos(valueOr(API_RO(wrap_local_pos_x), 0.f));
});

DEFINE_API(void, PopTextWrapPos, (ImGui_Context*,ctx),
"",
{
  FRAME_GUARD;
  ImGui::PopTextWrapPos();
});

DEFINE_API(void, AlignTextToFramePadding, (ImGui_Context*,ctx),
"Vertically align upcoming text baseline to ImGui_StyleVar_FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)",
{
  FRAME_GUARD;
  ImGui::AlignTextToFramePadding();
});

DEFINE_API(double, GetTextLineHeight, (ImGui_Context*,ctx),
"Same as ImGui_GetFontSize",
{
  FRAME_GUARD;
  return ImGui::GetTextLineHeight();
});

DEFINE_API(double, GetTextLineHeightWithSpacing, (ImGui_Context*,ctx),
"~ ImGui_GetFontSize + ImGui_StyleVar_ItemSpacing.y (distance in pixels between 2 consecutive lines of text)",
{
  FRAME_GUARD;
  return ImGui::GetTextLineHeightWithSpacing();
});

DEFINE_API(void, CalcTextSize, (ImGui_Context*,ctx)
(const char*,text)(double*,API_W(w))(double*,API_W(h))
(bool*,API_RO(hide_text_after_double_hash))(double*,API_RO(wrap_width)),
"Default values: hide_text_after_double_hash = false, wrap_width = -1.0",
{
  FRAME_GUARD;
  const ImVec2 &size {
    ImGui::CalcTextSize(text, nullptr,
      valueOr(API_RO(hide_text_after_double_hash), false),
      valueOr(API_RO(wrap_width), -1.0))
  };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
});

DEFINE_API(void, DebugTextEncoding, (ImGui_Context*,ctx)
(const char*,text),
"Helper tool to diagnose between text encoding issues and font loading issues. Pass your UTF-8 string and verify that there are correct.",
{
  FRAME_GUARD;
  ImGui::DebugTextEncoding(text);
});
