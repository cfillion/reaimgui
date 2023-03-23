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
#include "api_eel.hpp"

#include "context.hpp"

#include <cassert>
#include <reaper_plugin_functions.h>

using namespace API;

static eel_function_table g_eelFuncs;

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

EELFunc::EELFunc(const char *name, const char *definition,
                 VarArgFunc impl, const int argc)
  : m_name { name }, m_definition { definition },
    m_impl { impl }, m_argc { std::max(1, argc) }
{
  // std::max as workaround for EEL needing argc >= 1 because it does
  // nseel_resolve_named_symbol(..., np<1 ? 1 : np, ...)
  //
  // As a side effect it will only error out if giving >1 args and the message
  // will say "needs 1 parms". REAPER's EEL scripts behave like this too:
  // ImGui_WindowFlags_None()     -> OK
  // ImGui_WindowFlags_None(1)    -> OK
  // ImGui_WindowFlags_None(1, 2) -> ERROR "'funcname' needs 1 parms"
}

void EELFunc::announce(const bool init) const
{
  if(!init)
    return;

  constexpr int ExactArgCount { 1 };
  NSEEL_addfunc_varparm_ex(m_name, m_argc, ExactArgCount,
                         NSEEL_PProc_THIS, m_impl, &g_eelFuncs);
}

EELVar::EELVar(const char *name, const char *definition)
  : m_name { name }, m_definition { definition }
{
}

const Symbol *API::head() // immutable public accessor
{
  return lastSymbol();
}

eel_function_table *API::eelFunctionTable()
{
  return &g_eelFuncs;
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
