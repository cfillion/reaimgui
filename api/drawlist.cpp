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

#include "color.hpp"
#include "font.hpp"
#include "resource_proxy.hpp"

#include <reaper_plugin_secrets.h> // reaper_array
#include <vector>

struct ImGui_DrawList {
  enum Key {
    Window     = 0x574e444c, // WNDL
    Background = 0x6267646c, // BGDL
    Foreground = 0x6667646c, // FGDL
  };

  ImDrawList *get(Context **p_ctx = nullptr)
  {
    ResourceProxy::Key drawList {};
    Context *ctx { DrawList.decode<Context>(this, &drawList) };

    if(p_ctx)
      *p_ctx = ctx;

    switch(drawList) {
    case Window:
      ctx->enterFrame();
      return ImGui::GetWindowDrawList();
    case Background:
      ctx->enterFrame();
      return ImGui::GetBackgroundDrawList();
    case Foreground:
      ctx->enterFrame();
      return ImGui::GetForegroundDrawList();
    default:
      throw reascript_error { "expected a valid ImGui_DrawList*" };
    }
  }
};

ResourceProxy DrawList {
  ImGui_DrawList::Window,
  ImGui_DrawList::Background,
  ImGui_DrawList::Foreground
};

DEFINE_API(ImGui_DrawList*, GetWindowDrawList, (ImGui_Context*,ctx),
"The draw list associated to the current window, to append your own drawing primitives",
{
  return ResourceProxy::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Window);
});

DEFINE_API(ImGui_DrawList*, GetBackgroundDrawList, (ImGui_Context*,ctx),
"This draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.",
{
  return ResourceProxy::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Background);
});

DEFINE_API(ImGui_DrawList*, GetForegroundDrawList, (ImGui_Context*,ctx),
"This draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.",
{
  return ResourceProxy::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Foreground);
});

DEFINE_API(void, DrawList_AddLine, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(int,col_rgba)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  draw_list->get()->AddLine(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddRect, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)(int,col_rgba)
(double*,API_RO(rounding))(int*,API_RO(flags))
(double*,API_RO(thickness)),
"Default values: rounding = 0.0, flags = ImGui_DrawFlags_None, thickness = 1.0",
{
  draw_list->get()->AddRect(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(rounding), 0.f),
    valueOr(API_RO(flags), ImDrawFlags_None),
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddRectFilled, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)(int,col_rgba)
(double*,API_RO(rounding))(int*,API_RO(flags)),
"Default values: rounding = 0.0, flags = ImGui_DrawFlags_None",
{
  draw_list->get()->AddRectFilled(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(rounding), 0.f),
    valueOr(API_RO(flags), ImDrawFlags_None));
});

DEFINE_API(void, DrawList_AddRectFilledMultiColor, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)
(int,col_upr_left)(int,col_upr_right)(int,col_bot_right)(int,col_bot_left),
"",
{
  draw_list->get()->AddRectFilledMultiColor(
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
  draw_list->get()->AddQuad(
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
  draw_list->get()->AddQuadFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    ImVec2(p4_x, p4_y), Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_AddTriangle, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(int,col_rgba)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  draw_list->get()->AddTriangle(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::rgba2abgr(col_rgba), valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddTriangleFilled, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(int,col_rgba),
"",
{
  draw_list->get()->AddTriangleFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_AddCircle, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int*,API_RO(num_segments))(double*,API_RO(thickness)),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).

Default values: num_segments = 0, thickness = 1.0)",
{
  draw_list->get()->AddCircle(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), valueOr(API_RO(num_segments), 0),
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddCircleFilled, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int*,API_RO(num_segments)),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).

Default values: num_segments = 0)",
{
  draw_list->get()->AddCircleFilled(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), valueOr(API_RO(num_segments), 0));
});

DEFINE_API(void, DrawList_AddNgon, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int,num_segments)(double*,API_RO(thickness)),
"Default values: thickness = 1.0",
{
  draw_list->get()->AddNgon(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), num_segments,
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_AddNgonFilled, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int,num_segments),
"",
{
  draw_list->get()->AddNgonFilled(ImVec2(center_x, center_y),
    radius, Color::rgba2abgr(col_rgba), num_segments);
});

DEFINE_API(void, DrawList_AddText, (ImGui_DrawList*,draw_list)
(double,x)(double,y)(int,col_rgba)(const char*,text),
"",
{
  draw_list->get()->AddText(ImVec2(x, y), Color::rgba2abgr(col_rgba), text);
});

