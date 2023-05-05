#ifndef REAPER_EEL2_IMPORT_HPP
#define REAPER_EEL2_IMPORT_HPP

class eel_string_context_state;

// <WDL/eel2/eel2-import.h> is missing many types and its function pointers
// cannot be shared across multiple files (no 'extern' support)

#include <eel2/ns-eel.h>
#include <eel2/ns-eel-int.h>

#ifdef REAPERAPI_IMPLEMENT
#  define EELAPI_DEF
#else
#  define EELAPI_DEF extern
#endif

#define EELFUNC(func) EELAPI_DEF decltype(&::func) func

namespace eel2_import {
  EELFUNC(NSEEL_addfunc_ret_type);
  EELFUNC(NSEEL_addfunc_varparm_ex);
  EELFUNC(NSEEL_code_compile_ex);
  EELFUNC(NSEEL_code_execute);
  EELFUNC(NSEEL_code_free);
  EELFUNC(NSEEL_code_getcodeerror);
  EELFUNC(nseel_int_register_var);
  EELFUNC(NSEEL_PProc_THIS);
  EELFUNC(nseel_stringsegments_tobuf);
  EELFUNC(NSEEL_VM_alloc);
  EELFUNC(NSEEL_VM_enumallvars);
  EELFUNC(NSEEL_VM_free);
  EELFUNC(NSEEL_VM_getramptr);
  EELFUNC(NSEEL_VM_SetCustomFuncThis);
  EELFUNC(NSEEL_VM_SetFunctionTable);
  EELFUNC(NSEEL_VM_SetStringFunc);
}

#undef EELFUNC
#undef EELAPI_DEF

#ifndef EEL2IMPORT_NO_GLOBAL
#  define NSEEL_addfunc_ret_type     eel2_import::NSEEL_addfunc_ret_type
#  define NSEEL_addfunc_varparm_ex   eel2_import::NSEEL_addfunc_varparm_ex
#  define NSEEL_code_compile_ex      eel2_import::NSEEL_code_compile_ex
#  define NSEEL_code_execute         eel2_import::NSEEL_code_execute
#  define NSEEL_code_free            eel2_import::NSEEL_code_free
#  define NSEEL_code_getcodeerror    eel2_import::NSEEL_code_getcodeerror
#  define nseel_int_register_var     eel2_import::nseel_int_register_var
#  define NSEEL_PProc_THIS           eel2_import::NSEEL_PProc_THIS
#  define nseel_stringsegments_tobuf eel2_import::nseel_stringsegments_tobuf
#  define NSEEL_VM_alloc             eel2_import::NSEEL_VM_alloc
#  define NSEEL_VM_enumallvars       eel2_import::NSEEL_VM_enumallvars
#  define NSEEL_VM_free              eel2_import::NSEEL_VM_free
#  define NSEEL_VM_SetCustomFuncThis eel2_import::NSEEL_VM_SetCustomFuncThis
#  define NSEEL_VM_SetFunctionTable  eel2_import::NSEEL_VM_SetFunctionTable
#  define NSEEL_VM_getramptr         eel2_import::NSEEL_VM_getramptr
#  define NSEEL_VM_SetStringFunc     eel2_import::NSEEL_VM_SetStringFunc
#endif

#endif
