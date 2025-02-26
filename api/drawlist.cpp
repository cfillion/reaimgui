/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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
#include "drawlist.hpp"

#include "../src/color.hpp"
#include "../src/font.hpp"
#include "../src/image.hpp"

#include <reaper_plugin_secrets.h> // reaper_array
#include <vector>

API_SECTION("Draw List",
R"(This is the low-level list of polygons that ImGui functions are filling.
At the end of the frame, all draw lists are passed to the GPU for rendering.

Each dear imgui window contains its own Draw List.
You can use GetWindowDrawList() to access the current window draw list and draw
custom primitives.

The Draw List API uses absolute coordinates (0,0 is the top-left corner of the
primary monitor, not of your window!). See GetCursorScreenPos.)");

API_FUNC(0_1, DrawListProxy*, GetWindowDrawList, (Context*,ctx),
"The draw list associated to the current window, to append your own drawing primitives")
{
  return DrawListProxy::encode<DrawListProxy::Window>(ctx);
}

API_FUNC(0_1, DrawListProxy*, GetBackgroundDrawList, (Context*,ctx),
R"(This draw list will be the first rendering one. Useful to quickly draw
shapes/text behind dear imgui contents.)")
{
  return DrawListProxy::encode<DrawListProxy::Background>(ctx);
}

API_FUNC(0_1, DrawListProxy*, GetForegroundDrawList, (Context*,ctx),
R"(This draw list will be the last rendered one. Useful to quickly draw
shapes/text over dear imgui contents.)")
{
  return DrawListProxy::encode<DrawListProxy::Foreground>(ctx);
}

API_FUNC(0_1, void, DrawList_PushClipRect, (DrawListProxy*,draw_list)
(double,clip_rect_min_x) (double,clip_rect_min_y)
(double,clip_rect_max_x) (double,clip_rect_max_y)
(RO<bool*>,intersect_with_current_clip_rect,false),
R"(Render-level scissoring. Prefer using higher-level PushClipRect to affect
logic (hit-testing and widget culling).)")
{
  draw_list->get()->PushClipRect(
    ImVec2(clip_rect_min_x, clip_rect_min_y),
    ImVec2(clip_rect_max_x, clip_rect_max_y),
    API_GET(intersect_with_current_clip_rect));
}

API_FUNC(0_1, void, DrawList_PushClipRectFullScreen, (DrawListProxy*,draw_list),
"")
{
  draw_list->get()->PushClipRectFullScreen();
}

API_FUNC(0_1, void, DrawList_PopClipRect, (DrawListProxy*,draw_list),
"See DrawList_PushClipRect")
{
  draw_list->get()->PopClipRect();
}

API_ENUM(0_2, Im, DrawFlags_None, "");
API_ENUM(0_2, Im, DrawFlags_Closed,
R"(DrawList_PathStroke, DrawList_AddPolyline: specify that shape should be
   closed (Important: this is always == 1 for legacy reason).)");
API_ENUM(0_2, Im, DrawFlags_RoundCornersTopLeft,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   top-left corner only (when rounding > 0.0, we default to all corners).)");
API_ENUM(0_2, Im, DrawFlags_RoundCornersTopRight,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   top-right corner only (when rounding > 0.0, we default to all corners).)");
API_ENUM(0_2, Im, DrawFlags_RoundCornersBottomLeft,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   bottom-left corner only (when rounding > 0.0, we default to all corners).)");
API_ENUM(0_2, Im, DrawFlags_RoundCornersBottomRight,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   bottom-right corner only (when rounding > 0.0, we default to all corners).)");
API_ENUM(0_2, Im, DrawFlags_RoundCornersNone,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: disable rounding
   on all corners (when rounding > 0.0). This is NOT zero, NOT an implicit flag!.)");
API_ENUM(0_2, Im, DrawFlags_RoundCornersTop,    "");
API_ENUM(0_2, Im, DrawFlags_RoundCornersBottom, "");
API_ENUM(0_2, Im, DrawFlags_RoundCornersLeft,   "");
API_ENUM(0_2, Im, DrawFlags_RoundCornersRight,  "");
API_ENUM(0_2, Im, DrawFlags_RoundCornersAll,    "");

