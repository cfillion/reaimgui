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

#include "renderer.hpp"

#include "settings.hpp"
#include "viewport_forwarder.hpp"
#include "window.hpp"

#include <cassert>
#include <imgui/imgui.h>

static auto &typeHead()
{
  static RendererType *head;
  return head;
}

static bool operator<(const RendererType &a, const RendererType &b)
{
  if(a.priority != b.priority)
    return a.priority < b.priority;
  return strcmp(a.id, b.id) < 0;
}

const RendererType *RendererType::head()
{
  return typeHead(); // const public view
}

const RendererType *RendererType::bestMatch(const char *id)
{
  for(const RendererType *type {head()}; type; type = type->next) {
    if(!strcmp(id, type->id))
      return type;
  }
  return head();
}

RendererType::Register::Register(RendererType *type)
{
  if(!(type->flags & RendererType::Available))
    return;
  RendererType **insertionPoint {&typeHead()};
  while(*insertionPoint && **insertionPoint < *type)
    insertionPoint = &(*insertionPoint)->next;
  type->next = *insertionPoint;
  *insertionPoint = type;
}

// Caches the renderer settings for the entire lifetime of the Context
RendererFactory::RendererFactory()
  : m_type {Settings::Renderer}, m_forceSoftware {Settings::ForceSoftware}
{
  assert(m_type);
}

std::unique_ptr<Renderer> RendererFactory::create(Window *window)
{
  return m_type->creator(this, window);
}

void Renderer::install()
{
  using Forwarder = ViewportForwarder<&ImGuiViewport::RendererUserData>;

  // cannot use Renderer_{Create,Destroy}Window because it would
  // create renderers for inactive dockers
  ImGuiPlatformIO &pio {ImGui::GetPlatformIO()};
  // pio.Renderer_CreateWindow  = &createViewport;
  // pio.Renderer_DestroyWindow = &destroyViewport;
  pio.Renderer_SetWindowSize = &Forwarder::wrap<&Renderer::setSize>;
  pio.Renderer_RenderWindow  = &Forwarder::wrap<&Renderer::render>;
  pio.Renderer_SwapBuffers   = &Forwarder::wrap<&Renderer::swapBuffers>;
}

Renderer::Renderer(Window *window)
  : m_window {window}
{
  m_window->viewport()->RendererUserData = this;
}

Renderer::~Renderer()
{
  m_window->viewport()->RendererUserData = nullptr;
}

Renderer::ProjMtx::ProjMtx(const ImVec2 &pos, const ImVec2 &size, const bool flip)
{
  float L {pos.x},
        R {pos.x + size.x},
        T {pos.y},
        B {pos.y + size.y};

  if(flip)
    std::swap(T, B);

  m_data = {{
    {2.f/(R-L),   0.f,         0.f, 0.f},
    {0.f,         2.f/(T-B),   0.f, 0.f},
    {0.f,         0.f,         1.f, 0.f},
    {(R+L)/(L-R), (T+B)/(B-T), 0.f, 1.f},
  }};
}

// The top/left point must be clamped for Metal because it limits the total
// width and height to the viewport's. When top/left is negative the effective
// clipping area does not cover the entire window anymore.
Renderer::ClipRect::ClipRect
    (const ImVec4 &rect, const ImVec2 &offset, const ImVec2 &scale)
  : left   {std::max(0l, static_cast<long>((rect.x - offset.x) * scale.x))},
    top    {std::max(0l, static_cast<long>((rect.y - offset.y) * scale.y))},
    right  {static_cast<long>((rect.z - offset.x) * scale.x)},
    bottom {static_cast<long>((rect.w - offset.y) * scale.y)}
{
}

Renderer::ClipRect::operator bool() const
{
  return right > left && bottom > top;
}
