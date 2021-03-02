#include "api.hpp"
#include "version.hpp"
#include "window.hpp"

#define REAPERAPI_IMPLEMENT
#include <reaper_plugin_functions.h>
#include <reaper_plugin_secrets.h>

#include <imgui/imgui.h>

#define IMPORT(name) { reinterpret_cast<void **>(&name), #name }

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
  struct ApiImport { void **ptr; const char *name; };

  const ApiImport funcs[] {
    IMPORT(Splash_GetWnd), // v4.7

    IMPORT(AttachWindowTopmostButton),
    IMPORT(DockWindowRemove),
    IMPORT(GetColorThemeStruct),
    IMPORT(GetMainHwnd),
    IMPORT(plugin_getapi),
    IMPORT(plugin_register),
    IMPORT(realloc_cmd_ptr),
    IMPORT(ReaScriptError),
    IMPORT(ShowConsoleMsg),
    IMPORT(ShowMessageBox),
  };

  for(const ApiImport &func : funcs) {
    *func.ptr = getFunc(func.name);

    if(*func.ptr == nullptr) {
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
    fatalError("More than one copy of ReaImGui is currently installed. "
               "Only one will be loaded.");
    return true;
  }

  return false;
}

extern "C" REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(
  REAPER_PLUGIN_HINSTANCE instance, reaper_plugin_info_t *rec)
{
  if(!rec) {
    API::unregisterAll();
    return 0;
  }
  else if(rec->caller_version != REAPER_PLUGIN_VERSION
      || !loadAPI(rec->GetFunc) || isAlreadyLoaded())
    return 0;

  IMGUI_CHECKVERSION();

  Window::s_instance = instance;
  API::registerAll();

  return 1;
}
