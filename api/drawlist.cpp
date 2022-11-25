/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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
#include "drawlist.hpp"
#include "font.hpp"
#include "image.hpp"
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
      assertFrame(ctx);
      return ImGui::GetWindowDrawList();
    case Background:
      assertFrame(ctx);
      return ImGui::GetBackgroundDrawList();
    case Foreground:
      assertFrame(ctx);
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

API_SECTION("Draw List",
R"(This is the low-level list of polygons that ImGui functions are filling.
At the end of the frame, all draw lists are passed to the GPU for rendering.

Each dear imgui window contains its own Draw List.
You can use GetWindowDrawList() to access the current window draw list and draw
custom primitives.

The Draw List API uses absolute coordinates (0,0 is the top-left corner of the
rimary monitor, not of your window!). See GetCursorScreenPos.)");

DEFINE_API(ImGui_DrawList*, GetWindowDrawList, (ImGui_Context*,ctx),
"The draw list associated to the current window, to append your own drawing primitives",
{
  return ResourceProxy::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Window);
});

DEFINE_API(ImGui_DrawList*, GetBackgroundDrawList, (ImGui_Context*,ctx),
R"(This draw list will be the first rendering one. Useful to quickly draw
shapes/text behind dear imgui contents.)",
{
  return ResourceProxy::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Background);
});

DEFINE_API(ImGui_DrawList*, GetForegroundDrawList, (ImGui_Context*,ctx),
R"(This draw list will be the last rendered one. Useful to quickly draw
shapes/text over dear imgui contents.)",
{
  return ResourceProxy::encode<ImGui_DrawList>(ctx, ImGui_DrawList::Foreground);
});

DEFINE_API(void, DrawList_PushClipRect, (ImGui_DrawList*,draw_list)
(double,clip_rect_min_x)(double,clip_rect_min_y)
(double,clip_rect_max_x)(double,clip_rect_max_y)
(bool*,API_RO(intersect_with_current_clip_rect),false),
R"(Render-level scissoring. Prefer using higher-level PushClipRect to affect
logic (hit-testing and widget culling).)",
{
  draw_list->get()->PushClipRect(
    ImVec2(clip_rect_min_x, clip_rect_min_y),
    ImVec2(clip_rect_max_x, clip_rect_max_y),
    API_RO_GET(intersect_with_current_clip_rect));
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

DEFINE_ENUM(Im, DrawFlags_None, "");
DEFINE_ENUM(Im, DrawFlags_Closed,
R"(DrawList_PathStroke, DrawList_AddPolyline: specify that shape should be
   closed (Important: this is always == 1 for legacy reason).)");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTopLeft,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   top-left corner only (when rounding > 0.0, we default to all corners).)");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTopRight,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   top-right corner only (when rounding > 0.0, we default to all corners).)");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottomLeft,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   bottom-left corner only (when rounding > 0.0, we default to all corners).)");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottomRight,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: enable rounding
   bottom-right corner only (when rounding > 0.0, we default to all corners).)");
DEFINE_ENUM(Im, DrawFlags_RoundCornersNone,
R"(DrawList_AddRect, DrawList_AddRectFilled, DrawList_PathRect: disable rounding
   on all corners (when rounding > 0.0). This is NOT zero, NOT an implicit flag!.)");
DEFINE_ENUM(Im, DrawFlags_RoundCornersTop,    "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersBottom, "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersLeft,   "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersRight,  "");
DEFINE_ENUM(Im, DrawFlags_RoundCornersAll,    "");

API_SUBSECTION("Primitives",
R"(Filled shapes must always use clockwise winding order. The anti-aliasing
fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.

For rectangular primitives, "p_min" and "p_max" represent the upper-left and
lower-right corners.

For circle primitives, use "num_segments == 0" to automatically calculate
tessellation (preferred).)");

DEFINE_API(void, DrawList_AddLine, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(int,col_rgba)(double*,API_RO(thickness),1.0),
"",
{
  draw_list->get()->AddLine(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    Color::fromBigEndian(col_rgba), API_RO_GET(thickness));
});

DEFINE_API(void, DrawList_AddRect, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)(int,col_rgba)
(double*,API_RO(rounding),0.0)(int*,API_RO(flags),ImDrawFlags_None)
(double*,API_RO(thickness),1.0),
"",
{
  draw_list->get()->AddRect(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::fromBigEndian(col_rgba),
    API_RO_GET(rounding), API_RO_GET(flags), API_RO_GET(thickness));
});

