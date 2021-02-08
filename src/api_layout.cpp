#include "api_helper.hpp"

#undef SetCursorPos // comes from SWELL, TODO remove reaper_plugin_functions.h include from api_helper

DEFINE_API(void, Separator, ((Window*,window)),
"Separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.",
{
  USE_WINDOW(window);
  ImGui::Separator();
});

DEFINE_API(void, SameLine, ((Window*,window))
((double*,offsetFromStartXInOptional))((double*,spacingInOptional)),
R"(Call between widgets or groups to layout them horizontally. X position given in window coordinates.

'offsetFromStartX' defaults to 0.0, 'spacing' defaults to -1.0.)",
{
  USE_WINDOW(window);
  ImGui::SameLine(valueOr(offsetFromStartXInOptional, 0.0),
    valueOr(spacingInOptional, -1.0));
});

DEFINE_API(void, NewLine, ((Window*,window)),
"Undo a SameLine() or force a new line when in an horizontal-layout context.",
{
  USE_WINDOW(window);
  ImGui::NewLine();
});

DEFINE_API(void, Spacing, ((Window*,window)),
"Add vertical spacing.",
{
  USE_WINDOW(window);
  ImGui::Spacing();
});

DEFINE_API(void, Dummy, ((Window*,window))((double,w))((double,h)),
"Add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.",
{
  USE_WINDOW(window);
  ImGui::Dummy(ImVec2(w, h));
});

// TODO style
DEFINE_API(void, Indent, ((Window*,window))((double*,indentWidthInOptional)),
"Move content position toward the right, by 'indentWidth', or style.IndentSpacing if 'indentWidth' <= 0",
{
  USE_WINDOW(window);
  ImGui::Indent(valueOr(indentWidthInOptional, 0.0));
});

// TODO style
DEFINE_API(void, Unindent, ((Window*,window))((double*,indentWidthInOptional)),
"Move content position back to the left, by 'indentWidth', or style.IndentSpacing if 'indentWidth' <= 0",
{
  USE_WINDOW(window);
  ImGui::Unindent(valueOr(indentWidthInOptional, 0.0));
});

    // IMGUI_API void          BeginGroup();                                                   // lock horizontal starting position
    // IMGUI_API void          EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)

DEFINE_API(void, GetCursorPos, ((Window*,window))
((double*,xOut))((double*,yOut)),
"Cursor position in window",
{
  USE_WINDOW(window);
  const ImVec2 &pos { ImGui::GetCursorPos() };
  if(xOut) *xOut = pos.x;
  if(yOut) *yOut = pos.y;
});

DEFINE_API(void, SetCursorPos, ((Window*,window))
((double,x))((double,y)),
"Cursor position in window",
{
  USE_WINDOW(window);
  ImGui::SetCursorPos(ImVec2(x, y));
});

    // IMGUI_API ImVec2        GetCursorStartPos();                                            // initial cursor position in window coordinates

DEFINE_API(void, GetCursorScreenPos, ((Window*,window))
((double*,xOut))((double*,yOut)),
"Cursor position in absolute screen coordinates [0..io.DisplaySize] (useful to work with ImDrawList API)",
{
  USE_WINDOW(window);
  const ImVec2 &pos { ImGui::GetCursorScreenPos() };
  if(xOut) *xOut = pos.x;
  if(yOut) *yOut = pos.y;
});

DEFINE_API(void, SetCursorScreenPos, ((Window*,window))
((double,x))((double,y)),
"Cursor position in absolute screen coordinates [0..io.DisplaySize]",
{
  USE_WINDOW(window);
  ImGui::SetCursorScreenPos(ImVec2(x, y));
});

DEFINE_API(void, AlignTextToFramePadding, ((Window*,window)),
"Vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)",
{
  USE_WINDOW(window);
  ImGui::AlignTextToFramePadding();
});

DEFINE_API(double, GetTextLineHeight, ((Window*,window)),
"Same as ImGui_GetFontSize",
{
  USE_WINDOW(window, 0.0);
  return ImGui::GetTextLineHeight();
});
    // IMGUI_API float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
    // IMGUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    // IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)
