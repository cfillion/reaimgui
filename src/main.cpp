#include "api.hpp"
#include "window.hpp"

#define REAPERAPI_IMPLEMENT
#include <reaper_plugin_functions.h>

#include <imgui/imgui.h>

#define IMPORT(name) { reinterpret_cast<void **>(&name), #name }

static bool loadAPI(void *(*getFunc)(const char *))
{
  struct ApiImport { void **ptr; const char *name; };

  const ApiImport funcs[] {
    IMPORT(plugin_getapi),
    IMPORT(plugin_register),
    IMPORT(GetMainHwnd),
    IMPORT(GetColorThemeStruct),
    IMPORT(ReaScriptError),
    IMPORT(ShowMessageBox),
    IMPORT(realloc_cmd_ptr),
  };

  for(const ApiImport &func : funcs) {
    *func.ptr = getFunc(func.name);

    if(*func.ptr == nullptr)
      return false;
  }

  return true;
}

static bool isAlreadyLoaded()
{
  // TODO: return true of our API functions have already been registered
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
