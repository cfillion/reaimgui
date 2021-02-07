#ifndef REAIMGUI_API_HELPER_HPP
#define REAIMGUI_API_HELPER_HPP

#include "api.hpp"

#include <boost/preprocessor.hpp>
#include <tuple>

template<typename T>
struct ReaScriptAPI;

template<typename R, typename... Args>
struct ReaScriptAPI<R(*)(Args...)>
{
  static void *applyVarArg(R(*fn)(Args...), void **argv, int argc)
  {
    if(static_cast<size_t>(argc) < sizeof...(Args))
      return nullptr;

    const auto &args { makeTuple(argv, std::index_sequence_for<Args...>{}) };

    if constexpr (std::is_void_v<R>) {
      std::apply(fn, args);
      return nullptr;
    }
    else if constexpr (std::is_floating_point_v<R>) {
      const auto value { std::apply(fn, args) };
      void *storage { argv[argc - 1] };
      *static_cast<double *>(storage) = value;
      return storage;
    }
    else {
      // cast numbers to have the same size as a pointer to avoid warnings
      using IntPtrR = std::conditional_t<std::is_pointer_v<R>, R, intptr_t>;
      const auto value { static_cast<IntPtrR>(std::apply(fn, args)) };
      return reinterpret_cast<void *>(value);
    }
  }

private:
  template<size_t I>
  using NthType = typename std::tuple_element<I, std::tuple<Args...>>::type;

  template<size_t... I>
  static auto makeTuple(void **argv, std::index_sequence<I...>)
  {
    // C++17 is amazing
    return std::make_tuple(
      std::is_floating_point_v<NthType<I>> ?
        *reinterpret_cast<NthType<I>*>(argv[I]) :
        (NthType<I>)reinterpret_cast<intptr_t>(argv[I])
      ...
    );
  }
};

template<auto fn>
void *InvokeReaScriptAPI(void **argv, int argc)
{
  return ReaScriptAPI<decltype(fn)>::applyVarArg(fn, argv, argc);
}

// int* fooInOptional -> can be null, input and output (but not visible in the Lua return list)
// int* fooInOutOptional -> cannot be null, input and output (visible in the Lua return list)

#define ARG_TYPE(arg) BOOST_PP_TUPLE_ELEM(2, 0, arg)
#define ARG_NAME(arg) BOOST_PP_TUPLE_ELEM(2, 1, arg)

#define DEFARGS(r, data, i, arg) BOOST_PP_COMMA_IF(i) ARG_TYPE(arg) ARG_NAME(arg)
#define DOCARGS(r, macro, i, arg) \
  BOOST_PP_EXPR_IF(i, ",") BOOST_PP_STRINGIZE(macro(arg))

#define DEFINE_API(type, name, args, help, ...)                                 \
  static type API_##name(BOOST_PP_SEQ_FOR_EACH_I(DEFARGS, _, args)) __VA_ARGS__ \
                                                                                \
  static API API_reg_##name { #name,                                            \
    reinterpret_cast<void *>(&API_##name),                                      \
    reinterpret_cast<void *>(&InvokeReaScriptAPI<&API_##name>),                 \
    reinterpret_cast<void *>(const_cast<char *>(                                \
      #type "\0"                                                                \
      BOOST_PP_SEQ_FOR_EACH_I(DOCARGS, ARG_TYPE, args) "\0"                     \
      BOOST_PP_SEQ_FOR_EACH_I(DOCARGS, ARG_NAME, args) "\0"                     \
      help                                                                      \
    ))                                                                          \
  }

#include "window.hpp"
#include <reaper_plugin_functions.h>

#define CHECK_WINDOW(win, ...)                            \
  if(!Window::exists(win)) {                              \
    ReaScriptError("ReaImGui: Invalid window reference"); \
    return __VA_ARGS__;                                   \
  }

#define USE_WINDOW(win, ...) \
  CHECK_WINDOW(win, __VA_ARGS__);    \
  window->enterFrame();

// https://forum.cockos.com/showthread.php?t=211620
struct reaper_array {
  unsigned int size, alloc;
  double data[];
};

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

#endif
