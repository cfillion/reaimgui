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

#ifndef REAIMGUI_CALLCONV_HPP
#define REAIMGUI_CALLCONV_HPP

#include "../src/api.hpp"
#include "../src/error.hpp"

#include <cstdint>
#include <functional>
#include <tuple>

namespace CallConv {

template<typename T>
struct ReaScript;

template<typename R, typename... Args>
struct ReaScript<R(*)(Args...) noexcept>
{
  template<R(*fn)(Args...)>
  static const void *apply(void **argv, const int argc)
  {
    if(static_cast<size_t>(argc) < sizeof...(Args))
      return nullptr;

    const auto &args { makeTuple(argv, std::index_sequence_for<Args...>{}) };

    if constexpr(std::is_void_v<R>) {
      std::apply(fn, args);
      return nullptr;
    }
    else if constexpr(std::is_floating_point_v<R>) {
      const auto value { std::apply(fn, args) };
      void *storage { argv[argc - 1] };
      *static_cast<double *>(storage) = value;
      return storage;
    }
    else {
      // cast numbers to have the same size as a pointer to avoid warnings
      using IntPtrR = std::conditional_t<std::is_pointer_v<R>, R, uintptr_t>;
      const auto value { static_cast<IntPtrR>(std::apply(fn, args)) };
      return reinterpret_cast<const void *>(value);
    }
  }

private:
  template<size_t I>
  using NthType = typename std::tuple_element<I, std::tuple<Args...>>::type;

  template<size_t... I>
  static auto makeTuple(void **argv, std::index_sequence<I...>)
  {
    // C++17 is amazing
    return std::make_tuple(
      std::is_floating_point_v<NthType<I>> ?
        *reinterpret_cast<NthType<I>*>(argv[I]) :
        (NthType<I>)reinterpret_cast<uintptr_t>(argv[I])
      ...
    );
  }
};

template<auto fn>
inline constexpr auto applyReaScript = &ReaScript<decltype(fn)>::template apply<fn>;

template<typename T>
struct Safe;

template<typename R, typename... Args>
struct Safe<R(*)(Args...)>
{
  template<R(*fn)(Args...), auto name>
  static R invoke(Args... args) noexcept
  try {
    // TODO: API::clearError() for C++, clearContext for correct destruction?
    return std::invoke(fn, args...);
  }
  catch(const imgui_error &e) { // TODO: recoverable_error base class
    API::handleError(*name, e);
    return static_cast<R>(0);
  }
  catch(const reascript_error &e) {
    API::handleError(*name, e);
    return static_cast<R>(0);
  }
};

template<auto fn, auto name>
inline constexpr auto invokeSafe = &Safe<decltype(fn)>::template invoke<fn, name>;

}

#endif
