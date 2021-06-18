/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021  Christian Fillion
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

#include "api.hpp"
#include "api_vararg.hpp"
#include "context.hpp"

#include <boost/preprocessor.hpp>
#include <boost/type_index.hpp>
#include <cstring> // strlen

// user-facing aliases
class Font;
using ImGui_Context = Context;
using ImGui_Font = Font;

#define ARG_TYPE(arg) BOOST_PP_TUPLE_ELEM(2, 0, arg)
#define ARG_NAME(arg) BOOST_PP_TUPLE_ELEM(2, 1, arg)

#define DEFARGS(r, data, i, arg) BOOST_PP_COMMA_IF(i) ARG_TYPE(arg) ARG_NAME(arg)
#define DOCARGS(r, macro, i, arg) \
  BOOST_PP_EXPR_IF(i, ",") BOOST_PP_STRINGIZE(macro(arg))

#define API_CATCH(name, type, except) \
  catch(const except &e) {            \
    API::handleError(#name, e);       \
    return static_cast<type>(0);      \
  }

#define DEFINE_API(type, name, args, help, ...)                 \
  type API_##name(BOOST_PP_SEQ_FOR_EACH_I(DEFARGS, _,           \
    BOOST_PP_VARIADIC_SEQ_TO_SEQ(args))) noexcept               \
  try __VA_ARGS__                                               \
  API_CATCH(name, type, reascript_error)                        \
  API_CATCH(name, type, imgui_error)                            \
                                                                \
  static const API API_reg_##name { #name,                      \
    reinterpret_cast<void *>(&API_##name),                      \
    reinterpret_cast<void *>(&InvokeReaScriptAPI<&API_##name>), \
    reinterpret_cast<void *>(const_cast<char *>(                \
      #type "\0"                                                \
      BOOST_PP_SEQ_FOR_EACH_I(DOCARGS, ARG_TYPE,                \
        BOOST_PP_VARIADIC_SEQ_TO_SEQ(args)) "\0"                \
      BOOST_PP_SEQ_FOR_EACH_I(DOCARGS, ARG_NAME,                \
        BOOST_PP_VARIADIC_SEQ_TO_SEQ(args)) "\0"                \
      help                                                      \
    ))                                                          \
  }

#define NO_ARGS (,)

#define API_RO(var)       var##InOptional // read, optional/nullable (except string, use nullIfEmpty)
#define API_RW(var)       var##InOut      // read/write
#define API_W(var)        var##Out        // write
#define API_W_SZ(var)     var##Out_sz     // write
// Not using varInOutOptional because REAPER refuses to pass NULL to them
#define API_RWBIG(var)    var##InOutNeedBig    // read/write, resizable (realloc_cmd_ptr)
#define API_RWBIG_SZ(var) var##InOutNeedBig_sz // size of previous API_RWBIG buffer
#define API_WBIG(var)     var##OutNeedBig
#define API_WBIG_SZ(var)  var##OutNeedBig_sz

#define FRAME_GUARD assertValid(ctx); ctx->enterFrame()

template<typename Output, typename Input>
inline Output valueOr(const Input *ptr, const Output fallback)
{
  return ptr ? static_cast<Output>(*ptr) : fallback;
}

// const char *foobarInOptional from REAPER is never null no matter what
inline void nullIfEmpty(const char *&string)
{
  if(string && !strlen(string))
    string = nullptr;
}

template<typename T>
inline void assertValid(T *ptr)
{
  if constexpr (std::is_base_of_v<Resource, T>) {
    if(Resource::exists(ptr))
      return;
  }
  else if(ptr)
    return;

  std::string type;
  if constexpr (std::is_class_v<T>)
    type = T::api_type_name;
  else
    type = boost::typeindex::type_id<T>().pretty_name();

  char message[255];
  snprintf(message, sizeof(message),
    "expected valid %s*, got %p", type.c_str(), ptr);
  throw reascript_error { message };
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
