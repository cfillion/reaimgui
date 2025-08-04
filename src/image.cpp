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
#include "context.hpp"
#include "error.hpp"
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

struct Bitmap::Update {
  Update(unsigned short x, unsigned short y, unsigned short w, unsigned short h,
      unsigned int v)
    : rect {x, y, w, h}, version {v}, age{}
  {}

  ImTextureRect rect;
  unsigned int version;
  unsigned char age;
};

Bitmap::Bitmap() : m_version {}
{}

Bitmap::~Bitmap() = default;

Bitmap::Bitmap(const int width, const int height, const int format)
  : Bitmap {}
{
  resize(width, height, format);
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

bool Bitmap::heartbeat()
{
  if(!Resource::heartbeat())
    return false;

  auto it {m_updates.begin()};
  while(it != m_updates.end()) {
    // must leave 1 grace frame because Context's heartbeat may be called after ours
    constexpr unsigned char UPDATE_TTL {Context::SUBRESOURCE_TTL + 1};
    if(++it->age >= UPDATE_TTL)
      it = m_updates.erase(it);
    else
      ++it;
  }

  return true;
}

template void Bitmap::copyPixels<false>(int, int, unsigned int, unsigned int,
  reaper_array *, unsigned int, unsigned int);
template void Bitmap::copyPixels<true>(int, int, unsigned int, unsigned int,
  const reaper_array *, unsigned int, unsigned int);

template<bool Write>
void Bitmap::copyPixels(int x, int y, unsigned int w, unsigned int h,
  std::conditional_t<Write, const reaper_array*, reaper_array*> pixels,
  unsigned int offset, unsigned int pitch)
{
  if(x >= m_width || y >= m_height)
    return;

  if(!pitch)
    pitch = w;
  else if(pitch < w)
    throw reascript_error {"pitch is smaller than width"};

  if(offset >= pixels->size || pitch * h > pixels->size - offset)
    throw reascript_error
      {"pixel array size must be at least {}", offset + (pitch * h)};

  if(x < 0)
    w += x, offset -= x, x = 0;
  if(y < 0)
    h += y, offset -= y * pitch, y = 0;

  w = std::min<unsigned int>(w, m_width  - x);
  h = std::min<unsigned int>(h, m_height - y);
  if(w < 1 || h < 1)
    return;

  const auto outPitch {m_width * 4};
  auto *out {m_pixels.data() + (x + (y * m_width)) * 4};
  auto *in {pixels->data + offset};

  for(unsigned short iy {}; iy < h; ++iy) {
    for(unsigned short ix {}; ix < w; ++ix) {
      auto pixel {reinterpret_cast<uint32_t *>(&out[ix * 4])};
      if constexpr(Write) // double to int overflow is undefined behavior!
        *pixel = Color::fromBigEndian(static_cast<int64_t>(in[ix]));
      else
        in[ix] = Color::toBigEndian(*pixel);
    }
    out += outPitch, in += pitch;
  }

  if constexpr(Write)
    m_updates.emplace_back(x, y, w, h, ++m_version);
}

struct ImageTextureData {
  ImTextureData *tex;
  unsigned int version;
};

ImTextureRef Bitmap::texture(Context *ctx)
{
  return ctx->touch<ImageTextureData>(this)->tex->GetTexRef();
}

static void uninstall(Context *, ImageTextureData *data)
{
  data->tex->Status = ImTextureStatus_WantDestroy;
  data->tex->Pixels = nullptr; // ensure it can't accidentally be used after free
  delete data;
}

SubresourceData Bitmap::install(Context *ctx)
{
  ImTextureData *tex {ctx->createTexture()};
  tex->UniqueID = uniqId();
  tex->Status = ImTextureStatus_WantCreate;
  tex->Format = ImTextureFormat_RGBA32;
  tex->Width = m_width;
  tex->Height = m_height;
  tex->BytesPerPixel = 4;
  tex->Pixels = m_pixels.data();
  tex->RefCount = 1;

  return {new ImageTextureData {tex, m_version}, &uninstall};
}

void Bitmap::update(Context *, void *user)
{
  auto data {static_cast<ImageTextureData *>(user)};
  const auto prev {std::find_if(m_updates.rbegin(), m_updates.rend(),
    [data](const Update &u) { return u.version == data->version; })};
  IM_ASSERT(data->tex->Updates.Size == 0);
  for(auto it = prev.base(); it != m_updates.end(); ++it)
    data->tex->Updates.push_back(it->rect);
  data->version = m_version;
  if(data->tex->Status == ImTextureStatus_OK && data->tex->Updates.Size > 0)
    data->tex->SetStatus(ImTextureStatus_WantUpdates);
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

ImTextureRef ImageSet::texture(Context *ctx)
{
  return select().image->texture(ctx);
}

bool ImageSet::heartbeat()
{
  if(!Resource::heartbeat())
    return false;

  for(const auto &item : m_images)
    item.image->keepAlive();

  return true;
}
