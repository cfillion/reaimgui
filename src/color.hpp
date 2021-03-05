/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
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
  static uint32_t rgba2abgr(const uint32_t rgba);
  static uint32_t abgr2rgba(const uint32_t abgr) { return rgba2abgr(abgr); }
  static Color fromTheme(const uint32_t themeColor);

  Color(); // opaque black
  Color(uint32_t rgba, bool alpha = true);
  Color(const ImVec4 &, bool alpha = true);
  Color(const float rgba[4], bool alpha = true);

  operator ImVec4() const;
  void unpack(float rgba[4]) const;
  uint32_t pack(bool alpha = true, uint32_t prewextra = 0) const;

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
