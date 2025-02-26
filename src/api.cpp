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

#include "api.hpp"
#include "api_eel.hpp"

#include "context.hpp"

#include <cassert>
#include <reaper_plugin_functions.h>
#include <unordered_map>

using namespace API;

static eel_function_table g_eelFuncs;
static std::string g_lastError;

using CallableMap = std::unordered_map<std::string_view, Callable *>;

static CallableMap &callables()
{
  static CallableMap map;
  return map;
}

static VerNum &latestVersion()
{
  static VerNum version;
  return version;
}

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

static ImportTable *&lastImportTable()
{
  static ImportTable *table;
  return table;
}

Callable::Callable(const VerNum since, const VerNum until, const char *name)
  : m_since {since}, m_until {until}
{
  if(since > latestVersion())
    latestVersion() = since;
  if(until != VerNum::MAX && until > latestVersion())
    latestVersion() = until;

  auto [it, isNew] {callables().try_emplace(name, this)};
  if(isNew)
    m_precursor = nullptr;
  else if(since >= it->second->m_since) {
    m_precursor = it->second;
    if(since < m_precursor->m_until)
      throw reascript_error {"overlapping callable version range"};
    it->second = this;
  }
  else {
    Callable *precursor = it->second;
    while(precursor->m_precursor && since < precursor->m_precursor->m_since)
      precursor = precursor->m_precursor;
    m_precursor = precursor->m_precursor;
    if(until > precursor->m_since)
      throw reascript_error {"overlapping callable version range"};
    precursor->m_precursor = this;
  }
}

const Callable *Callable::lookup(const VerNum version, const char *name)
{
  const auto &map {callables()};
  const auto it {map.find(name)};
  return it == map.end() ? nullptr : it->second->rollback(version);
}

const Callable *Callable::rollback(const VerNum version) const
{
  const Callable *match {this};
  while(match && match->m_since > version)
    match = match->m_precursor;
  if(match && match->m_until <= version)
    return nullptr;
  return match;
}

std::string Callable::serializeAll(const VerNum version)
{
  enum Flags { IsConst = 1<<0, IsShim = 1<<1 };
  std::string out;
  for(const auto &pair : callables()) {
    if(pair.first[0] == '_')
      continue;
    const Callable *match {pair.second->rollback(version)};
    if(!match)
      continue;
    char flags {};
    if(match->isConstant())
      flags |= IsConst;
    if(typeid(*match) == typeid(ShimFunc))
      flags |= IsShim;
    out += flags;
    out += pair.first;
    out += '\0';
  }
  return out;
}

StoreLineNumber::StoreLineNumber(LineNumber line)
{
  lastLine() = line;
}

Section::Section(const Section *parent, const char *file,
    const char *title, const char *help)
  : parent {parent}, file {file}, title {title}, help {help}
{
  lastSection() = this;
}

Symbol::Symbol(const int flags)
  : m_section {lastSection()}, m_next {lastSymbol()}, m_line {lastLine()},
    m_flags {flags}
{
  lastSymbol() = this;
}

// Symbol::~Symbol()
// {
//   assert(lastSymbol() == this);
//   lastSymbol() = const_cast<Symbol *>(m_next);
// }

static const char *extractRegName(const PluginRegisterBase &reg)
{
  return &reg.key()[strlen("-API_" API_PREFIX)];
}

constexpr bool isDefConstant(const char *definition)
{
  using namespace std::literals;
  constexpr std::string_view signature {"int\0\0"sv};
  return std::string_view {definition, signature.size()} == signature;
}

ReaScriptFunc::ReaScriptFunc(const VerNum version, void *impl,
                             const PluginRegisterBase &native,
                             const PluginRegisterBase &reascript,
                             const PluginRegisterBase &desc)
  : Callable {version, VerNum::MAX, extractRegName(native)},
    Symbol {TargetNative | TargetScript |
      (isDefConstant(desc.value<const char *>()) ? Constant : 0)},
    m_impl {impl}, m_regs {native, reascript, desc}
{
}

void ReaScriptFunc::announce(const bool init) const
{
  for(const PluginRegisterBase &reg : m_regs)
    reg.announce(init);
}

