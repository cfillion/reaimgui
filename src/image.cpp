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

#include "image.hpp"

#include "context.hpp"
#include "error.hpp"
#include "texture.hpp"
#include "win32_unicode.hpp"

#include <boost/iostreams/stream.hpp>
#include <fstream>

static const Image::RegisterType *&typeHead()
{
  static const Image::RegisterType *head;
  return head;
}

Image::RegisterType::RegisterType(const TestFunc test, const CreateFunc create)
  : m_test { test }, m_create { create }, m_next { typeHead() }
{
  typeHead() = this;
}

static Image *create(std::istream &stream)
{
  for(const Image::RegisterType *type { typeHead() }; type; type = type->m_next) {
    if(type->m_test(stream))
      return type->m_create(stream);
    else
      stream.seekg(0);
  }

  throw reascript_error { "unsupported format" };
}

Image *Image::fromFile(const char *file)
{
  std::ifstream stream;
  stream.open(WIDEN(file), std::ios_base::binary);
  if(!stream.good())
    throw reascript_error { strerror(errno) };
  return create(stream);
}

Image *Image::fromMemory(const char *data, const int size)
{
  using boost::iostreams::array_source;
  boost::iostreams::stream<array_source> stream { data, size };
  return create(stream);
}

const unsigned char *Image::getPixels(void *object, const float scale,
  int *width, int *height)
{
  printf("getPixels obj=%p scale=%f\n", object, scale);
  const Image *image { static_cast<Image *>(object) };
  *width = image->m_width, *height = image->m_height;
  return image->m_pixels.data();
}

size_t Image::makeTexture(Context *ctx, const float)
{
  keepAlive();
  Texture tex { this, 1.f, &getPixels };
  tex.m_isValid = &Resource::isValid;
  return ctx->textureManager()->touch(tex);
}
