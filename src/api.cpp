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

#include "api.hpp"

#include "context.hpp"

#include <cassert>
#include <reaper_plugin_functions.h>

using namespace API;

static const Symbol *&lastSymbol()
{
  static const Symbol *head;
  return head;
}

static auto &lastLine()
{
  static LineNumber storedLine;
  return storedLine;
}

static const Section *&lastSection()
{
  static const Section *section;
  return section;
}

API::StoreLineNumber::StoreLineNumber(LineNumber line)
{
  lastLine() = line;
}

Section::Section(const Section *parent, const char *file,
    const char *title, const char *help)
  : parent { parent }, file { file }, title { title }, help { help }
{
  lastSection() = this;
}

Symbol::Symbol()
  : m_section { lastSection() }, m_next { lastSymbol() }, m_line { lastLine() }
{
  lastSymbol() = this;
}

// Symbol::~Symbol()
// {
//   assert(lastSymbol() == this);
//   lastSymbol() = const_cast<Symbol *>(m_next);
// }

void PluginRegister::announce(const bool init) const
{
  // the original key string must remain valid even when unregistering
  // in REAPER < 6.67 (see reapack#56)
  plugin_register(key + init, value);
}

ReaScriptFunc::ReaScriptFunc(const PluginRegister &native,
                             const PluginRegister &reascript,
                             const PluginRegister &desc)
  : m_regs { native, reascript, desc }
{
}

void ReaScriptFunc::announce(const bool init) const
{
  for(const PluginRegister &reg : m_regs)
    reg.announce(init);
}

const Symbol *API::head() // immutable public accessor
{
  return lastSymbol();
}

void API::announceAll(const bool add)
{
  for(const Symbol *sym { lastSymbol() }; sym; sym = sym->m_next)
    sym->announce(add);
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
