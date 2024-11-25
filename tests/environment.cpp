#include "../src/function.hpp"

#define EEL2IMPORT_NO_GLOBAL
#include <eel2_import.hpp>
#include <gtest/gtest.h>
#include <reaper_plugin_functions.h>

void NSEEL_HOSTSTUB_EnterMutex() {}
void NSEEL_HOSTSTUB_LeaveMutex() {}

struct Environment : testing::Environment { void SetUp() override; };

static const auto g_setup
  { testing::AddGlobalTestEnvironment(new Environment) };

void Environment::SetUp()
{
  get_config_var  = [](const char *, int *) -> void * { return nullptr; };
  GetAppVersion   = []() { return "5.99"; };
  GetMainHwnd     = []() -> HWND { return nullptr; };
  plugin_register = [](const char *, void *) { return 0; };
  Splash_GetWnd   = []() -> HWND { return nullptr; };

#ifndef _WIN32
  GetWindowLong = [](HWND, int)           -> LONG_PTR { return 0; };
  SetWindowLong = [](HWND, int, LONG_PTR) -> LONG_PTR { return 0; };
#endif

#define EEL_IMPORT(name) eel2_import::name = &name
  EEL_IMPORT(NSEEL_addfunc_ret_type);
  EEL_IMPORT(NSEEL_addfunc_varparm_ex);
  EEL_IMPORT(NSEEL_code_compile_ex);
  EEL_IMPORT(NSEEL_code_execute);
  EEL_IMPORT(NSEEL_code_free);
  EEL_IMPORT(NSEEL_code_getcodeerror);
  EEL_IMPORT(nseel_int_register_var);
  EEL_IMPORT(NSEEL_PProc_THIS);
  EEL_IMPORT(nseel_stringsegments_tobuf);
  EEL_IMPORT(NSEEL_VM_alloc);
  EEL_IMPORT(NSEEL_VM_enumallvars);
  EEL_IMPORT(NSEEL_VM_free);
  EEL_IMPORT(NSEEL_VM_getramptr);
  EEL_IMPORT(NSEEL_VM_SetCustomFuncThis);
  EEL_IMPORT(NSEEL_VM_SetFunctionTable);
  EEL_IMPORT(NSEEL_VM_SetStringFunc);
#undef EEL_IMPORT

  Function::setup();
}
