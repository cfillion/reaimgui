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
#include <reaper_plugin_functions.h>

static API *&lastFunc()
{
  static API *head;
  return head;
}

static auto &firstLine()
{
  static unsigned int storedLine;
  return storedLine;
}

static const API::Section *&lastSection()
{
  static const API::Section *section;
  return section;
}

const API *API::head()
{
  return lastFunc();
}

API::FirstLine::FirstLine(unsigned int line)
{
  firstLine() = line;
}

API::Section::Section(const Section *parent, const char *file,
    const char *title, const char *help)
  : parent { parent }, file { file }, title { title }, help { help }
{
  lastSection() = this;
}

API::API(const RegKeys &keys, void *impl, void *vararg,
         const char *defdoc, const unsigned int lastLine)
  : m_section { lastSection() }, m_lines { firstLine(), lastLine },
    m_regs {
      { keys.impl,   impl   },
      { keys.vararg, vararg },
      { keys.defdoc, reinterpret_cast<void *>(const_cast<char *>(defdoc)) },
    }
{
  m_next = lastFunc();
  lastFunc() = this;
}

// API::~API()
// {
//   assert(lastFunc() == this);
//   lastFunc() = const_cast<API *>(m_next);
// }

void API::RegDesc::announce(const bool add) const
{
  // the original key string must remain valid even when unregistering
  // in REAPER < 6.67 (see reapack#56)
  if(value)
    plugin_register(add ? key + 1 : key, value);
}

void API::announceAll(const bool add)
{
  for(const API *func { head() }; func; func = func->m_next) {
    for(const RegDesc &reg : func->m_regs)
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
