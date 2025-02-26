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

#ifndef REAIMGUI_CONFIGVAR_HPP
#define REAIMGUI_CONFIGVAR_HPP

#include <reaper_plugin_functions.h>

template<typename T>
class ConfigVar {
public:
  ConfigVar(const char *name)
  {
    int size;
    m_value = static_cast<T *>(get_config_var(name, &size));
    if(size != sizeof(T))
      m_value = nullptr;
  }

  operator bool() const { return m_value != nullptr; }
  T &operator*() { return *m_value; }
  const T &operator*() const { return *m_value; }
  T value_or(T fallback) const { return m_value ? *m_value : fallback; }

private:
  T *m_value;
};

#endif
