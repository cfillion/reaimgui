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

static void frame()
{
  static int click_count;
  static char text[255] { "The quick brown fox jumps over the lazy dog" };

  if(ImGui::Button(g_ctx, "Click me!"))
    ++click_count;

  if(click_count % 2 == 1) {
    ImGui::SameLine(g_ctx);
    ImGui::Text(g_ctx, R"(\o/)");
  }

  ImGui::InputText(g_ctx, "text input", text, sizeof(text));
}

static void loop()
{
  if(!g_ctx)
    g_ctx = ImGui::CreateContext("My extension");

  int cond { ImGui::Cond_FirstUseEver };
  ImGui::SetNextWindowSize(g_ctx, 400, 80, &cond);

  bool open { true };
  if(ImGui::Begin(g_ctx, "ReaImGui C++ example", &open)) {
    frame();
    ImGui::End(g_ctx);
  }

  if(!open || !ImGui::ValidatePtr(g_ctx, "ImGui_Context*")) {
    plugin_register("-timer", reinterpret_cast<void *>(&loop));
    g_ctx = nullptr;
  }
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
