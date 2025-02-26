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

#include "viewport.hpp"

#include "context.hpp"
#include "docker.hpp"
#include "error.hpp"
#include "platform.hpp"
#include "viewport_forwarder.hpp"
#include "window.hpp"

#include <imgui/imgui.h>
#include <reaper_plugin_functions.h>

class MainViewport final : public Viewport {
public:
  MainViewport();

  void create() override {}
  void destroy() override {}
  HWND nativeHandle() const override { return m_hwnd; }
  void show() override {}
  void setPosition(ImVec2) override {}
  void setSize(ImVec2) override {}
  void setFocus() override {}
  bool hasFocus() const override { return false; }
  bool isMinimized() const override;
  void setTitle(const char *) override {}
  void setAlpha(float) override {}
  void update() override {};
  float scaleFactor() const override;
  void onChanged() override {}
  void setIME(ImGuiPlatformImeData *) override {}

private:
  HWND m_hwnd;
};

static void createViewport(ImGuiViewport *viewport)
{
  Viewport *instance;

  if(Docker *docker {Context::current()->dockers().findByViewport(viewport)})
    instance = new DockerHost {docker, viewport};
  else
    instance = Platform::createWindow(viewport);

  try {
    instance->create();
  }
  catch(const backend_error &) {
    delete instance;
    throw;
  }

  // PlatformUserData must not be set if create throws a backend_error
  // Otherwise dear imgui will complain during the destruction of the context
  viewport->PlatformUserData = instance;
  viewport->PlatformHandle   = instance->nativeHandle();
}

static void destroyViewport(ImGuiViewport *viewport)
{
  if(Viewport *instance {static_cast<Viewport *>(viewport->PlatformUserData)}) {
    instance->destroy();
    delete instance;
  }

  // dear imgui will assert if any of these remain set
  viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

Viewport::Viewport(ImGuiViewport *viewport)
  : m_ctx {Context::current()}, m_viewport {viewport}
{
}

Viewport::~Viewport()
{
}

void Viewport::install()
{
  using Forwarder = ViewportForwarder<&ImGuiViewport::PlatformUserData>;

  ImGuiPlatformIO &pio {ImGui::GetPlatformIO()};
  pio.Platform_CreateWindow       = &createViewport;
  pio.Platform_DestroyWindow      = &destroyViewport;
  pio.Platform_ShowWindow         = &Forwarder::wrap<&Viewport::show>;
  pio.Platform_SetWindowPos       = &Forwarder::wrap<&Viewport::setPosition>;
  pio.Platform_GetWindowPos       = &Forwarder::wrap<&Viewport::getPosition>;
  pio.Platform_SetWindowSize      = &Forwarder::wrap<&Viewport::setSize>;
  pio.Platform_GetWindowSize      = &Forwarder::wrap<&Viewport::getSize>;
  pio.Platform_SetWindowFocus     = &Forwarder::wrap<&Viewport::setFocus>;
  pio.Platform_GetWindowFocus     = &Forwarder::wrap<&Viewport::hasFocus>;
  pio.Platform_GetWindowMinimized = &Forwarder::wrap<&Viewport::isMinimized>;
  pio.Platform_SetWindowTitle     = &Forwarder::wrap<&Viewport::setTitle>;
  pio.Platform_SetWindowAlpha     = &Forwarder::wrap<&Viewport::setAlpha>;
  pio.Platform_UpdateWindow       = &Forwarder::wrap<&Viewport::update>;
  pio.Platform_GetWindowDpiScale  = &Forwarder::wrap<&Viewport::scaleFactor>;
  pio.Platform_OnChangedViewport  = &Forwarder::wrap<&Viewport::onChanged>;

  ImGuiIO &io {ImGui::GetIO()};
  io.SetPlatformImeDataFn = &Forwarder::wrap<&Viewport::setIME>;

  new MainViewport; // lifetime managed by Dear ImGui
}

ImVec2 Viewport::getPosition() const
{
  POINT point {};
  ClientToScreen(nativeHandle(), &point);

  ImVec2 pos;
  pos.x = point.x;
  pos.y = point.y;
  Platform::scalePosition(&pos, false, m_viewport);

  return pos;
}

ImVec2 Viewport::getSize() const
{
  RECT rect;
  GetClientRect(nativeHandle(), &rect);

  ImVec2 size;
  size.x = rect.right - rect.left;
  size.y = rect.bottom - rect.top;

#ifndef __APPLE__
  size.x /= m_viewport->DpiScale;
  size.y /= m_viewport->DpiScale;
#endif

  return size;
}

MainViewport::MainViewport()
  : Viewport {ImGui::GetMainViewport()}, m_hwnd {GetMainHwnd()}
{
  m_viewport->PlatformUserData = this;
  m_viewport->PlatformHandle = m_hwnd;
  m_viewport->DpiScale = scaleFactor();
}

bool MainViewport::isMinimized() const
{
  // Disable hosting windows in the non-rendered main viewport.
  // ConfigViewportsNoAutoMerge doesn't apply to popups/tooltips/menus.
  // CanHostOtherWindows applies to everything but it's reset at every NewFrame.
  // ViewportFlags_IsMinimized bypasses UpdateTryMergeWindowIntoHostViewport.
  return true;
}

float MainViewport::scaleFactor() const
{
  return Platform::scaleForWindow(m_hwnd);
}
