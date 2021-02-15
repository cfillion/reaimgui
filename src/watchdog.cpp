#include "watchdog.hpp"

#include "context.hpp"

#include <reaper_plugin_functions.h>

std::shared_ptr<Watchdog> Watchdog::get()
{
  static std::weak_ptr<Watchdog> g_instance;

  if(!g_instance.expired())
    return g_instance.lock();

  auto instance { std::make_shared<Watchdog>() };
  g_instance = instance;

  return instance;
}

Watchdog::Watchdog()
{
  plugin_register("timer", reinterpret_cast<void *>(&Context::heartbeat));
}

Watchdog::~Watchdog()
{
  plugin_register("-timer", reinterpret_cast<void *>(&Context::heartbeat));
}
