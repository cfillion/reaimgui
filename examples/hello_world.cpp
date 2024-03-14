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
// c++ -fPIC -std=c++17 -O2 -IWDL/WDL -shared hello_world.cpp -o reaper_hello_world.so
//
// macOS
// =====
//
// c++ -fPIC -std=c++17 -O2 -IWDL/WDL -dynamiclib hello_world.cpp -o reaper_hello_world.dylib
//
// Windows
// =======
//
// (Use the VS Command Prompt matching your REAPER architecture, eg. x64 to use the 64-bit compiler)
// cl /nologo /std:c++17 /O2 /MT /DUNICODE main.cpp /link /DLL /OUT:reaper_hello_world.dll

#define REAIMGUIAPI_IMPLEMENT
#include "reaper_imgui_functions.h"

#define REAPERAPI_IMPLEMENT
#include "reaper_plugin_functions.h"

#include <memory>

class Example {
public:
  static void start();
  ~Example();

private:
  static void loop();
  static std::unique_ptr<Example> s_inst;

  Example();
  void frame();

  ImGui_Context *m_ctx;
  int m_click_count;
  char m_text[255];
};

constexpr const char *g_name { "ReaImGui C++ example" };
static int g_actionId;
std::unique_ptr<Example> Example::s_inst;

static void reportError(const ImGui_Error &e)
{
  ShowMessageBox(e.what(), g_name, 0);
}

Example::Example()
  : m_ctx {}, m_click_count {},
    m_text { "The quick brown fox jumps over the lazy dog" }
{
  ImGui::init(plugin_getapi);
  m_ctx = ImGui::CreateContext(g_name);
  plugin_register("timer", reinterpret_cast<void *>(&loop));
}

Example::~Example()
{
  plugin_register("-timer", reinterpret_cast<void *>(&loop));
}

void Example::start()
try {
  if(s_inst)
    ImGui::SetNextWindowFocus(s_inst->m_ctx);
  else
    s_inst.reset(new Example);
}
catch(const ImGui_Error &e) {
  reportError(e);
  s_inst.reset();
}

void Example::loop()
try {
  s_inst->frame();
}
catch(const ImGui_Error &e) {
  reportError(e);
  s_inst.reset();
}

void Example::frame()
{
  ImGui::SetNextWindowSize(m_ctx, 400, 80, ImGui::Cond_FirstUseEver);

  bool open { true };
  if(ImGui::Begin(m_ctx, g_name, &open)) {
    if(ImGui::Button(m_ctx, "Click me!"))
      ++m_click_count;

    if(m_click_count & 1) {
      ImGui::SameLine(m_ctx);
      ImGui::Text(m_ctx, R"(\o/)");
    }

    ImGui::InputText(m_ctx, "text input", m_text, sizeof(m_text));
    ImGui::End(m_ctx);
  }

  if(!open)
    return s_inst.reset();
}

static bool commandHook(KbdSectionInfo *sec, const int command,
  const int val, const int valhw, const int relmode, HWND hwnd)
{
  (void)sec; (void)val; (void)valhw; (void)relmode; (void)hwnd; // unused

  if(command != g_actionId)
    return false;

  Example::start();
  return true;
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
    (rec->GetFunc("plugin_getapi"));
  plugin_register = reinterpret_cast<decltype(plugin_register)>
    (rec->GetFunc("plugin_register"));
  ShowMessageBox  = reinterpret_cast<decltype(ShowMessageBox)>
    (rec->GetFunc("ShowMessageBox"));

  custom_action_register_t action { 0, "REAIMGUI_CPP", "ReaImGui C++ example" };
  g_actionId = plugin_register("custom_action", &action);

  plugin_register("hookcommand2", reinterpret_cast<void *>(&commandHook));

  return 1;
}
