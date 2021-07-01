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

DEFINE_API(__LINE__, void, Separator, (ImGui_Context*,ctx),
"Separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.",
{
  FRAME_GUARD;
  ImGui::Separator();
});

DEFINE_API(__LINE__, void, SameLine, (ImGui_Context*,ctx)
(double*,API_RO(offset_from_start_x))(double*,API_RO(spacing)),
R"(Call between widgets or groups to layout them horizontally. X position given in window coordinates.

Default values: offset_from_start_x = 0.0, spacing = -1.0.)",
{
  FRAME_GUARD;
  ImGui::SameLine(valueOr(API_RO(offset_from_start_x), 0.f),
    valueOr(API_RO(spacing), -1.0));
});

DEFINE_API(__LINE__, void, NewLine, (ImGui_Context*,ctx),
"Undo a SameLine() or force a new line when in an horizontal-layout context.",
{
  FRAME_GUARD;
  ImGui::NewLine();
});

DEFINE_API(__LINE__, void, Spacing, (ImGui_Context*,ctx),
"Add vertical spacing.",
{
  FRAME_GUARD;
  ImGui::Spacing();
});

DEFINE_API(__LINE__, void, Dummy, (ImGui_Context*,ctx)(double,size_w)(double,size_h),
"Add a dummy item of given size. unlike ImGui_InvisibleButton, Dummy() won't take the mouse click or be navigable into.",
{
  FRAME_GUARD;
  ImGui::Dummy(ImVec2(size_w, size_h));
});

DEFINE_API(__LINE__, void, Indent, (ImGui_Context*,ctx)(double*,API_RO(indent_w)),
R"(Move content position toward the right, by 'indent_w', or ImGui_StyleVar_IndentSpacing if 'indent_w' <= 0

Default values: indent_w = 0.0)",
{
  FRAME_GUARD;
  ImGui::Indent(valueOr(API_RO(indent_w), 0.f));
});

DEFINE_API(__LINE__, void, Unindent, (ImGui_Context*,ctx)(double*,API_RO(indent_w)),
R"(Move content position back to the left, by 'indent_w', or ImGui_StyleVar_IndentSpacing if 'indent_w' <= 0

Default values: indent_w = 0.0)",
{
  FRAME_GUARD;
  ImGui::Unindent(valueOr(API_RO(indent_w), 0.f));
});

DEFINE_API(__LINE__, void, BeginGroup, (ImGui_Context*,ctx),
"Lock horizontal starting position. See ImGui_EndGroup.",
{
  FRAME_GUARD;
  ImGui::BeginGroup();
});

DEFINE_API(__LINE__, void, EndGroup, (ImGui_Context*,ctx),
R"(Unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use ImGui_IsItemHovered or layout primitives such as ImGui_SameLine on whole group, etc.).

See ImGui_BeginGroup.)",
{
  FRAME_GUARD;
  ImGui::EndGroup();
});

DEFINE_API(__LINE__, void, GetCursorPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Cursor position in window",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetCursorPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(__LINE__, double, GetCursorPosX, (ImGui_Context*,ctx),
"Cursor X position in window",
{
  FRAME_GUARD;
  return ImGui::GetCursorPosX();
});

DEFINE_API(__LINE__, double, GetCursorPosY, (ImGui_Context*,ctx),
"Cursor Y position in window",
{
  FRAME_GUARD;
  return ImGui::GetCursorPosY();
});

DEFINE_API(__LINE__, void, SetCursorPos, (ImGui_Context*,ctx)
(double,local_pos_x)(double,local_pos_y),
"Cursor position in window",
{
  FRAME_GUARD;
  ImGui::SetCursorPos(ImVec2(local_pos_x, local_pos_y));
});

DEFINE_API(__LINE__, void, SetCursorPosX, (ImGui_Context*,ctx)
(double,local_x),
"Cursor X position in window",
{
  FRAME_GUARD;
  ImGui::SetCursorPosX(local_x);
});

DEFINE_API(__LINE__, void, SetCursorPosY, (ImGui_Context*,ctx)
(double,local_y),
"Cursor Y position in window",
{
  FRAME_GUARD;
  ImGui::SetCursorPosY(local_y);
});

DEFINE_API(__LINE__, void, GetCursorStartPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Initial cursor position in window coordinates.",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetCursorStartPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(__LINE__, void, GetCursorScreenPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Cursor position in absolute screen coordinates (useful to work with the DrawList API).",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetCursorScreenPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(__LINE__, void, SetCursorScreenPos, (ImGui_Context*,ctx)
(double,pos_x)(double,pos_y),
"Cursor position in absolute screen coordinates.",
{
  FRAME_GUARD;
  ImGui::SetCursorScreenPos(ImVec2(pos_x, pos_y));
});

DEFINE_API(__LINE__, double, GetFrameHeight, (ImGui_Context*,ctx),
"~ ImGui_GetFontSize + ImGui_StyleVar_FramePadding.y * 2",
{
  FRAME_GUARD;
  return ImGui::GetFrameHeight();
});

DEFINE_API(__LINE__, double, GetFrameHeightWithSpacing, (ImGui_Context*,ctx),
"~ ImGui_GetFontSize + ImGui_StyleVar_FramePadding.y * 2 + ImGui_StyleVar_ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)",
{
  FRAME_GUARD;
  return ImGui::GetFrameHeightWithSpacing();
});

DEFINE_API(__LINE__, void, PushClipRect, (ImGui_Context*,ctx)
(double,clip_rect_min_x)(double,clip_rect_min_y)
(double,clip_rect_max_x)(double,clip_rect_max_y)
(bool,intersect_with_current_clip_rect),
"Mouse hovering is affected by PushClipRect() calls, unlike direct calls to ImGui_DrawList_PushClipRect which are render only. See ImGui_PopClipRect.",
{
  FRAME_GUARD;
  ImGui::PushClipRect(
    ImVec2(clip_rect_min_x, clip_rect_min_y),
    ImVec2(clip_rect_max_x, clip_rect_max_y),
    intersect_with_current_clip_rect
  );
});

DEFINE_API(__LINE__, void, PopClipRect, (ImGui_Context*,ctx),
"See ImGui_PushClipRect",
{
  FRAME_GUARD;
  ImGui::PopClipRect();
});

DEFINE_API(__LINE__, bool, IsRectVisible, (ImGui_Context*,ctx)
(double,size_w)(double,size_h),
"Test if rectangle (of given size, starting from cursor position) is visible / not clipped.",
{
  FRAME_GUARD;
  return ImGui::IsRectVisible(ImVec2(size_w, size_h));
});

DEFINE_API(__LINE__, bool, IsRectVisibleEx, (ImGui_Context*,ctx)
(double,rect_min_x)(double,rect_min_y)(double,rect_max_x)(double,rect_max_y),
"Test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.",
{
  FRAME_GUARD;
  return ImGui::IsRectVisible(
    ImVec2(rect_min_x, rect_min_y), ImVec2(rect_max_x, rect_max_y));
});
