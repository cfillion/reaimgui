/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
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

#include "resource.hpp"

#include "errors.hpp"

#include <reaper_plugin_functions.h>
#include <unordered_set>
#include <WDL/wdltypes.h>

constexpr size_t MAX_INSTANCES { 99 };

static std::unordered_set<Resource *> g_rsx;
static unsigned int g_reentrant;
#ifndef __APPLE__
static WNDPROC g_mainProc;
#endif

class Resource::Timer {
public:
  Timer();
  ~Timer();

private:
  static void tick();
  static LRESULT CALLBACK mainProcOverride(HWND, unsigned int, WPARAM, LPARAM);
};

Resource::Timer::Timer()
{
#ifndef __APPLE__
  LONG_PTR newProc { reinterpret_cast<LONG_PTR>(&mainProcOverride) },
           oldProc { SetWindowLongPtr(GetMainHwnd(), GWLP_WNDPROC, newProc) };
  g_mainProc = reinterpret_cast<WNDPROC>(oldProc);
#endif

  plugin_register("timer", reinterpret_cast<void *>(&Timer::tick));
}

Resource::Timer::~Timer()
{
  plugin_register("-timer", reinterpret_cast<void *>(&Timer::tick));

#ifndef __APPLE__
  HWND mainWnd { GetMainHwnd() };
  LONG_PTR expectedProc { reinterpret_cast<LONG_PTR>(&mainProcOverride) },
           previousProc { reinterpret_cast<LONG_PTR>(g_mainProc) };
  if(GetWindowLongPtr(mainWnd, GWLP_WNDPROC) == expectedProc)
    SetWindowLongPtr(mainWnd, GWLP_WNDPROC, previousProc);
#endif
}

void Resource::Timer::tick()
{
  // REAPER v6.19+ does not execute deferred script callbacks
  // when the splash screen is open.
  static bool pauseDuringLoad { atof(GetAppVersion()) >= 6.19 };
  if((pauseDuringLoad && Splash_GetWnd()) || g_reentrant > 1)
    return;

  auto it { g_rsx.begin() };

  while(it != g_rsx.end()) {
    Resource *rs { *it++ };
    if(!rs->heartbeat())
      delete rs;
  }
}

#ifndef __APPLE__
LRESULT CALLBACK Resource::Timer::mainProcOverride(HWND hwnd,
  unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  // Timers are reentrant on Windows and Linux, but deferred ReaScripts aren't.
  // The workaround below suspends ReaImGui when a script opens a modal dialog,
  // in order to not flag active contexts as being unused and destroying them.
  //
  // 0x29a is REAPER's "misc timer". It's responsible for triggering both
  // deferred ReaScripts and extension timer callbacks (among other things).
  if(msg == WM_TIMER && wParam == 0x29a) {
    g_reentrant += 1;
    const LRESULT ret { CallWindowProc(g_mainProc, hwnd, msg, wParam, lParam) };
    g_reentrant -= 1;
    return ret;
  }

  return CallWindowProc(g_mainProc, hwnd, msg, wParam, lParam);
}
#endif

Resource::Resource()
{
  if(g_rsx.size() >= MAX_INSTANCES)
    throw reascript_error { "exceeded maximum object allocation limit" };

  static std::weak_ptr<Timer> g_timer;

  if(g_timer.expired())
    g_timer = m_timer = std::make_shared<Timer>();
  else
    m_timer = g_timer.lock();

  g_rsx.emplace(this);
}

Resource::~Resource()
{
  g_rsx.erase(this);
}

template<>
bool Resource::exists<Resource>(Resource *rs)
{
  return g_rsx.count(rs) > 0;
}
