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

#ifndef REAIMGUI_GDK_EVENT_HANDLER_HPP
#define REAIMGUI_GDK_EVENT_HANDLER_HPP

#include <gdk/gdk.h>

class GdkEventHandler {
public:
  GdkEventHandler();
  ~GdkEventHandler();

  static GdkEvent *currentEvent();

  template<typename T>
  static T *currentEvent(const int expectedType)
  {
    GdkEvent *event { currentEvent() };
    return event && event->type == expectedType
      ? reinterpret_cast<T *>(event) : nullptr;
  }
};

#endif
