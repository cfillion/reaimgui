/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2023  Christian Fillion
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

#ifndef REAIMGUI_VARIANT_HPP
#define REAIMGUI_VARIANT_HPP

#include <config.hpp>

// Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90943
// Fixed in GCC 11.3
#ifdef __GLIBCXX__ // Clang also defines __GNUC__
#  if __GNUC__ < 11 || (__GNUC__ == 11 && __GNUC_MINOR__ < 3)
#    undef HAVE_STD_VARIANT
#  endif
#endif

#ifdef HAVE_STD_VARIANT
#  include <variant>
#else
  // Xcode 9 for macOS 32-bit builds
#  include <boost/variant2/variant.hpp>
  namespace std {
    using namespace boost::variant2;
    using boost::variant2::get;
  }
#endif

#endif