API_SUBSECTION("Primitives",
R"(Filled shapes must always use clockwise winding order! The anti-aliasing
fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
So e.g. `DrawList_PathArcTo(center, radius, PI * -0.5, PI)` is ok, whereas
`DrawList_PathArcTo(center, radius, PI, PI * -0.5)` won't have correct
anti-aliasing when followed by DrawList_PathFillConvex.

For rectangular primitives, "p_min" and "p_max" represent the upper-left and
lower-right corners.

For circle primitives, use "num_segments == 0" to automatically calculate
tessellation (preferred).)");

API_FUNC(0_1, void, DrawList_AddLine, (DrawListProxy*,draw_list)
(double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y)
(int,col_rgba) (RO<double*>,thickness,1.0),
"")
{
  draw_list->get()->AddLine(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    Color::fromBigEndian(col_rgba), API_GET(thickness));
}

API_FUNC(0_1, void, DrawList_AddRect, (DrawListProxy*,draw_list)
(double,p_min_x) (double,p_min_y) (double,p_max_x) (double,p_max_y) (int,col_rgba)
(RO<double*>,rounding,0.0) (RO<int*>,flags,ImDrawFlags_None)
(RO<double*>,thickness,1.0),
"")
{
  draw_list->get()->AddRect(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::fromBigEndian(col_rgba),
    API_GET(rounding), API_GET(flags), API_GET(thickness));
}

API_FUNC(0_1, void, DrawList_AddRectFilled, (DrawListProxy*,draw_list)
(double,p_min_x) (double,p_min_y) (double,p_max_x) (double,p_max_y) (int,col_rgba)
(RO<double*>,rounding,0.0) (RO<int*>,flags,ImDrawFlags_None),
"")
{
  draw_list->get()->AddRectFilled(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::fromBigEndian(col_rgba), API_GET(rounding), API_GET(flags));
}

API_FUNC(0_1, void, DrawList_AddRectFilledMultiColor, (DrawListProxy*,draw_list)
(double,p_min_x) (double,p_min_y) (double,p_max_x) (double,p_max_y)
(int,col_upr_left) (int,col_upr_right) (int,col_bot_right) (int,col_bot_left),
"")
{
  draw_list->get()->AddRectFilledMultiColor(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::fromBigEndian(col_upr_left),  Color::fromBigEndian(col_upr_right),
    Color::fromBigEndian(col_bot_right), Color::fromBigEndian(col_bot_left));
}

API_FUNC(0_1, void, DrawList_AddQuad, (DrawListProxy*,draw_list)
(double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y)
(double,p3_x) (double,p3_y) (double,p4_x) (double,p4_y)
(int,col_rgba) (RO<double*>,thickness,1.0),
"")
{
  draw_list->get()->AddQuad(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    ImVec2(p4_x, p4_y), Color::fromBigEndian(col_rgba), API_GET(thickness));
}

API_FUNC(0_1, void, DrawList_AddQuadFilled, (DrawListProxy*,draw_list)
(double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y)
(double,p3_x) (double,p3_y) (double,p4_x) (double,p4_y)
(int,col_rgba),
"")
{
  draw_list->get()->AddQuadFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    ImVec2(p4_x, p4_y), Color::fromBigEndian(col_rgba));
}

API_FUNC(0_1, void, DrawList_AddTriangle, (DrawListProxy*,draw_list)
(double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y)
(double,p3_x) (double,p3_y) (int,col_rgba) (RO<double*>,thickness,1.0),
"")
{
  draw_list->get()->AddTriangle(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::fromBigEndian(col_rgba), API_GET(thickness));
}

API_FUNC(0_1, void, DrawList_AddTriangleFilled, (DrawListProxy*,draw_list)
(double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y)
(double,p3_x) (double,p3_y) (int,col_rgba),
"")
{
  draw_list->get()->AddTriangleFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::fromBigEndian(col_rgba));
}

API_FUNC(0_1, void, DrawList_AddCircle, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius) (int,col_rgba)
(RO<int*>,num_segments,0) (RO<double*>,thickness,1.0),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).)")
{
  draw_list->get()->AddCircle(ImVec2(center_x, center_y), radius,
    Color::fromBigEndian(col_rgba),
    API_GET(num_segments), API_GET(thickness));
}

