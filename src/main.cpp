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

#define REAPERAPI_IMPLEMENT
#include <reaper_plugin_functions.h>
#include <reaper_plugin_secrets.h>

#include "api.hpp"
#include "resource.hpp"
#include "settings.hpp"
#include "version.hpp"
#include "window.hpp"

#include <imgui/imgui.h>

#define IMPORT(name, ...) { reinterpret_cast<void **>(&name), #name, __VA_ARGS__ }

#ifdef MessageBox
#  undef MessageBox
#  define MessageBox MessageBoxA
#endif

static void fatalError(const char *message)
{
  HWND parent { Splash_GetWnd ? Splash_GetWnd() : nullptr };
  MessageBox(parent, message, "ReaImGui (reaper_imgui)", MB_OK);
}

static bool loadAPI(void *(*getFunc)(const char *))
{
  struct ApiImport { void **ptr; const char *name; bool required = true; };

  const ApiImport funcs[] {
    IMPORT(Splash_GetWnd), // v4.7

    IMPORT(AttachWindowTopmostButton),
    IMPORT(DetachWindowTopmostButton),
    IMPORT(Dock_UpdateDockID),
    IMPORT(DockGetPosition, false), // v6.02
    IMPORT(DockIsChildOfDock),
    IMPORT(DockWindowActivate),
    IMPORT(DockWindowAddEx),
    IMPORT(DockWindowRemove),
    IMPORT(get_config_var),
    IMPORT(get_ini_file),
    IMPORT(GetAppVersion),
    IMPORT(GetColorThemeStruct),
    IMPORT(GetMainHwnd),
    IMPORT(GetResourcePath),
    IMPORT(plugin_getapi),
    IMPORT(plugin_register),
    IMPORT(realloc_cmd_ptr), // v5.26
    IMPORT(ReaScriptError),
    IMPORT(RecursiveCreateDirectory),
    IMPORT(ViewPrefs),

    IMPORT(LICE_CreateBitmap),
    IMPORT(LICE__Destroy),
    IMPORT(LICE__GetBits),
    IMPORT(LICE__GetHeight),
    IMPORT(LICE__GetRowSpan),
    IMPORT(LICE__GetWidth),
    IMPORT(LICE__resize),
  };

  for(const ApiImport &func : funcs) {
    *func.ptr = getFunc(func.name);

    if(*func.ptr == nullptr && func.required) {
      char message[1024];
      snprintf(message, sizeof(message),
        "ReaImGui v%s is incompatible with this version of REAPER.\n\n"
        "Unable to import the following API function: %s.",
        REAIMGUI_VERSION, func.name);
      fatalError(message);
      return false;
    }
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
    API::announceAll(false);
    Resource::destroyAll(); // save context settings
    Settings::teardown();
    return 0;
  }
  else if(rec->caller_version != REAPER_PLUGIN_VERSION
      || !loadAPI(rec->GetFunc) || isAlreadyLoaded())
    return 0;

  IMGUI_CHECKVERSION();

  Window::s_instance = instance;
  API::announceAll(true);
  Settings::setup();

  return 1;
}

#ifndef _WIN32
#  include "dialog.hpp"
#  include <swell/swell-dlggen.h>
#  include "dialog.rc_mac_dlg"
#  include <swell/swell-menugen.h>
#  include "dialog.rc_mac_menu"
#endif
