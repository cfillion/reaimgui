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

#ifndef REAIMGUI_API_VARARG_HPP
#define REAIMGUI_API_VARARG_HPP

#include <tuple>

template<typename T>
struct ReaScriptAPI;

template<typename R, typename... Args>
struct ReaScriptAPI<R(*)(Args...) noexcept>
{
  static const void *applyVarArg(R(*fn)(Args...), void **argv, const int argc)
  {
    if(static_cast<size_t>(argc) < sizeof...(Args))
      return nullptr;

    const auto &args { makeTuple(argv, std::index_sequence_for<Args...>{}) };

    if constexpr (std::is_void_v<R>) {
      std::apply(fn, args);
      return nullptr;
    }
    else if constexpr (std::is_floating_point_v<R>) {
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
const void *InvokeReaScriptAPI(void **argv, int argc)
{
  return ReaScriptAPI<decltype(fn)>::applyVarArg(fn, argv, argc);
}

#endif
