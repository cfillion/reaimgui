#include "api_helper.hpp"

DEFINE_API(void, Separator, (ImGui_Context*,ctx),
"Separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.",
{
  FRAME_GUARD;
  ImGui::Separator();
});

DEFINE_API(void, SameLine, (ImGui_Context*,ctx)
(double*,API_RO(offset_from_start_x))(double*,API_RO(spacing)),
R"(Call between widgets or groups to layout them horizontally. X position given in window coordinates.

Default values: offset_from_start_x = 0.0, spacing = -1.0.)",
{
  FRAME_GUARD;
  ImGui::SameLine(valueOr(API_RO(offset_from_start_x), 0.0),
    valueOr(API_RO(spacing), -1.0));
});

DEFINE_API(void, NewLine, (ImGui_Context*,ctx),
"Undo a SameLine() or force a new line when in an horizontal-layout context.",
{
  FRAME_GUARD;
  ImGui::NewLine();
});

DEFINE_API(void, Spacing, (ImGui_Context*,ctx),
"Add vertical spacing.",
{
  FRAME_GUARD;
  ImGui::Spacing();
});

DEFINE_API(void, Dummy, (ImGui_Context*,ctx)(double,size_w)(double,size_h),
"Add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.",
{
  FRAME_GUARD;
  ImGui::Dummy(ImVec2(size_w, size_h));
});

DEFINE_API(void, Indent, (ImGui_Context*,ctx)(double*,API_RO(indent_w)),
R"(Move content position toward the right, by 'indent_w', or style.IndentSpacing if 'indent_w' <= 0

Default values: indent_w = 0.0)",
{
  FRAME_GUARD;
  ImGui::Indent(valueOr(API_RO(indent_w), 0.0));
});

DEFINE_API(void, Unindent, (ImGui_Context*,ctx)(double*,API_RO(indent_w)),
R"(Move content position back to the left, by 'indent_w', or style.IndentSpacing if 'indent_w' <= 0

Default values: indent_w = 0.0)",
{
  FRAME_GUARD;
  ImGui::Unindent(valueOr(API_RO(indent_w), 0.0));
});

DEFINE_API(void, BeginGroup, (ImGui_Context*,ctx),
"Lock horizontal starting position. See ImGui_EndGroup.",
{
  FRAME_GUARD;
  ImGui::BeginGroup();
});

DEFINE_API(void, EndGroup, (ImGui_Context*,ctx),
R"(Unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.).

See ImGui_BeginGroup.)",
{
  FRAME_GUARD;
  ImGui::EndGroup();
});

DEFINE_API(void, GetCursorPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Cursor position in window",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetCursorPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(double, GetCursorPosX, (ImGui_Context*,ctx),
"Cursor X position in window",
{
  FRAME_GUARD;
  return ImGui::GetCursorPosX();
});

DEFINE_API(double, GetCursorPosY, (ImGui_Context*,ctx),
"Cursor Y position in window",
{
  FRAME_GUARD;
  return ImGui::GetCursorPosY();
});

DEFINE_API(void, SetCursorPos, (ImGui_Context*,ctx)
(double,local_pos_x)(double,local_pos_y),
"Cursor position in window",
{
  FRAME_GUARD;
  ImGui::SetCursorPos(ImVec2(local_pos_x, local_pos_y));
});

DEFINE_API(void, SetCursorPosX, (ImGui_Context*,ctx)
(double,local_x),
"Cursor X position in window",
{
  FRAME_GUARD;
  ImGui::SetCursorPosX(local_x);
});

DEFINE_API(void, SetCursorPosY, (ImGui_Context*,ctx)
(double,local_y),
"Cursor Y position in window",
{
  FRAME_GUARD;
  ImGui::SetCursorPosY(local_y);
});

DEFINE_API(void, GetCursorStartPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Initial cursor position in window coordinates",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetCursorStartPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(void, GetCursorScreenPos, (ImGui_Context*,ctx)
(double*,API_W(x))(double*,API_W(y)),
"Cursor position in absolute screen coordinates [0..io.DisplaySize] (useful to work with ImDrawList API)",
{
  FRAME_GUARD;
  const ImVec2 &pos { ImGui::GetCursorScreenPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(void, SetCursorScreenPos, (ImGui_Context*,ctx)
(double,pos_x)(double,pos_y),
"Cursor position in absolute screen coordinates [0..io.DisplaySize]",
{
  FRAME_GUARD;
  ImGui::SetCursorScreenPos(ImVec2(pos_x, pos_y));
});

DEFINE_API(void, AlignTextToFramePadding, (ImGui_Context*,ctx),
"Vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)",
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
"~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)",
{
  FRAME_GUARD;
  return ImGui::GetTextLineHeightWithSpacing();
});

DEFINE_API(double, GetFrameHeight, (ImGui_Context*,ctx),
"~ FontSize + style.FramePadding.y * 2",
{
  FRAME_GUARD;
  return ImGui::GetFrameHeight();
});

DEFINE_API(double, GetFrameHeightWithSpacing, (ImGui_Context*,ctx),
"~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)",
{
  FRAME_GUARD;
  return ImGui::GetFrameHeightWithSpacing();
});

// Clipping
DEFINE_API(void, PushClipRect, (ImGui_Context*,ctx)
(double,clip_rect_min_x)(double,clip_rect_min_y)
(double,clip_rect_max_x)(double,clip_rect_max_y)
(bool,intersect_with_current_clip_rect),
"Mouse hovering is affected by ImGui::PushClipRect() calls, unlike direct calls to ImDrawList::PushClipRect() which are render only. See ImGui_PopClipRect.",
{
  FRAME_GUARD;
  ImGui::PushClipRect(
    ImVec2(clip_rect_min_x, clip_rect_min_y),
    ImVec2(clip_rect_max_x, clip_rect_max_y),
    intersect_with_current_clip_rect
  );
});

DEFINE_API(void, PopClipRect, (ImGui_Context*,ctx),
"See ImGui_PushClipRect",
{
  FRAME_GUARD;
  ImGui::PopClipRect();
});
