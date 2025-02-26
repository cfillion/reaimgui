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

#include "helper.hpp"

#include "../src/function.hpp"

#include <reaper_plugin_secrets.h>

API_SECTION("Function",
R"(Create reusable programs that can passed to and called back from functions
such as InputText* and SetNextWindowSizeConstraints.

They can also be used standalone with Function_Execute
(eg. faster-than-Lua DSP processing).)");

API_FUNC(0_9, Function*, CreateFunctionFromEEL,
(const char*,code),
R"(Compile an EEL program.

Standard EEL [math](https://www.reaper.fm/sdk/js/basiccode.php#js_basicfunc)
and [string](https://www.reaper.fm/sdk/js/strings.php#js_string_funcs)
functions are available in addition to callback-specific functions
(see InputTextCallback_*).)")
{
  return new Function {code};
}

API_FUNC(0_8_5, void, Function_Execute, (Function*,func),
"")
{
  assertValid(func);
  func->execute();
}

API_FUNC(0_8_5, double, Function_GetValue, (Function*,func)
(const char*,name),
"")
{
  assertValid(func);
  if(const auto &value {func->getDouble(name)})
    return *value;
  throw reascript_error {"could not read number value"};
}

API_FUNC(0_8_5, void, Function_SetValue, (Function*,func)
(const char*,name) (double,value),
"")
{
  assertValid(func);
  if(!func->setDouble(name, value))
    throw reascript_error {"could not write number value"};
}

API_FUNC(0_8_5, void, Function_GetValue_Array, (Function*,func)
(const char*,name) (reaper_array*,values),
R"(Copy the values in the function's memory starting at the address stored
in the given variable into the array.)")
{
  assertValid(func);
  assertValid(values);
  if(!func->getArray(name, values))
    throw reascript_error {"could not read array values"};
}

API_FUNC(0_8_5, void, Function_SetValue_Array, (Function*,func)
(const char*,name) (reaper_array*,values),
R"(Copy the values in the array to the function's memory at the address stored
in the given variable.)")
{
  assertValid(func);
  assertValid(values);
  if(!func->setArray(name, values))
    throw reascript_error {"could not write array values"};
}

API_FUNC(0_8_5, void, Function_GetValue_String, (Function*,func)
(const char*,name) (WB<char*>,value) (WBS<int>,value_sz),
"Read from a string slot or a named string (when name starts with a `#`).")
{
  assertValid(func);
  assertValid(value);

  const auto &string {func->getString(name)};
  if(!string)
    throw reascript_error {"could not read string value"};
  copyToBigBuf(value, value_sz, *string);
}

API_FUNC(0_8_5, void, Function_SetValue_String, (Function*,func)
(const char*,name) (const char*,value) (int,value_sz),
"Write to a string slot or a named string (when name starts with a `#`).")
{
  assertValid(func);

  static bool hasAccurateSize {atof(GetAppVersion()) >= 6.44};
  if(!hasAccurateSize)
    value_sz = strlen(value);
  else if(value_sz > 0)
    --value_sz; // don't include the null terminator

  if(!func->setString(name, {value, static_cast<size_t>(value_sz)}))
    throw reascript_error {"could not write string value"};
}
