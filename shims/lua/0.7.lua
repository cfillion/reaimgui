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
