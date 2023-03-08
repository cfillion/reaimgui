#include <gtest/gtest.h>
#include <reaper_plugin_functions.h>

struct Environment : testing::Environment { void SetUp() override; };

static const auto g_setup
  { testing::AddGlobalTestEnvironment(new Environment) };

void Environment::SetUp()
{
  GetMainHwnd     = []() -> HWND { return nullptr; };
  plugin_register = [](const char *, void *) { return 0; };

#ifndef _WIN32
  GetWindowLong = [](HWND, int)           -> LONG_PTR { return 0; };
  SetWindowLong = [](HWND, int, LONG_PTR) -> LONG_PTR { return 0; };
#endif
}
