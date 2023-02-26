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

#include "function.hpp"

#include "error.hpp"

#include <reaper_plugin_secrets.h> // reaper_array
#include <string>

#define NSEEL_ADDFUNC_DESTINATION (&g_eel_function_table)
#include <eel2_import.hpp>

static eel_function_table g_eel_function_table;

// WDL_FastString is missing a copy constructor/assignment operator
#ifdef HAS_DEPRECATED_COPY // GCC 12
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
// missing includes for eel_strings.h
#include <cstring>
#include <WDL/assocarray.h>
#include <WDL/ptrlist.h>
static eel_string_context_state *EEL_STRING_GET_CONTEXT_POINTER(void *self)
{
  return static_cast<eel_string_context_state *>(self);
}
static void EEL_STRING_DEBUGOUT(const char *fmt, ...) noexcept
{
  va_list args;
  char msg[255];

  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  ReaScriptError(msg); // no leading '!' = non-fatal error (script can continue)
}
#define EEL_STRING_DEBUGOUT EEL_STRING_DEBUGOUT // eel_strings.h does ifdef

#include <eel2/eel_strings.h>
#ifdef HAS_DEPRECATED_COPY
#  pragma GCC diagnostic pop
#endif

using namespace std::string_literals;

void Function::setup()
{
  EEL_string_register();
}

Function::Function(const char *eel2Code)
  : m_strings { std::make_unique<eel_string_context_state>() },
    m_vm { NSEEL_VM_alloc() }
{
  NSEEL_VM_SetCustomFuncThis(m_vm.get(), m_strings.get());
  NSEEL_VM_SetFunctionTable(m_vm.get(), NSEEL_ADDFUNC_DESTINATION);
  eel_string_initvm(m_vm.get());

  m_program.reset(NSEEL_code_compile_ex(m_vm.get(), eel2Code, 0, 0));

  // NSEEL_code_compile_ex checks if m_vm is null
  if(m_program)
    m_strings->update_named_vars(m_vm.get());
  else {
    if(const char *err { NSEEL_code_getcodeerror(m_vm.get()) })
      throw reascript_error { "failed to compile EEL code: "s + err };
    else
      throw reascript_error { "failed to compile EEL code" };
  }
}

Function::~Function()
{
}

void Function::ProgramDeleter::operator()(NSEEL_CODEHANDLE code)
{
  NSEEL_code_free(code);
}

void Function::VMDeleter::operator()(NSEEL_VMCTX vm)
{
  NSEEL_VM_free(vm);
}

void Function::execute() noexcept
{
  NSEEL_code_execute(m_program.get());
}

std::optional<double> Function::getDouble(const char *name) const
{
  // nseel_int_register_var always create a new variable unless
  // isReg=0 and NSEEL_VM_set_var_resolver is used
  // GetNamedVar lookups from a cache of all registered variables
  // (built using NSEEL_VM_enumallvars)
  if(EEL_F *variable { m_strings->GetNamedVar(name, false, nullptr) })
    return *variable;

  return std::nullopt;
}

bool Function::setDouble(const char *name, const double value)
{
  if(EEL_F *variable { m_strings->GetNamedVar(name, true, nullptr) }) {
    *variable = value;
    return true;
  }

  return false;
}

std::optional<std::string_view> Function::getString(const char *name) const
{
  WDL_FastString *storage {};
  const int index { m_strings->m_named_strings_names.Get(name) };
  if(!index || !m_strings->GetStringForIndex(index, &storage, false) || !storage)
    return std::nullopt;
  return std::string_view
    { storage->Get(), static_cast<size_t>(storage->GetLength()) };
}

bool Function::setString(const char *name, const std::string_view &value)
{
  char stringName[255];
  snprintf(stringName, sizeof(stringName), "#%s", name);

  EEL_F index;
  if(!m_strings->GetNamedVar(stringName, true, &index))
    return false;
  WDL_FastString *storage {};
  if(!m_strings->GetStringForIndex(index, &storage, true) || !storage)
    return false;
  storage->SetRaw(value.data(), value.size());
  return true;
}

template<bool WriteToEEL>
static bool copyArray(NSEEL_VMCTX vm, std::optional<double> slot,
                      std::conditional_t<WriteToEEL,
                                         const reaper_array *,
                                         reaper_array *> values)
{
  if(!slot)
    return false;

  auto *data { values->data },
       *end  { values->data + values->size };

  while(data < end) {
    EEL_F *ram;
    int validCount;
    if(!(ram = NSEEL_VM_getramptr(vm, *slot, &validCount)))
      return false;

    const int blockSize { std::min(static_cast<int>(end - data), validCount) };
    static_assert(sizeof(EEL_F[0xFF]) == sizeof(double[0xFF]));
    if constexpr(WriteToEEL)
      std::memcpy(ram, data, blockSize * sizeof(double));
    else
      std::memcpy(data, ram, blockSize * sizeof(double));

    data  += blockSize;
    *slot += blockSize;
  }

  return true;
}

bool Function::getArray(const char *name, reaper_array *values) const
{
  return copyArray<false>(m_vm.get(), getDouble(name), values);
}

bool Function::setArray(const char *name, const reaper_array *values)
{
  return copyArray<true>(m_vm.get(), getDouble(name), values);
}
