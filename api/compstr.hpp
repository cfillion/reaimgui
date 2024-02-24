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

#include "compstr_utils.hpp"
#include "types.hpp"

#include <array>
#include <cstddef>
#include <string_view>

namespace CompStr {

template<auto fn>
class Basename {
  static constexpr auto compute()
  {
    constexpr const char *start { rfind<fn>('/', *fn, *fn) },
                         *end   { rfind<fn>('.', start, *fn + sizeof(*fn)) };
    std::array<char, end - start> name {};
    for(size_t i {}; i < name.size() - 1; ++i)
      name[i] = start[i];
    return name;
  }

public:
  static constexpr auto value { compute() };
};

template<auto input>
static constexpr const char *basename { Basename<input>::value.data() };

template<auto ver>
class Version {
  static constexpr auto compute()
  {
    constexpr const char *start { **ver == 'v' ? *ver + 1 : *ver },
                         *end   { lfind<ver>('-', start, *ver + sizeof(*ver)) };
    std::array<char, end - start> version {};
    for(size_t i {}; i < version.size() - 1; ++i) {
      const char c { start[i] };
      version[i] = c == '_' ? '.' : c;
    }
    return version;
  }

public:
  static constexpr auto value { compute() };
};

template<auto input>
static constexpr const char *version { Version<input>::value.data() };

template<auto fn>
class APIDef;

template<typename R, typename... Args, R (*fn)(Args...)>
class APIDef<fn>
{
  static constexpr auto compute()
  {
    constexpr std::string_view help { "Internal use only." };
    constexpr auto length {
      TypeInfo<R>::type().size() + 1 +
      [] {
        if constexpr(sizeof...(Args) == 0)
          return 2;
        else
         return (TypeInfo<Args>::type().size() + ...) + sizeof...(Args) +
                (TypeInfo<Args>::name().size() + ...) + sizeof...(Args);
      }() + help.size() + 1
    };
    std::array<char, length> def {};
    char *p { def.data() };
    append(p, TypeInfo<R>::type(), '\0');
    if constexpr(sizeof...(Args) == 0)
      p += 2;
    else {
      ((append(p, TypeInfo<Args>::type(), ',')), ...) = '\0';
      ((append(p, TypeInfo<Args>::name(), ',')), ...) = '\0';
    }
    append(p, help, '\0');

    return def;
  }

public:
  static constexpr auto value { compute() };
};

template<auto func>
static constexpr const char *apidef { APIDef<func>::value.data() };

}

#endif