API_FUNC(0_1, void, DrawList_AddCircleFilled, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius) (int,col_rgba)
(RO<int*>,num_segments,0),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).)")
{
  draw_list->get()->AddCircleFilled(ImVec2(center_x, center_y), radius,
    Color::fromBigEndian(col_rgba), API_GET(num_segments));
}

API_FUNC(0_1, void, DrawList_AddNgon, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius) (int,col_rgba)
(int,num_segments) (RO<double*>,thickness,1.0),
"")
{
  draw_list->get()->AddNgon(ImVec2(center_x, center_y), radius,
    Color::fromBigEndian(col_rgba), num_segments, API_GET(thickness));
}

API_FUNC(0_1, void, DrawList_AddNgonFilled, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius) (int,col_rgba)
(int,num_segments),
"")
{
  draw_list->get()->AddNgonFilled(ImVec2(center_x, center_y),
    radius, Color::fromBigEndian(col_rgba), num_segments);
}

API_FUNC(0_9, void, DrawList_AddEllipse, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius_x) (double,radius_y) (int,col_rgba)
(RO<double*>,rot,0.0) (RO<int*>,num_segments,0) (RO<double*>,thickness,1.0),
"")
{
  draw_list->get()->AddEllipse(
    ImVec2(center_x, center_y), ImVec2(radius_x, radius_y),
    Color::fromBigEndian(col_rgba), API_GET(rot), API_GET(num_segments),
    API_GET(thickness));
}

API_FUNC(0_9, void, DrawList_AddEllipseFilled, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius_x) (double,radius_y) (int,col_rgba)
(RO<double*>,rot,0.0) (RO<int*>,num_segments,0),
"")
{
  draw_list->get()->AddEllipseFilled(
    ImVec2(center_x, center_y), ImVec2(radius_x, radius_y),
    Color::fromBigEndian(col_rgba), API_GET(rot), API_GET(num_segments));
}

API_FUNC(0_1, void, DrawList_AddText, (DrawListProxy*,draw_list)
(double,x) (double,y) (int,col_rgba) (const char*,text),
"")
{
  draw_list->get()->AddText(ImVec2(x, y), Color::fromBigEndian(col_rgba), text);
}

API_FUNC(0_4, void, DrawList_AddTextEx, (DrawListProxy*,draw_list)
(Font*,font) (double,font_size) (double,pos_x) (double,pos_y)
(int,col_rgba) (const char*,text) (RO<double*>,wrap_width,0.0)
(RO<double*>,cpu_fine_clip_rect_min_x) (RO<double*>,cpu_fine_clip_rect_min_y)
(RO<double*>,cpu_fine_clip_rect_max_x) (RO<double*>,cpu_fine_clip_rect_max_y),
R"(The last pushed font is used if font is nil.
The size of the last pushed font is used if font_size is 0.
cpu_fine_clip_rect_* only takes effect if all four are non-nil.)")
{
  col_rgba = Color::fromBigEndian(col_rgba);

  ImVec4 cpu_fine_clip_rect, *cpu_fine_clip_rect_ptr;
  if(cpu_fine_clip_rect_min_x && cpu_fine_clip_rect_min_y &&
      cpu_fine_clip_rect_max_x && cpu_fine_clip_rect_max_y) {
    cpu_fine_clip_rect.x = *cpu_fine_clip_rect_min_x;
    cpu_fine_clip_rect.y = *cpu_fine_clip_rect_min_y;
    cpu_fine_clip_rect.z = *cpu_fine_clip_rect_max_x;
    cpu_fine_clip_rect.w = *cpu_fine_clip_rect_max_y;
    cpu_fine_clip_rect_ptr = &cpu_fine_clip_rect;
  }
  else
    cpu_fine_clip_rect_ptr = nullptr;

  Context *ctx;
  draw_list->get(&ctx)->AddText(ctx->fonts().instanceOf(font), font_size,
    ImVec2(pos_x, pos_y), col_rgba, text, nullptr, API_GET(wrap_width),
    cpu_fine_clip_rect_ptr);
}

