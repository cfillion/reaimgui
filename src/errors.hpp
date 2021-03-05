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

#ifndef REAIMGUI_ERRORS_HPP
#define REAIMGUI_ERRORS_HPP

#include <stdexcept>

#define DEFINE_EXCEPT(type)                \
  class type : public std::runtime_error { \
  public:                                  \
    using runtime_error::runtime_error;    \
  }

DEFINE_EXCEPT(reascript_error);
DEFINE_EXCEPT(imgui_error);

#undef DEFINE_EXCEPT

#endif
