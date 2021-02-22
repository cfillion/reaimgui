#ifndef REAPER_PLUGIN_SECRETS_H
#define REAPER_PLUGIN_SECRETS_H

#include "reaper_plugin_functions.h"

// https://forum.cockos.com/showthread.php?t=211620
struct reaper_array {
  const unsigned int size, alloc;
  double data[1];
};

// The following API functions are undocumented and not included in
// reaper_plugin_functions.h.

REAPERAPI_DEF void (*AttachWindowTopmostButton)(HWND);

#endif
