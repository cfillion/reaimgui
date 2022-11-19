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

#ifndef REAIMGUI_VARIANT_HPP
#define REAIMGUI_VARIANT_HPP

#ifdef HAS_STD_VARIANT
#  include <variant>
#else
  // Xcode 9 for macOS 32-bit builds
#  include <boost/variant/apply_visitor.hpp>
#  include <boost/variant/get.hpp>
#  include <boost/variant/variant.hpp>
  namespace std {
    template<typename... T>
    using variant = boost::variant<T...>;

    template<typename Visitor, typename Variant>
    auto visit(const Visitor &visitor, Variant &&operand)
    { return boost::apply_visitor(visitor, operand); }

    template<typename T, typename... V>
    auto &get(std::variant<V...> &v) { return boost::get<T>(v); }

    template<typename T, typename... V>
    auto get_if(const std::variant<V...> *v) { return boost::get<T>(v); }
  }
#endif

#endif
