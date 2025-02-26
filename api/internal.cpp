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

#include <version.hpp>

API_SECTION("Internal");

static void assertVersion(const VerNum requested)
{
  static constexpr char gitVersion[] {REAIMGUI_VERSION};
  constexpr VerNum current {CompStr::version<&gitVersion>};
  if(std::max(current, API::version()) >= requested)
    return;

  throw reascript_error {
    "ReaImGui {} is too old (action requires API {} or newer)",
    REAIMGUI_VERSION, requested.toString()
  };
}

API_FUNC(0_9, void*, _getapi, (const char*,version) (const char*,symbol_name),
API_DO_NOT_USE)
{
  const VerNum vernum {version};
  assertVersion(vernum);
  if(auto callable {API::Callable::lookup(vernum, symbol_name)})
    return callable->safeImpl();
  throw reascript_error {"function '{}' not found or missing shim for v{}",
    symbol_name, vernum.toString()};
}

// special definition to not have CallConv::Safe clear the last error
struct GetErrMeta {
  static constexpr std::string_view help {API_DO_NOT_USE};
  static constexpr std::array<std::string_view, 0> argn {};
};
_API_EXPORT(ReaScriptFunc, 0_9, _geterr) {
  {}, reinterpret_cast<void *>(&API::lastError),
  {"-API_"       API_PREFIX "_geterr", &API::lastError},
  {"-APIvararg_" API_PREFIX "_geterr",
   CallConv::ReaScript<&API::lastError>::apply},
  {"-APIdef_"    API_PREFIX "_geterr",
   CompStr::apidef<&API::lastError, GetErrMeta>},
};

API_FUNC(0_9, void, _init, (RWB<char*>,buf) (RWBS<int>,buf_sz),
API_DO_NOT_USE)
{
  assertValid(buf);
  const VerNum version {buf};
  assertVersion(version);
  copyToBigBuf(buf, buf_sz, API::Callable::serializeAll(version));
}

API_FUNC(0_9, void, _setshim, (const char*,version) (const char*,symbol_name),
API_DO_NOT_USE)
{
  auto shim {API::Callable::lookup(version, symbol_name)};
  if(shim && typeid(*shim) == typeid(ShimFunc))
    return static_cast<const ShimFunc *>(shim)->activate();

  throw reascript_error {"no suitable implementation available"};
}

API_FUNC(0_9, void, _shim, API_NO_ARGS, API_DO_NOT_USE) {}
