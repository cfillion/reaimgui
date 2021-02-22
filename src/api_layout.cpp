#include "api_helper.hpp"

#undef SetCursorPos // comes from SWELL, TODO remove reaper_plugin_functions.h include from api_helper

DEFINE_API(void, Separator, ((ImGui_Context*,ctx)),
"Separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.",
{
  ensureContext(ctx)->enterFrame();
  ImGui::Separator();
});

DEFINE_API(void, SameLine, ((ImGui_Context*,ctx))
((double*,API_RO(offsetFromStartX)))((double*,API_RO(spacing))),
R"(Call between widgets or groups to layout them horizontally. X position given in window coordinates.

Default values: offsetFromStartX = 0.0, spacing = -1.0.)",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SameLine(valueOr(API_RO(offsetFromStartX), 0.0),
    valueOr(API_RO(spacing), -1.0));
});

DEFINE_API(void, NewLine, ((ImGui_Context*,ctx)),
"Undo a SameLine() or force a new line when in an horizontal-layout context.",
{
  ensureContext(ctx)->enterFrame();
  ImGui::NewLine();
});

DEFINE_API(void, Spacing, ((ImGui_Context*,ctx)),
"Add vertical spacing.",
{
  ensureContext(ctx)->enterFrame();
  ImGui::Spacing();
});

DEFINE_API(void, Dummy, ((ImGui_Context*,ctx))((double,w))((double,h)),
"Add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.",
{
  ensureContext(ctx)->enterFrame();
  ImGui::Dummy(ImVec2(w, h));
});

DEFINE_API(void, Indent, ((ImGui_Context*,ctx))((double*,API_RO(indentWidth))),
R"(Move content position toward the right, by 'indentWidth', or style.IndentSpacing if 'indentWidth' <= 0

Default values: indentWidth = 0.0)",
{
  ensureContext(ctx)->enterFrame();
  ImGui::Indent(valueOr(API_RO(indentWidth), 0.0));
});

DEFINE_API(void, Unindent, ((ImGui_Context*,ctx))((double*,API_RO(indentWidth))),
"Move content position back to the left, by 'indentWidth', or style.IndentSpacing if 'indentWidth' <= 0",
{
  ensureContext(ctx)->enterFrame();
  ImGui::Unindent(valueOr(API_RO(indentWidth), 0.0));
});

DEFINE_API(void, BeginGroup, ((ImGui_Context*,ctx)),
"Lock horizontal starting position. See ImGui_EndGroup.",
{
  ensureContext(ctx)->enterFrame();
  ImGui::BeginGroup();
});

DEFINE_API(void, EndGroup, ((ImGui_Context*,ctx)),
R"(Unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.).

See ImGui_BeginGroup.)",
{
  ensureContext(ctx)->enterFrame();
  ImGui::EndGroup();
});

DEFINE_API(void, GetCursorPos, ((ImGui_Context*,ctx))
((double*,API_W(x)))((double*,API_W(y))),
"Cursor position in window",
{
  ensureContext(ctx)->enterFrame();
  const ImVec2 &pos { ImGui::GetCursorPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(double, GetCursorPosX, ((ImGui_Context*,ctx)),
"Cursor X position in window",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::GetCursorPosX();
});

DEFINE_API(double, GetCursorPosY, ((ImGui_Context*,ctx)),
"Cursor Y position in window",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::GetCursorPosY();
});

DEFINE_API(void, SetCursorPos, ((ImGui_Context*,ctx))
((double,x))((double,y)),
"Cursor position in window",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SetCursorPos(ImVec2(x, y));
});

DEFINE_API(void, SetCursorPosX, ((ImGui_Context*,ctx))
((double,local_x)),
"Cursor X position in window",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SetCursorPosX(local_x);
});

DEFINE_API(void, SetCursorPosY, ((ImGui_Context*,ctx))
((double,local_x)),
"Cursor Y position in window",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SetCursorPosY(local_x);
});

    // IMGUI_API ImVec2        GetCursorStartPos();                                            // initial cursor position in window coordinates

DEFINE_API(void, GetCursorScreenPos, ((ImGui_Context*,ctx))
((double*,API_W(x)))((double*,API_W(y))),
"Cursor position in absolute screen coordinates [0..io.DisplaySize] (useful to work with ImDrawList API)",
{
  ensureContext(ctx)->enterFrame();
  const ImVec2 &pos { ImGui::GetCursorScreenPos() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
});

DEFINE_API(void, SetCursorScreenPos, ((ImGui_Context*,ctx))
((double,x))((double,y)),
"Cursor position in absolute screen coordinates [0..io.DisplaySize]",
{
  ensureContext(ctx)->enterFrame();
  ImGui::SetCursorScreenPos(ImVec2(x, y));
});

DEFINE_API(void, AlignTextToFramePadding, ((ImGui_Context*,ctx)),
"Vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)",
{
  ensureContext(ctx)->enterFrame();
  ImGui::AlignTextToFramePadding();
});

DEFINE_API(double, GetTextLineHeight, ((ImGui_Context*,ctx)),
"Same as ImGui_GetFontSize",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::GetTextLineHeight();
});

DEFINE_API(double, GetTextLineHeightWithSpacing, ((ImGui_Context*,ctx)),
"~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)",
{
  ensureContext(ctx)->enterFrame();
  return ImGui::GetTextLineHeightWithSpacing();
});
    // IMGUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    // IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)
