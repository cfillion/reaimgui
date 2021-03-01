#include "api_helper.hpp"

#include <reaper_plugin_secrets.h> // reaper_array
#include <vector>

struct ImGui_DrawList {
  enum Key : uint64_t {
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

DEFINE_API(void, DrawList_AddLine, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(int,col_rgba)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  ImGui_DrawList::get(draw_list)->AddLine(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddRect, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)(int,col_rgba)
(double*,API_RO(rounding))(int*,API_RO(rounding_corners))
(double*,API_RO(thickness)),
"Default values: rounding = 0.0, rounding_corners = ImGui_DrawCornerFlags_All, thickness = 1.0",
{
  ImGui_DrawList::get(draw_list)->AddRect(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(rounding), 0.0),
    valueOr(API_RO(rounding_corners), ImDrawCornerFlags_All),
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddRectFilled, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)(int,col_rgba)
(double*,API_RO(rounding))(int*,API_RO(rounding_corners)),
"Default values: rounding = 0.0, rounding_corners = ImGui_DrawCornerFlags_All",
{
  ImGui_DrawList::get(draw_list)->AddRectFilled(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(rounding), 0.0),
    valueOr(API_RO(rounding_corners), ImDrawCornerFlags_All));
});

DEFINE_API(void, DrawList_AddRectFilledMultiColor, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)
(int,col_upr_left)(int,col_upr_right)(int,col_bot_right)(int,col_bot_left),
"",
{
  ImGui_DrawList::get(draw_list)->AddRectFilledMultiColor(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::rgba2abgr(col_upr_left),  Color::rgba2abgr(col_upr_right),
    Color::rgba2abgr(col_bot_right), Color::rgba2abgr(col_bot_left));
});

DEFINE_API(void, DrawList_AddQuad, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int,col_rgba)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  ImGui_DrawList::get(draw_list)->AddQuad(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    ImVec2(p4_x, p4_y), Color::rgba2abgr(col_rgba),
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddQuadFilled, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int,col_rgba),
"",
{
  ImGui_DrawList::get(draw_list)->AddQuadFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    ImVec2(p4_x, p4_y), Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_AddTriangle, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(int,col_rgba)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  ImGui_DrawList::get(draw_list)->AddTriangle(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddTriangleFilled, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(int,col_rgba),
"",
{
  ImGui_DrawList::get(draw_list)->AddTriangleFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_AddCircle, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int*,API_RO(num_segments))(double*,API_RO(thickness)),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).

Default values: num_segments = 0, thickness = 1.0)",
{
  ImGui_DrawList::get(draw_list)->AddCircle(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), valueOr(API_RO(num_segments), 0),
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddCircleFilled, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int*,API_RO(num_segments)),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).

Default values: num_segments = 0)",
{
  ImGui_DrawList::get(draw_list)->AddCircleFilled(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), valueOr(API_RO(num_segments), 0));
});

DEFINE_API(void, DrawList_AddNgon, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int,num_segments)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  ImGui_DrawList::get(draw_list)->AddNgon(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), num_segments,
    valueOr(API_RO(thickness), 1.0));
});


DEFINE_API(void, DrawList_AddNgonFilled, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int,num_segments),
"",
{
  ImGui_DrawList::get(draw_list)->AddNgonFilled(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), num_segments);
});

DEFINE_API(void, DrawList_AddText, (ImGui_DrawList*,draw_list)
(double,x)(double,y)(int,col_rgba)(const char*,text),
"",
{
  ImDrawList *drawList { ImGui_DrawList::get(draw_list) };
  drawList->AddText(ImVec2(x, y), Color::rgba2abgr(col_rgba), text);
});

static std::vector<ImVec2> makePointsArray(const reaper_array *points)
{
  assertValid(points);

  if(points->size % 2)
    throw reascript_error { "an odd amount of points was provided (expected x,y pairs)" };

  std::vector<ImVec2> out;
  out.reserve(points->size / 2);
  for(unsigned int i = 0; i < points->size - 1; i += 2)
    out.push_back(ImVec2(points->data[i], points->data[i+1]));
  return out;
}

DEFINE_API(void, DrawList_AddPolyline, (ImGui_DrawList*,draw_list)
(reaper_array*,points)(int,col_rgba)(bool,closed)(double,thickness),
"Points is a list of x,y coordinates.",
{
  const std::vector<ImVec2> vec2points { makePointsArray(points) };
  ImGui_DrawList::get(draw_list)->AddPolyline(
    vec2points.data(), vec2points.size(), Color::rgba2abgr(col_rgba),
    closed, thickness);
});

