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

#ifndef REAIMGUI_CALLBACK_HPP
#define REAIMGUI_CALLBACK_HPP

#include "helper.hpp"

#include "../src/context.hpp"
#include "../src/error.hpp"
#include "../src/function.hpp"

template<typename Data>
class Callback {
public:
  struct DataAccess {
    DataAccess()  { assertValid(); loadVars(function()); }
    ~DataAccess() { storeVars(function()); }

    Data *operator->() const { return Callback<Data>::s_data; }
  };

  template<typename T = void>
  static T(*use(Function *func))(Data *)
  {
    if(!func)
      return nullptr;

    ::assertValid(func);
    return &invoke<T>;
  }

  template<typename T = void>
  static T invoke(Data *data)
  {
    SetData raii {data};
    storeVars(function());
    function()->execute();
    loadVars(function());

    // prevent accessing the context after it has been destructed
    // after handling an exception during execution of the callback
    // (exceptions cannot cross EEL's boundary so they're handled earlier)
    assertValid();

    if constexpr(!std::is_void_v<T>)
      return {};
  }

protected:
  static Data *s_data;
  static void storeVars(Function *);
  static void loadVars(const Function *);

private:
  struct SetData {
    SetData(Data *data) { s_data = data; }
    ~SetData() { s_data = nullptr; }
  };

  static Function *function()
    { return static_cast<Function *>(s_data->UserData); }
  static void assertValid()
  {
    if(!s_data)
      throw reascript_error {"cannot be used outside of a callback"};
    if(API::lastError())
      throw reascript_error {"an error occurred during callback execution"};
  }
};

template<typename Data>
Data *Callback<Data>::s_data;

#endif
