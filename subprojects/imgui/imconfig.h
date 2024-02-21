#ifndef REAIMGUI_IMCONFIG_H
#define REAIMGUI_IMCONFIG_H

namespace Error {
  [[noreturn]] void imguiAssertionFailure(const char *);
  [[noreturn]] void imguiDebugBreak();
};

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_USE_WCHAR32

#define ImTextureID size_t

#define IM_ASSERT(_EXPR) (_EXPR ? (void)0 : Error::imguiAssertionFailure(#_EXPR))
#define IM_DEBUG_BREAK() Error::imguiDebugBreak();

#endif