DEFINE_API(void, DrawList_AddRectFilled, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)(int,col_rgba)
(double*,API_RO(rounding),0.0)(int*,API_RO(flags),ImDrawFlags_None),
"",
{
  draw_list->get()->AddRectFilled(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::fromBigEndian(col_rgba), API_RO_GET(rounding), API_RO_GET(flags));
});

DEFINE_API(void, DrawList_AddRectFilledMultiColor, (ImGui_DrawList*,draw_list)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)
(int,col_upr_left)(int,col_upr_right)(int,col_bot_right)(int,col_bot_left),
"",
{
  draw_list->get()->AddRectFilledMultiColor(
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    Color::fromBigEndian(col_upr_left),  Color::fromBigEndian(col_upr_right),
    Color::fromBigEndian(col_bot_right), Color::fromBigEndian(col_bot_left));
});

DEFINE_API(void, DrawList_AddQuad, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int,col_rgba)(double*,API_RO(thickness),1.0),
"",
{
  draw_list->get()->AddQuad(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    ImVec2(p4_x, p4_y), Color::fromBigEndian(col_rgba), API_RO_GET(thickness));
});

DEFINE_API(void, DrawList_AddQuadFilled, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int,col_rgba),
"",
{
  draw_list->get()->AddQuadFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    ImVec2(p4_x, p4_y), Color::fromBigEndian(col_rgba));
});

DEFINE_API(void, DrawList_AddTriangle, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(int,col_rgba)(double*,API_RO(thickness),1.0),
"",
{
  draw_list->get()->AddTriangle(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::fromBigEndian(col_rgba), API_RO_GET(thickness));
});

DEFINE_API(void, DrawList_AddTriangleFilled, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(int,col_rgba),
"",
{
  draw_list->get()->AddTriangleFilled(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::fromBigEndian(col_rgba));
});

DEFINE_API(void, DrawList_AddCircle, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int*,API_RO(num_segments),0)(double*,API_RO(thickness),1.0),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).)",
{
  draw_list->get()->AddCircle(ImVec2(center_x, center_y), radius,
    Color::fromBigEndian(col_rgba),
    API_RO_GET(num_segments), API_RO_GET(thickness));
});

DEFINE_API(void, DrawList_AddCircleFilled, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int*,API_RO(num_segments),0),
R"(Use "num_segments == 0" to automatically calculate tessellation (preferred).)",
{
  draw_list->get()->AddCircleFilled(ImVec2(center_x, center_y), radius,
    Color::fromBigEndian(col_rgba), API_RO_GET(num_segments));
});

DEFINE_API(void, DrawList_AddNgon, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int,num_segments)(double*,API_RO(thickness),1.0),
"",
{
  draw_list->get()->AddNgon(ImVec2(center_x, center_y), radius,
    Color::fromBigEndian(col_rgba), num_segments, API_RO_GET(thickness));
});

DEFINE_API(void, DrawList_AddNgonFilled, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(int,col_rgba)
(int,num_segments),
"",
{
  draw_list->get()->AddNgonFilled(ImVec2(center_x, center_y),
    radius, Color::fromBigEndian(col_rgba), num_segments);
});

DEFINE_API(void, DrawList_AddText, (ImGui_DrawList*,draw_list)
(double,x)(double,y)(int,col_rgba)(const char*,text),
"",
{
  draw_list->get()->AddText(ImVec2(x, y), Color::fromBigEndian(col_rgba), text);
});

