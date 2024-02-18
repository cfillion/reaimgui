/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2024  Christian Fillion
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
#include <boost/type_index.hpp>
#include <type_traits>

#define _API_ARG_TYPE(arg) BOOST_PP_TUPLE_ELEM(0, arg)
#define _API_ARG_NAME(arg) BOOST_PP_TUPLE_ELEM(1, arg)
#define _API_ARG_DEFV(arg) BOOST_PP_TUPLE_ELEM(2, arg)
#define _API_ARG_DEFV_T(arg) decltype(_API_ARG_DEFV(arg))

#define _API_FOREACH_ARG(macro, data, args) \
  BOOST_PP_SEQ_FOR_EACH_I(macro, data, BOOST_PP_VARIADIC_SEQ_TO_SEQ(args))
#define _API_SIGARG(r, data, i, arg) \
  BOOST_PP_COMMA_IF(i) _API_ARG_TYPE(arg) _API_ARG_NAME(arg)
#define _API_STRARG(r, macro, i, arg) \
  BOOST_PP_EXPR_IF(i, ",") BOOST_PP_STRINGIZE(macro(arg))
#define _API_STRARGUS(r, macro, i, arg) \
  BOOST_PP_EXPR_IF(i, "\31") BOOST_PP_STRINGIZE(macro(arg))

template<typename T>
using DefArgVal = std::conditional_t<
  std::is_same_v<const char *, T>, T, std::remove_pointer_t<T>
>;

#define _API_DEFARG_ID(argName) BOOST_PP_CAT(argName, Default)
#define _API_DEFARG(r, name, i, arg)                          \
  BOOST_PP_EXPR_IF(                                           \
    BOOST_PP_GREATER_EQUAL(BOOST_PP_TUPLE_SIZE(arg), 3),      \
    constexpr DefArgVal<_API_ARG_TYPE(arg)>                   \
      _API_DEFARG_ID(_API_ARG_NAME(arg))(_API_ARG_DEFV(arg)); \
  )

// error out if API_SECTION() was not used in the file
#define _API_CHECKROOTSECTION static_assert(&ROOT_SECTION + 1 > &ROOT_SECTION);

#define _API_DEF(type, args, help)            \
  #type                                  "\0" \
  _API_FOREACH_ARG(_API_STRARG, _API_ARG_TYPE, args) "\0" \
  _API_FOREACH_ARG(_API_STRARG, _API_ARG_NAME, args) "\0" \
  help                                   "\0" \
  _API_FOREACH_ARG(_API_STRARGUS, _API_ARG_DEFV, args)

#define _API_STORE_LINE \
  static const API::StoreLineNumber BOOST_PP_CAT(line, __LINE__) { __LINE__ };

