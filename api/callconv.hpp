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

#ifndef REAIMGUI_CALLCONV_HPP
#define REAIMGUI_CALLCONV_HPP

#include "../src/api.hpp"
#include "../src/error.hpp"

#include <cstdint>
#include <functional>
#include <tuple>

namespace CallConv {

template<auto fn>
struct ReaScript;

template<typename R, typename... Args, R (*fn)(Args...) noexcept>
struct ReaScript<fn>
{
  static const void *apply(void **argv, const int argc)
  {
    if(static_cast<size_t>(argc) < sizeof...(Args))
      return nullptr;

    const auto &args {makeTuple(argv, std::index_sequence_for<Args...>{})};

    if constexpr(std::is_void_v<R>) {
      std::apply(fn, args);
      return nullptr;
    }
    else if constexpr(std::is_floating_point_v<R>) {
      const auto value {std::apply(fn, args)};
      void *storage {argv[argc - 1]};
      *static_cast<double *>(storage) = value;
      return storage;
    }
    else {
      // cast numbers to have the same size as a pointer to avoid warnings
      using IntPtrR = std::conditional_t<std::is_pointer_v<R>, R, uintptr_t>;
      const auto value {static_cast<IntPtrR>(std::apply(fn, args))};
      return reinterpret_cast<const void *>(value);
    }
  }

private:
  template<size_t I>
  using NthType = typename std::tuple_element_t<I, std::tuple<Args...>>;

  template<size_t... I>
  static auto makeTuple(void **argv, std::index_sequence<I...>)
  {
    // C++17 is amazing
    return std::make_tuple(
      std::is_floating_point_v<NthType<I>> ?
        *reinterpret_cast<NthType<I>*>(argv[I]) :
        *const_cast<NthType<I>*>(reinterpret_cast<const NthType<I>*>(&argv[I]))
      ...
    );
  }
};

template<auto fn, typename Meta>
struct Safe;

template<typename R, typename... Args, R (*fn)(Args...), typename Meta>
struct Safe<fn, Meta>
{
  static R invoke(Args... args) noexcept
  try {
    // only clear errors when first entering into an API function
    // not when reentering (eg. EEL Function callback) so that we can still
    // check for previous failure using API::lastError
    API::ErrorClearer reentrant {};
    return std::invoke(fn, args...);
  }
  catch(const imgui_error &e) {
    API::handleError(Meta::name, e);
    return static_cast<R>(0);
  }
  catch(const reascript_error &e) {
    API::handleError(Meta::name, e);
    return static_cast<R>(0);
  }
};

}

#endif
