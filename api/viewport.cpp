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

#include "helper.hpp"

#include "resource_proxy.hpp"

struct ImGui_Viewport {
  enum Key {
    Main = 0x4d4e5650, // MNVP
  };

  ImGuiViewport *get()
  {
    ResourceProxy::Key viewport {};
    Context *ctx { Viewport.decode<Context>(this, &viewport) };

    switch(viewport) {
    case Main:
      ctx->setCurrent();
      return ImGui::GetMainViewport();
    default:
      throw reascript_error { "expected a valid ImGui_Viewport*" };
    }
  }
};

ResourceProxy Viewport { ImGui_Viewport::Main };

DEFINE_API(ImGui_Viewport*, GetMainViewport, (ImGui_Context*,ctx),
R"(Currently represents REAPER's main window (arrange view). This may change in the future.",

- Main Area = entire viewport.
- Work Area = entire viewport minus sections used by main menu bars (for platform windows), or by task bar (for platform monitor).

Windows are generally trying to stay within the Work Area of their host viewport.)",
{
  return ResourceProxy::encode<ImGui_Viewport>(ctx, ImGui_Viewport::Main);
}

DEFINE_API(void, Viewport_GetPos, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
"Main Area: Position of the viewport (Dear ImGui coordinates are the same as OS desktop/native coordinates)",
{
  const ImVec2 &pos { viewport->get()->Pos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(void, Viewport_GetSize, (ImGui_Viewport*,viewport)
(double*,API_W(w))(double*,API_W(h)),
"Main Area: Size of the viewport.",
{
  const ImVec2 &size { viewport->get()->Size };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
}

DEFINE_API(void, Viewport_GetCenter, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
"Main Area: Center of the viewport.",
{
  const ImVec2 &pos { viewport->get()->GetCenter() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(void, Viewport_GetWorkPos, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
"Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)",
{
  const ImVec2 &pos { viewport->get()->WorkPos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(void, Viewport_GetWorkSize, (ImGui_Viewport*,viewport)
(double*,API_W(w))(double*,API_W(h)),
"Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)",
{
  const ImVec2 &size { viewport->get()->WorkSize };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
}

DEFINE_API(void, Viewport_GetWorkCenter, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
"Work Area: Center of the viewport.",
{
  const ImVec2 &pos { viewport->get()->GetWorkCenter() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}
