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

#ifndef REAIMGUI_FUNCTION_HPP
#define REAIMGUI_FUNCTION_HPP

#include "resource.hpp"

#include <memory>
#include <optional>
#include <string_view>

using NSEEL_VMCTX      = void *;
using NSEEL_CODEHANDLE = void *;

class eel_string_context_state;
struct reaper_array;

class Function : public Resource {
public:
  static void setup();

  Function(const char *eel2Code);
  ~Function();

  void execute() noexcept;
  std::optional<double> getDouble(const char *name) const;
  std::optional<std::string_view> getString(const char *name) const;
  bool getArray(const char *name,  reaper_array *values) const;

  bool setDouble(const char *name, double value);
  bool setString(const char *name, const std::string_view &);
  bool setArray(const char *name,  const reaper_array *values);

  bool attachable(const Context *) const override { return true; }

  // string access by index
  std::optional<std::string_view> getString(double) const;

private:
  friend eel_string_context_state *EEL_STRING_GET_CONTEXT_POINTER(void *);
  std::unique_ptr<eel_string_context_state> m_strings;

  using NSEEL_VMCTX_REF      = std::remove_pointer_t<NSEEL_VMCTX>;
  using NSEEL_CODEHANDLE_REF = std::remove_pointer_t<NSEEL_CODEHANDLE>;
  struct VMDeleter { void operator()(NSEEL_VMCTX); };
  struct ProgramDeleter { void operator()(NSEEL_CODEHANDLE); };
  std::unique_ptr<NSEEL_VMCTX_REF, VMDeleter> m_vm;
  std::unique_ptr<NSEEL_CODEHANDLE_REF, ProgramDeleter> m_program;
};

API_REGISTER_OBJECT_TYPE(Function);

#endif
