/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
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

#ifndef REAIMGUI_IMPORT_HPP
#define REAIMGUI_IMPORT_HPP

#include <utility>
#ifdef _WIN32
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

template<typename Proc, typename = std::enable_if_t<std::is_function_v<Proc>>>
class FuncImport {
public:
#ifdef _WIN32
  FuncImport(const wchar_t *dll, const char *func)
#else
  FuncImport(const char *lib, const char *func)
#endif
  {
#ifdef _WIN32
    if((m_lib = LoadLibrary(dll)))
      m_proc = reinterpret_cast<Proc *>(GetProcAddress(m_lib, func));
#else
    if((m_lib = dlopen(lib, RTLD_LAZY)))
      m_proc = reinterpret_cast<Proc *>(dlsym(m_lib, func));
#endif
    else
      m_proc = nullptr;
  }

  ~FuncImport()
  {
    if(m_lib)
#ifdef _WIN32
      FreeLibrary(m_lib);
#else
      dlclose(m_lib);
#endif
  }

  operator bool() const { return m_proc != nullptr; }

  template<typename... Args>
  auto operator()(Args&&... args) const { return m_proc(std::forward<Args>(args)...); }

private:
#ifdef _WIN32
  HINSTANCE m_lib;
#else
  void *m_lib;
#endif
  Proc *m_proc;
};

#ifdef __OBJC__
#  include <objc/runtime.h>

class ClassImport {
public:
  ClassImport(const char *lib, const char *name)
  {
    if((m_lib = dlopen(lib, RTLD_LAZY)))
      m_class = objc_getClass(name);
    else
      m_class = nil;
  }

  ~ClassImport()
  {
    if(m_lib)
      dlclose(m_lib);
  }

  operator bool() { return m_class != nil; }
  operator id() { return m_class; }

private:
  void *m_lib;
  id m_class;
};
#endif

#endif
