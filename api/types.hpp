/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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

#ifndef REAIMGUI_TYPES_HPP
#define REAIMGUI_TYPES_HPP

#include "compstr.hpp"

#include <string_view>

template<typename T, typename = void>
struct TypeInfo;

template<typename T>
struct TypeInfo<T, typename std::enable_if_t<std::is_pointer_v<T>> >
{
  using underlying = std::remove_pointer_t<T>;
  static constexpr auto type()
  {
    constexpr auto name { TypeInfo<underlying>::type() };
    std::array<char, name.size() + 1> out {};
    char *p { out.data() };
    CompStr::Utils::append(p, name, '*');
    return out;
  }
};

#define API_REGISTER_TYPE(T, N)   \
  template<> struct TypeInfo<T> { \
    static constexpr std::string_view type() { return N;   } \
  }

#define API_REGISTER_BASIC_TYPE(T)  API_REGISTER_TYPE(T, #T)
#define API_REGISTER_OBJECT_TYPE(T) API_REGISTER_TYPE(T*, "ImGui_" #T "*")

API_REGISTER_BASIC_TYPE(bool);
API_REGISTER_BASIC_TYPE(char*);
API_REGISTER_BASIC_TYPE(const char*);
API_REGISTER_BASIC_TYPE(double);
API_REGISTER_BASIC_TYPE(int);
API_REGISTER_BASIC_TYPE(void);

// https://forum.cockos.com/showthread.php?t=211620
struct reaper_array {
  const unsigned int size, alloc;
  double data[1];
};
API_REGISTER_TYPE(reaper_array*, "reaper_array*");

#endif
