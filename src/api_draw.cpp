#include "api_helper.hpp"

    // Primitives
    // - For rectangular primitives, "p_min" and "p_max" represent the upper-left and lower-right corners.
    // - For circle primitives, use "num_segments == 0" to automatically calculate tessellation (preferred).
    //   In older versions (until Dear ImGui 1.77) the AddCircle functions defaulted to num_segments == 12.
    //   In future versions we will use textures to provide cheaper and higher-quality circles.
    //   Use AddNgon() and AddNgonFilled() functions if you need to guaranteed a specific number of sides.
    // IMGUI_API void  AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
DEFINE_API(void, DrawList_AddRect, ((ImGui_Context*,ctx))
((double,fromX))((double,fromY))((double,toX))((double,toY))((int,colorRGBA))
((double*,roundingInOptional))((int*,roundingCornersInOptional))
((double*,thicknessInOptional)),
"Default values: rounding = 0.0, roundingCorners = ImGui_DrawCornerFlags_All, thickness = 1.0",
{
  ENTER_CONTEXT(ctx);
  ImDrawList *drawList { ImGui::GetWindowDrawList() };
  drawList->AddRect(ImVec2(fromX, fromY), ImVec2(toX, toY), Color::rgba2abgr(colorRGBA),
    valueOr(roundingInOptional, 0.0), valueOr(roundingCornersInOptional, ImDrawCornerFlags_All),
    valueOr(thicknessInOptional, 1.0));
});

DEFINE_API(void, DrawList_AddRectFilled, ((ImGui_Context*,ctx))
((double,fromX))((double,fromY))((double,toX))((double,toY))((int,colorRGBA))
((double*,roundingInOptional))((int*,roundingCornersInOptional)),
"Default values: rounding = 0.0, roundingCorners = ImGui_DrawCornerFlags_All, thickness = 1.0",
{
  ENTER_CONTEXT(ctx);
  ImDrawList *drawList { ImGui::GetWindowDrawList() };
  drawList->AddRectFilled(ImVec2(fromX, fromY), ImVec2(toX, toY), Color::rgba2abgr(colorRGBA),
    valueOr(roundingInOptional, 0.0), valueOr(roundingCornersInOptional, ImDrawCornerFlags_All));
});

    // IMGUI_API void  AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
    // IMGUI_API void  AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness = 1.0f);
    // IMGUI_API void  AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col);
    // IMGUI_API void  AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness = 1.0f);
    // IMGUI_API void  AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col);
    // IMGUI_API void  AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f);
    // IMGUI_API void  AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments = 0);
    // IMGUI_API void  AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness = 1.0f);
    // IMGUI_API void  AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments);
    // IMGUI_API void  AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL);
    // IMGUI_API void  AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
    // IMGUI_API void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, bool closed, float thickness);
    // IMGUI_API void  AddConvexPolyFilled(const ImVec2* points, int num_points, ImU32 col); // Note: Anti-aliased filling requires points to be in clockwise order.
    // IMGUI_API void  AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
    // IMGUI_API void  AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness, int num_segments = 0);               // Quadratic Bezier (3 control points)
    //
