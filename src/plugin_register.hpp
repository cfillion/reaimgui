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

#ifndef REAIMGUI_PLUGIN_REGISTER_HPP
#define REAIMGUI_PLUGIN_REGISTER_HPP

class PluginRegisterBase {
public:
  template<typename T>
  PluginRegisterBase(const char *key, T value)
    : m_key {key}, m_value {(void *)value}
  {
    // assert(m_key[0] == '-');
  }

  void announce(const bool init) const
  {
    // don't include reaper_plugin_functions.h here to avoid pulling in
    // SWELL/Win32 and their macro pollution
    extern int (*plugin_register)(const char *, void *);

    // the original key string must remain valid even when unregistering
    // in REAPER < 6.67 (see reapack#56)
    plugin_register(m_key + init, m_value);
  }

  const char *key() const { return m_key; }
  template<typename T = void *>
  T value() const { return reinterpret_cast<T>(m_value); }

private:
  const char *m_key;
  void *m_value;
};

class PluginRegister : private PluginRegisterBase {
public:
  template<typename T>
  PluginRegister(const char *key, T value)
    : PluginRegisterBase {key, value}
  {
    announce(true);
  }

  ~PluginRegister()
  {
    announce(false);
  }

  using PluginRegisterBase::key;
  using PluginRegisterBase::value;
};

#endif
