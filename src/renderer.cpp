/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#include "renderer.hpp"

#include "window.hpp"

#include <imgui/imgui.h>

template<auto fn, typename... Args>
static auto instanceProxy(ImGuiViewport *viewport, Args... args)
{
  using R = std::result_of_t<decltype(fn)(Renderer *, Args...)>;

  if(Renderer *instance { static_cast<Renderer *>(viewport->RendererUserData) })
    return (instance->*fn)(args...);

  if constexpr(!std::is_void_v<R>)
    return R{};
}

void Renderer::install()
{
  // cannot use Renderer_{Create,Destroy}Window because it would
  // create renderers for inactive dockers
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  // pio.Renderer_CreateWindow  = &createViewport;
  // pio.Renderer_DestroyWindow = &destroyViewport;
  pio.Renderer_SetWindowSize = &instanceProxy<&Renderer::setSize>;
  pio.Renderer_RenderWindow  = &instanceProxy<&Renderer::render>;
  pio.Renderer_SwapBuffers   = &instanceProxy<&Renderer::swapBuffers>;
}

Renderer::Renderer(Window *window)
  : m_window { window }
{
  m_window->viewport()->RendererUserData = this;
}

Renderer::~Renderer()
{
  m_window->viewport()->RendererUserData = nullptr;
}