static std::vector<ImVec2> makePointsArray(const reaper_array *points)
{
  assertValid(points);

  if(points->size % 2) {
    throw reascript_error
      {"an odd amount of points was provided (expected x,y pairs)"};
  }

  std::vector<ImVec2> out;
  out.reserve(points->size / 2);
  for(unsigned int i {}; i < points->size; i += 2)
    out.push_back(ImVec2(points->data[i], points->data[i+1]));
  return out;
}

API_FUNC(0_1, void, DrawList_AddBezierCubic, (DrawListProxy*,draw_list)
(double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y)
(double,p3_x) (double,p3_y) (double,p4_x) (double,p4_y)
(int,col_rgba) (double,thickness) (RO<int*>,num_segments,0),
"Cubic Bezier (4 control points)")
{
  draw_list->get()->AddBezierCubic(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    Color::fromBigEndian(col_rgba), thickness, API_GET(num_segments));
}

API_FUNC(0_1, void, DrawList_AddBezierQuadratic, (DrawListProxy*,draw_list)
(double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y) (double,p3_x) (double,p3_y)
(int,col_rgba) (double,thickness) (RO<int*>,num_segments,0),
"Quadratic Bezier (3 control points)")
{
  draw_list->get()->AddBezierQuadratic(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::fromBigEndian(col_rgba), thickness, API_GET(num_segments));
}

API_FUNC(0_2, void, DrawList_AddPolyline, (DrawListProxy*,draw_list)
(reaper_array*,points) (int,col_rgba) (int,flags) (double,thickness),
"Points is a list of x,y coordinates.")
{
  const std::vector<ImVec2> &vec2points {makePointsArray(points)};
  draw_list->get()->AddPolyline(
    vec2points.data(), vec2points.size(), Color::fromBigEndian(col_rgba),
    flags, thickness);
}

API_FUNC(0_6, void, DrawList_AddConvexPolyFilled, (DrawListProxy*,draw_list)
(reaper_array*,points) (int,col_rgba),
"Note: Anti-aliased filling requires points to be in clockwise order.")
{
  const std::vector<ImVec2> &vec2points {makePointsArray(points)};
  draw_list->get()->AddConvexPolyFilled(
    vec2points.data(), vec2points.size(), Color::fromBigEndian(col_rgba));
}

API_FUNC(0_9, void, DrawList_AddConcavePolyFilled, (DrawListProxy*,draw_list)
(reaper_array*,points) (int,col_rgba),
"Concave polygon fill is more expensive than convex one: it has O(N^2) complexity.")
{
  const std::vector<ImVec2> &vec2points {makePointsArray(points)};
  draw_list->get()->AddConcavePolyFilled(
    vec2points.data(), vec2points.size(), Color::fromBigEndian(col_rgba));
}

API_FUNC(0_8, void, DrawList_AddImage, (DrawListProxy*,draw_list)
(Image*,image)
(double,p_min_x) (double,p_min_y) (double,p_max_x) (double,p_max_y)
(RO<double*>,uv_min_x,0.0) (RO<double*>,uv_min_y,0.0)
(RO<double*>,uv_max_x,1.0) (RO<double*>,uv_max_y,1.0)
(RO<int*>,col_rgba,0xFFFFFFFF),
"")
{
  Context *ctx;
  ImDrawList *dl {draw_list->get(&ctx)};
  assertValid(image);
  dl->AddImage(image->makeTexture(ctx->textureManager()),
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    ImVec2(API_GET(uv_min_x), API_GET(uv_min_y)),
    ImVec2(API_GET(uv_max_x), API_GET(uv_max_y)),
    Color::fromBigEndian(API_GET(col_rgba)));
}