DEFINE_API(void, DrawList_AddTextEx, (ImGui_DrawList*,draw_list)
(ImGui_Font*,font)(double,font_size)(double,pos_x)(double,pos_y)
(int,col_rgba)(const char*,text)(double*,API_RO(wrap_width))
(double*,API_RO(cpu_fine_clip_rect_x))(double*,API_RO(cpu_fine_clip_rect_y))
(double*,API_RO(cpu_fine_clip_rect_w))(double*,API_RO(cpu_fine_clip_rect_h)),
R"(The default font is used if font = nil. cpu_fine_clip_rect_* only takes effect if all four are non-nil.

Default values: wrap_width = 0.0, cpu_fine_clip_rect_x = nil, cpu_fine_clip_rect_y = nil, cpu_fine_clip_rect_w = nil, cpu_fine_clip_rect_h = nil)",
{
  col_rgba = Color::rgba2abgr(col_rgba);

  ImVec2 pos;
  pos.x = pos_x;
  pos.y = pos_y;

  const float wrap_width { valueOr(API_RO(wrap_width), 0.f) };

  ImVec4 cpu_fine_clip_rect, *cpu_fine_clip_rect_ptr;
  if(API_RO(cpu_fine_clip_rect_x) && API_RO(cpu_fine_clip_rect_y) &&
      API_RO(cpu_fine_clip_rect_w) && API_RO(cpu_fine_clip_rect_h)) {
    cpu_fine_clip_rect.x = *API_RO(cpu_fine_clip_rect_x);
    cpu_fine_clip_rect.y = *API_RO(cpu_fine_clip_rect_y);
    cpu_fine_clip_rect.z = *API_RO(cpu_fine_clip_rect_w);
    cpu_fine_clip_rect.w = *API_RO(cpu_fine_clip_rect_h);
    cpu_fine_clip_rect_ptr = &cpu_fine_clip_rect;
  }
  else
    cpu_fine_clip_rect_ptr = nullptr;

  Context *ctx;
  draw_list->get(&ctx)->AddText(ctx->fonts().instanceOf(font), font_size,
    pos, col_rgba, text, nullptr, wrap_width, cpu_fine_clip_rect_ptr);
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
(reaper_array*,points)(int,col_rgba)(int,flags)(double,thickness),
"Points is a list of x,y coordinates.",
{
  const std::vector<ImVec2> vec2points { makePointsArray(points) };
  draw_list->get()->AddPolyline(
    vec2points.data(), vec2points.size(), Color::rgba2abgr(col_rgba),
    flags, thickness);
});

DEFINE_API(void, DrawList_AddConvexPolyFilled, (ImGui_DrawList*,draw_list)
(reaper_array*,points)(int,num_points)(int,col_rgba),
"Note: Anti-aliased filling requires points to be in clockwise order.",
{
  const std::vector<ImVec2> vec2points { makePointsArray(points) };
  draw_list->get()->AddConvexPolyFilled(
    vec2points.data(), vec2points.size(), Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_AddBezierCubic, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int,col_rgba)(double,thickness)(int*,API_RO(num_segments)),
R"(Cubic Bezier (4 control points)

Default values: num_segments = 0)",
{
  draw_list->get()->AddBezierCubic(
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
  draw_list->get()->AddBezierQuadratic(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    ImVec2(p3_x, p3_y), Color::rgba2abgr(col_rgba),
    thickness, valueOr(API_RO(num_segments), 0));
});

DEFINE_API(void, DrawList_PushClipRect, (ImGui_DrawList*,draw_list)
(double,clip_rect_min_x)(double,clip_rect_min_y)
(double,clip_rect_max_x)(double,clip_rect_max_y)
(bool*,API_RO(intersect_with_current_clip_rect)),
R"(Render-level scissoring. Prefer using higher-level ImGui_PushClipRect to affect logic (hit-testing and widget culling).

Default values: intersect_with_current_clip_rect = false)",
{
  draw_list->get()->PushClipRect(
    ImVec2(clip_rect_min_x, clip_rect_min_y),
    ImVec2(clip_rect_max_x, clip_rect_max_y),
    valueOr(API_RO(intersect_with_current_clip_rect), false));
});

DEFINE_API(void, DrawList_PushClipRectFullScreen, (ImGui_DrawList*,draw_list),
"",
{
  draw_list->get()->PushClipRectFullScreen();
});

