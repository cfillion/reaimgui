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

#include "platform.hpp"

void Platform::translatePosition(ImVec2 *pos, const bool toHiDpi)
{
  const float fromOriginX { pos->x - m_viewport->Pos.x },
              fromOriginY { pos->y - m_viewport->Pos.y };

  float scale { m_viewport->DpiScale };
  if(!toHiDpi)
    scale = 1.f / scale;

  pos->x = m_viewport->Pos.x + (fromOriginX * scale);
  pos->y = m_viewport->Pos.y + (fromOriginY * scale);
}
