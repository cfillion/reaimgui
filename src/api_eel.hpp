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

#ifndef REAIMGUI_API_EEL
#define REAIMGUI_API_EEL

#include "api.hpp"

#include "function.hpp"

#define NSEEL_ADDFUNC_DESTINATION API::eelFunctionTable()
#include <eel2_import.hpp>
#include <tuple>

namespace API {
  eel_function_table *eelFunctionTable();

  class EELFunc : public Symbol {
  public:
    using VarArgFunc = EEL_F (NSEEL_CGEN_CALL *)(void *, INT_PTR, EEL_F **);

    EELFunc(const char *name, const char *definition, VarArgFunc impl, int argc);
    void announce(bool) const override;

    const char *name() const override { return m_name; }
    const char *definition() const override { return m_definition; }
    unsigned int flags() const override { return TargetEEL; }

  private:
    const char *m_name, *m_definition;
    VarArgFunc m_impl;
    int m_argc;
  };

  class EELVar : public Symbol {
  public:
    EELVar(const char *name, const char *help);
    void announce(bool) const override {}

    const char *name() const override { return m_name; }
    const char *definition() const override { return m_definition; }
    unsigned int flags() const override { return TargetEEL | Variable; }

    operator const char *() const { return m_name; }

  private:
    const char *m_name, *m_definition;
  };
};

template<typename T>
T fetchEELArgument(const Function *, EEL_F value)
{
  return value;
}

template<>
inline std::string_view fetchEELArgument(const Function *func, EEL_F value)
{
  return func->getString(value).value_or("");
}

template<typename T>
struct EELAPI;

template<typename R, typename... Args>
struct EELAPI<R(*)(Args...) noexcept> {
  static constexpr size_t ARGC { sizeof...(Args) };

  static EEL_F applyVarArg(Function *func, R(*fn)(Args...), EEL_F **argv, size_t argc)
  {
    if(static_cast<size_t>(argc) < sizeof...(Args))
      return 0;

    const auto &args
      { makeTuple(func, argv, std::index_sequence_for<Args...>{}) };
    if constexpr(std::is_void_v<R>) {
      std::apply(fn, args);
      return 0;
    }
    else
      return std::apply(fn, args);
  }

private:
  template<size_t I>
  using NthType = typename std::tuple_element<I, std::tuple<Args...>>::type;

  template<size_t... I>
  static auto makeTuple([[maybe_unused]] const Function *self,
                        EEL_F **argv, std::index_sequence<I...>)
  {
    return std::make_tuple(fetchEELArgument<NthType<I>>(self, *(argv[I]))...);
  }
};

template<auto fn>
EEL_F NSEEL_CGEN_CALL InvokeEELAPI(void *self, INT_PTR argc, EEL_F **argv)
{
  Function *func { static_cast<Function *>(self) };
  return EELAPI<decltype(fn)>::applyVarArg(func, fn, argv, argc);
}

#endif
