-- Usage:
-- package.path = reaper.ImGui_GetBuiltinPath() .. '/?.lua'
-- local ImGui = require 'imgui' 'API version (eg. 0.9)'

local error,  pcall = error, pcall
local unpack,  init = string.unpack, reaper.ImGui__init
local setshim, shim = reaper.ImGui__setshim, reaper.ImGui__shim

local function shimResult(level, ok, ...)
  if not ok then error(..., level) end
  return ...
end

local function makeShim(api_version, name)
  return function(...)
    setshim(api_version, name)
    return shimResult(2, pcall(shim, ...))
  end
end

local metatable = {
  __index = function(ImGui, key)
    error(("attempt to access a nil value (field '%s')"):format(key), 2)
  end,
}

return function(api_version)
  local api, i, ImGui = shimResult(3, pcall(init, api_version)), 1, {}

  while i < #api do
    local flags, name
    flags, name, i = unpack('Bz', api, i)
    local fullname = 'ImGui_' .. name
    local unshimed = '__' .. fullname

    if flags & 2 ~= 0 then
      if not reaper[unshimed] then
        reaper[unshimed] = reaper[fullname]
      end
      local shim = makeShim(api_version, name)
      ImGui[name], reaper[fullname] = shim, shim
    else
      ImGui[name] = reaper[unshimed] or reaper[fullname]
    end

    if flags & 1 ~= 0 then
      ImGui[name] = ImGui[name]()
    end
  end

  return setmetatable(ImGui, metatable)
end
