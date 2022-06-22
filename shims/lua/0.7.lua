-- KeyModFlags to ModFlags rename
reaper.ImGui_KeyModFlags_None  = reaper.ImGui_ModFlags_None
reaper.ImGui_KeyModFlags_Ctrl  = reaper.ImGui_ModFlags_Ctrl
reaper.ImGui_KeyModFlags_Shift = reaper.ImGui_ModFlags_Shift
reaper.ImGui_KeyModFlags_Alt   = reaper.ImGui_ModFlags_Alt
reaper.ImGui_KeyModFlags_Super = reaper.ImGui_ModFlags_Super

-- Capture*FromApp to SetNextFrameWantCapture* rename
reaper.ImGui_CaptureKeyboardFromApp = reaper.SetNextFrameWantCaptureKeyboard

-- non-vanilla HSVtoRGB/RGBtoHSV packing and optional alpha parameter
local function shimColConv(convFunc)
  return function(x, y, z, a)
    local x, y, z = convFunc(x, y, z)
    local rgba = reaper.ImGui_ColorConvertDouble4ToU32(x, y, z, a or 1.0)
    return (rgba & 0xffffffff) >> (a and 0 or 8), a, x, y, z
  end
end
reaper.ImGui_ColorConvertHSVtoRGB = shimColConv(reaper.ImGui_ColorConvertHSVtoRGB)
reaper.ImGui_ColorConvertRGBtoHSV = shimColConv(reaper.ImGui_ColorConvertRGBtoHSV)

-- ConfigVar API
function reaper.ImGui_GetConfigFlags(ctx)
  return reaper.ImGui_GetConfigVar(ctx, reaper.ImGui_ConfigVar_Flags())
end
function reaper.ImGui_SetConfigFlags(ctx, flags)
  return reaper.ImGui_SetConfigVar(ctx, reaper.ImGui_ConfigVar_Flags(), flags)
end

-- null-terminated combo and list box items
local Combo, ListBox = reaper.ImGui_Combo, reaper.ImGui_ListBox
function reaper.ImGui_Combo(ctx, label, current_item, items, popup_max_height_in_items)
  return Combo(ctx, label, current_item, items:gsub('\31', '\0'), popup_max_height_in_items)
end
function reaper.ImGui_ListBox(ctx, label, current_item, items, height_in_items)
  return ListBox(ctx, label, current_item, items:gsub('\31', '\0'), height_in_items)
end

-- Addition of IMGUI_VERSION_NUM to GetVersion
local GetVersion = reaper.ImGui_GetVersion
function reaper.ImGui_GetVersion()
  local imgui_version, imgui_version_num, reaimgui_version = GetVersion()
  return imgui_version, reaimgui_version
end
