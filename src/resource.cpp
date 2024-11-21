/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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
#include "error.hpp"

#include <cassert>
#include <functional>

#include <reaper_plugin_functions.h>
#include <WDL/wdltypes.h>

// Splash_GetWnd may be NULL for a brief moment after _s_splash_thread_running
// is set internally. The misc timer may fire during this time, skipping
// deferred scripts while still running extension callbacks (seen on Windows).
// This value must not be under 2 for working around that by waiting an extra
// frame before collecting unused objects [p=2450259].
constexpr unsigned char KEEP_ALIVE_FRAMES { 2 };

// How many back-to-back GC frames to tolerate before complaining
constexpr unsigned char MAX_GC_FRAMES { 120 };

enum Flags {
  BypassGCCheck = 1<<0,
};

FlatSet<Resource *> Resource::g_rsx;
Resource::Timer *Resource::g_timer;

static unsigned int  g_reentrant;
static unsigned char g_consecutiveGcFrames;
static WNDPROC g_mainProc;
static bool g_disableProcOverride, g_bypassGCCheckOnce;
#ifndef __APPLE__
static bool g_disabledViewports;
#endif

static bool isDeferLoopBlocked()
{
  // REAPER v6.19+ does not execute deferred script callbacks
  // when the splash screen is open.
  static bool pauseDuringLoad { atof(GetAppVersion()) >= 6.19 };
  return (pauseDuringLoad && Splash_GetWnd()) || g_reentrant > 1;
}

struct Resource::Timer {
  Timer();
  ~Timer();

  static void tick();
  static LRESULT CALLBACK mainProcOverride(HWND, unsigned int, WPARAM, LPARAM);
};

Resource::Timer::Timer()
{
  if(!g_disableProcOverride) {
    LONG_PTR newProc { reinterpret_cast<LONG_PTR>(&mainProcOverride) },
             oldProc { SetWindowLongPtr(GetMainHwnd(), GWLP_WNDPROC, newProc) };
    g_mainProc = reinterpret_cast<WNDPROC>(oldProc);
  }

  plugin_register("timer", reinterpret_cast<void *>(&Timer::tick));
}

Resource::Timer::~Timer()
{
  plugin_register("-timer", reinterpret_cast<void *>(&Timer::tick));
  g_consecutiveGcFrames = 0;

  HWND mainWnd { GetMainHwnd() };
  LONG_PTR expectedProc { reinterpret_cast<LONG_PTR>(&mainProcOverride) },
           previousProc { reinterpret_cast<LONG_PTR>(g_mainProc) };
  if(GetWindowLongPtr(mainWnd, GWLP_WNDPROC) == expectedProc)
    SetWindowLongPtr(mainWnd, GWLP_WNDPROC, previousProc);
  else // prevent mainProcOverride from calling itself next time
    g_disableProcOverride = true;
}

void Resource::Timer::tick()
{
  const bool blocked { isDeferLoopBlocked() };

#ifndef __APPLE__
  if(blocked != g_disabledViewports) {
    using namespace std::placeholders;
    Resource::foreach<Context>(std::bind(&Context::enableViewports, _1, !blocked));
    g_disabledViewports = blocked;
  }
#endif

  if(blocked)
    return;

  auto it { g_rsx.begin() };
  bool didGc { false };

  while(it != g_rsx.end()) {
    Resource *rs { *it };
    if(rs->heartbeat())
      ++it;
    else {
      didGc |= !(rs->m_flags & BypassGCCheck);
      delete rs; // invalidates the iterator
      it = g_rsx.lowerBound(rs);
    }
  }

  if(didGc)
    ++g_consecutiveGcFrames;
  else
    g_consecutiveGcFrames = 0;
}

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

Resource::Resource()
  : m_keepAlive { KEEP_ALIVE_FRAMES }, m_flags {}
{
  if(g_bypassGCCheckOnce) {
    // < 0.9 backward compatibility
    g_bypassGCCheckOnce = false;
    m_flags |= BypassGCCheck;
  }
  else if(g_consecutiveGcFrames >= MAX_GC_FRAMES)
    throw reascript_error { "excessive creation of short-lived resources" };

  if(!g_timer)
    g_timer = new Timer;

  g_rsx.insert(this);
}

Resource::~Resource()
{
  g_rsx.erase(this);

  if(g_rsx.empty()) {
    delete g_timer;
    g_timer = nullptr;
  }
}

void Resource::keepAlive()
{
  m_keepAlive = KEEP_ALIVE_FRAMES;
}

bool Resource::heartbeat()
{
  if(m_keepAlive < 0)
    return false;

  --m_keepAlive;
  return true;
}

bool Resource::isValid() const
{
  // Prevent usage of resources within their last frame of existence
  // so that the texture manager doesn't refer to resources deleted at the
  // frame end's heartbeat that were previously valid when the frame started.
  return m_keepAlive >= 0;
}

void Resource::destroyAll()
{
  while(!g_rsx.empty())
    delete g_rsx.back();
}

void Resource::bypassGCCheckOnce()
{
  g_bypassGCCheckOnce = true;
}

void Resource::testHeartbeat()
{
  g_timer->tick();
}