DEFINE_API(void, DrawList_AddTextEx, (ImGui_DrawList*,draw_list)
(ImGui_Font*,font)(double,font_size)(double,pos_x)(double,pos_y)
(int,col_rgba)(const char*,text)(double*,API_RO(wrap_width),0.0)
(double*,API_RO(cpu_fine_clip_rect_x))(double*,API_RO(cpu_fine_clip_rect_y))
(double*,API_RO(cpu_fine_clip_rect_w))(double*,API_RO(cpu_fine_clip_rect_h)),
R"(The last pushed font is used if font is nil.
The size of the last pushed font is used if font_size is 0.
cpu_fine_clip_rect_* only takes effect if all four are non-nil.)",
{
  col_rgba = Color::fromBigEndian(col_rgba);

  ImVec2 pos;
  pos.x = pos_x;
  pos.y = pos_y;

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
    pos, col_rgba, text, nullptr, API_RO_GET(wrap_width),
    cpu_fine_clip_rect_ptr);
});

static std::vector<ImVec2> makePointsArray(const reaper_array *points)
{
  assertValid(points);

  if(points->size % 2) {
    throw reascript_error
      { "an odd amount of points was provided (expected x,y pairs)" };
  }

  std::vector<ImVec2> out;
  out.reserve(points->size / 2);
  for(unsigned int i {}; i < points->size; i += 2)
    out.push_back(ImVec2(points->data[i], points->data[i+1]));
  return out;
}

DEFINE_API(void, DrawList_AddPolyline, (ImGui_DrawList*,draw_list)
(reaper_array*,points)(int,col_rgba)(int,flags)(double,thickness),
"Points is a list of x,y coordinates.",
{
  const std::vector<ImVec2> &vec2points { makePointsArray(points) };
  draw_list->get()->AddPolyline(
    vec2points.data(), vec2points.size(), Color::fromBigEndian(col_rgba),
    flags, thickness);
});

DEFINE_API(void, DrawList_AddConvexPolyFilled, (ImGui_DrawList*,draw_list)
(reaper_array*,points)(int,col_rgba),
"Note: Anti-aliased filling requires points to be in clockwise order.",
{
  const std::vector<ImVec2> &vec2points { makePointsArray(points) };
  draw_list->get()->AddConvexPolyFilled(
    vec2points.data(), vec2points.size(), Color::fromBigEndian(col_rgba));
});

DEFINE_API(void, DrawList_AddBezierCubic, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(int,col_rgba)(double,thickness)(int*,API_RO(num_segments),0),
"Cubic Bezier (4 control points)",
{
  draw_list->get()->AddBezierCubic(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    Color::fromBigEndian(col_rgba), thickness, API_RO_GET(num_segments));
});

DEFINE_API(void, DrawList_AddBezierQuadratic, (ImGui_DrawList*,draw_list)
(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)(double,p3_x)(double,p3_y)
(int,col_rgba)(double,thickness)(int*,API_RO(num_segments),0),
"Quadratic Bezier (3 control points)",
{
  draw_list->get()->AddBezierQuadratic(
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y),
    Color::fromBigEndian(col_rgba), thickness, API_RO_GET(num_segments));
});

DEFINE_API(void, DrawList_AddImage, (ImGui_DrawList*,draw_list)
(ImGui_Image*,img)
(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)
(double*,API_RO(uv_min_x),0.0)(double*,API_RO(uv_min_y),0.0)
(double*,API_RO(uv_max_x),1.0)(double*,API_RO(uv_max_y),1.0)
(int*,API_RO(col_rgba),0xFFFFFFFF),
"",
{
  Context *ctx;
  ImDrawList *dl { draw_list->get(&ctx) };
  assertValid(img);
  dl->AddImage(img->makeTexture(ctx->textureManager()),
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    ImVec2(API_RO_GET(uv_min_x), API_RO_GET(uv_min_y)),
    ImVec2(API_RO_GET(uv_max_x), API_RO_GET(uv_max_y)),
    Color::fromBigEndian(API_RO_GET(col_rgba)));
});

