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

#include <version.hpp>

#define REAPERAPI_IMPLEMENT
#include <eel2_import.hpp>
#include <reaper_plugin_secrets.h>

#include "action.hpp"
#include "api.hpp"
#include "docker.hpp"
#include "function.hpp"
#include "resource.hpp"
#include "settings.hpp"
#include "win32_unicode.hpp"
#include "window.hpp"

#include <imgui/imgui.h>
#include <WDL/wdltypes.h> // WDL_DIRCHAR_STR

static void fatalError(const char *message)
{
  HWND parent {Splash_GetWnd ? Splash_GetWnd() : nullptr};
  MessageBox(parent, WIDEN(message), TEXT("ReaImGui (reaper_imgui)"),
    MB_OK | MB_ICONERROR);
}

struct ApiImport {
  template<typename T>
  ApiImport(const char *name, T *ptr, T fallback = nullptr)
    : name {name}, ptr {reinterpret_cast<void **>(ptr)},
      fallback {reinterpret_cast<void *>(fallback)}
  {}
  const char *name; void **ptr; void *fallback;
};

#define IMPORT(name, ...) {#name, &name, __VA_ARGS__}

static bool loadAPI(void *(*getFunc)(const char *))
{
  const ApiImport funcs[] {
    IMPORT(Splash_GetWnd), // v4.7, import first (used by fatalError)
    {"__localizeFunc", &LocalizeString}, // LocalizeString added in v6.11

    IMPORT(AttachWindowTopmostButton),
    IMPORT(DetachWindowTopmostButton),
    IMPORT(Dock_UpdateDockID),
    IMPORT(DockGetPosition, &CompatDockGetPosition), // v6.02
    IMPORT(DockIsChildOfDock),
    IMPORT(DockWindowActivate),
    IMPORT(DockWindowAddEx),
    IMPORT(DockWindowRemove),
    IMPORT(get_config_var),
    IMPORT(get_ini_file),
    IMPORT(GetAppVersion),
    IMPORT(GetMainHwnd),
    IMPORT(GetResourcePath),
    IMPORT(GetToggleCommandState),
    IMPORT(plugin_getapi),
    IMPORT(plugin_register),
    IMPORT(realloc_cmd_ptr), // v5.95
    IMPORT(ReaScriptError),
    IMPORT(RecursiveCreateDirectory),
    IMPORT(RefreshToolbar),
    IMPORT(screenset_registerNew), // v4
    IMPORT(screenset_unregisterByParam),
    IMPORT(ViewPrefs),

    IMPORT(LICE_CreateBitmap),
    IMPORT(LICE_FillRect),
    IMPORT(LICE__Destroy),
    IMPORT(LICE__GetBits),
    IMPORT(LICE__GetHeight),
    IMPORT(LICE__GetRowSpan),
    IMPORT(LICE__GetWidth),
    IMPORT(LICE__resize),

    IMPORT(NSEEL_addfunc_ret_type),
    IMPORT(NSEEL_addfunc_varparm_ex),
    IMPORT(NSEEL_code_compile_ex),
    IMPORT(NSEEL_code_execute),
    IMPORT(NSEEL_code_free),
    IMPORT(NSEEL_code_getcodeerror),
    IMPORT(nseel_int_register_var),
    IMPORT(NSEEL_PProc_THIS),
    IMPORT(nseel_stringsegments_tobuf),
    IMPORT(NSEEL_VM_alloc),
    IMPORT(NSEEL_VM_enumallvars),
    IMPORT(NSEEL_VM_free),
    IMPORT(NSEEL_VM_SetCustomFuncThis),
    IMPORT(NSEEL_VM_SetFunctionTable),
    IMPORT(NSEEL_VM_getramptr),
    IMPORT(NSEEL_VM_SetStringFunc),
  };

  for(const ApiImport &func : funcs) {
    if((*func.ptr = getFunc(func.name)) || (*func.ptr = func.fallback))
      continue;

    char message[1024];
    snprintf(message, sizeof(message),
      "ReaImGui v%s is incompatible with this version of REAPER.\n\n"
      "Unable to import the following API function: %s.",
      REAIMGUI_VERSION, func.name);
    fatalError(message);
    return false;
  }

  return true;
}

static bool isAlreadyLoaded()
{
  if(plugin_getapi("ImGui_GetVersion")) {
    fatalError("More than one copy of ReaImGui are currently installed. "
               "Only one will be loaded.");
    return true;
  }

  return false;
}

extern "C" REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(
  REAPER_PLUGIN_HINSTANCE instance, reaper_plugin_info_t *rec)
{
  if(!rec) {
    API::teardown();
    Resource::destroyAll(); // save context settings
    Settings::teardown();
    Action::teardown();
    return 0;
  }
  else if(rec->caller_version != REAPER_PLUGIN_VERSION ||
      !loadAPI(rec->GetFunc) || isAlreadyLoaded())
    return 0;

  IMGUI_CHECKVERSION();

  Window::s_instance = instance;
  API::setup();
  Action::setup();
  Settings::setup();
  Function::setup();

  new Action {"DOCUMENTATION", "Open ReaScript documentation (HTML)...", [] {
    const auto path {widen(GetResourcePath()) + TEXT(WDL_DIRCHAR_STR)
      TEXT("Data") TEXT(WDL_DIRCHAR_STR) TEXT("reaper_imgui_doc.html")};
    ShellExecute(nullptr, TEXT("open"), path.c_str(), nullptr, nullptr, SW_SHOW);
  }};

  return 1;
}

#ifndef _WIN32
#  include <dialog.rc>
#endif
