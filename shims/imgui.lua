local unpack,  init = string.unpack, reaper.ImGui__init
local setshim, shim = reaper.ImGui__setshim, reaper.ImGui__shim
local function makeShim(compat_version, name)
  return function(...) setshim(compat_version, name); return shim(...) end
end

return function(compat_version)
  local api, i, ImGui = init(compat_version), 1, {}

  while i < #api do
    local flags, name
    flags, name, i = unpack('Bz', api, i)
    local full_name = 'ImGui_' .. name
    local unshimed  = '__' .. full_name

    if flags & 2 ~= 0 then
      ImGui[name] = makeShim(compat_version, name)
      if not reaper[unshimed] then
        reaper[unshimed] = reaper[full_name]
      end
      reaper[full_name] = ImGui[name]
    else
      ImGui[name] = reaper[unshimed] or reaper[full_name]
    end
  end

  return ImGui
end
