-- ModFlags and Key_Mod to Mod rename
reaper.ImGui_ModFlags_None  = reaper.ImGui_Mod_None
reaper.ImGui_ModFlags_Ctrl  = reaper.ImGui_Mod_Ctrl
reaper.ImGui_ModFlags_Shift = reaper.ImGui_Mod_Shift
reaper.ImGui_ModFlags_Alt   = reaper.ImGui_Mod_Alt
reaper.ImGui_ModFlags_Super = reaper.ImGui_Mod_Super
reaper.ImGui_Key_ModCtrl  = reaper.ImGui_Mod_Ctrl
reaper.ImGui_Key_ModShift = reaper.ImGui_Mod_Shift
reaper.ImGui_Key_ModAlt   = reaper.ImGui_Mod_Alt
reaper.ImGui_Key_ModSuper = reaper.ImGui_Mod_Super

-- new generic object attachment functions
reaper.ImGui_AttachFont = reaper.ImGui_Attach
reaper.ImGui_DetachFont = reaper.ImGui_Detach

-- obsoleted window boundary extension via SetCursorPos (ocornut/imgui#5548)
local function shimWindowEnd(func)
  return function(ctx, ...)
    reaper.ImGui_SameLine(ctx, nil, 0)
    func(ctx, ...)
  end
end
reaper.ImGui_End           = shimWindowEnd(reaper.ImGui_End)
reaper.ImGui_EndChild      = shimWindowEnd(reaper.ImGui_EndChild)
reaper.ImGui_EndChildFrame = shimWindowEnd(reaper.ImGui_EndChildFrame)
reaper.ImGui_EndGroup      = shimWindowEnd(reaper.ImGui_EndGroup)
reaper.ImGui_TableNextColumn = shimWindowEnd(reaper.ImGui_TableNextColumn)
reaper.ImGui_TableNextRow    = shimWindowEnd(reaper.ImGui_TableNextRow)
reaper.ImGui_TableSetColumnIndex = shimWindowEnd(reaper.ImGui_TableSetColumnIndex)
