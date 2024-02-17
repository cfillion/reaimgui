#include "shims.hpp"

SHIM("0.8",
  (int, Mod_Ctrl)
  (int, Mod_Shift)
  (int, Mod_Alt)
  (int, Mod_Super)
  (int, GetKeyMods, ImGui_Context*)

  (void, Attach, ImGui_Context*, void*)
  (void, Detach, ImGui_Context*, void*)

  (void, SameLine, ImGui_Context*, double*, double*)
  (void, Spacing,  ImGui_Context*)

  (void, End,           ImGui_Context*)
  (void, EndChild,      ImGui_Context*)
  (void, EndChildFrame, ImGui_Context*)
  (void, EndGroup,      ImGui_Context*)
  (void, EndPopup,      ImGui_Context*)
  (void, EndTable,      ImGui_Context*)
  (void, EndTooltip,    ImGui_Context*)
  (bool, TableNextColumn, ImGui_Context*)
  (void, TableNextRow,    ImGui_Context*, int*, double*)
  (bool, TableSetColumnIndex, ImGui_Context*, int)
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
SHIM_CONST(0_7, ModFlags_Shift, ::ModFlags_Ctrl)
SHIM_CONST(0_7, ModFlags_Alt,   ::ModFlags_Ctrl)
SHIM_CONST(0_7, ModFlags_Super, ::ModFlags_Ctrl)
SHIM_ALIAS(0_6, Key_ModCtrl,  Mod_Ctrl)
SHIM_ALIAS(0_6, Key_ModShift, Mod_Shift)
SHIM_ALIAS(0_6, Key_ModAlt,   Mod_Alt)
SHIM_ALIAS(0_6, Key_ModSuper, Mod_Super)
SHIM_FUNC(0_1, int, GetKeyMods, (ImGui_Context*,ctx))
{
  int oldmods {};
  const int newmods { api.GetKeyMods(ctx) };
  const struct { int newval, oldval; } mods[] {
    { api.Mod_Ctrl(),  ModFlags_Ctrl  },
    { api.Mod_Shift(), ModFlags_Shift },
    { api.Mod_Alt(),   ModFlags_Alt   },
    { api.Mod_Super(), ModFlags_Super },
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
SHIM_FUNC(0_1, bool, IsWindowCollapsed, (ImGui_Context*,ctx))
{
  return false;
}

// obsoleted window boundary extension via SetCursorPos (ocornut/imgui#5548)
SHIM_PROXY_BEGIN(ShimWindowEnd, func, args)
{
  ImGui_Context *ctx { std::get<0>(args) };
  double spacing { 0.0 };
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
