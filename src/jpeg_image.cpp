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

#include <array>
#include <cstring> // memcmp

// https://github.com/libjpeg-turbo/libjpeg-turbo/raw/main/example.txt
// https://github.com/libjpeg-turbo/libjpeg-turbo/raw/main/jibjpeg.txt
#include <jpeglib.h>
#include <jerror.h>

class JPEGImage final : public Bitmap {
public:
  JPEGImage(std::istream &);
};

static bool isJPEG(std::istream &stream)
{
  constexpr const unsigned char jpeg[] {0xFF, 0xD8, 0xFF};
  char magic[sizeof(jpeg)];
  stream.read(magic, sizeof(jpeg));
  return memcmp(jpeg, magic, sizeof(jpeg)) == 0;
}

static Image *create(std::istream &stream)
{
  return new JPEGImage {stream};
}

static const Image::RegisterType JPEG {&isJPEG, &create};

static void error(j_common_ptr jpeg)
{
  char message[JMSG_LENGTH_MAX];
  jpeg->err->format_message(jpeg, message);
  throw reascript_error {message};
}

struct StreamSource : public jpeg_source_mgr {
  StreamSource(std::istream &stream)
    : jpeg_source_mgr {
        nullptr, 0, init, fill, skip, jpeg_resync_to_restart, term,
      }, stream {stream}
  {
  }

  std::istream &stream;
  std::array<char, 4096> buffer;

  static void init(j_decompress_ptr);
  static void term(j_decompress_ptr) {}
  static boolean fill(j_decompress_ptr);
  static void skip(j_decompress_ptr, long bytes);
};

void StreamSource::init(j_decompress_ptr jpeg)
{
  StreamSource *src {static_cast<StreamSource *>(jpeg->src)};
  src->stream.seekg(0); // un-read the magic number bytes
}

boolean StreamSource::fill(j_decompress_ptr jpeg)
{
  StreamSource *src {static_cast<StreamSource *>(jpeg->src)};
  if(!src->stream)
    ERREXIT(jpeg, JERR_INPUT_EOF);
  src->stream.read(src->buffer.data(), src->buffer.size());
  src->next_input_byte = reinterpret_cast<unsigned char *>(src->buffer.data());
  src->bytes_in_buffer = src->stream.gcount();
  return TRUE;
}

void StreamSource::skip(j_decompress_ptr jpeg, long bytes)
{
  if(bytes <= 0)
    return; // no-op as per libjpeg's recommendation

  unsigned int skips {};
  StreamSource *src {static_cast<StreamSource *>(jpeg->src)};
  while(bytes > static_cast<long>(src->bytes_in_buffer)) {
    bytes -= src->bytes_in_buffer;
    src->bytes_in_buffer = src->buffer.size();
    ++skips;
  }
  if(skips > 0) {
    src->stream.seekg((skips - 1) * src->buffer.size(), std::ios::cur);
    fill(jpeg);
  }
  src->next_input_byte += bytes;
  src->bytes_in_buffer -= bytes;
}

JPEGImage::JPEGImage(std::istream &stream)
{
  struct JPEG {
    ~JPEG() { jpeg_destroy_decompress(&info); }
    operator jpeg_decompress_struct *() { return &info; }
    jpeg_decompress_struct *operator->() { return &info; }
    jpeg_decompress_struct info;
  } jpeg;

  jpeg_error_mgr err;
  jpeg->err = jpeg_std_error(&err);
  err.error_exit = error;
  err.output_message = error; // warnings as errors

  jpeg_create_decompress(jpeg);
  StreamSource src {stream};
  jpeg->src = &src;
  jpeg_read_header(jpeg, TRUE);
  jpeg->out_color_space = JCS_EXT_RGBA; // TODO: save memory with RGB textures
  jpeg_start_decompress(jpeg);

  resize(jpeg->output_width, jpeg->output_height, jpeg->output_components);
  std::vector<unsigned char *> scanlines {makeScanlines()};

  // jpeg_read_scanlines does not decompress the entire image at once
  while(jpeg->output_scanline < jpeg->output_height) {
    jpeg_read_scanlines(jpeg, &scanlines[jpeg->output_scanline],
                        jpeg->output_height - jpeg->output_scanline);
  }

  jpeg_finish_decompress(jpeg);
}
