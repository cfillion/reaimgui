/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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
#include "helper.hpp"

API_SECTION("Viewport");

DEFINE_API(ImGui_Viewport*, GetMainViewport, (ImGui_Context*,ctx),
R"(Currently represents REAPER's main window (arrange view).
WARNING: This may change or be removed in the future.)")
{
  return ViewportProxy::encode<ViewportProxy::Main>(ctx);
}

DEFINE_API(ImGui_Viewport*, GetWindowViewport, (ImGui_Context*,ctx),
"Get viewport currently associated to the current window.")
{
  return ViewportProxy::encode<ViewportProxy::Window>(ctx);
}

DEFINE_API(void, Viewport_GetPos, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
"Main Area: Position of the viewport")
{
  const ImVec2 &pos { viewport->get()->Pos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(void, Viewport_GetSize, (ImGui_Viewport*,viewport)
(double*,API_W(w))(double*,API_W(h)),
"Main Area: Size of the viewport.")
{
  const ImVec2 &size { viewport->get()->Size };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
}

DEFINE_API(void, Viewport_GetCenter, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
"Center of the viewport.")
{
  const ImVec2 &pos { viewport->get()->GetCenter() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

API_SUBSECTION("Work Area", "Viewport minus task bars, menu bars, status bars");

DEFINE_API(void, Viewport_GetWorkPos, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
">= Viewport_GetPos")
{
  const ImVec2 &pos { viewport->get()->WorkPos };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}

DEFINE_API(void, Viewport_GetWorkSize, (ImGui_Viewport*,viewport)
(double*,API_W(w))(double*,API_W(h)),
"<= Viewport_GetSize")
{
  const ImVec2 &size { viewport->get()->WorkSize };
  if(API_W(w)) *API_W(w) = size.x;
  if(API_W(h)) *API_W(h) = size.y;
}

DEFINE_API(void, Viewport_GetWorkCenter, (ImGui_Viewport*,viewport)
(double*,API_W(x))(double*,API_W(y)),
"Center of the viewport's work area.")
{
  const ImVec2 &pos { viewport->get()->GetWorkCenter() };
  if(API_W(x)) *API_W(x) = pos.x;
  if(API_W(y)) *API_W(y) = pos.y;
}
