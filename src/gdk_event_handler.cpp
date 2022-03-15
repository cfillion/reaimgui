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

#include "gdk_event_handler.hpp"

#include <array>
#include <execinfo.h>
#include <reaper_plugin_functions.h>
#include <swell/swell.h>
#include <sys/mman.h>
#include <unistd.h>

struct __attribute__((packed)) Jump {
  uintptr_t retaddr() const { return reinterpret_cast<uintptr_t>(this) + sizeof(*this); }

#if defined(__x86_64__) || defined(__i686__)
  // E8 71 57 FB FF
  unsigned int opcode  : 8;
  signed   int reldest : 32;

  bool valid() const { return opcode == 0xE8; } // relative CALL
  uintptr_t target() const { return retaddr() + reldest; }
  void setTarget(uintptr_t dest) { reldest = dest - retaddr(); }
#elif __ARM_ARCH_7A__
  // ARM Architecture Reference Manual ARMv7-A
  // A8.8.25 BL, BLX (immediate) [page 346]
  // A8.3 Conditional execution  [page 286]
  // https://documentation-service.arm.com/static/5f8daeb7f86e16515cdb8c4e
  //
  // observed EB FF F7 D1 in disassembly, seems to be encoding A1
  unsigned int cond : 4;
  unsigned int a1   : 4;
  signed   int imm  : 24;

  bool valid() const { return cond == 0b1110 && a1 == 0b1011; } // cond=always
  uintptr_t target() const { return retaddr() + imm; }
  void setTarget(uintptr_t dest) { imm = dest - retaddr(); }
#elif __aarch64__
  // ARM Architecture Reference Manual for A-profile architecture
  // https://documentation-service.arm.com/static/61fbe8f4fa8173727a1b734e
  // 6.2.34  BL [page 1205]
  //
  // observed 97 FF F8 7B in disassembly
  unsigned int op  : 1;
  unsigned int bl  : 5;
  signed   int imm : 26;

  bool valid() const { return op == 1 && bl == 0b00101; }
  uintptr_t target() const { return retaddr() + imm; }
  void setTarget(uintptr_t dest) { imm = dest - retaddr(); }
#else
#  warn Unsupported architecture.
return nullptr;
#endif
};

class SWELL_RunEvents {
public:
  SWELL_RunEvents();
  bool found() const { return m_addr != 0; }
  bool find();
  bool findEventHandlerCall();

  GdkEventFunc eventHandler() const;
  bool setEventHandler(GdkEventFunc);

private:
  uintptr_t findSwellEventHandlerReturnAddress();
  uintptr_t m_addr; // not the entry point, but before the event handler call
  Jump *m_eventHandlerCall;
};

static SWELL_RunEvents g_runEvents;
static GdkEventFunc swell_gdkEventHandler;
static GdkEvent *g_curEvent;
static WNDPROC g_wndProc;

static LRESULT CALLBACK findEventHandlerProc(HWND hwnd,
  unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  if(!swell_gdkEventHandler && g_runEvents.findEventHandlerCall()) {
    swell_gdkEventHandler = g_runEvents.eventHandler();
    printf("reaper_imgui: found event handler at %p\n", swell_gdkEventHandler);

    LONG_PTR expectedProc { reinterpret_cast<LONG_PTR>(&findEventHandlerProc) },
             previousProc { reinterpret_cast<LONG_PTR>(g_wndProc) };
    if(GetWindowLong(hwnd, GWL_WNDPROC) == expectedProc)
      SetWindowLong(hwnd, GWL_WNDPROC, previousProc);
  }

  return CallWindowProc(g_wndProc, hwnd, msg, wParam, lParam);
}

static GdkFilterReturn xEventFilter(GdkXEvent *, GdkEvent *, gpointer)
{
  if(!g_runEvents.found() && g_runEvents.find()) {
    gdk_window_remove_filter(nullptr, &xEventFilter, nullptr);

    LONG_PTR newProc { reinterpret_cast<LONG_PTR>(&findEventHandlerProc) },
             oldProc { SetWindowLong(GetMainHwnd(), GWL_WNDPROC, newProc) };
    g_wndProc = reinterpret_cast<WNDPROC>(oldProc);
  }

  return GDK_FILTER_CONTINUE;
}

SWELL_RunEvents::SWELL_RunEvents()
  : m_addr { 0 }, m_eventHandlerCall { nullptr }
{
  gdk_window_add_filter(nullptr, &xEventFilter, nullptr);
}

