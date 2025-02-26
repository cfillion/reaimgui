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

#ifndef REAIMGUI_WIN32_UNICODE_HPP
#define REAIMGUI_WIN32_UNICODE_HPP

#include <string>

#ifdef _WIN32
#  include <windows.h>
#  define WIDEN(cstr)  widen(cstr).c_str()
#  define NARROW(cstr) narrow(cstr).c_str()

inline std::string narrow(const std::wstring_view &input,
  const unsigned int codepage = CP_UTF8)
{
  const int size {WideCharToMultiByte(codepage, 0,
    input.data(), input.size(), nullptr, 0, nullptr, nullptr)};

  std::string output(size, L'\0');
  WideCharToMultiByte(codepage, 0, input.data(), input.size(),
    output.data(), size, nullptr, nullptr);

  return output;
}

inline std::wstring widen(const std::string_view &input,
  const unsigned int codepage = CP_UTF8)
{
  const int size {
    MultiByteToWideChar(codepage, 0, input.data(), input.size(), nullptr, 0)
  };

  std::wstring output(size, L'\0');
  MultiByteToWideChar(codepage, 0, input.data(), input.size(), output.data(), size);

  return output;
}
#else
#  define TCHAR char
#  define TEXT(str) str
#  define WIDEN(cstr) cstr
#  define NARROW(cstr) cstr
inline const std::string &narrow(const std::string &input,
  const unsigned int = 0) { return input; }
inline const std::string &widen(const std::string &input,
  const unsigned int = 0) { return input; }
#endif

#endif