DEFINE_API(void, DrawList_AddConvexPolyFilled, (ImGui_DrawList*,draw_list)
(reaper_array*,points)(int,num_points)(int,col_rgba),
"Note: Anti-aliased filling requires points to be in clockwise order.",
{
  const std::vector<ImVec2> vec2points { makePointsArray(points) };
  ImGui_DrawList::get(draw_list)->AddConvexPolyFilled(
    vec2points.data(), vec2points.size(), Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_AddBezierCubic, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int,col_rgba)(double,thickness)(int*,API_RO(num_segments)),
R"(Cubic Bezier (4 control points)

Default values: num_segments = 0)",
{
  ImGui_DrawList::get(draw_list)->AddBezierCubic(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y), Color::rgba2abgr(col_rgba),
    thickness, valueOr(API_RO(num_segments), 0));
});


DEFINE_API(void, DrawList_AddBezierQuadratic, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)(double,p3_x)(double,p3_y)
(int,col_rgba)(double,thickness)(int*,API_RO(num_segments)),
R"(Quadratic Bezier (3 control points)

Default values: num_segments = 0)",
{
  ImGui_DrawList::get(draw_list)->AddBezierQuadratic(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    ImVec2(p3_x, p3_y), Color::rgba2abgr(col_rgba),
    thickness, valueOr(API_RO(num_segments), 0));
});

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

DEFINE_API(void, DrawList_PushClipRectFullScreen, (ImGui_DrawList*,draw_list),
"",
{
  ImGui_DrawList::get(draw_list)->PushClipRectFullScreen();
});

DEFINE_API(void, DrawList_PopClipRect, (ImGui_DrawList*,draw_list),
"See DrawList_PushClipRect",
{
  ImGui_DrawList::get(draw_list)->PopClipRect();
});

DEFINE_API(void, DrawList_PathClear, (ImGui_DrawList*,draw_list),
"",
{
  ImGui_DrawList::get(draw_list)->PathClear();
});

DEFINE_API(void, DrawList_PathLineTo, (ImGui_DrawList*,draw_list)
(double,pos_x)(double,pos_y),
"Stateful path API, add points then finish with PathFillConvex() or PathStroke()",
{
  ImGui_DrawList::get(draw_list)->PathLineToMergeDuplicate(ImVec2(pos_x, pos_y));
});

DEFINE_API(void, PathFillConvex, (ImGui_DrawList*,draw_list)
(int,col_rgba),
"Note: Anti-aliased filling requires points to be in clockwise order.",
{
  ImGui_DrawList::get(draw_list)->PathFillConvex(Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_PathStroke, (ImGui_DrawList*,draw_list)
(int,col_rgba)(bool,closed)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  ImGui_DrawList::get(draw_list)->PathStroke(
    Color::rgba2abgr(col_rgba), closed, valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_PathArcTo, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(double,a_min)(double,a_max)
(int*,API_RO(num_segments)),
"Default values: num_segments = 10",
{
  ImGui_DrawList::get(draw_list)->PathArcTo(ImVec2(center_x, center_y),
    radius, a_min, a_max, valueOr(API_RO(num_segments), 10));
});

DEFINE_API(void, DrawList_PathArcToFast, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)
(int,a_min_of_12)(int,a_max_of_12),
"Use precomputed angles for a 12 steps circle.",
{
  ImGui_DrawList::get(draw_list)->PathArcToFast(
    ImVec2(center_x, center_y), radius, a_min_of_12, a_max_of_12);
});

DEFINE_API(void, DrawList_PathBezierCubicCurveTo, (ImGui_DrawList*,draw_list)
(double,p2_x)(double,p2_y)(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int*,API_RO(num_segments)),
R"(Cubic Bezier (4 control points)

Default values: num_segments = 0)",
{
  ImGui_DrawList::get(draw_list)->PathBezierCubicCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    valueOr(API_RO(num_segments), 0));
});

DEFINE_API(void, DrawList_PathBezierQuadraticCurveTo, (ImGui_DrawList*,draw_list)
(double,p2_x)(double,p2_y)(double,p3_x)(double,p3_y)(int*,API_RO(num_segments)),
R"(Quadratic Bezier (3 control points)

Default values: num_segments = 0)",
{
  ImGui_DrawList::get(draw_list)->PathBezierQuadraticCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), valueOr(API_RO(num_segments), 0));
});

DEFINE_API(void, DrawList_PathRect, (ImGui_DrawList*,draw_list)
(double,rect_min_x)(double,rect_min_y)(double,rect_max_x)(double,rect_max_y)
(double*,API_RO(rounding))(int*,API_RO(rounding_corners)),
"Default values: rounding = 0.0, rounding_corners = ImGui_DrawCornerFlags_All",
{
  ImGui_DrawList::get(draw_list)->PathRect(ImVec2(rect_min_x, rect_min_y),
    ImVec2(rect_max_x, rect_max_y), valueOr(API_RO(rounding), 0.0),
    valueOr(API_RO(rounding_corners), ImDrawCornerFlags_All));
});
