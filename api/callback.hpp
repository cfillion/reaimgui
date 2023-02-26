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
    DataAccess()  { if(s_data) loadVars(function());  }
    ~DataAccess() { if(s_data) storeVars(function()); }

    operator bool()    const { return !!s_data; }
    Data *operator->() const { return Callback<Data>::s_data; }
  };

  template<typename T = void>
  static T(*use(Function *func))(Data *)
  {
    if(!func)
      return nullptr;

    assertValid(func);
    func->keepAlive();
    return &invoke<T>;
  }

  template<typename T = void>
  static T invoke(Data *data)
  {
    s_data = data;
    storeVars(function());
    function()->execute();
    loadVars(function());
    s_data = nullptr;

    // prevent accessing the context after it has been destructed
    // after handling an exception during execution of the callback
    // (exceptions cannot cross EEL's boundary so they're handled earlier)
    if(!Resource::isValid(Context::current()))
      throw reascript_error { "an error occurred during callback execution" };

    if constexpr(!std::is_void_v<T>)
      return {};
  }

private:
  static Function *function()
    { return static_cast<Function *>(s_data->UserData); }
  static void storeVars(Function *);
  static void loadVars(const Function *);

  static Data *s_data;
};

template<typename Data>
Data *Callback<Data>::s_data;

#endif
