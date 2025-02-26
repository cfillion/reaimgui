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

#include "helper.hpp"

#include "viewport.hpp"

API_SECTION("Viewport");

API_FUNC(0_1, ViewportProxy*, GetMainViewport, (Context*,ctx),
R"(Currently represents REAPER's main window (arrange view).
WARNING: This may change or be removed in the future.)")
{
  return ViewportProxy::encode<ViewportProxy::Main>(ctx);
}

API_FUNC(0_7, ViewportProxy*, GetWindowViewport, (Context*,ctx),
"Get viewport currently associated to the current window.")
{
  return ViewportProxy::encode<ViewportProxy::Window>(ctx);
}

API_FUNC(0_1, void, Viewport_GetPos, (ViewportProxy*,viewport)
(W<double*>,x) (W<double*>,y),
"Main Area: Position of the viewport")
{
  const ImVec2 &pos {viewport->get()->Pos};
  if(x) *x = pos.x;
  if(y) *y = pos.y;
}

API_FUNC(0_1, void, Viewport_GetSize, (ViewportProxy*,viewport)
(W<double*>,w) (W<double*>,h),
"Main Area: Size of the viewport.")
{
  const ImVec2 &size {viewport->get()->Size};
  if(w) *w = size.x;
  if(h) *h = size.y;
}

API_FUNC(0_1, void, Viewport_GetCenter, (ViewportProxy*,viewport)
(W<double*>,x) (W<double*>,y),
"Center of the viewport.")
{
  const ImVec2 &pos {viewport->get()->GetCenter()};
  if(x) *x = pos.x;
  if(y) *y = pos.y;
}

API_SUBSECTION("Work Area", "Viewport minus task bars, menu bars, status bars");

API_FUNC(0_1, void, Viewport_GetWorkPos, (ViewportProxy*,viewport)
(W<double*>,x) (W<double*>,y),
">= Viewport_GetPos")
{
  const ImVec2 &pos {viewport->get()->WorkPos};
  if(x) *x = pos.x;
  if(y) *y = pos.y;
}

API_FUNC(0_1, void, Viewport_GetWorkSize, (ViewportProxy*,viewport)
(W<double*>,w) (W<double*>,h),
"<= Viewport_GetSize")
{
  const ImVec2 &size {viewport->get()->WorkSize};
  if(w) *w = size.x;
  if(h) *h = size.y;
}

API_FUNC(0_1, void, Viewport_GetWorkCenter, (ViewportProxy*,viewport)
(W<double*>,x) (W<double*>,y),
"Center of the viewport's work area.")
{
  const ImVec2 &pos {viewport->get()->GetWorkCenter()};
  if(x) *x = pos.x;
  if(y) *y = pos.y;
}
