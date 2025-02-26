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

#include "color.hpp"
#include "error.hpp"
#include "texture.hpp"
#include "win32_unicode.hpp"

#include <boost/iostreams/stream.hpp>
#include <cmath> // abs
#include <fstream>
#include <imgui/imgui.h>
#include <reaper_plugin_functions.h>

static const Image::RegisterType *&typeHead()
{
  static const Image::RegisterType *head;
  return head;
}

Image::RegisterType::RegisterType(const TestFunc test, const CreateFunc create)
  : m_test {test}, m_create {create}, m_next {typeHead()}
{
  typeHead() = this;
}

static Image *create(std::istream &stream)
{
  for(const Image::RegisterType *type {typeHead()}; type; type = type->m_next) {
    if(type->m_test(stream))
      return type->m_create(stream);
    else
      stream.seekg(0);
  }

  throw reascript_error {"unsupported format"};
}

Image *Image::fromFile(const char *file)
{
  std::ifstream stream;
  stream.open(WIDEN(file), std::ios_base::binary);
  if(!stream.good())
    throw reascript_error {strerror(errno)};
  return create(stream);
}

Image *Image::fromMemory(const char *data, const int size)
{
  using boost::iostreams::array_source;
  boost::iostreams::stream<array_source> stream {data, size};
  return create(stream);
}

LICEBitmap::LICEBitmap(LICE_IBitmap *bitmap)
{
  resize(LICE__GetWidth(bitmap), LICE__GetHeight(bitmap), 4);

  const auto stride {LICE__GetRowSpan(bitmap)};
  auto pixels {static_cast<const LICE_pixel *>(LICE__GetBits(bitmap))};

  for(unsigned char *row : makeScanlines()) {
    for(size_t i {}; i < width(); ++i) {
      reinterpret_cast<uint32_t *>(row)[i] = (pixels[i] & 0xFF00FF00) |
        (pixels[i] >> 16 & 0x0000FF) | (pixels[i] << 16 & 0xFF0000);
    }
    pixels += stride;
  }
}

const unsigned char *Bitmap::getPixels(
  const Texture &texture, int *width, int *height)
{
  const Bitmap *image {static_cast<Bitmap *>(texture.object())};
  *width = image->m_width, *height = image->m_height;
  return image->m_pixels.data();
}

void Bitmap::resize(const int width, const int height, const int format)
try
{
  constexpr int MAX_SIZE {0x2000}; // Direct3D10 Texture2D limit

  if(format != 4)
    throw reascript_error {"BUG: unexpected pixel format, missing transform?"};
  if(height > MAX_SIZE || width > MAX_SIZE)
    throw reascript_error {"image is too big"};
  m_width = width, m_height = height;
  m_pixels.resize(m_width * m_height * format);
}
catch(const std::bad_alloc &)
{
  throw reascript_error {"cannot allocate memory"};
}

std::vector<unsigned char *> Bitmap::makeScanlines()
{
  std::vector<unsigned char *> scanlines;
  scanlines.reserve(m_height);
  const auto rowStride {m_width * 4};
  for(auto it {m_pixels.begin()}; it < m_pixels.end(); it += rowStride)
    scanlines.push_back(&*it);
  return scanlines;
}

size_t Bitmap::makeTexture(TextureManager *textureManager)
{
  return textureManager->touch(this, 1.f, &getPixels, &Resource::isValid<void>);
}

void ImageSet::add(const float scale, Image *img)
{
  // don't allow infinite recursion
  if(dynamic_cast<ImageSet *>(img))
    throw reascript_error {"image cannot be a set"};

  const auto it {std::lower_bound(m_images.begin(), m_images.end(), scale)};
  if(it != m_images.end() && it->scale == scale)
    throw reascript_error {"scale is already in the set"};

  m_images.emplace(it, scale, img);
}

const ImageSet::Item &ImageSet::select() const
{
  if(m_images.empty())
    throw reascript_error {"image set is empty"};

  const float scale {ImGui::GetWindowDpiScale()};
  const auto it {std::lower_bound(m_images.begin(), m_images.end(), scale)};
  if(it == m_images.begin())
    return *it;
  else if(it == m_images.end())
    return m_images.back();

  const auto prev {std::prev(it)};
  if(std::abs(prev->scale - scale) < std::abs(it->scale - scale))
    return *prev;
  else
    return *it;
}

size_t ImageSet::width() const
{
  const Item &item {select()};
  return item.image->width() / item.scale;
}

size_t ImageSet::height() const
{
  const Item &item {select()};
  return item.image->height() / item.scale;
}

size_t ImageSet::makeTexture(TextureManager *textureManager)
{
  return select().image->makeTexture(textureManager);
}

bool ImageSet::heartbeat()
{
  if(!Resource::heartbeat())
    return false;

  for(const auto &item : m_images)
    item.image->keepAlive();

  return true;
}