const char *ReaScriptFunc::name() const
{
  return extractRegName(m_regs[0]);
}

EELFunc::EELFunc(const VerNum version, const char *name, const char *definition,
                 VarArgFunc impl, const int argc)
  : Symbol {TargetEELFunc}, m_name {name}, m_definition {definition},
    m_impl {impl}, m_version {version}, m_argc {std::max(1, argc)}
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

  constexpr int ExactArgCount {1};
  NSEEL_addfunc_varparm_ex(m_name, m_argc, ExactArgCount,
                         NSEEL_PProc_THIS, m_impl, &g_eelFuncs);
}

EELVar::EELVar(const VerNum version, const char *name, const char *definition)
  : Symbol {TargetEELFunc | Variable},
    m_name {name}, m_definition {definition}, m_version {version}
{
}

ShimFunc::ShimFunc(const VerNum since, const VerNum until,
                   const char *name, const char *definition,
                   void *safeImpl, void *varargImpl, void *unsafeImpl)
  : Callable {since, until, name}, m_definition {definition},
    m_safeImpl {safeImpl}, m_varargImpl {varargImpl},
    m_unsafeImpl {unsafeImpl}, m_isConstant {isDefConstant(definition)}
{
}

void ShimFunc::activate() const
{
#define SHIM_FUNC API_PREFIX "_shim"
  plugin_register("API_"       SHIM_FUNC, m_safeImpl);
  plugin_register("APIvararg_" SHIM_FUNC, m_varargImpl);
  plugin_register("APIdef_"    SHIM_FUNC, const_cast<char *>(m_definition));
#undef SHIM_FUNC
}

ImportTable::ImportTable(const VerNum version, const size_t size)
  : m_next {lastImportTable()}, m_ftable {offset(size)}, m_version {version}
{
  lastImportTable() = this;
}

void **ImportTable::offset(const size_t bytes)
{
  return reinterpret_cast<void **>(reinterpret_cast<char *>(this) + bytes);
}

void ImportTable::resolve()
{
  for(void **func {offset(sizeof(*this))}; func < m_ftable; ++func) {
    const char *name {static_cast<const char *>(*func)};
    *func = Callable::lookup(m_version, name)->unsafeImpl();
  }
}

const API::Symbol *API_head() // immutable public accessor
{
  return lastSymbol();
}

void API_version(char *out, size_t size)
{
  snprintf(out, size, "%s", latestVersion().toString().c_str());
}

eel_function_table *API::eelFunctionTable()
{
  return &g_eelFuncs;
}

static void announceAll(const bool add)
{
  for(const Symbol *sym {lastSymbol()}; sym; sym = sym->m_next)
    sym->announce(add);
}

void API::setup()
{
  announceAll(true);

  for(ImportTable *tbl {lastImportTable()}; tbl; tbl = tbl->m_next)
    tbl->resolve();
}

void API::teardown()
{
  announceAll(false);
}

VerNum API::version()
{
  return latestVersion();
}

void API::clearError()
{
  g_lastError.clear();
#ifndef NDEBUG
  Context::clearCurrent();
#endif
}

static unsigned int g_reentrant;

ErrorClearer::ErrorClearer()
{
  if(!g_reentrant++)
    clearError();
}

ErrorClearer::~ErrorClearer()
{
  --g_reentrant;
}

const char *API::lastError() noexcept
{
  return g_lastError.empty() ? nullptr : &g_lastError[1];
}

template<typename... Args>
void setError(std::format_string<Args...> fmt, Args&&... args)
{
  // only report the first error per CallConv::Safe API call
  // so that EEL callback errors are not masked by later ones
  if(!g_lastError.empty())
    return;

  g_lastError = std::vformat(fmt.get(), std::make_format_args(args...));
  ReaScriptError(g_lastError.c_str());
}

// REAPER 6.29+ uses the '!' prefix to abort the calling Lua script's execution
void API::handleError(const char *fnName, const reascript_error &e)
{
  setError("!" API_PREFIX "{}: {}", fnName, e.what());
}

void API::handleError(const char *fnName, const imgui_error &e)
{
  setError("!" API_PREFIX "{}: ImGui assertion failed: {}", fnName, e.what());
  assert(Context::current());
  delete Context::current();
}
