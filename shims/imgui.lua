local error,  pcall = error, pcall
local unpack,  init = string.unpack, reaper.ImGui__init
local setshim, shim = reaper.ImGui__setshim, reaper.ImGui__shim

local function shimResult(level, ok, ...)
  if not ok then error(..., level) end
  return ...
end

local function makeShim(compat_version, name)
  return function(...)
    setshim(compat_version, name)
    return shimResult(2, pcall(shim, ...))
  end
end

local metatable = {
  __index = function(ImGui, key)
    error(("attempt to access a nil value (field '%s')"):format(key), 2)
  end,
}

return function(compat_version)
  local api, i, ImGui = shimResult(3, pcall(init, compat_version)), 1, {}

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

    if flags & 1 ~= 0 then
      ImGui[name] = ImGui[name]()
    end
  end

  return setmetatable(ImGui, metatable)
end
