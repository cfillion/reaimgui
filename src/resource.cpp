/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2022  Christian Fillion
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

#include "context.hpp"
#include "errors.hpp"

#include <cassert>
#include <reaper_plugin_functions.h>
#include <vector>
#include <WDL/wdltypes.h>

// sizeof(ImGuiContext) = ~30 KB, not including windows and other dynamic allocs
constexpr size_t MAX_INSTANCES { 1'000 };

// Splash_GetWnd may be NULL for a brief moment after _s_splash_thread_running
// is set internally. The misc timer may fire during this time, skipping
// deferred scripts while still running extension callbacks (seen on Windows).
// To workaround this, wait an extra frame before collecting unused objects.
// [p=2450259]
constexpr unsigned int KEEP_ALIVE_FRAMES { 2 };

static std::vector<Resource *> g_rsx;
static unsigned int g_reentrant;
#ifndef __APPLE__
static WNDPROC g_mainProc;
static bool g_disabledViewports, g_disableProcOverride;
#endif

static auto findLowerBound(Resource *rs)
{
  return std::lower_bound(g_rsx.begin(), g_rsx.end(), rs);
}

static bool isDeferLoopBlocked()
{
  // REAPER v6.19+ does not execute deferred script callbacks
  // when the splash screen is open.
  static bool pauseDuringLoad { atof(GetAppVersion()) >= 6.19 };
  return (pauseDuringLoad && Splash_GetWnd()) || g_reentrant > 1;
}

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
  if(!g_disableProcOverride) {
    LONG_PTR newProc { reinterpret_cast<LONG_PTR>(&mainProcOverride) },
             oldProc { SetWindowLongPtr(GetMainHwnd(), GWLP_WNDPROC, newProc) };
    g_mainProc = reinterpret_cast<WNDPROC>(oldProc);
  }
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
  else // prevent mainProcOverride from calling itself next time
    g_disableProcOverride = true;
#endif
}

void Resource::Timer::tick()
{
  const bool blocked { isDeferLoopBlocked() };

#ifndef __APPLE__
  if(blocked != g_disabledViewports) {
    for(Resource *rs : g_rsx) {
      if(Context *ctx { dynamic_cast<Context *>(rs) })
        ctx->enableViewports(!blocked);
    }
    g_disabledViewports = blocked;
  }
#endif

  if(blocked)
    return;

  auto it { g_rsx.begin() };

  while(it != g_rsx.end()) {
    Resource *rs { *it };
    if(rs->heartbeat())
      ++it;
    else {
      delete rs; // invalidates the iterator
      it = findLowerBound(rs);
    }
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
  : m_keepAlive { KEEP_ALIVE_FRAMES }
{
  if(g_rsx.size() >= MAX_INSTANCES)
    throw reascript_error { "exceeded maximum object allocation limit" };

  static std::weak_ptr<Timer> g_timer;

  if(g_timer.expired())
    g_timer = m_timer = std::make_shared<Timer>();
  else
    m_timer = g_timer.lock();

  g_rsx.insert(findLowerBound(this), this);
}

Resource::~Resource()
{
  g_rsx.erase(findLowerBound(this));
}

void Resource::keepAlive()
{
  m_keepAlive = KEEP_ALIVE_FRAMES;
}

bool Resource::heartbeat()
{
  if(!m_keepAlive)
    return false;

  --m_keepAlive;
  return true;
}

bool Resource::isValid() const
{
  return true;
}

template<>
bool Resource::isValid<Resource>(Resource *rs)
{
  if(!rs)
    return false;

  const auto it { findLowerBound(rs) };
  return it != g_rsx.end() && *it == rs;
}

void Resource::destroyAll()
{
  while(!g_rsx.empty())
    delete g_rsx.back();
}