DEFINE_API(void, DrawList_AddImageQuad, (ImGui_DrawList*,draw_list)
(ImGui_Image*,img)(double,p1_x)(double,p1_y)(double,p2_x)(double,p2_y)
(double,p3_x)(double,p3_y)(double,p4_x)(double,p4_y)
(double*,API_RO(uv1_x),0.0)(double*,API_RO(uv1_y),0.0)
(double*,API_RO(uv2_x),1.0)(double*,API_RO(uv2_y),0.0)
(double*,API_RO(uv3_x),1.0)(double*,API_RO(uv3_y),1.0)
(double*,API_RO(uv4_x),0.0)(double*,API_RO(uv4_y),1.0)
(int*,API_RO(col_rgba),0xFFFFFFFF),
"",
{
  Context *ctx;
  ImDrawList *dl { draw_list->get(&ctx) };
  assertValid(img);
  dl->AddImageQuad(img->makeTexture(ctx->textureManager()),
    ImVec2(p1_x, p1_y), ImVec2(p2_x, p2_y),
    ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    ImVec2(API_RO_GET(uv1_x), API_RO_GET(uv1_y)),
    ImVec2(API_RO_GET(uv2_x), API_RO_GET(uv2_y)),
    ImVec2(API_RO_GET(uv3_x), API_RO_GET(uv3_y)),
    ImVec2(API_RO_GET(uv4_x), API_RO_GET(uv4_y)),
    Color::fromBigEndian(API_RO_GET(col_rgba)));
});

DEFINE_API(void, DrawList_AddImageRounded, (ImGui_DrawList*,draw_list)
(ImGui_Image*,img)(double,p_min_x)(double,p_min_y)(double,p_max_x)(double,p_max_y)
(double,uv_min_x)(double,uv_min_y)(double,uv_max_x)(double,uv_max_y)
(int,col_rgba)(double,rounding)(int*,API_RO(flags),ImDrawFlags_None),
"",
{
  Context *ctx;
  ImDrawList *dl { draw_list->get(&ctx) };
  assertValid(img);
  dl->AddImageRounded(img->makeTexture(ctx->textureManager()),
    ImVec2(p_min_x, p_min_y), ImVec2(p_max_x, p_max_y),
    ImVec2(uv_min_x, uv_min_y), ImVec2(uv_max_x, uv_max_y),
    Color::fromBigEndian(col_rgba), rounding, API_RO_GET(flags));
});

API_SUBSECTION("Stateful Path",
"Stateful path API, add points then finish with PathFillConvex() or PathStroke().");

DEFINE_API(void, DrawList_PathClear, (ImGui_DrawList*,draw_list),
"",
{
  draw_list->get()->PathClear();
});

DEFINE_API(void, DrawList_PathLineTo, (ImGui_DrawList*,draw_list)
(double,pos_x)(double,pos_y),
"",
{
  draw_list->get()->PathLineToMergeDuplicate(ImVec2(pos_x, pos_y));
});

DEFINE_API(void, DrawList_PathFillConvex, (ImGui_DrawList*,draw_list)
(int,col_rgba),
"Note: Anti-aliased filling requires points to be in clockwise order.",
{
  draw_list->get()->PathFillConvex(Color::fromBigEndian(col_rgba));
});

DEFINE_API(void, DrawList_PathStroke, (ImGui_DrawList*,draw_list)
(int,col_rgba)(int*,API_RO(flags),ImDrawFlags_None)(double*,API_RO(thickness),1.0),
"",
{
  draw_list->get()->PathStroke(
    Color::fromBigEndian(col_rgba), API_RO_GET(flags), API_RO_GET(thickness));
});

DEFINE_API(void, DrawList_PathArcTo, (ImGui_DrawList*,draw_list)
(double,center_x)(double,center_y)(double,radius)(double,a_min)(double,a_max)
(int*,API_RO(num_segments),0),
"",
{
  draw_list->get()->PathArcTo(ImVec2(center_x, center_y),
    radius, a_min, a_max, API_RO_GET(num_segments));
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
(int*,API_RO(num_segments),0),
"Cubic Bezier (4 control points)",
{
  draw_list->get()->PathBezierCubicCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), ImVec2(p4_x, p4_y),
    API_RO_GET(num_segments));
});

