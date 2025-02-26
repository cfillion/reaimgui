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

#ifndef REAIMGUI_ACTION_HPP
#define REAIMGUI_ACTION_HPP

#include <functional>
#include <string>

#include <reaper_plugin.h>

class Action {
public:
  static void setup();
  static void teardown();
  static void refreshAll();

  using RunFunc   = std::function<void()>;
  using StateFunc = std::function<bool()>;

  Action(const std::string &name, const std::string &desc,
         const RunFunc &run, const StateFunc &getState = nullptr);
  Action(const Action &) = delete;
  ~Action();

  auto id() const { return m_cmd.accel.cmd; }
  void run() const { m_run(); }
  int state() const { return m_getState ? m_getState() : -1; }

  void refresh();

private:
  std::string m_name, m_desc;
  gaccel_register_t m_cmd;
  RunFunc m_run;
  StateFunc m_getState;
};

#endif
