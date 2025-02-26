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

#ifndef REAIMGUI_VIEWPORT_HPP
#define REAIMGUI_VIEWPORT_HPP

#include "../src/resource_proxy.hpp"

#include "../src/context.hpp"

struct ViewportProxy : ResourceProxy<ViewportProxy, Context, ImGuiViewport> {
  template<Key KeyValue, auto GetterFunc>
  struct Getter {
    static constexpr Key key {KeyValue};
    static ImGuiViewport *get(Context *ctx)
    {
      assertFrame(ctx);
      return GetterFunc();
    }
  };

  using Main   = Getter<'MNVP', ImGui::GetMainViewport>;
  using Window = Getter<'WNVP', ImGui::GetWindowViewport>;

  using Decoder = MakeDecoder<Main, Window>;
};

API_REGISTER_TYPE(ViewportProxy*, "ImGui_Viewport*");

#endif
