// This extension adds an action named "ReaImGui C++ example"
//
// Prerequisites
// =============
//
// 1. Grab reaper_plugin.h from https://github.com/justinfrankel/reaper-sdk/raw/main/sdk/reaper_plugin.h
// 2. Grab reaper_plugin_functions.h by running the REAPER action "[developer] Write C++ API functions header"
// 3. Grab reaper_imgui_functions.h from https://github.com/cfillion/reaimgui/releases
// 4. Grab WDL: git clone https://github.com/justinfrankel/WDL.git
// 5. Build then copy or link the binary file into <REAPER resource directory>/UserPlugins
//
// Linux
// =====
//
// c++ -fPIC -O2 -std=c++14 -IWDL/WDL -shared hello_world.cpp -o reaper_hello_world.so
//
// macOS
// =====
//
// c++ -fPIC -O2 -std=c++14 -IWDL/WDL -dynamiclib hello_world.cpp -o reaper_hello_world.dylib
//
// Windows
// =======
//
// (Use the VS Command Prompt matching your REAPER architecture, eg. x64 to use the 64-bit compiler)
// cl /nologo /O2 /Z7 /Zo /DUNICODE main.cpp /link /DEBUG /OPT:REF /DLL /OUT:reaper_hello_world.dll

#define REAPERAPI_IMPLEMENT
#include "reaper_plugin_functions.h"

#define REAIMGUIAPI_IMPLEMENT
#include "reaper_imgui_functions.h"

static int g_actionId;
static ImGui_Context *g_ctx;
static ImGui_Viewport *g_viewport;

static void frame()
{
  static int click_count;
  static char text[255] { "hello dear imgui" };

  if(ImGui_Button(g_ctx, "Click me!", nullptr, nullptr))
    ++click_count;

  if(click_count % 2 == 1) {
    ImGui_SameLine(g_ctx, nullptr, nullptr);
    ImGui_Text(g_ctx, R"(\o/)");
  }

  ImGui_InputText(g_ctx, "text input", text, sizeof(text), nullptr);
}

static void loop()
{
  if(!g_ctx) {
    g_ctx = ImGui_CreateContext("My extension", 300, 60, nullptr, nullptr, nullptr, nullptr);
    g_viewport = ImGui_GetMainViewport(g_ctx);
  }

  if(ImGui_IsCloseRequested(g_ctx)) {
    plugin_register("-timer", reinterpret_cast<void *>(&loop));
    ImGui_DestroyContext(g_ctx);
    g_ctx = nullptr;
    return;
  }

  double x, y, w, h;
  ImGui_Viewport_GetPos(g_viewport, &x, &y);
  ImGui_Viewport_GetSize(g_viewport, &w, &h);
  ImGui_SetNextWindowPos(g_ctx, x, y, nullptr, nullptr, nullptr);
  ImGui_SetNextWindowSize(g_ctx, w, h, nullptr);

  int window_flags { ImGui_WindowFlags_NoDecoration };
  ImGui_Begin(g_ctx, "Window", nullptr, &window_flags);
  frame();
  ImGui_End(g_ctx);
}

static bool commandHook(KbdSectionInfo *sec, const int command,
  const int val, const int valhw, const int relmode, HWND hwnd)
{
  (void)sec; (void)val; (void)valhw; (void)relmode; (void)hwnd; // unused

  if(command == g_actionId) {
    if(!g_ctx)
      plugin_register("timer", reinterpret_cast<void *>(&loop));
    return true;
  }

  return false;
}

extern "C" REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(
  REAPER_PLUGIN_HINSTANCE instance, reaper_plugin_info_t *rec)
{
  if(!rec)
    return 0; // cleanup here
  else if(rec->caller_version != REAPER_PLUGIN_VERSION)
    return 0;

  // see also https://gist.github.com/cfillion/350356a62c61a1a2640024f8dc6c6770
  plugin_getapi   = reinterpret_cast<decltype(plugin_getapi)>
    (rec->GetFunc("plugin_getapi")); // used by reaper_imgui_functions.h
  plugin_register = reinterpret_cast<decltype(plugin_register)>
    (rec->GetFunc("plugin_register"));

  custom_action_register_t action { 0, "REAIMGUI_CPP", "ReaImGui C++ example" };
  g_actionId = plugin_register("custom_action", &action);

  plugin_register("hookcommand2", reinterpret_cast<void *>(&commandHook));

  return 1;
}
