#ifndef REAIMGUI_IMCONFIG_H
#define REAIMGUI_IMCONFIG_H

namespace Error {
  [[noreturn]] void throwAssertionFailure(const char *);
  [[noreturn]] void throwDebugBreak();
};

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_USE_WCHAR32

#define ImTextureID_Invalid ((ImTextureID)-1)

#define IM_ASSERT(_EXPR) (_EXPR ? (void)0 : Error::throwAssertionFailure(#_EXPR))
#define IM_DEBUG_BREAK() Error::throwDebugBreak();

#endif
