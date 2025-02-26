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

#ifndef REAIMGUI_SHIMS_HPP
#define REAIMGUI_SHIMS_HPP

#include "../api/helper.hpp"

#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/tuple/pop_front.hpp>

#define _SHIM_EACH_IMPORT(r, data, i, import)                     \
  SHIM_IMPORT(                                                    \
    BOOST_PP_TUPLE_ELEM(0, import),                               \
    BOOST_PP_TUPLE_ELEM(1, import),                               \
    BOOST_PP_TUPLE_REM_CTOR(BOOST_PP_IF(                          \
      BOOST_PP_GREATER(BOOST_PP_TUPLE_SIZE(import), 2),           \
      BOOST_PP_TUPLE_POP_FRONT(BOOST_PP_TUPLE_POP_FRONT(import)), \
      (void)                                                      \
    ))                                                            \
  )

#define SHIM_IMPORT(type, name, ...) \
  type(*name)(__VA_ARGS__) = reinterpret_cast<type(*)(__VA_ARGS__)>( \
    const_cast<char *>(BOOST_PP_STRINGIZE(name)));

#define SHIM(vernum, imports) namespace {           \
  constexpr VerNum api_version {vernum};            \
  struct ImportTable : API::ImportTable {           \
    ImportTable() : API::ImportTable {              \
      api_version, sizeof(*this) } {}               \
    _API_FOREACH_ARG(_SHIM_EACH_IMPORT, _, imports) \
  }; } static ImportTable api

#define SHIM_FUNC(vernum, type, name, args)                     \
  _API_FUNC_DECL(vernum, type, name, args, API_DO_NOT_USE)      \
  _API_EXPORT(ShimFunc, version, name) {                        \
    API::v##vernum::name::meta::version, api_version, #name,    \
    _API_DEF(vernum, name, false),                              \
    reinterpret_cast<void *>(_API_SAFECALL(vernum, name)),      \
    reinterpret_cast<void *>(                                   \
      CallConv::ReaScript<_API_SAFECALL(vernum, name)>::apply), \
    reinterpret_cast<void *>(&API::v##vernum::name::impl)       \
  };                                                            \
  _API_FUNC_DEF(vernum, type, name, args)

#define SHIM_CONST(version, name, value) \
  SHIM_FUNC(version, int, name, API_NO_ARGS) { return value; }

#define _SHIM_EXPORT(vernum, name, func)                       \
  namespace API::v##vernum::name {                             \
    struct meta {                                              \
      static constexpr char na##me[] = #name, vn[] = #vernum;  \
      static constexpr std::string_view help {API_DO_NOT_USE}; \
      static constexpr VerNum version {CompStr::version<&vn>}; \
    };                                                         \
  }                                                            \
  _API_EXPORT(ShimFunc, vernum, name) {                        \
    API::v##vernum::name::meta::version, api_version, #name,   \
    CompStr::apidef<func, API::v##vernum::name::meta, false>,  \
    reinterpret_cast<void *>(CallConv::Safe<                   \
      func, API::v##vernum::name::meta>::invoke),              \
    reinterpret_cast<void *>(                                  \
      CallConv::ReaScript<CallConv::Safe<                      \
        func, API::v##vernum::name::meta>::invoke>::apply),    \
    reinterpret_cast<void *>(func),                            \
  };

#define _SHIM_WRAP(func) ShimWrap<&api, &decltype(api)::func>
#define _SHIM_PROXY(Proxy, name) \
  _SHIM_WRAP(name)::proxy<Proxy<&decltype(api)::name>>
#define SHIM_PROXY(version, name, Proxy) \
  _SHIM_EXPORT(version, name, &_SHIM_PROXY(Proxy, name))
// #define SHIM_PROXY(version, name, Proxy, func) // BOOST_PP_OVERLOAD?
//   _SHIM_EXPORT(version, name, &_SHIM_PROXY(Proxy, func))
#define SHIM_ALIAS(version, name, func) \
  _SHIM_EXPORT(version, name, &_SHIM_WRAP(func)::alias)

template<auto ftable, auto fptr>
struct ShimWrap;

template<typename R, typename... Args, auto ftable,
  R (*std::remove_reference_t<decltype(*ftable)>::*fptr)(Args...)>
struct ShimWrap<ftable, fptr>
{
  static R alias(Args... args) { return (ftable->*fptr)(args...); }

  template<typename Proxy>
  static R proxy(Args... args)
  {
    return Proxy::apply(std::make_tuple(args...));
  }
};

#define SHIM_PROXY_BEGIN(name, fptr, args) \
  namespace { template<auto fptr> struct name { \
  template<typename Args> static auto apply(Args &&args) {
#define SHIM_PROXY_END() } }; }

#endif
