#include "context.hpp"

#include "window.hpp"

#include <imgui/imgui.h>
#include <reaper_plugin_functions.h>

static std::weak_ptr<Context> g_context;

std::shared_ptr<Context> Context::get()
{
  if(!g_context.expired())
    return g_context.lock();

  auto context { create() };
  g_context = context;
  return context;
}

Context::Context()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGuiIO &io { ImGui::GetIO() };
  io.IniFilename = nullptr;

  plugin_register("timer", reinterpret_cast<void *>(&timerTick));
}

Context::~Context()
{
  plugin_register("-timer", reinterpret_cast<void *>(&timerTick));

  ImGui::DestroyContext();
}

void Context::timerTick()
{
  Window::heartbeat();
}
