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

#include "viewport.hpp"

#include "context.hpp"
#include "docker.hpp"
#include "window.hpp"

#include <imgui/imgui.h>

static void createViewport(ImGuiViewport *viewport)
{
  Viewport *instance;

  if(Docker *docker { Context::current()->dockers()->findByViewport(viewport) })
    instance = new DockerHost { docker, viewport };
  else
    instance = new Window { viewport };

  // PlatformUserData must not be set if create throws a backend_error
  // Otherwise dear imgui will complain during the destruction of the context
  viewport->PlatformHandle   = instance->create();
  viewport->PlatformUserData = instance;
}

static void destroyViewport(ImGuiViewport *viewport)
{
  if(Viewport *instance { static_cast<Viewport *>(viewport->PlatformUserData) })
    delete instance;

  // dear imgui will assert if any of these remain set
  viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

template<auto fn, typename... Args>
static auto instanceProxy(ImGuiViewport *viewport, Args... args)
{
  using R = std::result_of_t<decltype(fn)(Viewport *, Args...)>;

  if(Viewport *instance { static_cast<Viewport *>(viewport->PlatformUserData) })
    return (instance->*fn)(args...);

  if constexpr(!std::is_void_v<R>)
    return R{};
}

Viewport::Viewport(ImGuiViewport *viewport)
  : m_ctx { Context::current() }, m_viewport { viewport }
{
}

Viewport::~Viewport()
{
}

void Viewport::install()
{
  ImGuiPlatformIO &pio { ImGui::GetPlatformIO() };
  pio.Platform_CreateWindow       = &createViewport;
  pio.Platform_DestroyWindow      = &destroyViewport;
  pio.Platform_ShowWindow         = &instanceProxy<&Viewport::show>;
  pio.Platform_SetWindowPos       = &instanceProxy<&Viewport::setPosition>;
  pio.Platform_GetWindowPos       = &instanceProxy<&Viewport::getPosition>;
  pio.Platform_SetWindowSize      = &instanceProxy<&Viewport::setSize>;
  pio.Platform_GetWindowSize      = &instanceProxy<&Viewport::getSize>;
  pio.Platform_SetWindowFocus     = &instanceProxy<&Viewport::setFocus>;
  pio.Platform_GetWindowFocus     = &instanceProxy<&Viewport::hasFocus>;
  pio.Platform_GetWindowMinimized = &instanceProxy<&Viewport::isVisible>;
  pio.Platform_SetWindowTitle     = &instanceProxy<&Viewport::setTitle>;
  // TODO: SetWindowAlpha
  pio.Platform_UpdateWindow       = &instanceProxy<&Viewport::update>;
  pio.Platform_RenderWindow       = &instanceProxy<&Viewport::render>;
  pio.Platform_GetWindowDpiScale  = &instanceProxy<&Viewport::scaleFactor>;
  pio.Platform_OnChangedViewport  = &instanceProxy<&Viewport::onChanged>;
  pio.Platform_SetImeInputPos     = &instanceProxy<&Viewport::setImePosition>;
}

void Viewport::show()
{
  // FIXME: Undo this weird thing ImGui does before calling ShowWindow
  if(ImGui::GetFrameCount() < 3)
    m_viewport->Flags &= ~ImGuiViewportFlags_NoFocusOnAppearing;
}
