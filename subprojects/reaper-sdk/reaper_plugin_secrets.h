#ifndef REAPER_PLUGIN_SECRETS_H
#define REAPER_PLUGIN_SECRETS_H

#include "reaper_plugin_functions.h"

// The following API functions are undocumented and not included in
// reaper_plugin_functions.h.

REAPERAPI_DEF void (*AttachWindowTopmostButton)(HWND);
REAPERAPI_DEF void (*DetachWindowTopmostButton)(HWND, bool);

enum LocalizeFlags {
  LOCALIZE_FMT     = 1<<0,
  LOCALIZE_NOCACHE = 1<<1,
};

REAPERAPI_DEF DLGPROC (*__localizePrepareDialog)(
  const char *cat, HINSTANCE, const char *tpl, DLGPROC, LPARAM, void **p, int pcnt);
REAPERAPI_DEF const char *(*__localizeFunc)(const char *str, const char *ctx, int flags);

#endif
