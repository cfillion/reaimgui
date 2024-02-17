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

#ifndef REAIMGUI_COMPSTR_HPP
#define REAIMGUI_COMPSTR_HPP

#include <array>
#include <cstddef>

namespace CompStr {

template<auto fn>
class Basename {
  static constexpr size_t N { sizeof(*fn) };

  static constexpr const char *after(const char match,
    const char *start, const char *fallback)
  {
    size_t i { N - 1 };
    const auto end { static_cast<size_t>(start - *fn) };
    do { if((*fn)[i] == match) return *fn + i + 1; } while(i-- > end);
    return fallback;
  }

  static constexpr auto compute()
  {
    constexpr const char *start { after('/', *fn, *fn) },
                         *end   { after('.', start, *fn + N) };
    std::array<char, end - start> name {};
    for(size_t i {}; i < name.size() - 1; ++i)
      name[i] = start[i];
    return name;
  }

public:
  static constexpr auto value { compute() };
};

template<auto filename>
static constexpr const char *basename { Basename<filename>::value.data() };

}

#endif
