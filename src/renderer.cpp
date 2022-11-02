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

#include "settings.hpp"
#include "window.hpp"

#include <cassert>
#include <imgui/imgui.h>

static auto &knownRendererTypes()
{
  static std::vector<const RendererType *> types;
  return types;
}

const std::vector<const RendererType *> &RendererType::knownTypes()
{
  return knownRendererTypes(); // const public view
}

static bool operator<(const RendererType *a, const RendererType &b)
{
  return a->priority < b.priority || strcmp(a->id, b.id) < 0;
}

const RendererType *RendererType::bestMatch(const char *id)
{
  const RendererType *result {};
  for(auto it { knownTypes().rbegin() }; it < knownTypes().rend(); ++it) {
    result = *it;
    if(!strcmp(id, (*it)->id))
      break;
  }
  return result;
}

RendererType::Register::Register(const RendererType *type)
{
  auto &types { knownRendererTypes() };
  const auto it { std::lower_bound(types.begin(), types.end(), *type) };
  types.insert(it, type);
}

RendererFactory::RendererFactory()
  : m_type { Settings::Renderer }
{
  assert(m_type);
}

std::unique_ptr<Renderer> RendererFactory::create(Window *window)
{
  return m_type->creator(this, window);
}

template<auto fn, typename... Args>
static auto instanceProxy(ImGuiViewport *viewport, Args... args)
{
#ifdef HAS_CPP_20
  using R = std::invoke_result_t<decltype(fn), Renderer *, Args...>;
#else
  using R = std::result_of_t<decltype(fn)(Renderer *, Args...)>;
#endif

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

Renderer::ProjMtx::ProjMtx(const ImVec2 &pos, const ImVec2 &size, const bool flip)
{
  float L { pos.x },
        R { pos.x + size.x },
        T { pos.y },
        B { pos.y + size.y };

  if(flip)
    std::swap(T, B);

  m_data = {{
    { 2.f/(R-L),   0.f,         0.f, 0.f },
    { 0.f,         2.f/(T-B),   0.f, 0.f },
    { 0.f,         0.f,        -1.f, 0.f },
    { (R+L)/(L-R), (T+B)/(B-T), 0.f, 1.f },
  }};
}

Renderer::ClipRect::ClipRect
    (const ImVec4 &rect, const ImVec2 &offset, const ImVec2 &scale)
  : left   { static_cast<long>((rect.x - offset.x) * scale.x) },
    top    { static_cast<long>((rect.y - offset.y) * scale.y) },
    right  { static_cast<long>((rect.z - offset.x) * scale.x) },
    bottom { static_cast<long>((rect.w - offset.y) * scale.y) }
{
}

Renderer::ClipRect::operator bool() const
{
  return right > left && bottom > top;
}