DEFINE_API(void, DrawList_PathBezierQuadraticCurveTo, (ImGui_DrawList*,draw_list)
(double,p2_x)(double,p2_y)(double,p3_x)(double,p3_y)(int*,API_RO(num_segments),0),
"Quadratic Bezier (3 control points)",
{
  draw_list->get()->PathBezierQuadraticCurveTo(
    ImVec2(p2_x, p2_y), ImVec2(p3_x, p3_y), API_RO_GET(num_segments));
});

DEFINE_API(void, DrawList_PathRect, (ImGui_DrawList*,draw_list)
(double,rect_min_x)(double,rect_min_y)(double,rect_max_x)(double,rect_max_y)
(double*,API_RO(rounding),0.0)(int*,API_RO(flags),ImDrawFlags_None),
"",
{
  draw_list->get()->PathRect(ImVec2(rect_min_x, rect_min_y),
    ImVec2(rect_max_x, rect_max_y), API_RO_GET(rounding), API_RO_GET(flags));
});

DrawListSplitter::DrawListSplitter(ImGui_DrawList *draw_list)
  : m_drawlist { draw_list }, m_lastList { draw_list->get() }
{
}

bool DrawListSplitter::isValid() const
{
  ResourceProxy::Key proxyKey;
  return !!DrawList.decode<Context>(m_drawlist, &proxyKey);
}

ImDrawListSplitter *DrawListSplitter::operator->()
{
  assertValid(this);
  keepAlive();
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
    throw reascript_error { "cannot use ImGui_DrawListSplitter over multiple windows" };
}

API_SUBSECTION("Splitter",
R"(Split/Merge functions are used to split the draw list into different layers
which can be drawn into out of order (e.g. submit FG primitives before BG primitives).

Use to minimize draw calls (e.g. if going back-and-forth between multiple
clipping rectangles, prefer to append into separate channels then merge at the end).

Usage:

    if not reaper.ImGui_ValidatePtr(splitter, 'ImGui_DrawListSplitter*') then
      splitter = reaper.ImGui_CreateDrawListSplitter(draw_list)
    end
    reaper.ImGui_DrawListSplitter_Split(splitter, 2)
    reaper.ImGui_DrawListSplitter_SetCurrentChannel(splitter, 0)
    reaper.ImGui_DrawList_AddRectFilled(draw_list, ...) -- background
    reaper.ImGui_DrawListSplitter_SetCurrentChannel(splitter, 1)
    reaper.ImGui_DrawList_AddRectFilled(draw_list, ...) -- foreground
    reaper.ImGui_DrawListSplitter_SetCurrentChannel(splitter, 0)
    reaper.ImGui_DrawList_AddRectFilled(draw_list, ...) -- background
    reaper.ImGui_DrawListSplitter_Merge(splitter))");

DEFINE_API(ImGui_DrawListSplitter*, CreateDrawListSplitter,
(ImGui_DrawList*,draw_list),
"",
{
  return new DrawListSplitter { draw_list };
});

DEFINE_API(void, DrawListSplitter_Clear, (ImGui_DrawListSplitter*,splitter),
"",
{
  (*splitter)->Clear();
});

DEFINE_API(void, DrawListSplitter_Split, (ImGui_DrawListSplitter*,splitter)
(int,count),
"",
{
  (*splitter)->Split(splitter->drawList(), count);
});

DEFINE_API(void, DrawListSplitter_Merge, (ImGui_DrawListSplitter*,splitter),
"",
{
  (*splitter)->Merge(splitter->drawList());
});

DEFINE_API(void, DrawListSplitter_SetCurrentChannel,
(ImGui_DrawListSplitter*,splitter)(int,channel_idx),
"",
{
  (*splitter)->SetCurrentChannel(splitter->drawList(), channel_idx);
});
