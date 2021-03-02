/* ReaImGui: ReaScript binding for dear imgui
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

#include "resource.hpp"

#include "errors.hpp"

#include <reaper_plugin_functions.h>
#include <unordered_set>

constexpr size_t MAX_INSTANCES { 99 };

static std::unordered_set<Resource *> g_rsx;

class Resource::Timer {
public:
  Timer()
  {
    plugin_register("timer", reinterpret_cast<void *>(&Timer::tick));
  }

  ~Timer()
  {
    plugin_register("-timer", reinterpret_cast<void *>(&Timer::tick));
  }

private:
  static void tick()
  {
    auto it { g_rsx.begin() };

    while(it != g_rsx.end()) {
      Resource *rs { *it++ };
      rs->heartbeat(); // may delete rs
    }
  }
};

Resource::Resource()
{
  if(g_rsx.size() >= MAX_INSTANCES)
    throw reascript_error { "exceeded maximum object allocation limit" };

  static std::weak_ptr<Timer> g_timer;

  if(g_timer.expired())
    g_timer = m_timer = std::make_shared<Timer>();
  else
    m_timer = g_timer.lock();

  g_rsx.emplace(this);
}

Resource::~Resource()
{
  g_rsx.erase(this);
}

template<>
bool Resource::exists<Resource>(Resource *rs)
{
  return g_rsx.count(rs) > 0;
}
