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

#include "helper.hpp"

#include "../src/function.hpp"

#include <reaper_plugin_secrets.h>

API_SECTION("Function",
R"(Create reusable programs that can passed to and called back from functions
such as InputText* and SetNextWindowSizeConstraints.

They can also be used standalone with Function_Execute
(eg. faster-than-Lua DSP processing).)");

DEFINE_API(ImGui_Function*, CreateFunctionFromEEL, (const char*,code),
R"(Compile an EEL program.

Standard EEL [math](https://www.reaper.fm/sdk/js/basiccode.php#js_basicfunc)
and [string](https://www.reaper.fm/sdk/js/strings.php#js_string_funcs)
functions are available in addition to callback-specific functions
(see InputTextCallback_*).)")
{
  return new Function { code };
}

DEFINE_API(void, Function_Execute, (ImGui_Function*,func),
"")
{
  assertValid(func);
  func->keepAlive();
  func->execute();
}

DEFINE_API(bool, Function_GetValue, (ImGui_Function*,func)
(const char*,name)(double*,API_W(value)),
"")
{
  assertValid(func);
  assertValid(API_W(value));
  func->keepAlive();
  if(const auto &value { func->getDouble(name) }) {
    *API_W(value) = *value;
    return true;
  }
  return false;
}

DEFINE_API(bool, Function_SetValue, (ImGui_Function*,func)
(const char*,name)(double,value),
"")
{
  assertValid(func);
  func->keepAlive();
  return func->setDouble(name, value);
}

DEFINE_API(bool, Function_GetValue_Array, (ImGui_Function*,func)
(const char*,name)(reaper_array*,values),
R"(Copy the values in the function's memory starting at the address stored
in the given variable into the array.)")
{
  assertValid(func);
  assertValid(values);
  func->keepAlive();
  return func->getArray(name, values);
}

DEFINE_API(bool, Function_SetValue_Array, (ImGui_Function*,func)
(const char*,name)(reaper_array*,values),
R"(Copy the values in the array to the function's memory at the address stored
in the given variable.)")
{
  assertValid(func);
  assertValid(values);
  func->keepAlive();
  return func->setArray(name, values);
}

DEFINE_API(bool, Function_GetValue_String, (ImGui_Function*,func)
(const char*,name)(char*,API_WBIG(value))(int,API_WBIG_SZ(value)),
"Read a named string.")
{
  assertValid(func);
  assertValid(API_WBIG(value));
  func->keepAlive();

  const auto &value { func->getString(name) };
  if(!value)
    return false;
  if(!realloc_cmd_ptr(&API_WBIG(value), &API_WBIG_SZ(value), value->size()))
    return false;
  memcpy(API_WBIG(value), value->data(), API_WBIG_SZ(value));
  return true;
}

DEFINE_API(bool, Function_SetValue_String, (ImGui_Function*,func)
(const char*,name)(const char*,value)(int,value_sz),
"Write to a named string.")
{
  assertValid(func);
  func->keepAlive();

  static bool hasAccurateSize { atof(GetAppVersion()) >= 6.44 };
  if(!hasAccurateSize)
    value_sz = strlen(value);
  else if(value_sz > 0)
    --value_sz; // don't include the null terminator

  return func->setString(name, { value, static_cast<size_t>(value_sz) });
}
