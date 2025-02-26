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

#include "../src/color.hpp"
#include "../src/image.hpp"

#include <reaper_plugin_functions.h>

API_SECTION("Image",
R"(ReaImGui currently supports loading PNG and JPEG bitmap images.
Flat vector images may be loaded as fonts, see CreateFont.

UV parameters are texture coordinates in a scale of 0.0 (top/left) to 1.0
(bottom/right). Use values below 0.0 or above 1.0 to tile the image.

Width/height are limited to 8192 pixels.

There are also image functions in the DrawList API such as
DrawList_AddImageQuad and DrawList_AddImageRounded.

Caching of image objects may be implemented like this:

    local images = {}
    local function imageFromCache(fn)
      local img = images[fn]
      if not img then
        img = {}
        images[fn] = img
      end

      if not ImGui.ValidatePtr(img.inst, 'ImGui_Image*') then
        if img.inst then images[img.inst] = nil end
        img.inst = ImGui.CreateImage(fn)
        local prev = images[img.inst]
        if prev and prev ~= img then prev.inst = nil end
        images[img.inst] = img
      end

      return img.inst
    end)");

API_FUNC(0_9, Image*, CreateImage,
(const char*,file) (RO<int*>,flags),
R"(The returned object is valid as long as it is used in each defer cycle
unless attached to a context (see Attach).

('flags' currently unused and reserved for future expansion))")
{
  (void)flags;
  return Image::fromFile(file);
}

API_FUNC(0_9, Image*, CreateImageFromMem,
(const char*,data) (int,data_sz) (RO<int*>,flags),
R"(Requires REAPER v6.44 or newer for EEL and Lua. Load from a file using
CreateImage or explicitely specify data_sz to support older versions.)")
{
  // data_sz is inaccurate before REAPER 6.44
  return Image::fromMemory(data, data_sz);
}

API_REGISTER_BASIC_TYPE(LICE_IBitmap*);

template<>
void assertValid(LICE_IBitmap *ptr)
{
  static int (*JS_LICE_GetWidth)(LICE_IBitmap *);
  if(!JS_LICE_GetWidth)
    JS_LICE_GetWidth = reinterpret_cast<decltype(JS_LICE_GetWidth)>
      (plugin_getapi("JS_LICE_GetWidth"));
  if(!JS_LICE_GetWidth)
    throw reascript_error {"cannot load JS_LICE_GetWidth"};
  if(!JS_LICE_GetWidth(ptr))
    Error::invalidObject(ptr);
}

API_FUNC(0_9_2, Image*, CreateImageFromLICE,
(LICE_IBitmap*,bitmap) (RO<int*>,flags),
"Copies pixel data from a LICE bitmap created using JS_LICE_CreateBitmap.")
{
  assertValid(bitmap);
  return new LICEBitmap(bitmap);
}

API_FUNC(0_8, void, Image_GetSize, (class Image*,image)
(W<double*>,w) (W<double*>,h),
"")
{
  assertValid(image);
  if(w) *w = image->width();
  if(h) *h = image->height();
}

API_FUNC(0_8, void, Image, (Context*,ctx)
(class Image*,image) (double,image_size_w) (double,image_size_h)
(RO<double*>,uv0_x,0.0) (RO<double*>,uv0_y,0.0)
(RO<double*>,uv1_x,1.0) (RO<double*>,uv1_y,1.0)
(RO<int*>,tint_col_rgba,0xFFFFFFFF) (RO<int*>,border_col_rgba,0x00000000),
"Adds 2.0 to the provided size if a border is visible.")
{
  FRAME_GUARD;
  assertValid(image);

  const ImTextureID tex {image->makeTexture(ctx->textureManager())};
  ImGui::Image(tex, ImVec2(image_size_w, image_size_h),
    ImVec2(API_GET(uv0_x), API_GET(uv0_y)),
    ImVec2(API_GET(uv1_x), API_GET(uv1_y)),
    Color(API_GET(tint_col_rgba)), Color(API_GET(border_col_rgba)));
}

API_FUNC(0_8, bool, ImageButton, (Context*,ctx)
(const char*,str_id) (class Image*,image) (double,image_size_w) (double,image_size_h)
(RO<double*>,uv0_x,0.0) (RO<double*>,uv0_y,0.0)
(RO<double*>,uv1_x,1.0) (RO<double*>,uv1_y,1.0)
(RO<int*>,bg_col_rgba,0x00000000) (RO<int*>,tint_col_rgba,0xFFFFFFFF),
"Adds StyleVar_FramePadding*2.0 to provided size.")
{
  FRAME_GUARD;
  assertValid(image);

  const ImTextureID tex {image->makeTexture(ctx->textureManager())};
  return ImGui::ImageButton(str_id, tex, ImVec2(image_size_w, image_size_h),
    ImVec2(API_GET(uv0_x), API_GET(uv0_y)),
    ImVec2(API_GET(uv1_x), API_GET(uv1_y)),
    Color(API_GET(bg_col_rgba)), Color(API_GET(tint_col_rgba)));
}

API_SUBSECTION("Image Set",
R"(Helper to automatically select and scale an image to the DPI scale of
the current window upon usage.

ImageSet objects may be used in any function that expect an image as parameter.

Usage:

    local set = ImGui.CreateImageSet()
    ImGui.ImageSet_Add(set, 1.0, ImGui.CreateImage('32x32.png'))
    ImGui.ImageSet_Add(set, 2.0, ImGui.CreateImage('64x64.png'))

    local function frame()
      ImGui.Image(ctx, set, ImGui.Image_GetSize(set))
      -- ...
    end)");

API_FUNC(0_9, ImageSet*, CreateImageSet, API_NO_ARGS,
"")
{
  return new ImageSet;
}

API_FUNC(0_8, void, ImageSet_Add, (class ImageSet*,set)
(double,scale) (class Image*,image),
"'img' cannot be another ImageSet.")
{
  assertValid(set);
  assertValid(image);
  set->add(scale, image);
}
