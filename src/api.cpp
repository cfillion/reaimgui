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

#include "api.hpp"

#include "context.hpp"

#include <cassert>
#include <deque>
#include <reaper_plugin_functions.h>

using namespace std::string_literals;

#define KEY(prefix) (prefix "_ImGui_"s + name)

static auto &knownFuncs()
{
  static std::deque<const API *> funcs;
  return funcs;
}

static auto &firstLine()
{
  static unsigned int storedLine;
  return storedLine;
}

const API *API::enumAPI()
{
  const auto &container { knownFuncs() };
  static auto it { container.begin() };
  return it == container.end() ? nullptr : *it++;
}

API::FirstLine::FirstLine(unsigned int line)
{
  firstLine() = line;
}

API::API(const char *name, void *cImpl, void *reascriptImpl,
        const char *definition, const char *file, unsigned int lastLine)
  : m_regs {
      { KEY("API"),       cImpl         },
      { KEY("APIvararg"), reascriptImpl },
      { KEY("APIdef"),    reinterpret_cast<void *>(const_cast<char *>(definition)) },
    }, m_file { file }, m_lines { firstLine(), lastLine }
{
  knownFuncs().push_back(this);
}

API::~API()
{
  assert(knownFuncs().back() == this);
  knownFuncs().pop_back();
}

void API::RegInfo::announce(const bool add) const
{
  // the original key string must remain valid even when unregistering
  // in REAPER < 6.67 (see reapack#56)
  if(value)
    plugin_register(add ? key.c_str() : ("-" + key).c_str(), value);
}

void API::announceAll(const bool add)
{
  for(const API *func : knownFuncs()) {
    for(const RegInfo &reg : func->m_regs)
      reg.announce(add);
  }
}

// REAPER 6.29+ uses the '!' prefix to abort the calling Lua script's execution
void API::handleError(const char *fnName, const reascript_error &e)
{
  char message[1024];
  snprintf(message, sizeof(message), "!ImGui_%s: %s", fnName, e.what());
  ReaScriptError(message);
}

void API::handleError(const char *fnName, const imgui_error &e)
{
  char message[1024];
  snprintf(message, sizeof(message),
    "!ImGui_%s: ImGui assertion failed: %s", fnName, e.what());
  ReaScriptError(message);

  delete Context::current();
}
