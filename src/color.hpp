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

#ifndef REAIMGUI_COLORS_HPP
#define REAIMGUI_COLORS_HPP

#include <cstdint>
#include <tuple>

struct ImVec4;

class Color {
public:
  static uint32_t fromBigEndian(const uint32_t rgba);
  static uint32_t toBigEndian(const uint32_t abgr) { return fromBigEndian(abgr); }
  static uint32_t convertNative(const uint32_t rgb);
  static Color fromNative(const uint32_t rgb);

  Color(); // opaque black
  Color(uint32_t rgba, bool alpha = true);
  Color(const ImVec4 &, bool alpha = true);
  Color(const float rgba[4], bool alpha = true);

  operator ImVec4() const;
  void unpack(float rgba[4]) const;
  uint32_t pack(bool alpha, uint32_t extra = 0) const;

  template <class F>
  constexpr decltype(auto) apply(F&& f) const
  {
    return std::apply(f, m_store);
  }

private:
  using Tuple = std::tuple<float, float, float, float>;
  Tuple m_store;
};

#endif
