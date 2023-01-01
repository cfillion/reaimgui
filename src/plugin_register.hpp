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

#include <reaper_plugin_functions.h>

class PluginRegister {
public:
  PluginRegister(const char *key, void *value)
    : m_key { key }, m_value { value }
  {
    // assert(m_key[0] == '-');
    plugin_register(m_key + 1, m_value);
  }

  ~PluginRegister()
  {
    // the original m_key passed when registering must remain valid in REAPER < 6.67
    plugin_register(m_key, m_value);
  }

private:
  const char *m_key;
  void *m_value;
};
