#ifndef REAIMGUI_DLLIMPORT_HPP
#define REAIMGUI_DLLIMPORT_HPP

#ifndef _WIN32
#  error This file is only meant to be included on Windows.
#endif

#include <windows.h>

template<typename Proc, typename = std::enable_if_t<std::is_function_v<Proc>>>
class DllImport {
public:
  DllImport(const wchar_t *dll, const char *func)
  {
    if(m_lib = LoadLibrary(dll))
      m_proc = reinterpret_cast<Proc *>(GetProcAddress(m_lib, func));
    else
      m_proc = nullptr;
  }

  ~DllImport()
  {
    if(m_lib)
      FreeLibrary(m_lib);
  }

  operator bool() const { return m_proc != nullptr; }

  template<typename... Args>
  auto operator()(Args&&... args) const { return m_proc(std::forward<Args>(args)...); }

private:
  HINSTANCE m_lib;
  Proc *m_proc;
};

#endif
