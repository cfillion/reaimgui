#include "heartbeat.hpp"

#include "window.hpp"

#include <reaper_plugin_functions.h>

static std::weak_ptr<Heartbeat> g_heartbeat;

std::shared_ptr<Heartbeat> Heartbeat::get()
{
  if(!g_heartbeat.expired())
    return g_heartbeat.lock();

  auto heartbeat { std::make_shared<Heartbeat>() };
  g_heartbeat = heartbeat;
  return heartbeat;
}

Heartbeat::Heartbeat()
{
  plugin_register("timer", reinterpret_cast<void *>(&tick));
}

Heartbeat::~Heartbeat()
{
  plugin_register("-timer", reinterpret_cast<void *>(&tick));
}

void Heartbeat::tick()
{
  Window::heartbeat();
}
