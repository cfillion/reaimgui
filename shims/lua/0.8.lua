-- ModFlags and Key_Mod to Mod rename
-- the new Mod_* values are different than the old ModFlags_*
reaper.ImGui_ModFlags_None  = function() return 0 end
reaper.ImGui_ModFlags_Ctrl  = function() return 1 end
reaper.ImGui_ModFlags_Shift = function() return 2 end
reaper.ImGui_ModFlags_Alt   = function() return 4 end
reaper.ImGui_ModFlags_Super = function() return 8 end
reaper.ImGui_Key_ModCtrl  = reaper.ImGui_Mod_Ctrl
reaper.ImGui_Key_ModShift = reaper.ImGui_Mod_Shift
reaper.ImGui_Key_ModAlt   = reaper.ImGui_Mod_Alt
reaper.ImGui_Key_ModSuper = reaper.ImGui_Mod_Super

local GetKeyMods = reaper.ImGui_GetKeyMods
function reaper.ImGui_GetKeyMods(ctx)
  local old_mods, new_mods = 0, GetKeyMods(ctx)
  local mods = { 'Ctrl', 'Shift', 'Alt', 'Super' }
  for _, mod in ipairs(mods) do
    if (new_mods & reaper['ImGui_Mod_' .. mod]()) ~= 0 then
      old_mods = old_mods | reaper['ImGui_ModFlags_' .. mod]()
    end
  end
  return old_mods
end

-- new generic object attachment functions
reaper.ImGui_AttachFont = reaper.ImGui_Attach
reaper.ImGui_DetachFont = reaper.ImGui_Detach

-- obsoleted window boundary extension via SetCursorPos (ocornut/imgui#5548)
local function shimWindowEnd(func)
  return function(ctx, ...)
    reaper.ImGui_SetCursorPos(ctx, 0, 0)
    func(ctx, ...)
  end
end
reaper.ImGui_End           = shimWindowEnd(reaper.ImGui_End)
reaper.ImGui_EndChild      = shimWindowEnd(reaper.ImGui_EndChild)
reaper.ImGui_EndChildFrame = shimWindowEnd(reaper.ImGui_EndChildFrame)
reaper.ImGui_EndGroup      = shimWindowEnd(reaper.ImGui_EndGroup)
reaper.ImGui_EndPopup      = shimWindowEnd(reaper.ImGui_EndPopup)
reaper.ImGui_EndTooltip    = shimWindowEnd(reaper.ImGui_EndTooltip)
reaper.ImGui_TableNextColumn = shimWindowEnd(reaper.ImGui_TableNextColumn)
reaper.ImGui_TableNextRow    = shimWindowEnd(reaper.ImGui_TableNextRow)
reaper.ImGui_TableSetColumnIndex = shimWindowEnd(reaper.ImGui_TableSetColumnIndex)