DEFINE_API(void, DrawList_PopClipRect, (ImGui_DrawList*,draw_list),
"See DrawList_PushClipRect",
{
  draw_list->get()->PopClipRect();
});

DEFINE_API(void, DrawList_PathClear, (ImGui_DrawList*,draw_list),
"",
{
  draw_list->get()->PathClear();
});

DEFINE_API(void, DrawList_PathLineTo, (ImGui_DrawList*,draw_list)
(double,pos_x)(double,pos_y),
"Stateful path API, add points then finish with ImGui_DrawList_PathFillConvex or ImGui_DrawList_PathStroke.",
{
  draw_list->get()->PathLineToMergeDuplicate(ImVec2(pos_x, pos_y));
});

DEFINE_API(void, DrawList_PathFillConvex, (ImGui_DrawList*,draw_list)
(int,col_rgba),
"Note: Anti-aliased filling requires points to be in clockwise order.",
{
  draw_list->get()->PathFillConvex(Color::rgba2abgr(col_rgba));
});

DEFINE_API(void, DrawList_PathStroke, (ImGui_DrawList*,draw_list)
(int,col_rgba)(int*,API_RO(flags))(double*,API_RO(thickness)),
"Default values: flags = ImGui_DrawFlags_None, thickness = 1.0",
{
  draw_list->get()->PathStroke(
    Color::rgba2abgr(col_rgba), valueOr(API_RO(flags), ImDrawFlags_None),
    valueOr(API_RO(thickness), 1.0));
});

DEFINE_API(void, DrawList_PathArcTo, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(double,a_min)(double,a_max)
(int*,API_RO(num_segments)),
"Default values: num_segments = 0",
{
  draw_list->get()->PathArcTo(ImVec2(center_x, center_y),
    radius, a_min, a_max, valueOr(API_RO(num_segments), 10));
});

DEFINE_API(void, DrawList_PathArcToFast, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)
(int,a_min_of_12)(int,a_max_of_12),
"Use precomputed angles for a 12 steps circle.",
{
  draw_list->get()->PathArcToFast(
    ImVec2(center_x, center_y), radius, a_min_of_12, a_max_of_12);
});

DEFINE_API(void, DrawList_PathBezierCubicCurveTo, (ImGui_DrawList*,draw_list)
(double,p2_x)(double,p2_y)(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int*,API_RO(num_segments)),
R"(Cubic Bezier (4 control points)

Default values: num_segments = 0)",
{
  draw_list->get()->PathBezierCubicCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    valueOr(API_RO(num_segments), 0));
});

DEFINE_API(void, DrawList_PathBezierQuadraticCurveTo, (ImGui_DrawList*,draw_list)
(double,p2_x)(double,p2_y)(double,p3_x)(double,p3_y)(int*,API_RO(num_segments)),
R"(Quadratic Bezier (3 control points)

Default values: num_segments = 0)",
{
  draw_list->get()->PathBezierQuadraticCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), valueOr(API_RO(num_segments), 0));
});

DEFINE_API(void, DrawList_PathRect, (ImGui_DrawList*,draw_list)
(double,rect_min_x)(double,rect_min_y)(double,rect_max_x)(double,rect_max_y)
(double*,API_RO(rounding))(int*,API_RO(flags)),
"Default values: rounding = 0.0, flags = ImGui_DrawFlags_None",
{
  draw_list->get()->PathRect(ImVec2(rect_min_x, rect_min_y),
    ImVec2(rect_max_x, rect_max_y), valueOr(API_RO(rounding), 0.f),
    valueOr(API_RO(flags), ImDrawFlags_None));
});

// ImDrawFlags
DEFINE_ENUM(Im, DrawFlags_None,                         "");
DEFINE_ENUM(Im, DrawFlags_Closed,                       "ImGui_DrawList_PathStroke, ImGui_DrawList_AddPolyline: specify that shape should be closed (Important: this is always == 1 for legacy reason).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTopLeft,          "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding top-left corner only (when rounding > 0.0f, we default to all corners).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTopRight,         "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding top-right corner only (when rounding > 0.0f, we default to all corners).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottomLeft,       "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners).");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottomRight,      "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners).");

DEFINE_ENUM(Im, DrawFlags_RoundCornersNone            , "ImGui_DrawList_AddRect, ImGui_DrawList_AddRectFilled, ImGui_DrawList_PathRect: disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!.");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTop             , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottom          , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersLeft            , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersRight           , "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersAll             , "");
