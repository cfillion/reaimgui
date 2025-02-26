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

#ifndef REAIMGUI_API_HELPER_HPP
#define REAIMGUI_API_HELPER_HPP

// prevent Windows.h from defining unwanted macros (eg. CreateFont)
#ifdef _WIN32
#  define NOGDI
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
DECLARE_HANDLE(HDROP);
#endif

#include "callconv.hpp"
#include "compstr.hpp"

#include "../src/api.hpp"
#include "../src/context.hpp"

#include <array>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/greater_equal.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <cstring> // memcpy
#include <type_traits>

#define _API_ARG_TYPE(arg) BOOST_PP_TUPLE_ELEM(0, arg)
#define _API_ARG_NAME(arg) BOOST_PP_TUPLE_ELEM(1, arg)
#define _API_ARG_DEFV(arg) BOOST_PP_TUPLE_ELEM(2, arg)

#define _API_FOREACH_ARG(macro, data, args) \
  BOOST_PP_SEQ_FOR_EACH_I(macro, data, BOOST_PP_VARIADIC_SEQ_TO_SEQ(args))
#define _API_SIGARG(r, data, i, arg) \
  BOOST_PP_COMMA_IF(i) _API_ARG_TYPE(arg) _API_ARG_NAME(arg)
#define _API_STRARR(r, macro, i, arg) \
  BOOST_PP_COMMA_IF(i) BOOST_PP_STRINGIZE(macro(arg))
#define _API_STRARR_US(r, macro, i, arg) \
  BOOST_PP_EXPR_IF(i, "\31") BOOST_PP_STRINGIZE(macro(arg))

template<typename T, typename = void>
struct DefVal { using type = std::remove_pointer_t<T>; };

template<>
struct DefVal<const char *> { using type = const char *; };

template<typename T, auto tags>
struct DefVal<Tag<T, tags>> { using type = typename DefVal<T>::type; };

#define _API_DEFARG_ID(argName) BOOST_PP_CAT(argName, Default)
#define _API_DEFARG(r, name, i, arg)                          \
  BOOST_PP_EXPR_IF(                                           \
    BOOST_PP_GREATER_EQUAL(BOOST_PP_TUPLE_SIZE(arg), 3),      \
    constexpr DefVal<_API_ARG_TYPE(arg)>::type                \
      _API_ARG_NAME(arg)(_API_ARG_DEFV(arg)); \
  )

// error out if API_SECTION() was not used in the file
#define _API_CHECKROOTSECTION static_assert(&ROOT_SECTION + 1 > &ROOT_SECTION);

#define _API_STORE_LINE \
  static const API::StoreLineNumber BOOST_PP_CAT(line, __LINE__) {__LINE__};

#define _API_FUNC_DECL(vernum, type, name, args, help)                 \
  namespace API::v##vernum::name {                                     \
    namespace defaults {                                               \
      _API_FOREACH_ARG(_API_DEFARG, name, args)                        \
    }                                                                  \
    static type impl(_API_FOREACH_ARG(_API_SIGARG, _, args));          \
    struct meta {                                                      \
      static constexpr char na##me[] = #name, vn[] = #vernum;          \
      static constexpr std::string_view he##lp {[] {                   \
        using namespace std::string_view_literals;                     \
        return help "\0"                                               \
          _API_FOREACH_ARG(_API_STRARR_US, _API_ARG_DEFV, args) ""sv;  \
      }()};                                                            \
      static constexpr VerNum version {CompStr::version<&vn>};         \
      static constexpr std::array<std::string_view,                    \
        BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_SEQ_TO_SEQ(args))> argn    \
        {_API_FOREACH_ARG(_API_STRARR, _API_ARG_NAME, args)};          \
    };                                                                 \
  }

#define _API_FUNC_DEF(vernum, type, name, args) \
  type API::v##vernum::name::impl(_API_FOREACH_ARG(_API_SIGARG, _, args))

