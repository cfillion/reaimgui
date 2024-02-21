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
#include <optional>

namespace CompStr {

namespace Utils {
  template<auto str>
  constexpr const char *lfind(const char match,
    const char *start, const char *fallback)
  {
    const char *end { *str + sizeof(*str) - 1 };
    do { if (*start == match) return start + 1; } while(++start < end);
    return fallback;
  }

  template<auto str>
  constexpr const char *rfind(const char match,
    const char *start, const char *fallback)
  {
    const char *p { *str + sizeof(*str) - 1 };
    do { if(*p == match) return p + 1; } while(--p > start);
    return fallback;
  }

  template<typename T>
  constexpr char &append(char *&p, const T &str,
    const std::optional<char> sep = std::nullopt)
  {
    for(size_t i {}; i < str.size(); ++i, ++p)
      *p = str[i];
    if(!sep)
      return *(p - 1);
    *p = *sep;
    return *p++;
  }
}

template<auto fn>
class Basename {
  static constexpr auto compute()
  {
    constexpr const char *start { Utils::rfind<fn>('/', *fn, *fn) },
                         *end   { Utils::rfind<fn>('.', start, *fn + sizeof(*fn)) };
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
                         *end   { Utils::lfind<ver>('-', start, *ver + sizeof(*ver)) };
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

}

#endif
