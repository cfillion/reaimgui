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

#include "gdk_event_mitm.hpp"

#include <array>
#include <execinfo.h>
#include <swell/swell.h>

static uintptr_t SWELL_RunEvents;
static void (*swell_gdkEventHandler)(GdkEvent *, gpointer);
static GdkEvent *g_curEvent;

// Discover a memory address slightly before the call to
// swell_gdkEventHandler within SWELL_RunEvents.
// Call stack here is SWELL_RunEvents -> gdk_event_get -> ... > xEventFilter
static uintptr_t findSWELLRunEvents()
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
    else if(so.dli_fbase != gdk.dli_fbase)
      return reinterpret_cast<uintptr_t>(bt[i]);
  }

  return 0;
}

// Find the address of the instruction after the call to swell_gdkEventHandler
//
// Path 1
// SWELL_RunMessageLoop > SWELL_MessageQueue_Flush > ... > WndProc
// Path 2 (the interesting one)
// SWELL_RunMessageLoop > SWELL_RunEvents > swell_gdkEventHandler > ... > WndProc
// Path 3
// _gdk_event_emit -> swell_gdkEventHandler > ... > WndProc
static uintptr_t findReturnAddress(const uintptr_t startAddr)
{
  Dl_info swell;
  dladdr(reinterpret_cast<const void *>(SWELL_CreateDialog), &swell);

  void *bt[20];
  const int size { backtrace(bt, std::size(bt)) };
  uintptr_t retAddr { 0 }, lastOffset { UINTPTR_MAX };

  for(int i {}; i < size; ++i) {
    Dl_info so;
    dladdr(bt[i], &so);

    // if(so.dli_fbase == gdk.dli_fbase) // _gdk_event_emit
    //   return reinterpret_cast<uintptr_t>(bt[i]); // path 3 (indirect call)
    if(so.dli_fbase != swell.dli_fbase)
      continue; // skip stack frames outside of SWELL

    // path 2
    const uintptr_t addr   { reinterpret_cast<uintptr_t>(bt[i]) },
                    offset { addr - startAddr };
    if(addr >= startAddr && addr < startAddr + 0x20 && offset < lastOffset)
      retAddr = addr;

    lastOffset = offset;
  }

  return retAddr;
}

struct __attribute__((packed)) Jump {
  uintptr_t retaddr() const { return reinterpret_cast<uintptr_t>(this) + sizeof(*this); }

#if defined(__x86_64__) || defined(__i686__)
  // E8 71 57 FB FF
  unsigned int opcode  : 8;
  signed   int reldest : 32;

  bool valid() const { return opcode == 0xE8; } // relative CALL
  intptr_t target() const { return retaddr() + reldest; }
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
  intptr_t target() const { return retaddr() + imm; }
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
  intptr_t target() const { return retaddr() + imm; }
#else
#  warn Unsupported architecture.
return nullptr;
#endif
};

static void *decodeJump(const uintptr_t retAddr)
{
  const Jump *jump { reinterpret_cast<Jump *>(retAddr - sizeof(Jump)) };
  if(retAddr && jump->valid())
    return reinterpret_cast<void *>(jump->target());
  else
    return nullptr;
}

static GdkFilterReturn xEventFilter(GdkXEvent *, GdkEvent *, gpointer)
{
  SWELL_RunEvents = findSWELLRunEvents();
  if(SWELL_RunEvents)
    gdk_window_remove_filter(nullptr, &xEventFilter, nullptr);
  return GDK_FILTER_CONTINUE;
}

static void eventHandler(GdkEvent *event, gpointer data)
{
  g_curEvent = event;
  swell_gdkEventHandler(event, data);
  g_curEvent = nullptr;
}

GdkEventMITM::GdkEventMITM()
{
  gdk_window_add_filter(nullptr, &xEventFilter, nullptr);
}

GdkEventMITM::~GdkEventMITM()
{
  if(swell_gdkEventHandler)
    gdk_event_handler_set(swell_gdkEventHandler, nullptr, nullptr);
  swell_gdkEventHandler = nullptr;
}

bool GdkEventMITM::active()
{
  return !!swell_gdkEventHandler;
}

void GdkEventMITM::install()
{
  if(void *addr { decodeJump(findReturnAddress(SWELL_RunEvents)) }) {
    printf("reaper_imgui: installed event handler -> %p\n", addr);
    swell_gdkEventHandler =
      reinterpret_cast<decltype(swell_gdkEventHandler)>(addr);
    gdk_event_handler_set(&eventHandler, nullptr, nullptr);
  }
}

GdkEvent *GdkEventMITM::currentEvent()
{
  return g_curEvent;
}