// extern for link-time duplicate detection
// must NOT be const: Callable constructor of other functions may write to us
#define _API_EXPORT(type, vernum, name) \
  namespace API::v##vernum::name { extern API::type symbol; } \
  API::type API::v##vernum::name::symbol

#define _API_SAFECALL(vernum, name) &CallConv::Safe< \
  &API::v##vernum::name::impl, API::v##vernum::name::meta>::invoke

#define _API_DEF(vernum, name, named) \
  CompStr::apidef<&API::v##vernum::name::impl, API::v##vernum::name::meta, named>

#define API_FUNC _API_STORE_LINE _API_FUNC
#define _API_FUNC(vernum, type, name, args, help)                \
  _API_CHECKROOTSECTION                                          \
  _API_FUNC_DECL(vernum, type, name, args, help)                 \
  _API_EXPORT(ReaScriptFunc, vernum, name) {                     \
    API::v##vernum::name::meta::version,                         \
    reinterpret_cast<void *>(&API::v##vernum::name::impl),       \
    {"-API_" API_PREFIX #name, _API_SAFECALL(vernum, name)},     \
    {"-APIvararg_" API_PREFIX #name,                             \
      CallConv::ReaScript<_API_SAFECALL(vernum, name)>::apply},  \
    {"-APIdef_" API_PREFIX #name, _API_DEF(vernum, name, true)}, \
  };                                                             \
  _API_FUNC_DEF(vernum, type, name, args)

#define API_ENUM _API_STORE_LINE _API_ENUM
#define _API_ENUM(vernum, prefix, name, doc) \
  _API_FUNC(vernum, int, name, API_NO_ARGS, doc) { return prefix##name; }

#define API_EELFUNC _API_STORE_LINE _API_EELFUNC
#define _API_EELFUNC(vernum, type, name, args, help)    \
  _API_CHECKROOTSECTION                                 \
  _API_FUNC_DECL(vernum, type, name, args, help)        \
  _API_EXPORT(EELFunc, vernum, name) {                  \
    API::v##vernum::name::meta::version,                \
    #name, _API_DEF(vernum, name, true),                \
    &CallConv::EEL<_API_SAFECALL(vernum, name)>::apply, \
     CallConv::EEL<_API_SAFECALL(vernum, name)>::ARGC,  \
  };                                                    \
  _API_FUNC_DEF(vernum, type, name, args)

#define API_EELVAR _API_STORE_LINE _API_EELVAR
#define _API_EELVAR(vernum, type, name, help)           \
  _API_CHECKROOTSECTION                                 \
  namespace API::v##vernum::EELVar_##name {             \
    constexpr char vn[] {#vernum};                      \
    constexpr VerNum version {CompStr::version<&vn>};   \
  }                                                     \
  const API::EELVar EELVar_##name {                     \
    API::v##vernum::EELVar_##name::version, #name, #type "\0\0\0" help "\0" }

#define API_SECTION_DEF(id, parent, ...) static const API::Section id \
  {&parent, ROOT_FILE, __VA_ARGS__};

// shortcuts with auto-generated identifier name for the section object
#define _API_UNIQ_SEC_ID BOOST_PP_CAT(section, __LINE__)
#define API_SECTION(...)                                    \
  constexpr char FILE_PATH[] {__FILE__};                    \
  constexpr auto ROOT_FILE {CompStr::basename<&FILE_PATH>}; \
  static const API::Section ROOT_SECTION                    \
    {nullptr, ROOT_FILE, __VA_ARGS__}
#define API_SUBSECTION(...) \
  API_SECTION_DEF(_API_UNIQ_SEC_ID, ROOT_SECTION, __VA_ARGS__)
#define API_SECTION_P(parent, ...) \
  API_SECTION_DEF(_API_UNIQ_SEC_ID, parent,       __VA_ARGS__)

#define API_DO_NOT_USE "Internal use only."
#define API_NO_ARGS

#define API_GET(var) [](const auto &v, const auto &d) -> const auto & { \
  if constexpr(std::is_pointer_v<std::remove_reference_t<decltype(d)>>) \
    return v ? static_cast<decltype(d)>(v) : d; \
  else                                          \
    return v ? *v : d;                          \
}(var, defaults::var)

// const char *foobarInOptional from REAPER are never null before 6.58
inline void nullIfEmpty(const char *&string)
{
  extern const char *(*GetAppVersion)();
  static bool hasNullableStrings {atof(GetAppVersion()) >= 6.58};
  if(!hasNullableStrings && string && !string[0] /* empty */)
    string = nullptr;
}

template<typename T>
void assertValid(T *ptr)
{
  if constexpr(std::is_base_of_v<Resource, T>) {
    if(Resource::isValid(ptr)) {
      ptr->keepAlive();
      return;
    }
  }
  else if(ptr)
    return;

  Error::invalidObject(ptr);
}

template<typename T, auto tags>
void assertValid(Tag<T, tags> ptr)
{
  assertValid(static_cast<T>(ptr));
}

inline void assertFrame(Context *ctx)
{
  if(!ctx->enterFrame()) {
    delete ctx;
    throw reascript_error {"frame initialization failed"};
  }
}

#define FRAME_GUARD assertValid(ctx); assertFrame(ctx)

template <typename PtrType, typename ValType, size_t N>
class ReadWriteArray {
public:
  template<typename... Args,
    typename = typename std::enable_if_t<sizeof...(Args) == N>>
  ReadWriteArray(Args&&... args)
    : m_inputs {std::forward<Args>(args)...}
  {
    size_t i {0};
    for(const PtrType *ptr : m_inputs) {
      assertValid(ptr);
      m_values[i++] = *ptr;
    }
  }

  size_t size() const { return N; }
  ValType *data() { return m_values.data(); }
  ValType &operator[](const size_t i) { return m_values[i]; }

  bool commit()
  {
    size_t i {0};
    for(const ValType value : m_values)
      *m_inputs[i++] = value;
    return true;
  }

private:
  std::array<PtrType*, N> m_inputs;
  std::array<ValType,  N> m_values;
};

// EEL may pass a null buffer, make sure it's valid before calling these
inline void copyToBigBuf(char *&buf, int &bufSize,
  const void *data, size_t dataSize, const bool mayHaveNulls = true)
{
  extern bool (*realloc_cmd_ptr)(char **ptr, int *ptr_size, int new_size);
  const bool wantRealloc {mayHaveNulls || dataSize >= static_cast<size_t>(bufSize)};
  if(wantRealloc && dataSize > 0 && dataSize < INT_MAX &&
      realloc_cmd_ptr(&buf, &bufSize, dataSize)) {
    // the buffer is no longer null-terminated after using realloc_cmd_ptr!
    std::memcpy(buf, data, bufSize);
  }
  else if(bufSize > 1) {
    const size_t limit {std::min<size_t>(bufSize - 1, dataSize)};
    std::memcpy(buf, data, limit);
    buf[limit] = '\0';
  }
}

template<typename T, typename = std::enable_if_t<std::is_class_v<T>>>
auto copyToBigBuf(char *&buf, int &bufSize, const T &value, const bool mayHaveNulls = true)
{
  const size_t byteSize {value.size() * sizeof(typename T::value_type)};
  return copyToBigBuf(buf, bufSize, value.data(), byteSize, mayHaveNulls);
}

// Common behavior for p_open throughout the API.
// When false, set output to true to signal it's open to the caller, but give
// NULL to Dear ImGui to signify not closable.
inline bool *openPtrBehavior(bool *p_open)
{
  if(p_open && !*p_open) {
    *p_open = true;
    return nullptr;
  }

  return p_open;
}

#endif