API_FUNC(0_8, void, DrawList_AddImageQuad, (DrawListProxy*,draw_list)
(Image*,image) (double,p1_x) (double,p1_y) (double,p2_x) (double,p2_y)
(double,p3_x) (double,p3_y) (double,p4_x) (double,p4_y)
(RO<double*>,uv1_x,0.0) (RO<double*>,uv1_y,0.0)
(RO<double*>,uv2_x,1.0) (RO<double*>,uv2_y,0.0)
(RO<double*>,uv3_x,1.0) (RO<double*>,uv3_y,1.0)
(RO<double*>,uv4_x,0.0) (RO<double*>,uv4_y,1.0)
(RO<int*>,col_rgba,0xFFFFFFFF),
"")
{
  Context *ctx;
  ImDrawList *dl {draw_list->get(&ctx)};
  assertValid(image);
  dl->AddImageQuad(image->makeTexture(ctx->textureManager()),
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    ImVec2(API_GET(uv1_x), API_GET(uv1_y)),
    ImVec2(API_GET(uv2_x), API_GET(uv2_y)),
    ImVec2(API_GET(uv3_x), API_GET(uv3_y)),
    ImVec2(API_GET(uv4_x), API_GET(uv4_y)),
    Color::fromBigEndian(API_GET(col_rgba)));
}

API_FUNC(0_8, void, DrawList_AddImageRounded, (DrawListProxy*,draw_list)
(Image*,image)
(double,p_min_x) (double,p_min_y) (double,p_max_x) (double,p_max_y)
(double,uv_min_x) (double,uv_min_y) (double,uv_max_x) (double,uv_max_y)
(int,col_rgba) (double,rounding) (RO<int*>,flags,ImDrawFlags_None),
"")
{
  Context *ctx;
  ImDrawList *dl {draw_list->get(&ctx)};
  assertValid(image);
  dl->AddImageRounded(image->makeTexture(ctx->textureManager()),
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    ImVec2(uv_min_x, uv_min_y), ImVec2(uv_max_x, uv_max_y),
    Color::fromBigEndian(col_rgba), rounding, API_GET(flags));
}

API_SUBSECTION("Stateful Path",
"Stateful path API, add points then finish with PathFillConvex() or PathStroke().");

API_FUNC(0_1, void, DrawList_PathClear, (DrawListProxy*,draw_list),
"")
{
  draw_list->get()->PathClear();
}

API_FUNC(0_1, void, DrawList_PathLineTo, (DrawListProxy*,draw_list)
(double,pos_x) (double,pos_y),
"")
{
  draw_list->get()->PathLineToMergeDuplicate(ImVec2(pos_x, pos_y));
}

API_FUNC(0_5_1, void, DrawList_PathFillConvex, (DrawListProxy*,draw_list)
(int,col_rgba),
"")
{
  draw_list->get()->PathFillConvex(Color::fromBigEndian(col_rgba));
}

API_FUNC(0_9, void, DrawList_PathFillConcave, (DrawListProxy*,draw_list)
(int,col_rgba),
"")
{
  draw_list->get()->PathFillConcave(Color::fromBigEndian(col_rgba));
}

API_FUNC(0_2, void, DrawList_PathStroke, (DrawListProxy*,draw_list)
(int,col_rgba) (RO<int*>,flags,ImDrawFlags_None) (RO<double*>,thickness,1.0),
"")
{
  draw_list->get()->PathStroke(
    Color::fromBigEndian(col_rgba), API_GET(flags), API_GET(thickness));
}

API_FUNC(0_1, void, DrawList_PathArcTo, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius) (double,a_min) (double,a_max)
(RO<int*>,num_segments,0),
"")
{
  draw_list->get()->PathArcTo(ImVec2(center_x, center_y),
    radius, a_min, a_max, API_GET(num_segments));
}

API_FUNC(0_1, void, DrawList_PathArcToFast, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius)
(int,a_min_of_12) (int,a_max_of_12),
"Use precomputed angles for a 12 steps circle.")
{
  draw_list->get()->PathArcToFast(
    ImVec2(center_x, center_y), radius, a_min_of_12, a_max_of_12);
}

API_FUNC(0_9, void, DrawList_PathEllipticalArcTo, (DrawListProxy*,draw_list)
(double,center_x) (double,center_y) (double,radius_x) (double,radius_y)
(double,rot) (double,a_min) (double,a_max) (RO<int*>,num_segments,0),
"Ellipse")
{
  draw_list->get()->PathEllipticalArcTo(
    ImVec2(center_x, center_y), ImVec2(radius_x, radius_y),
    rot, a_min, a_max, API_GET(num_segments));
}

