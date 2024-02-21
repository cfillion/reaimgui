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

#include "helper.hpp"

#include <version.hpp>

API_SECTION("Internal");

#define DO_NOT_USE "Internal use only."

static void assertVersion(const VerNum requested)
{
  static constexpr const char gitVersion[] { REAIMGUI_VERSION };
  constexpr VerNum current { CompStr::version<&gitVersion> };
  if(std::max(current, API::version()) >= requested)
    return;

  throw reascript_error {
    "ReaImGui {} is too old (action requires {} or newer)",
    REAIMGUI_VERSION, requested.toString()
  };
}

API_FUNC(0_9, void*, _getapi, (const char*,version)(const char*,symbol_name),
DO_NOT_USE)
{
  if(auto callable { API::Callable::lookup(version, symbol_name) })
    return callable->safeImpl();
  return nullptr;
}

API_FUNC(0_9, void, _init, (char*,API_RWBIG(buf))(int,API_RWBIG_SZ(buf)), DO_NOT_USE)
{
  assertValid(API_RWBIG(buf));
  const VerNum version { API_RWBIG(buf) };
  assertVersion(version);
  copyToBigBuf(API_RWBIG(buf), API_RWBIG_SZ(buf), API::Callable::serializeAll(version));
}

API_FUNC(0_9, void, _setshim, (const char*,version)(const char*,symbol_name),
DO_NOT_USE)
{
  auto shim { API::Callable::lookup(version, symbol_name) };
  if(shim && typeid(*shim) == typeid(ShimFunc))
    return static_cast<const ShimFunc *>(shim)->activate();

  throw reascript_error { "no suitable implementation available" };
}

API_FUNC(0_9, void, _shim, NO_ARGS, DO_NOT_USE) {}
