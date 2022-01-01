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

#ifndef REAIMGUI_RESOURCE_PROXY_HPP
#define REAIMGUI_RESOURCE_PROXY_HPP

#include "resource.hpp"

#include <cassert>
#include <vector>

class ResourceProxy {
public:
  using Key = uint32_t;

  template<typename Output, typename Input>
  static Output *encode(const Input *in, const Key key)
  {
    uintptr_t out { reinterpret_cast<uintptr_t>(in) };
    out ^= static_cast<uintptr_t>(key);
    assert("out of double range (EEL incompatible)" && out <= 1ull<<53);
    return reinterpret_cast<Output *>(out);
  }

  ResourceProxy(const std::initializer_list<Key> &keys) : m_keys { keys } {}

  template<typename ResT>
  ResT *decode(void *ptr, Key *key)
  {
    ResT *res;

    for(const Key possibleKey : m_keys) {
      if(Resource::exists(res = encode<ResT>(ptr, possibleKey))) {
        *key = possibleKey;
        return res;
      }
    }

    return nullptr;
  }

private:
  std::vector<Key> m_keys;
};

extern ResourceProxy DrawList;
extern ResourceProxy Viewport;

#endif
