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

#ifndef REAIMGUI_IMPORT_HPP
#define REAIMGUI_IMPORT_HPP

#include <utility>
#include <windows.h>

template<typename Proc, typename = std::enable_if_t<std::is_function_v<Proc>>>
class FuncImport {
public:
  FuncImport(const wchar_t *dll, const char *func)
  {
    if((m_lib = LoadLibrary(dll)))
      m_proc = reinterpret_cast<Proc *>(GetProcAddress(m_lib, func));
    else
      m_proc = nullptr;
  }

  ~FuncImport()
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
