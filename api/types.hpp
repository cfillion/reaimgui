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

#ifndef REAIMGUI_TYPES_HPP
#define REAIMGUI_TYPES_HPP

#include "compstr_utils.hpp"

#include <array>
#include <climits> // CHAR_BIT
#include <string_view>

/* ┌─┬─┬─┬─┬─┐
   │R│ │ │O│ │ varInOptional    - use nullIfEmpty on strings for < 6.58
   │R│W│ │ │ │ varInOut
   │R│W│ │O│ │ varInOutOptional - for documentation/python only
   │ │W│ │ │ │ varOut
   │ │W│ │ │S│ varOut_sz        - string buffer size
   │R│W│B│ │ │ varInOutNeedBig  - resizable and binary safe, use copyToBigBuf
   │R│W│B│ │S│ varInOutNeedBig_sz
   │ │W│B│ │ │ varOutNeedBig
   │ │W│B│ │S│ varOutNeedBig_sz
   │ │ │ │ │S│ var_sz           - binary-safe string in REAPER >= 6.44
   └─┴─┴─┴─┴─┘ */

namespace Tags {
  enum {
    // bit order = appended suffix order
    R = 1<<0,
    W = 1<<1,
    B = 1<<2,
    O = 1<<3,
    S = 1<<4,
  };
}

template<typename T, unsigned char TagV>
class Tag {
public:
  Tag(T v) : m_val {v} {}
  operator T &() { return m_val; }
  operator const T &() const { return m_val; }

private:
  T m_val;
};

#define TAG_ALIAS(name, tags) template<typename T> using name = Tag<T, tags>
TAG_ALIAS(RO,   Tags::O | Tags::R);
TAG_ALIAS(RW,   Tags::R | Tags::W);
TAG_ALIAS(RWO,  Tags::R | Tags::W | Tags::O);
TAG_ALIAS(W,    Tags::W);
TAG_ALIAS(WS,   Tags::W | Tags::S);
TAG_ALIAS(WB,   Tags::W | Tags::B);
TAG_ALIAS(WBS,  Tags::W | Tags::B | Tags::S);
TAG_ALIAS(RWB,  Tags::R | Tags::W | Tags::B);
TAG_ALIAS(RWBS, Tags::R | Tags::W | Tags::B | Tags::S);
TAG_ALIAS(S,    Tags::S);
#undef TAG_ALIAS

template<typename T, typename = void>
struct TypeInfo;

template<typename T>
struct TypeInfo<T, typename std::enable_if_t<std::is_pointer_v<T>>>
{
  using underlying = std::remove_pointer_t<T>;
  static constexpr auto type()
  {
    constexpr auto name {TypeInfo<underlying>::type()};
    std::array<char, name.size() + 1> out {};
    char *p {out.data()};
    CompStr::append(p, name, '*');
    return out;
  }

  template<const auto &Names, size_t I>
  static constexpr auto name()
  {
    return TypeInfo<underlying>::template name<Names, I>();
  }
};

template<typename T, auto tags>
struct TypeInfo<Tag<T, tags>>
{
  static constexpr auto type() { return TypeInfo<T>::type(); }

  template<const auto &Names, size_t I>
  static constexpr auto name()
  {
    constexpr auto resolvedName {[] {
      constexpr auto name {TypeInfo<T>::template name<Names, I>()};

      // do not append _sz again when Names[I] already ends with it
      if constexpr(!!(tags & Tags::S)) {
        constexpr auto sz {suffixFor(Tags::S)};
        if constexpr(name.size() > sz.size() &&
            name.substr(name.size() - sz.size()) == sz)
          return name.substr(0, name.size() - sz.size());
        else
          return name;
      }
      else
        return name;
    }()};

    constexpr size_t length {resolvedName.size() + suffixesLength()};
    std::array<char, length> decorated {};
    char *p {decorated.data()};
    CompStr::append(p, resolvedName);
    for(size_t bit {}; bit < sizeof(tags) * CHAR_BIT; ++bit)
      CompStr::append(p, suffixFor(tags & (1 << bit)));
    return decorated;
  }

private:
  static constexpr std::string_view suffixFor(decltype(tags) tag)
  {
    switch(tag) {
    case Tags::R: return "In";
    case Tags::W: return "Out";
    case Tags::B: return "NeedBig";
    case Tags::O: return "Optional";
    case Tags::S: return "_sz";
    default:      return "";
    }
  }

  static constexpr size_t suffixesLength()
  {
    size_t prefixes {};
    for(size_t bit {}; bit < sizeof(tags) * CHAR_BIT; ++bit)
      prefixes += suffixFor(tags & (1 << bit)).size();
    return prefixes;
  }
};

#define API_REGISTER_TYPE(T, N)   \
  template<> struct TypeInfo<T> { \
    static constexpr std::string_view type() { return N; } \
                                                           \
    template<const auto &Names, size_t I>                  \
    static constexpr auto name() { return Names[I]; }      \
  }

#define API_REGISTER_BASIC_TYPE(T)  API_REGISTER_TYPE(T, #T)
#define API_REGISTER_OBJECT_TYPE(T) API_REGISTER_TYPE(T*, "ImGui_" #T "*")

API_REGISTER_BASIC_TYPE(bool);
API_REGISTER_BASIC_TYPE(char*);
API_REGISTER_BASIC_TYPE(const char*);
API_REGISTER_BASIC_TYPE(double);
API_REGISTER_BASIC_TYPE(int);
API_REGISTER_BASIC_TYPE(void);
API_REGISTER_TYPE(std::string_view, "const char*"); // EEL strings

// https://forum.cockos.com/showthread.php?t=211620
struct reaper_array {
  const unsigned int size, alloc;
  double data[1];
};
API_REGISTER_TYPE(reaper_array*, "reaper_array*");

#endif