API_FUNC(0_1, void, DrawList_PathBezierCubicCurveTo, (DrawListProxy*,draw_list)
(double,p2_x) (double,p2_y) (double,p3_x) (double,p3_y) (double,p4_x) (double,p4_y)
(RO<int*>,num_segments,0),
"Cubic Bezier (4 control points)")
{
  draw_list->get()->PathBezierCubicCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    API_GET(num_segments));
}

API_FUNC(0_1, void, DrawList_PathBezierQuadraticCurveTo, (DrawListProxy*,draw_list)
(double,p2_x) (double,p2_y) (double,p3_x) (double,p3_y) (RO<int*>,num_segments,0),
"Quadratic Bezier (3 control points)")
{
  draw_list->get()->PathBezierQuadraticCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), API_GET(num_segments));
}

API_FUNC(0_1, void, DrawList_PathRect, (DrawListProxy*,draw_list)
(double,rect_min_x) (double,rect_min_y) (double,rect_max_x) (double,rect_max_y)
(RO<double*>,rounding,0.0) (RO<int*>,flags,ImDrawFlags_None),
"")
{
  draw_list->get()->PathRect(ImVec2(rect_min_x, rect_min_y),
    ImVec2(rect_max_x, rect_max_y), API_GET(rounding), API_GET(flags));
}

DrawListSplitter::DrawListSplitter(DrawListProxy *draw_list)
  : m_drawlist {draw_list}, m_lastList {draw_list->get()}
{
}

bool DrawListSplitter::isValid() const
{
  return !!DrawListProxy::isValid(m_drawlist);
}

ImDrawListSplitter *DrawListSplitter::operator->()
{
  assertValid(this);
  return &m_splitter;
}

ImDrawList *DrawListSplitter::drawList() const
{
  // Prevent crashes when calling Merge on a different window and its drawlist
  // (eg. Foreground) is still unused and blank.
  //
  // Merge only copies indices and commands, NOT vertices.
  if(m_drawlist->get() == m_lastList)
    return m_lastList;
  else
    throw reascript_error
      {"cannot use DrawListSplitter over multiple windows"};
}

API_SUBSECTION("Splitter",
R"(Split/Merge functions are used to split the draw list into different layers
which can be drawn into out of order (e.g. submit FG primitives before BG primitives).

Use to minimize draw calls (e.g. if going back-and-forth between multiple
clipping rectangles, prefer to append into separate channels then merge at the end).

Usage:

    if not ImGui.ValidatePtr(splitter, 'ImGui_DrawListSplitter*') then
      splitter = ImGui.CreateDrawListSplitter(draw_list)
    end
    ImGui.DrawListSplitter_Split(splitter, 2)
    ImGui.DrawListSplitter_SetCurrentChannel(splitter, 0)
    ImGui.DrawList_AddRectFilled(draw_list, ...) -- background
    ImGui.DrawListSplitter_SetCurrentChannel(splitter, 1)
    ImGui.DrawList_AddRectFilled(draw_list, ...) -- foreground
    ImGui.DrawListSplitter_SetCurrentChannel(splitter, 0)
    ImGui.DrawList_AddRectFilled(draw_list, ...) -- background
    ImGui.DrawListSplitter_Merge(splitter))");

API_FUNC(0_9, DrawListSplitter*, CreateDrawListSplitter,
(DrawListProxy*,draw_list),
"")
{
  return new DrawListSplitter {draw_list};
}

API_FUNC(0_7_1, void, DrawListSplitter_Clear, (DrawListSplitter*,splitter),
"")
{
  (*splitter)->Clear();
}

API_FUNC(0_7_1, void, DrawListSplitter_Split, (DrawListSplitter*,splitter)
(int,count),
"")
{
  (*splitter)->Split(splitter->drawList(), count);
}

API_FUNC(0_7_1, void, DrawListSplitter_Merge, (DrawListSplitter*,splitter),
"")
{
  (*splitter)->Merge(splitter->drawList());
}

API_FUNC(0_7_1, void, DrawListSplitter_SetCurrentChannel,
(DrawListSplitter*,splitter) (int,channel_idx),
"")
{
  (*splitter)->SetCurrentChannel(splitter->drawList(), channel_idx);
}
