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

#include "action.hpp"

#include <memory>
#include <vector>

#include <reaper_plugin_functions.h>

using namespace std::string_literals;

static std::vector<std::unique_ptr<Action>> g_actions;

static bool operator<(const std::unique_ptr<Action> &a, const int id)
{
  return a->id() < id;
}

static Action *findAction(const int id)
{
  if(id < g_actions.front()->id() || id > g_actions.back()->id())
    return nullptr;

  const auto it {std::lower_bound(g_actions.begin(), g_actions.end(), id)};
  if(it == g_actions.end() || (*it)->id() != id)
    return nullptr;

  return it->get();
}

static bool commandHook(const int id, const int flag)
{
  (void)flag;

  if(Action *action {findAction(id)}) {
    action->run();
    return true;
  }

  return false;
}

static int toggleHook(const int id)
{
  if(const Action *action {findAction(id)})
    return action->state();

  return -1;
}

void Action::setup()
{
  plugin_register("hookcommand", reinterpret_cast<void *>(&commandHook));
  plugin_register("toggleaction", reinterpret_cast<void *>(&toggleHook));
}

void Action::teardown()
{
  g_actions.clear();
  plugin_register("-hookcommand", reinterpret_cast<void *>(&commandHook));
  plugin_register("-toggleaction", reinterpret_cast<void *>(&toggleHook));
}

void Action::refreshAll()
{
  for(const auto &action : g_actions) {
    if(action->m_getState)
      action->refresh();
  }
}

Action::Action(const std::string &name, const std::string &desc,
               const RunFunc &run, const StateFunc &getState)
  : m_name {"REAIMGUI_"s + name}, m_desc {"ReaImGui: "s + desc},
    m_cmd {}, m_run {run}, m_getState {getState}
{
  m_cmd.accel.cmd =
    plugin_register("command_id", const_cast<char *>(m_name.c_str()));
  m_cmd.desc = m_desc.c_str();
  plugin_register("gaccel", &m_cmd);

  auto it {std::lower_bound(g_actions.begin(), g_actions.end(), m_cmd.accel.cmd)};
  g_actions.emplace(it, this);
}

Action::~Action()
{
  plugin_register("-gaccel", &m_cmd);
  plugin_register("-command_id", const_cast<char *>(m_name.c_str()));
}

void Action::refresh()
{
  RefreshToolbar(m_cmd.accel.cmd);
}
