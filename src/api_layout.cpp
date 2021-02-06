#include "api_helper.hpp"

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
    // IMGUI_API ImVec2        GetCursorPos();                                                 // cursor position in window coordinates (relative to window position)
    // IMGUI_API float         GetCursorPosX();                                                //   (some functions are using window-relative coordinates, such as: GetCursorPos, GetCursorStartPos, GetContentRegionMax, GetWindowContentRegion* etc.
    // IMGUI_API float         GetCursorPosY();                                                //    other functions such as GetCursorScreenPos or everything in ImDrawList::
    // IMGUI_API void          SetCursorPos(const ImVec2& local_pos);                          //    are using the main, absolute coordinate system.
    // IMGUI_API void          SetCursorPosX(float local_x);                                   //    GetWindowPos() + GetCursorPos() == GetCursorScreenPos() etc.)
    // IMGUI_API void          SetCursorPosY(float local_y);                                   //
    // IMGUI_API ImVec2        GetCursorStartPos();                                            // initial cursor position in window coordinates
    // IMGUI_API ImVec2        GetCursorScreenPos();                                           // cursor position in absolute screen coordinates [0..io.DisplaySize] (useful to work with ImDrawList API)
    // IMGUI_API void          SetCursorScreenPos(const ImVec2& pos);                          // cursor position in absolute screen coordinates [0..io.DisplaySize]
DEFINE_API(void, AlignTextToFramePadding, ((Window*,window)),
"Vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)",
{
  USE_WINDOW(window);
  ImGui::AlignTextToFramePadding();
});
    // IMGUI_API float         GetTextLineHeight();                                            // ~ FontSize
    // IMGUI_API float         GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
    // IMGUI_API float         GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
    // IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)
