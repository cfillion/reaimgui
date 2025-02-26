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

#ifndef REAIMGUI_FLAT_SET_HPP
#define REAIMGUI_FLAT_SET_HPP

#include <algorithm>
#include <vector>

// meant to store sorted pointers (or other numbers) in contiguous memory
// caller is responsible for not inserting the same value more than once
template<typename T>
class FlatSet {
public:
  auto begin() { return m_data.begin(); }
  auto end()   { return m_data.end();   }
  auto begin() const { return m_data.begin(); }
  auto end()   const { return m_data.end();   }
  T front() const { return m_data.front(); }
  T back()  const { return m_data.back();  }
  size_t size() const { return m_data.size();  }
  bool empty()  const { return m_data.empty(); }
  void insert(T v) { m_data.insert(lowerBound(v), v); }
  void erase(T v)  { m_data.erase(lowerBound(v));     }
  auto lowerBound(T v)       { return std::lower_bound(begin(), end(), v); }
  auto lowerBound(T v) const { return std::lower_bound(begin(), end(), v); }
  bool contains(T v) const
  {
    const auto it {lowerBound(v)};
    return it != m_data.end() && *it == v;
  }

private:
  std::vector<T> m_data;
};

#endif
