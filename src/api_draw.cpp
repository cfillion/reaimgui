#include "api_helper.hpp"

struct ImGui_DrawList {
  enum Key {
    Window     = 0x77696e646f77646c, // WINDOWDL
    Background = 0x6267646c6267646c, // BGDLBGDL
    Foreground = 0x6667646c6667646c, // FGDLFGDL
  };

  template<typename Output, typename Input>
  static Output *encode(const Input *in, const Key key)
  {
    uintptr_t out { reinterpret_cast<uintptr_t>(in) };
    out ^= static_cast<uintptr_t>(key);
    return reinterpret_cast<Output *>(out);
  }

  static ImDrawList *get(const ImGui_DrawList *drawList)
  {
    Context *ctx;

    if(Resource::exists(ctx = encode<Context>(drawList, Window))) {
      ctx->enterFrame();
      return ImGui::GetWindowDrawList();
    }

    if(Resource::exists(ctx = encode<Context>(drawList, Background))) {
      ctx->enterFrame();
      return ImGui::GetBackgroundDrawList();
    }

    if(Resource::exists(ctx = encode<Context>(drawList, Foreground))) {
      ctx->enterFrame();
      return ImGui::GetForegroundDrawList();
    }

    throw reascript_error { "argument 1: expected ImGui_DrawList*" };
  }
};

DEFINE_API(ImGui_DrawList*, GetWindowDrawList, (ImGui_Context*,ctx),
"The draw list associated to the current window, to append your own drawing primitives",
{
  return ImGui_DrawList::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Window);
});

DEFINE_API(ImGui_DrawList*, GetBackgroundDrawList, (ImGui_Context*,ctx),
"This draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.",
{
  return ImGui_DrawList::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Background);
});

DEFINE_API(ImGui_DrawList*, GetForegroundDrawList, (ImGui_Context*,ctx),
"This draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.",
{
  return ImGui_DrawList::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Foreground);
});

    // Primitives
    // - For rectangular primitives, "p_min" and "p_max" represent the upper-left and lower-right corners.
    // - For circle primitives, use "num_segments == 0" to automatically calculate tessellation (preferred).
    //   In older versions (until Dear ImGui 1.77) the AddCircle functions defaulted to num_segments == 12.
    //   In future versions we will use textures to provide cheaper and higher-quality circles.
    //   Use AddNgon() and AddNgonFilled() functions if you need to guaranteed a specific number of sides.
    // IMGUI_API void  AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
DEFINE_API(void, DrawList_AddRect, (ImGui_DrawList*,draw_list)
(double,from_x)(double,from_y)(double,to_x)(double,to_y)(int,color_rgba)
(double*,API_RO(rounding))(int*,API_RO(rounding_corners))
(double*,API_RO(thickness)),
"Default values: rounding = 0.0, rounding_corners = ImGui_DrawCornerFlags_All, thickness = 1.0",
{
  ImDrawList *drawList { ImGui_DrawList::get(draw_list) };
  drawList->AddRect(ImVec2(from_x, from_y), ImVec2(to_x, to_y),
    Color::rgba2abgr(color_rgba), valueOr(API_RO(rounding), 0.0),
    valueOr(API_RO(rounding_corners), ImDrawCornerFlags_All),
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddRectFilled, (ImGui_DrawList*,draw_list)
(double,from_x)(double,from_y)(double,to_x)(double,to_y)(int,color_rgba)
(double*,API_RO(rounding))(int*,API_RO(rounding_corners)),
"Default values: rounding = 0.0, rounding_corners = ImGui_DrawCornerFlags_All, thickness = 1.0",
{
  ImDrawList *drawList { ImGui_DrawList::get(draw_list) };
  drawList->AddRectFilled(ImVec2(from_x, from_y), ImVec2(to_x, to_y),
    Color::rgba2abgr(color_rgba), valueOr(API_RO(rounding), 0.0),
    valueOr(API_RO(rounding_corners), ImDrawCornerFlags_All));
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

DEFINE_API(void, DrawList_AddText, (ImGui_DrawList*,draw_list)
(double,x)(double,y)(int,color_rgba)(const char*,text),
"",
{
  ImDrawList *drawList { ImGui_DrawList::get(draw_list) };
  drawList->AddText(ImVec2(x, y), Color::rgba2abgr(color_rgba), text);
});

    // IMGUI_API void  AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
    // IMGUI_API void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, bool closed, float thickness);
    // IMGUI_API void  AddConvexPolyFilled(const ImVec2* points, int num_points, ImU32 col); // Note: Anti-aliased filling requires points to be in clockwise order.
    // IMGUI_API void  AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
    // IMGUI_API void  AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness, int num_segments = 0);               // Quadratic Bezier (3 control points)

DEFINE_API(void, DrawList_PushClipRect, (ImGui_DrawList*,draw_list)
(double,clip_rect_min_x)(double,clip_rect_min_y)
(double,clip_rect_max_x)(double,clip_rect_max_y)
(bool*,API_RO(intersect_with_current_clip_rect)),
R"(Render-level scissoring. Prefer using higher-level ImGui_PushClipRect() to affect logic (hit-testing and widget culling). See ImGui_PushClipRect.

Default values: intersect_with_current_clip_rect = false)",
{
  ImGui_DrawList::get(draw_list)->PushClipRect(
    ImVec2(clip_rect_min_x, clip_rect_min_y),
    ImVec2(clip_rect_max_x, clip_rect_max_y),
    valueOr(API_RO(intersect_with_current_clip_rect), false));
});

    // IMGUI_API void  PushClipRectFullScreen();

DEFINE_API(void, DrawList_PopClipRect, (ImGui_DrawList*,draw_list),
"See DrawList_PushClipRect",
{
  ImGui_DrawList::get(draw_list)->PopClipRect();
});
