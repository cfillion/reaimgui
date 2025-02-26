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

#ifndef REAIMGUI_COMPSTR_HPP
#define REAIMGUI_COMPSTR_HPP

#include "compstr_utils.hpp"
#include "types.hpp"

#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>

namespace CompStr {

namespace {
template<auto fn>
class Basename {
  static constexpr auto compute()
  {
    constexpr const char *start {rfind<fn>('/', *fn, *fn)},
                         *end   {rfind<fn>('.', start, *fn + sizeof(*fn))};
    std::array<char, end - start> name {};
    for(size_t i {}; i < name.size() - 1; ++i)
      name[i] = start[i];
    return name;
  }

public:
  static constexpr auto value {compute()};
};
}

template<auto input>
static constexpr const char *basename {Basename<input>::value.data()};

namespace {
template<auto ver>
class Version {
  static constexpr auto compute()
  {
    constexpr const char *start {**ver == 'v' ? *ver + 1 : *ver},
                         *dirty {lfind<ver>('+', start, *ver + sizeof(*ver))},
                         *end   {lfind<ver>('-', start, dirty)};
    std::array<char, end - start> version {};
    for(size_t i {}; i < version.size() - 1; ++i) {
      const char c {start[i]};
      version[i] = c == '_' ? '.' : c;
    }
    return version;
  }

public:
  static constexpr auto value {compute()};
};
}

template<auto input>
static constexpr const char *version {Version<input>::value.data()};

namespace {
template<typename fn, typename Meta, bool UseArgNames = true>
class APIDef;

template<typename R, typename... Args, typename Meta, bool UseArgNames>
class APIDef<R(*)(Args...), Meta, UseArgNames>
{
  using Is = std::make_index_sequence<sizeof...(Args)>;

  static constexpr auto compute()
  {
    constexpr auto length {
      TypeInfo<R>::type().size() + 1 +
      std::apply([](auto... arg) {
        if constexpr(sizeof...(arg) == 0)
          return 2;
        else
          return (std::get<0>(arg).size() + ...) + sizeof...(arg) +
                 (std::get<1>(arg).size() + ...) + sizeof...(arg);
      }, args<Args...>(Is{})) +
      Meta::help.size() + 1
    };
    std::array<char, length> def {};
    char *p {def.data()};
    append(p, TypeInfo<R>::type(), '\0');
    std::apply([&p](auto... arg) {
      if constexpr(sizeof...(arg) == 0)
        p += 2;
      else {
        ((append(p, std::get<0>(arg), ',')), ...) = '\0';
        ((append(p, std::get<1>(arg), ',')), ...) = '\0';
      }
    }, args<Args...>(Is{}));
    append(p, Meta::help, '\0');

    return def;
  }

  static constexpr std::array<std::string_view, 1> noArgNames {"_"};

  template<typename... ArgsPart, std::size_t... Is>
  static constexpr auto args(std::index_sequence<Is...>)
  {
    [[maybe_unused]] // for GCC 7
    constexpr const auto &argn {[]() -> const auto & {
      if constexpr(UseArgNames)
        return Meta::argn;
      else
        return noArgNames;
    }()};

    return std::make_tuple(std::make_tuple(
      TypeInfo<ArgsPart>::type(),
      TypeInfo<ArgsPart>::template name<argn, UseArgNames ? Is : 0>()
    )...);
  }

public:
  static constexpr auto value {compute()};
};
}

template<typename R, typename... Args, typename Meta, bool UseArgNames>
class APIDef<R(*)(Args...) noexcept, Meta, UseArgNames>
  : public APIDef<R(*)(Args...), Meta, UseArgNames> {};

template<auto func, typename Meta, bool UseArgNames = true>
static constexpr const char *apidef
  {APIDef<decltype(func), Meta, UseArgNames>::value.data()};

} // namespace CompStr

#endif
