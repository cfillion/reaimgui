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

#ifndef REAIMGUI_VERNUM_HPP
#define REAIMGUI_VERNUM_HPP

#include "error.hpp"

#include <climits>
#include <cstdint>
#include <limits>
#include <string>

class VerNum {
  using ValT = uint32_t;

  static constexpr auto MAX_SEGS {4};
  static constexpr auto SEG_BITS {sizeof(ValT) * CHAR_BIT / MAX_SEGS};
  static constexpr auto SEG_MASK {(1<<SEG_BITS) - 1};

public:
  static constexpr auto MAX {std::numeric_limits<ValT>::max()};

  constexpr VerNum(ValT v = 0) : m_value {v} {}
  constexpr VerNum(const char *input)
    : m_value {}
  {
    if(!*input)
      throw reascript_error {"version number is empty"};

    ValT accumulator {}, seg {MAX_SEGS};
    while(true) {
      const char c {*input++};
      if(c < '0' || c > '9')
        throw reascript_error {"version contains non-numeric segments"};

      accumulator *= 10;
      accumulator += (c - '0');

      const char n {*input};
      if(n && n != '.')
        continue;

      if(accumulator & ~SEG_MASK)
        throw reascript_error {"version contains out of range segments"};

      m_value |= accumulator << (SEG_BITS * --seg);

      if(!n)
        break;

      accumulator = 0;
      ++input;
      if(!seg)
        throw reascript_error {"version contains too many segments"};
    }
  }

  operator ValT() const { return m_value; }

  std::string toString() const
  {
    std::string vernum;
    ValT accumulator {m_value}, seg {MAX_SEGS};
    while(accumulator || seg > MAX_SEGS - 2) {
      if(!vernum.empty())
        vernum += '.';
      vernum += std::to_string(accumulator >> (SEG_BITS * --seg));
      accumulator &= ~(SEG_MASK << (SEG_BITS * seg));
    }
    return vernum;
  }

  constexpr ValT operator[](const unsigned char seg) const
  {
    if(seg >= MAX_SEGS)
      throw reascript_error {"out of range segment"};
    return (m_value >> ((MAX_SEGS - seg - 1) * SEG_BITS)) & SEG_MASK;
  }

private:
  ValT m_value;
};

#endif
