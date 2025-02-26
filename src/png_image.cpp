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

#include "image.hpp"

#include "error.hpp"

#include <png.h>   // http://www.libpng.org/pub/png/libpng-manual.txt
#include <cstring> // strerror

constexpr size_t HEADER_SIZE {8}; // must not be > 8

class PNGImage final : public Bitmap {
public:
  PNGImage(std::istream &);
};

static bool isPNG(std::istream &stream)
{
  png_byte header[HEADER_SIZE];
  if(!stream.read(reinterpret_cast<char *>(header), sizeof(header)))
    return false;
  return png_check_sig(header, sizeof(header));
}

static Image *create(std::istream &stream)
{
  return new PNGImage {stream};
}

static const Image::RegisterType PNG {&isPNG, &create};

static void read(png_structp png, png_bytep data, const png_size_t length)
{
  std::istream &stream {*static_cast<std::istream *>(png_get_io_ptr(png))};
  if(!stream.read(reinterpret_cast<char *>(data), length))
    png_error(png, stream.eof() ? "premature end of file" : strerror(errno));
}

static void error(png_structp, const char *what)
{
  throw reascript_error {what};
}

static void transformToRGBA(png_structp png, png_infop info)
{
  const auto colorType {png_get_color_type(png, info)};
  const auto bitDepth  {png_get_bit_depth(png,  info)};

  if(bitDepth == 16)
    png_set_strip_16(png);

  if(colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  if(colorType == PNG_COLOR_TYPE_RGB ||
     colorType == PNG_COLOR_TYPE_GRAY ||
     colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if(colorType == PNG_COLOR_TYPE_GRAY ||
     colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);
}

PNGImage::PNGImage(std::istream &stream)
{
  struct PNG {
    ~PNG() { png_destroy_read_struct(&read, &info, nullptr); }
    png_structp read;
    png_infop   info;
  } png;

  if(!(png.read =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, error, nullptr)))
    throw reascript_error {"failed to create PNG read structure"};
  if(!(png.info = png_create_info_struct(png.read)))
    throw reascript_error {"failed to create PNG info structure"};

  // png_set_user_limits(png.read, maxWidth, maxHeight);

  png_set_read_fn(png.read, &stream, read);
  png_set_sig_bytes(png.read, HEADER_SIZE);
  png_read_info(png.read, png.info);
  transformToRGBA(png.read, png.info);

  resize(png_get_image_width(png.read,  png.info),
         png_get_image_height(png.read, png.info),
         png_get_rowbytes(png.read,     png.info) /
         png_get_image_width(png.read,  png.info));

  png_read_image(png.read, makeScanlines().data());
}
