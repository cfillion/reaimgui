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

#ifndef REAIMGUI_IMAGE_HPP
#define REAIMGUI_IMAGE_HPP

#include "resource.hpp"

#include <vector>
#include <istream>

class Context;

class Image : public Resource {
public:
  static constexpr const char *api_type_name { "ImGui_Image" };

  struct RegisterType {
    using TestFunc   = bool   (*)(std::istream &);
    using CreateFunc = Image *(*)(std::istream &);

    RegisterType(TestFunc, CreateFunc);

    const TestFunc   m_test;
    const CreateFunc m_create;
    const RegisterType * const m_next;
  };

  static Image *fromFile(const char *);

  auto width()  const { return m_width;  }
  auto height() const { return m_height; }

  size_t makeTexture(Context *, float scale);

protected:
  Image() = default;
  std::vector<unsigned char> m_pixels;
  size_t m_width, m_height;

private:
  static const unsigned char *getPixels(void *object, float scale,
    int *width, int *height);
};

using ImGui_Image = Image;

class PNGImage : public Image {
public:
  PNGImage(std::istream &);
};

#endif