// Discover a memory address slightly before the call to
// swell_gdkEventHandler within SWELL_RunEvents.
// Call stack here is SWELL_RunEvents -> gdk_event_get -> ... > xEventFilter
bool SWELL_RunEvents::find()
{
  Dl_info gdk;
  dladdr(reinterpret_cast<const void *>(&gdk_window_new), &gdk);

  bool foundGdk { false };
  void *bt[20];
  const int size { backtrace(bt, std::size(bt)) };

  for(int i {}; i < size; ++i) {
    Dl_info so;
    dladdr(bt[i], &so);

    if(!foundGdk)
      foundGdk = so.dli_fbase == gdk.dli_fbase;
    else if(so.dli_fbase != gdk.dli_fbase) {
      m_addr = reinterpret_cast<uintptr_t>(bt[i]);
      return true;
    }
  }

  return false;
}

// Find the address of the instruction after the call to swell_gdkEventHandler
//
// Path 1
// SWELL_RunMessageLoop > SWELL_MessageQueue_Flush > ... > WndProc
// Path 2 (the interesting one)
// SWELL_RunMessageLoop > SWELL_RunEvents > swell_gdkEventHandler > ... > WndProc
// Path 3
// _gdk_event_emit -> swell_gdkEventHandler > ... > WndProc
uintptr_t SWELL_RunEvents::findSwellEventHandlerReturnAddress()
{
  Dl_info swell;
  dladdr(reinterpret_cast<const void *>(SWELL_CreateDialog), &swell);

  void *bt[20];
  const int size { backtrace(bt, std::size(bt)) };
  uintptr_t closestRetAddr { 0 }, lastOffset { UINTPTR_MAX };

  for(int i {}; i < size; ++i) {
    Dl_info so;
    dladdr(bt[i], &so);

    // if(so.dli_fbase == gdk.dli_fbase) // _gdk_event_emit
    //   return reinterpret_cast<uintptr_t>(bt[i]); // path 3 (indirect call)
    if(so.dli_fbase != swell.dli_fbase)
      continue; // skip stack frames outside of SWELL

    // path 2
    const uintptr_t retAddr { reinterpret_cast<uintptr_t>(bt[i]) },
                    offset  { retAddr - m_addr };
    if(retAddr >= m_addr && retAddr < 0x20 + m_addr && offset < lastOffset)
      closestRetAddr = retAddr;

    lastOffset = offset;
  }

  return closestRetAddr;
}

bool SWELL_RunEvents::findEventHandlerCall()
{
  const auto retAddr { findSwellEventHandlerReturnAddress() };
  Jump *jump { reinterpret_cast<Jump *>(retAddr - sizeof(Jump)) };
  if(retAddr && jump->valid())
    m_eventHandlerCall = jump;
  return !!m_eventHandlerCall;
}

GdkEventFunc SWELL_RunEvents::eventHandler() const
{
  if(m_eventHandlerCall && m_eventHandlerCall->valid())
    return reinterpret_cast<GdkEventFunc>(m_eventHandlerCall->target());
  else
    return nullptr;
}

bool SWELL_RunEvents::setEventHandler(GdkEventFunc newHandler)
{
  const auto pageSize { sysconf(_SC_PAGE_SIZE) };
  const uintptr_t addr { reinterpret_cast<uintptr_t>(m_eventHandlerCall) };
  void *pageStart { reinterpret_cast<void *>(addr - (addr % pageSize)) };

  if(mprotect(pageStart, pageSize, PROT_READ | PROT_WRITE))
    return false;
  m_eventHandlerCall->setTarget(reinterpret_cast<uintptr_t>(newHandler));
  mprotect(pageStart, pageSize, PROT_READ | PROT_EXEC);
  return true;
}

static void eventHandler(GdkEvent *event, gpointer data)
{
  g_curEvent = event;
  swell_gdkEventHandler(event, data);
  g_curEvent = nullptr;
}

GdkEventHandler::GdkEventHandler()
{
  if(swell_gdkEventHandler) {
    gdk_event_handler_set(&eventHandler, nullptr, nullptr);
    if(!g_runEvents.setEventHandler(&eventHandler))
      fprintf(stderr, "reaper_imgui: could not patch SWELL_RunEvents\n");
  }
}

GdkEventHandler::~GdkEventHandler()
{
  if(swell_gdkEventHandler) {
    gdk_event_handler_set(swell_gdkEventHandler, nullptr, nullptr);
    if(g_runEvents.eventHandler() == &eventHandler)
      g_runEvents.setEventHandler(swell_gdkEventHandler);
    else
      fprintf(stderr, "reaper_imgui: left altered event handler\n");
  }
}

GdkEvent *GdkEventHandler::currentEvent()
{
  return g_curEvent;
}