#define _API_FUNC_DECL(vernum, type, name, args)                       \
  namespace API::v##vernum::name {                                     \
    _API_FOREACH_ARG(_API_DEFARG, name, args) /* default arg values */ \
    constexpr const char id[] { #name   };                             \
    constexpr const char vn[] { #vernum };                             \
    constexpr VerNum version  { CompStr::version<&vn> };               \
    static type impl(_API_FOREACH_ARG(_API_SIGARG, _, args));          \
  }

#define _API_FUNC_DEF(vernum, type, name, args) \
  type API::v##vernum::name::impl(_API_FOREACH_ARG(_API_SIGARG, _, args))

// extern for link-time duplicate detection
#define _API_EXPORT(type, vernum, name) \
  namespace API::v##vernum::name { extern const API::type symbol; } \
  const API::type API::v##vernum::name::symbol

#define _API_SAFECALL(vernum, apiName) &CallConv::Safe< \
  &API::v##vernum::apiName::impl, &API::v##vernum::apiName::id>::invoke

#define API_FUNC _API_STORE_LINE _API_FUNC
#define _API_FUNC(type, name, args, help)                        \
  _API_CHECKROOTSECTION                                          \
  _API_FUNC_DECL(0_1, type, name, args)                          \
  _API_EXPORT(ReaScriptFunc, 0_1, name) {                        \
    API::v0_1::name::version,                                    \
    { "-API_"       BOOST_PP_STRINGIZE(API_PREFIX) #name,        \
      reinterpret_cast<void *>(_API_SAFECALL(0_1, name)),        \
    },                                                           \
    { "-APIvararg_" BOOST_PP_STRINGIZE(API_PREFIX) #name,        \
      reinterpret_cast<void *>(                                  \
        CallConv::ReaScript<_API_SAFECALL(0_1, name)>::apply),   \
    },                                                           \
    { "-APIdef_"    BOOST_PP_STRINGIZE(API_PREFIX) #name,        \
      reinterpret_cast<void *>(const_cast<char *>(               \
        _API_DEF(type, args, help))),                            \
    },                                                           \
  };                                                             \
  _API_FUNC_DEF(0_1, type, name, args)

#define API_ENUM _API_STORE_LINE _API_ENUM
#define _API_ENUM(prefix, name, doc) \
  _API_FUNC(int, name, NO_ARGS, doc) { return prefix##name; }

#define API_EELFUNC _API_STORE_LINE _API_EELFUNC
#define _API_EELFUNC(type, name, args, help)            \
  _API_CHECKROOTSECTION                                 \
  _API_FUNC_DECL(0, type, name, args)                   \
  _API_EXPORT(EELFunc, 0, name) {                       \
    API::v0::name::version,                             \
    #name, _API_DEF(type, args, help),                  \
    &CallConv::EEL<_API_SAFECALL(0, name)>::apply,      \
     CallConv::EEL<_API_SAFECALL(0, name)>::ARGC,       \
  };                                                    \
  _API_FUNC_DEF(0, type, name, args)

#define API_EELVAR _API_STORE_LINE _API_EELVAR
#define _API_EELVAR(type, name, help)                    \
  _API_CHECKROOTSECTION                                  \
  namespace API::v0::EELVar_##name {                     \
    constexpr const char vn[] { "0"     };               \
    constexpr VerNum version  { CompStr::version<&vn> }; \
  }                                                      \
  const API::EELVar EELVar_##name {                      \
    API::v0::EELVar_##name::version, #name, #type "\0\0\0" help "\0" }

#define API_SECTION_DEF(id, parent, ...) static const API::Section id \
  { &parent, ROOT_FILE, __VA_ARGS__ };

// shortcuts with auto-generated identifier name for the section object
#define _API_UNIQ_SEC_ID BOOST_PP_CAT(section, __LINE__)
#define API_SECTION(...)                                      \
  constexpr const char FILE_PATH[] { __FILE__ };              \
  constexpr auto ROOT_FILE { CompStr::basename<&FILE_PATH> }; \
  static const API::Section ROOT_SECTION                      \
    { nullptr, ROOT_FILE, __VA_ARGS__ }
#define API_SUBSECTION(...) \
  API_SECTION_DEF(_API_UNIQ_SEC_ID, ROOT_SECTION, __VA_ARGS__)
#define API_SECTION_P(parent, ...) \
  API_SECTION_DEF(_API_UNIQ_SEC_ID, parent,       __VA_ARGS__)

#define NO_ARGS (,)
#define API_RO(var)       var##InOptional // read, optional/nullable (except string, use nullIfEmpty)
#define API_RW(var)       var##InOut      // read/write
#define API_RWO(var)      var##InOutOptional // documentation/python only
#define API_W(var)        var##Out        // write
#define API_W_SZ(var)     var##Out_sz     // write
// Not using varInOutOptional because REAPER refuses to pass NULL to them
#define API_RWBIG(var)    var##InOutNeedBig    // read/write, resizable (realloc_cmd_ptr)
#define API_RWBIG_SZ(var) var##InOutNeedBig_sz // size of previous API_RWBIG buffer
#define API_WBIG(var)     var##OutNeedBig
#define API_WBIG_SZ(var)  var##OutNeedBig_sz

#define _API_GET(var) [](const auto v, const auto d) { \
  if constexpr(std::is_pointer_v<decltype(d)>)         \
    return v ?  v : d;                                 \
  else                                                 \
    return v ? *v : d;                                 \
}(var, _API_DEFARG_ID(var))
#define API_RO_GET(var)  _API_GET(API_RO(var))
#define API_RWO_GET(var) _API_GET(API_RWO(var))

#define FRAME_GUARD assertValid(ctx); assertFrame(ctx)

// const char *foobarInOptional from REAPER are never null before 6.58
inline void nullIfEmpty(const char *&string)
{
  extern const char *(*GetAppVersion)();
  static bool hasNullableStrings { atof(GetAppVersion()) >= 6.58 };
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

  std::string type;
  if constexpr(std::is_class_v<T>)
    type = T::api_type_name;
  else
    type = boost::typeindex::type_id<T>().pretty_name();

  char message[255];
  snprintf(message, sizeof(message),
    "expected a valid %s*, got %p", type.c_str(), ptr);
  throw reascript_error { message };
}

inline void assertFrame(Context *ctx)
{
  if(!ctx->enterFrame()) {
    delete ctx;
    throw reascript_error { "frame initialization failed" };
  }
}

template <typename PtrType, typename ValType, size_t N>
class ReadWriteArray {
public:
  template<typename... Args,
    typename = typename std::enable_if_t<sizeof...(Args) == N>>
  ReadWriteArray(Args&&... args)
    : m_inputs { std::forward<Args>(args)... }
  {
    size_t i { 0 };
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
    size_t i { 0 };
    for(const ValType value : m_values)
      *m_inputs[i++] = value;
    return true;
  }

private:
  std::array<PtrType*, N> m_inputs;
  std::array<ValType, N> m_values;
};

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
