#ifndef REAPER_PLUGIN_SECRETS_H
#define REAPER_PLUGIN_SECRETS_H

#include "reaper_plugin_functions.h"

// The following API functions are undocumented and not included in
// reaper_plugin_functions.h.

REAPERAPI_DEF void (*AttachWindowTopmostButton)(HWND);
REAPERAPI_DEF void (*DetachWindowTopmostButton)(HWND, bool);

#endif
