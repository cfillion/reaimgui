local inst_cache, deferred_inst_cache_reset = {}, false
local function shimResourceCreator(func)
  return function(...)
    if not deferred_inst_cache_reset then
      deferred_inst_cache_reset = true
      reaper.defer(function()
        for k, sub_cache in pairs(inst_cache) do
          sub_cache.touched_this_frame = 0
        end
        deferred_inst_cache_reset = false
      end)
    end

    local args = { func, ... }
    for i, v in ipairs(args) do args[i] = tostring(v) end
    local key = table.concat(args, '\1')

    local sub_cache = inst_cache[key]
    if not sub_cache then
      sub_cache = { touched_this_frame = 0 }
      inst_cache[key] = sub_cache
    end

    sub_cache.touched_this_frame = sub_cache.touched_this_frame + 1
    local obj = sub_cache[sub_cache.touched_this_frame]
    if obj and reaper.ImGui_ValidatePtr(obj, 'ImGui_Resource*') then
      return obj
    end

    obj = func(...)

    -- handle pointer reuse
    for k, sc in pairs(inst_cache) do
      for i, v in ipairs(sc) do
        if v == obj then sc[i] = nil end
      end
    end

    sub_cache[sub_cache.touched_this_frame] = obj

    return obj
  end
end
for name, func in pairs(reaper) do
  if name:match('^ImGui_Create') and name ~= 'ImGui_CreateContext' then
    reaper[name] = shimResourceCreator(func)
  end
end

function reaper.ImGui_DestroyContext(ctx) end -- no-op
