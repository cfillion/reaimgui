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

#ifndef REAIMGUI_API_HPP
#define REAIMGUI_API_HPP

#include "error.hpp"

#include <string>

class API {
public:
  static void announceAll(bool add);
  static void handleError(const char *fnName, const reascript_error &);
  static void handleError(const char *fnName, const imgui_error &);

  API(const char *name, void *cImpl, void *reascriptImpl, void *definition);
  ~API();

private:
  struct RegInfo {
    std::string key;
    void *value;
    void announce(bool add) const;
  } m_regs[3];
};

#endif
