/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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

#ifndef REAIMGUI_BASENAME_HPP
#define REAIMGUI_BASENAME_HPP

#include <array>

template<auto Fn>
class MakeBasename {
  static constexpr size_t N { sizeof(*Fn) };

  static constexpr const char *after(const char match,
    const char *start, const char *fallback)
  {
    size_t i { N - 1 };
    const auto end { static_cast<size_t>(start - *Fn) };
    do { if((*Fn)[i] == match) return *Fn + i + 1; } while(i-- > end);
    return fallback;
  }

  static constexpr auto compute()
  {
    constexpr const char *start { after('/', *Fn, *Fn) },
                         *end   { after('.', start, *Fn + N) };
    std::array<char, end - start> name {};
    for(size_t i {}; i < name.size() - 1; ++i)
      name[i] = start[i];
    return name;
  }

public:
  static constexpr auto value { compute() };
};

template<auto Fn>
static constexpr const char *Basename { MakeBasename<Fn>::value.data() };

#endif
