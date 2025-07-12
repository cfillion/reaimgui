package.path = reaper.ImGui_GetBuiltinPath() .. '/?.lua'
local ImGui = require 'imgui' '0.9'

local SCRIPT_NAME = 'imgui.lua 0.9 upgrader'
local FLT_MIN, FLT_MAX = ImGui.NumericLimits_Float()
local ctx = ImGui.CreateContext(SCRIPT_NAME)
local code

local function upgrade()
  code = code:gsub('reaper%s*%.%s*ImGui_', 'ImGui.')
  code = code:gsub('r%s*%.%s*ImGui_',      'ImGui.')
  for key, val in pairs(ImGui) do
    if type(val) == 'number' then
      code = code:gsub('(ImGui%.' .. key .. ')%s*%([^%)]*%)', '%1')
    end
  end
end

local function window()
  if ImGui.Button(ctx, 'Upgrade', -FLT_MIN) then upgrade() end
  ImGui.InputTextMultiline(ctx, '##preamble',
    "package.path = reaper.ImGui_GetBuiltinPath() .. '/?.lua'\n\z
     local ImGui = require 'imgui' '0.9'", -FLT_MIN, 40, ImGui.InputTextFlags_ReadOnly)
  code = select(2, ImGui.InputTextMultiline(ctx, '##code', code,
    -FLT_MIN, -FLT_MIN, ImGui.InputTextFlags_AllowTabInput))
end

local function loop()
  ImGui.SetNextWindowSize(ctx, 512, 256, ImGui.Cond_FirstUseEver)
  local visible, open = ImGui.Begin(ctx, SCRIPT_NAME, true)
  if visible then window(); ImGui.End(ctx) end
  if open    then reaper.defer(loop)       end
end

reaper.defer(loop)
