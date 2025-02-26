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

#ifndef REAIMGUI_COMPSTR_UTILS_HPP
#define REAIMGUI_COMPSTR_UTILS_HPP

#include <optional>

namespace CompStr {

template<auto str>
constexpr const char *lfind(const char match,
  const char *start, const char *fallback)
{
  const char *end {*str + sizeof(*str) - 1};
  do { if (*start == match) return start + 1; } while(++start < end);
  return fallback;
}

template<auto str>
constexpr const char *rfind(const char match,
  const char *start, const char *fallback)
{
  const char *p {*str + sizeof(*str) - 1};
  do { if(*p == match) return p + 1; } while(--p > start);
  return fallback;
}

template<typename T>
constexpr char &append(char *&p, const T &str,
  const std::optional<char> sep = std::nullopt)
{
  for(size_t i {}; i < str.size(); ++i, ++p)
    *p = str[i];
  if(!sep)
    return *(p - 1);
  *p = *sep;
  return *p++;
}

}

#endif
