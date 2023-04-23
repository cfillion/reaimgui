#ifndef REAIMGUI_IMCONFIG_H
#define REAIMGUI_IMCONFIG_H

#include "src/error.hpp"

inline void ReaImGui_Assert(const bool ok, const char *message)
{
  if(!ok)
    throw imgui_error { message };
}

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_USE_WCHAR32

#define ImTextureID size_t

#define IM_ASSERT(_EXPR) ReaImGui_Assert(_EXPR, #_EXPR)
#define IM_DEBUG_BREAK() throw reascript_error { "debug break" }

#endif
