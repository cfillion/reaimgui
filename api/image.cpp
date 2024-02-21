/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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

#include "../src/color.hpp"
#include "../src/image.hpp"

API_SECTION("Image",
R"(ReaImGui currently supports loading PNG and JPEG bitmap images.
Flat vector images may be loaded as fonts, see CreateFont.

UV parameters are texture coordinates in a scale of 0.0 (top/left) to 1.0
(bottom/right). Use values below 0.0 or above 1.0 to tile the image.

Width/height are limited to 8192 pixels.

There are also image functions in the DrawList API such as
DrawList_AddImageQuad and DrawList_AddImageRounded.)");

API_FUNC(0_8, ImGui_Image*, CreateImage,
(const char*,file)(int*,API_RO(flags)),
R"(The returned object is valid as long as it is used in each defer cycle
unless attached to a context (see Attach).

('flags' currently unused and reserved for future expansion))")
{
  (void)API_RO(flags);
  return Image::fromFile(file);
}

API_FUNC(0_8, ImGui_Image*, CreateImageFromMem,
(const char*,data)(int,data_sz),
R"(Requires REAPER v6.44 or newer for EEL and Lua. Load from a file using
CreateImage or explicitely specify data_sz if supporting older versions.)")
{
  // data_sz is inaccurate before REAPER 6.44
  return Image::fromMemory(data, data_sz);
}

API_FUNC(0_8, void, Image_GetSize, (ImGui_Image*,img)
(double*,API_W(w))(double*,API_W(h)),
"")
{
  assertValid(img);
  if(API_W(w)) *API_W(w) = img->width();
  if(API_W(h)) *API_W(h) = img->height();
}

API_FUNC(0_8, void, Image, (ImGui_Context*,ctx)
(ImGui_Image*,img)(double,size_w)(double,size_h)
(double*,API_RO(uv0_x),0.0)(double*,API_RO(uv0_y),0.0)
(double*,API_RO(uv1_x),1.0)(double*,API_RO(uv1_y),1.0)
(int*,API_RO(tint_col_rgba),0xFFFFFFFF)(int*,API_RO(border_col_rgba),0x00000000),
"")
{
  FRAME_GUARD;
  assertValid(img);

  const ImTextureID tex { img->makeTexture(ctx->textureManager()) };
  ImGui::Image(tex, ImVec2(size_w, size_h),
    ImVec2(API_RO_GET(uv0_x), API_RO_GET(uv0_y)),
    ImVec2(API_RO_GET(uv1_x), API_RO_GET(uv1_y)),
    Color(API_RO_GET(tint_col_rgba)), Color(API_RO_GET(border_col_rgba)));
}

API_FUNC(0_8, bool, ImageButton, (ImGui_Context*,ctx)
(const char*,str_id)(ImGui_Image*,img)(double,size_w)(double,size_h)
(double*,API_RO(uv0_x),0.0)(double*,API_RO(uv0_y),0.0)
(double*,API_RO(uv1_x),1.0)(double*,API_RO(uv1_y),1.0)
(int*,API_RO(bg_col_rgba),0x00000000)(int*,API_RO(tint_col_rgba),0xFFFFFFFF),
"")
{
  FRAME_GUARD;
  assertValid(img);

  const ImTextureID tex { img->makeTexture(ctx->textureManager()) };
  return ImGui::ImageButton(str_id, tex, ImVec2(size_w, size_h),
    ImVec2(API_RO_GET(uv0_x), API_RO_GET(uv0_y)),
    ImVec2(API_RO_GET(uv1_x), API_RO_GET(uv1_y)),
    Color(API_RO_GET(bg_col_rgba)), Color(API_RO_GET(tint_col_rgba)));
}

API_SUBSECTION("Image Set",
R"(Helper to automatically select and scale an image to the DPI scale of
the current window upon usage.

ImGui_ImageSet objects can be given to any function that expect an image as
parameter.

Usage:

    local set = ImGui.CreateImageSet()
    ImGui.ImageSet_Add(set, 1.0, ImGui.CreateImage('32x32.png'))
    ImGui.ImageSet_Add(set, 2.0, ImGui.CreateImage('64x64.png'))

    local function frame()
      ImGui.Image(ctx, set, ImGui.Image_GetSize(set))
      -- ...
    end)");

API_FUNC(0_9, ImGui_ImageSet*, CreateImageSet, NO_ARGS,
"")
{
  return new ImageSet;
}

API_FUNC(0_8, void, ImageSet_Add, (ImGui_ImageSet*,set)
(double,scale)(ImGui_Image*,img),
"'img' cannot be another ImageSet.")
{
  assertValid(set);
  assertValid(img);
  set->add(scale, img);
}
