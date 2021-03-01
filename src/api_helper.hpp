#ifndef REAIMGUI_API_HELPER_HPP
#define REAIMGUI_API_HELPER_HPP

#include "api.hpp"
#include "context.hpp"

#include <boost/preprocessor.hpp>
#include <cstring> // strlen

using ImGui_Context = Context; // user-facing alias

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
    BOOST_PP_VARIADIC_SEQ_TO_SEQ(args)))                        \
  try __VA_ARGS__                                               \
  API_CATCH(name, type, reascript_error)                        \
  API_CATCH(name, type, imgui_error)                            \
                                                                \
  static API API_reg_##name { #name,                            \
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
// Not using varInOutOptional because REAPER refuses to give them as null
#define API_RWO(var)      var##InOptional // read/write, optional/nullable
#define API_RWBIG(var)    var##InOutNeedBig    // read/write, resizable (realloc_cmd_ptr)
#define API_RWBIG_SZ(var) var##InOutNeedBig_sz // size of previous API_RWBIG buffer
#define API_WBIG(var)     var##OutNeedBig
#define API_WBIG_SZ(var)  var##OutNeedBig_sz

template<typename T, typename Y>
inline T valueOr(const T *ptr, const Y fallback)
{
  return ptr ? *ptr : fallback;
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

  char message[255];
  snprintf(message, sizeof(message), "expected valid %s*, got %p",
    T::api_type_name, ptr);
  throw reascript_error { message };
}

#define FRAME_GUARD assertValid(ctx); ctx->enterFrame()

#endif
