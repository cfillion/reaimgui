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

#ifndef REAIMGUI_ERROR_HPP
#define REAIMGUI_ERROR_HPP

#include "../api/types.hpp"

#include <stdexcept>
#include <format>

class runtime_error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;

  template<typename... Args>
  runtime_error(std::format_string<Args...> fmt, Args&&... args)
    : std::runtime_error {std::vformat(fmt.get(), std::make_format_args(args...))}
  {
    static_assert(sizeof...(Args) > 0);
  }
};

#define DEFINE_EXCEPT(type)             \
  class type : public runtime_error {   \
  public:                               \
    using runtime_error::runtime_error; \
  };

DEFINE_EXCEPT(backend_error);
DEFINE_EXCEPT(imgui_error);
DEFINE_EXCEPT(reascript_error);

#undef DEFINE_EXCEPT

class Context;

namespace Error {
  [[noreturn]] void imguiAssertionFailure(const char *message);
  [[noreturn]] void imguiDebugBreak();

  template<typename T>
  [[noreturn]] void invalidObject(const T *ptr)
  {
    constexpr auto typeInfo {TypeInfo<T*>::type()};
    const std::string_view typeName {typeInfo.data(), typeInfo.size()};
    throw reascript_error 
      {"expected a valid {}, got {}", typeName, static_cast<const void *>(ptr)};
  }

  void report(Context *, const imgui_error &);
  void report(Context *, const backend_error &);
};

#endif
