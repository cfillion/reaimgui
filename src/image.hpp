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

#ifndef REAIMGUI_IMAGE_HPP
#define REAIMGUI_IMAGE_HPP

#include "resource.hpp"

#include <vector>
#include <istream>

class Context;
class LICE_IBitmap;
struct ImTextureRef;
struct reaper_array;

class Image : public Resource {
public:
  struct RegisterType {
    using TestFunc   = bool   (*)(std::istream &);
    using CreateFunc = Image *(*)(std::istream &);

    RegisterType(TestFunc, CreateFunc);

    const TestFunc   m_test;
    const CreateFunc m_create;
    const RegisterType * const m_next;
  };

  static Image *fromFile(const char *);
  static Image *fromMemory(const char *, int size);

  virtual size_t width()  const = 0;
  virtual size_t height() const = 0;
  virtual ImTextureRef texture(Context *) = 0;

  bool attachable(const Context *) const override { return true; }
};

API_REGISTER_OBJECT_TYPE(Image);

class Bitmap : public Image {
public:
  Bitmap(int width, int height, int format);

  size_t width()  const override { return m_width;  }
  size_t height() const override { return m_height; }
  ImTextureRef texture(Context *) override;

  SubresourceData install(Context *) override;
  void update(Context *, void *) override;

  template<bool Write>
  void copyPixels(int x, int y,
    unsigned int w, unsigned int h,
    std::conditional_t<Write, const reaper_array *, reaper_array *>,
    unsigned int offset = 0, unsigned int pitch = 0);

protected:
  Bitmap();

  void resize(int width, int height, int format);
  std::vector<unsigned char *> makeScanlines();

private:
  std::vector<unsigned char> m_pixels;
  unsigned int m_version;
  unsigned short m_width, m_height;
};

API_REGISTER_OBJECT_TYPE(Bitmap);

class LICEBitmap : public Bitmap {
public:
  LICEBitmap(LICE_IBitmap *bitmap);
};

class ImageSet final : public Image {
public:
  void add(float scale, Image *);

  size_t width() const override;
  size_t height() const override;
  ImTextureRef texture(Context *) override;

protected:
  bool heartbeat() override;

private:
  struct Item {
    Item(float scale, Image *image) : image {image}, scale {scale} {}
    Image *image;
    float scale;
    bool operator<(float targetScale) const { return scale < targetScale; }
  };

  const Item &select() const;

  std::vector<Item> m_images;
};

API_REGISTER_OBJECT_TYPE(ImageSet);

#endif
