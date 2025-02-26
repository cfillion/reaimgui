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

#ifndef REAIMGUI_VIEWPORT_FORWARDER_HPP
#define REAIMGUI_VIEWPORT_FORWARDER_HPP

#include <imgui/imgui.h>

using InstancePtr = void *ImGuiViewport::*;

template<InstancePtr instancePtr>
class ViewportForwarder {
private:
  template<typename T>
  struct FuncInfo;

  template<typename C, typename R, typename... Args>
  struct FuncInfo<R(C::*)(Args...)>
  {
    using InstanceType = C;
    using ReturnType = R;
  };

  template<typename C, typename R, typename... Args>
  struct FuncInfo<R(C::*)(Args...) const>
  {
    using InstanceType = C;
    using ReturnType = R;
  };

public:
  template<auto fn, typename... Args>
  static auto wrap(ImGuiViewport *viewport, Args... args)
  {
    using T = typename FuncInfo<decltype(fn)>::InstanceType;
    using R = typename FuncInfo<decltype(fn)>::ReturnType;

    if(T *instance {static_cast<T *>(viewport->*instancePtr)})
      return (instance->*fn)(args...);

    if constexpr(!std::is_void_v<R>)
      return R{};
  }
};

#endif
