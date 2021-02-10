#include "api_helper.hpp"

#undef SetCursorPos // comes from SWELL, TODO remove reaper_plugin_functions.h include from api_helper

DEFINE_API(void, Separator, ((ImGui_Context*,ctx)),
"Separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.",
{
  ENTER_CONTEXT(ctx);
  ImGui::Separator();
});

DEFINE_API(void, SameLine, ((ImGui_Context*,ctx))
((double*,offsetFromStartXInOptional))((double*,spacingInOptional)),
R"(Call between widgets or groups to layout them horizontally. X position given in window coordinates.

'offsetFromStartX' defaults to 0.0, 'spacing' defaults to -1.0.)",
{
  ENTER_CONTEXT(ctx);
  ImGui::SameLine(valueOr(offsetFromStartXInOptional, 0.0),
    valueOr(spacingInOptional, -1.0));
});

DEFINE_API(void, NewLine, ((ImGui_Context*,ctx)),
"Undo a SameLine() or force a new line when in an horizontal-layout context.",
{
  ENTER_CONTEXT(ctx);
  ImGui::NewLine();
});

DEFINE_API(void, Spacing, ((ImGui_Context*,ctx)),
"Add vertical spacing.",
{
  ENTER_CONTEXT(ctx);
  ImGui::Spacing();
});

DEFINE_API(void, Dummy, ((ImGui_Context*,ctx))((double,w))((double,h)),
"Add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.",
{
  ENTER_CONTEXT(ctx);
  ImGui::Dummy(ImVec2(w, h));
});

// TODO style
DEFINE_API(void, Indent, ((ImGui_Context*,ctx))((double*,indentWidthInOptional)),
"Move content position toward the right, by 'indentWidth', or style.IndentSpacing if 'indentWidth' <= 0",
{
  ENTER_CONTEXT(ctx);
  ImGui::Indent(valueOr(indentWidthInOptional, 0.0));
});

// TODO style
DEFINE_API(void, Unindent, ((ImGui_Context*,ctx))((double*,indentWidthInOptional)),
"Move content position back to the left, by 'indentWidth', or style.IndentSpacing if 'indentWidth' <= 0",
{
  ENTER_CONTEXT(ctx);
  ImGui::Unindent(valueOr(indentWidthInOptional, 0.0));
});

    // IMGUI_API void          BeginGroup();                                                   // lock horizontal starting position
    // IMGUI_API void          EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)

DEFINE_API(void, GetCursorPos, ((ImGui_Context*,ctx))
((double*,xOut))((double*,yOut)),
"Cursor position in window",
{
  ENTER_CONTEXT(ctx);
  const ImVec2 &pos { ImGui::GetCursorPos() };
  if(xOut) *xOut = pos.x;
  if(yOut) *yOut = pos.y;
});

DEFINE_API(void, SetCursorPos, ((ImGui_Context*,ctx))
((double,x))((double,y)),
"Cursor position in window",
{
  ENTER_CONTEXT(ctx);
  ImGui::SetCursorPos(ImVec2(x, y));
});

    // IMGUI_API ImVec2        GetCursorStartPos();                                            // initial cursor position in window coordinates

DEFINE_API(void, GetCursorScreenPos, ((ImGui_Context*,ctx))
((double*,xOut))((double*,yOut)),
"Cursor position in absolute screen coordinates [0..io.DisplaySize] (useful to work with ImDrawList API)",
{
  ENTER_CONTEXT(ctx);
  const ImVec2 &pos { ImGui::GetCursorScreenPos() };
  if(xOut) *xOut = pos.x;
  if(yOut) *yOut = pos.y;
});

DEFINE_API(void, SetCursorScreenPos, ((ImGui_Context*,ctx))
((double,x))((double,y)),
"Cursor position in absolute screen coordinates [0..io.DisplaySize]",
{
  ENTER_CONTEXT(ctx);
  ImGui::SetCursorScreenPos(ImVec2(x, y));
});

DEFINE_API(void, AlignTextToFramePadding, ((ImGui_Context*,ctx)),
"Vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)",
{
  ENTER_CONTEXT(ctx);
  ImGui::AlignTextToFramePadding();
});

DEFINE_API(double, GetTextLineHeight, ((ImGui_Context*,ctx)),
"Same as ImGui_GetFontSize",
{
  ENTER_CONTEXT(ctx, 0.0);
  return ImGui::GetTextLineHeight();
});
    // IMGUI_API float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
    // IMGUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    // IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)
