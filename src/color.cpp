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

#include "color.hpp"

#include <cmath>
#include <imgui/imgui.h>

uint32_t Color::fromBigEndian(const uint32_t rgba)
{
  // assumes a little-endian host platform
  // https://twitter.com/cfillion30/status/1519411808213348353
  return
    (rgba >> 24 & 0x000000FF) | // red
    (rgba >> 8  & 0x0000FF00) | // green
    (rgba << 8  & 0x00FF0000) | // blue
    (rgba << 24 & 0xFF000000) ; // alpha
}

uint32_t Color::convertNative(uint32_t rgb)
{
#ifdef _WIN32
  // swap red/blue channels, preserving unused data in the MSB
  rgb = (rgb >> 16 & 0x000000ff) |
        (rgb       & 0xff00ff00) |
        (rgb << 16 & 0x00ff0000) ;
#endif
  return rgb;
}

Color Color::fromNative(const uint32_t rgb)
{
  Color color {rgb, false};
#ifdef _WIN32
  // swap red/blue channels
  std::swap(std::get<0>(color.m_store), std::get<2>(color.m_store));
#endif
  return color;
}

Color::Color()
  : m_store {0.0f, 0.0f, 0.0f, 1.0f}
{
}

Color::Color(const uint32_t rgba, const bool alpha)
{
  uint32_t i {};
  auto &[r, g, b, a] {m_store};

  if(alpha)
    a = (rgba >> (8 * i++) & 0xFF) / 255.f;
  else
    a = 1.0f;
  b   = (rgba >> (8 * i++) & 0xFF) / 255.f;
  g   = (rgba >> (8 * i++) & 0xFF) / 255.f;
  r   = (rgba >> (8 * i++) & 0xFF) / 255.f;
}

Color::Color(const ImVec4 &vec, const bool alpha)
  : m_store {vec.x, vec.y, vec.z, alpha ? vec.w : 1.0f}
{
}

Color::Color(const float rgba[4], const bool alpha)
  : m_store {rgba[0], rgba[1], rgba[2], alpha ? rgba[3] : 1.0f}
{
}

Color::operator ImVec4() const
{
  const auto [r, g, b, a] {m_store};
  return {r, g, b, a};
}

uint32_t Color::pack(const bool alpha, const uint32_t extra) const
{
  uint32_t rgba {}, i {};
  const auto [r, g, b, a] {m_store};

  if(alpha)
    rgba |= static_cast<uint32_t>(std::round(a * 0xFF)) << (8 * i++);
  else if(extra) // preserve unused bits as-is (eg. REAPER's color enable flag)
    rgba |= extra & 0xFF000000;
  rgba   |= static_cast<uint32_t>(std::round(b * 0xFF)) << (8 * i++);
  rgba   |= static_cast<uint32_t>(std::round(g * 0xFF)) << (8 * i++);
  rgba   |= static_cast<uint32_t>(std::round(r * 0xFF)) << (8 * i++);

  return rgba;
}

void Color::unpack(float rgba[4]) const
{
  std::tie(rgba[0], rgba[1], rgba[2], rgba[3]) = m_store;
}
