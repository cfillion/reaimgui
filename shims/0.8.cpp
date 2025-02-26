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

#include "shims.hpp"

SHIM("0.8",
  (int, Mod_Ctrl)
  (int, Mod_Shift)
  (int, Mod_Alt)
  (int, Mod_Super)
  (int, GetKeyMods, Context*)

  (void, Attach, Context*, void*)
  (void, Detach, Context*, void*)

  (void, SameLine, Context*, RO<double*>, RO<double*>)
  (void, Spacing,  Context*)

  (void, End,           Context*)
  (void, EndChild,      Context*)
  (void, EndChildFrame, Context*)
  (void, EndGroup,      Context*)
  (void, EndPopup,      Context*)
  (void, EndTable,      Context*)
  (void, EndTooltip,    Context*)
  (bool, TableNextColumn, Context*)
  (void, TableNextRow,    Context*, RO<int*>, RO<double*>)
  (bool, TableSetColumnIndex, Context*, int)
);

// ModFlags and Key_Mod to Mod rename
// the new Mod_* values are different than the old ModFlags_*
enum ModFlags {
  ModFlags_None  = 0,
  ModFlags_Ctrl  = 1,
  ModFlags_Shift = 2,
  ModFlags_Alt   = 4,
  ModFlags_Super = 8,
};
SHIM_CONST(0_7, ModFlags_None,  ::ModFlags_None)
SHIM_CONST(0_7, ModFlags_Ctrl,  ::ModFlags_Ctrl)
SHIM_CONST(0_7, ModFlags_Shift, ::ModFlags_Shift)
SHIM_CONST(0_7, ModFlags_Alt,   ::ModFlags_Alt)
SHIM_CONST(0_7, ModFlags_Super, ::ModFlags_Super)
SHIM_ALIAS(0_6, Key_ModCtrl,  Mod_Ctrl)
SHIM_ALIAS(0_6, Key_ModShift, Mod_Shift)
SHIM_ALIAS(0_6, Key_ModAlt,   Mod_Alt)
SHIM_ALIAS(0_6, Key_ModSuper, Mod_Super)
SHIM_FUNC(0_1, int, GetKeyMods, (Context*,ctx))
{
  int oldmods {};
  const int newmods {api.GetKeyMods(ctx)};
  const struct { int newval, oldval; } mods[] {
    {api.Mod_Ctrl(),  ModFlags_Ctrl },
    {api.Mod_Shift(), ModFlags_Shift},
    {api.Mod_Alt(),   ModFlags_Alt  },
    {api.Mod_Super(), ModFlags_Super},
  };
  for(const auto &mod : mods) {
    if(newmods & mod.newval)
      oldmods |= mod.oldval;
  }
  return oldmods;
}

// new generic object attachment functions
SHIM_ALIAS(0_4, AttachFont, Attach)
SHIM_ALIAS(0_4, DetachFont, Detach)

// broken since v0.5
SHIM_FUNC(0_1, bool, IsWindowCollapsed, (Context*,ctx))
{
  return false;
}

// obsoleted window boundary extension via SetCursorPos (ocornut/imgui#5548)
SHIM_PROXY_BEGIN(ShimWindowEnd, func, args)
{
  Context *ctx {std::get<0>(args)};
  double spacing {0.0};
  api.SameLine(ctx, nullptr, &spacing);
  api.Spacing(ctx);
  return std::apply(api.*func, args);
}
SHIM_PROXY_END()
SHIM_PROXY(0_5, End,                 ShimWindowEnd)
SHIM_PROXY(0_5, EndChild,            ShimWindowEnd)
SHIM_PROXY(0_5, EndChildFrame,       ShimWindowEnd)
SHIM_PROXY(0_1, EndGroup,            ShimWindowEnd)
SHIM_PROXY(0_1, EndPopup,            ShimWindowEnd)
SHIM_PROXY(0_1, EndTable,            ShimWindowEnd)
SHIM_PROXY(0_1, EndTooltip,          ShimWindowEnd)
SHIM_PROXY(0_1, TableNextColumn,     ShimWindowEnd)
SHIM_PROXY(0_1, TableNextRow,        ShimWindowEnd)
SHIM_PROXY(0_1, TableSetColumnIndex, ShimWindowEnd)
