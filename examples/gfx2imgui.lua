-- Translation layer for running gfx code in ReaImGui
-- Made for fun. Not recommended for use in new (or old) designs.
--
-- gfx = dofile(reaper.GetResourcePath() .. '/Scripts/ReaTeam Extensions/API/gfx2imgui.lua')
-- gfx.init('My window', 200, 100)
-- local function loop()
--   gfx.x = 0
--   gfx.set(1, 1, 1)
--   gfx.drawstr('Hello World!')
--   gfx.set(1, 0, 0, 1)
--   gfx.rect(30, 30, 50, 50)
--   gfx.imgui(function(ctx, draw_list, screen_x, screen_y)
--     reaper.ImGui_Button(ctx, 'Brown fox')
--     reaper.ImGui_Button(ctx, 'Lazy dog')
--   end)
--   if gfx.getchar() >= 0 then
--     gfx.update()
--     reaper.defer(loop)
--   else
--     gfx.quit()
--   end
-- end
-- reaper.defer(loop)

local reaper, ogfx, print = reaper, gfx, print
local debug, math, string, table, utf8 = debug, math, string, table, utf8

local FLT_MIN, FLT_MAX = reaper.ImGui_NumericLimits_Float()
local CTX_FLAGS = reaper.ImGui_ConfigFlags_NoSavedSettings() |
                  reaper.ImGui_ConfigFlags_DockingEnable()
local CANARY_FLAGS = reaper.ImGui_ConfigFlags_NoSavedSettings()
local WND_FLAGS = reaper.ImGui_WindowFlags_NoScrollbar() |
                  reaper.ImGui_WindowFlags_NoScrollWithMouse() |
                  reaper.ImGui_WindowFlags_NoMove()
local CHILD_FLAGS = reaper.ImGui_WindowFlags_NoMouseInputs()
local LOG_WND_FLAGS = reaper.ImGui_WindowFlags_NoDocking()
local HOVERED_FLAGS = reaper.ImGui_HoveredFlags_ChildWindows()
local FOCUSED_FLAGS = reaper.ImGui_FocusedFlags_RootAndChildWindows()
local WINDOW_PADDING, WINDOW_BG, CHILD_BORDER_SIZE =
  reaper.ImGui_StyleVar_WindowPadding(), reaper.ImGui_Col_WindowBg(),
  reaper.ImGui_StyleVar_ChildBorderSize()
local ROUND_CORNERS = reaper.ImGui_DrawFlags_RoundCornersAll()
local NO_DECORATION = reaper.ImGui_ConfigVar_ViewportsNoDecoration()
local MACOS, WINDOWS = reaper.GetOS():find('OSX') ~= nil,
                       reaper.GetOS():find('Win') == 1
local CURSORS = {
  [0x7f00] = reaper.ImGui_MouseCursor_Arrow(),
  [0x7f01] = reaper.ImGui_MouseCursor_TextInput(),
  [0x7f82] = reaper.ImGui_MouseCursor_ResizeNWSE(),
  [0x7f83] = reaper.ImGui_MouseCursor_ResizeNESW(),
  [0x7f84] = reaper.ImGui_MouseCursor_ResizeEW(),
  [0x7f85] = reaper.ImGui_MouseCursor_ResizeNS(),
  [0x7f86] = reaper.ImGui_MouseCursor_ResizeAll(),
  [0x7f88] = reaper.ImGui_MouseCursor_NotAllowed(),
  [0x7f89] = reaper.ImGui_MouseCursor_Hand(),
}
local MOUSE_BTNS = {
  [reaper.ImGui_MouseButton_Left()  ] = 1<<0,
  [reaper.ImGui_MouseButton_Right() ] = 1<<1,
  [reaper.ImGui_MouseButton_Middle()] = 1<<6,
}
local KEY_MODS = {
  [reaper.ImGui_Key_ModCtrl() ] = 1<<2,
  [reaper.ImGui_Key_ModShift()] = 1<<3,
  [reaper.ImGui_Key_ModAlt()  ] = 1<<4,
  [reaper.ImGui_Key_ModSuper()] = 1<<5,
}
local CHAR_MOD_MASK = reaper.ImGui_ModFlags_Ctrl() |
                      reaper.ImGui_ModFlags_Alt()
local CHAR_MOD_BASE = {
  [reaper.ImGui_ModFlags_Ctrl()] = 0x001,
  [CHAR_MOD_MASK               ] = 0x101,
  [reaper.ImGui_ModFlags_Alt() ] = 0x141,
}
local MW_TICK = 6 -- gfx.mouse_[h]wheel increments per wheel tick
local MOD_CTRL, MOD_SHIFT, MOD_ALT, MOD_SUPER =
  reaper.ImGui_Key_ModCtrl(), reaper.ImGui_Key_ModShift(),
  reaper.ImGui_Key_ModAlt(),  reaper.ImGui_Key_ModSuper()
local KEYS = {
  [reaper.ImGui_Key_Backspace()]   = 0x00000008,
  [reaper.ImGui_Key_Delete()]      = 0x0064656c,
  [reaper.ImGui_Key_DownArrow()]   = 0x646f776e,
  [reaper.ImGui_Key_End()]         = 0x00656e64,
  [reaper.ImGui_Key_Enter()]       = 0x0000000d,
  [reaper.ImGui_Key_Escape()]      = 0x0000001b,
  [reaper.ImGui_Key_F1()]          = 0x00006631,
  [reaper.ImGui_Key_F2()]          = 0x00006632,
  [reaper.ImGui_Key_F3()]          = 0x00006633,
  [reaper.ImGui_Key_F4()]          = 0x00006634,
  [reaper.ImGui_Key_F5()]          = 0x00006635,
  [reaper.ImGui_Key_F6()]          = 0x00006636,
  [reaper.ImGui_Key_F7()]          = 0x00006637,
  [reaper.ImGui_Key_F8()]          = 0x00006638,
  [reaper.ImGui_Key_F9()]          = 0x00006639,
  [reaper.ImGui_Key_F10()]         = 0x00663130,
  [reaper.ImGui_Key_F11()]         = 0x00663131,
  [reaper.ImGui_Key_F12()]         = 0x00663132,
  [reaper.ImGui_Key_Home()]        = 0x686f6d65,
  [reaper.ImGui_Key_Insert()]      = 0x00696e73,
  [reaper.ImGui_Key_KeypadEnter()] = 0x0000000d,
  [reaper.ImGui_Key_LeftArrow()]   = 0x6c656674,
  [reaper.ImGui_Key_PageDown()]    = 0x7067646e,
  [reaper.ImGui_Key_PageUp()]      = 0x70677570,
  [reaper.ImGui_Key_RightArrow()]  = 0x72676874,
  [reaper.ImGui_Key_Tab()]         = 0x00000009,
  [reaper.ImGui_Key_UpArrow()]     = 0x00007570,
}
local KEY_A, KEY_Z = reaper.ImGui_Key_A(), reaper.ImGui_Key_Z()
local FONT_FLAGS = {
  [0]                = reaper.ImGui_FontFlags_None(),
  [string.byte('b')] = reaper.ImGui_FontFlags_Bold(),
  [string.byte('i')] = reaper.ImGui_FontFlags_Italic(),
}
local DEFAULT_FONT_SIZE = 13 -- gfx default texth is 8
local UNUSED_FONTS_CACHE_SIZE = GFX2IMGUI_UNUSED_FONTS_CACHE_SIZE or 8
local THROTTLE_FONT_LOADING_FRAMES = 16

-- gfx.mode bits
local BLIT_NO_SOURCE_ALPHA = 2

local gfx, global_state, state = {}, {
  commands   = {},
  fonts      = {},
  log        = { ptr=0, size=0, max_size=64 },
  log_lines  = {},
  imgdim     = {},
  dock       = 0,
  pos_x = 0, pos_y = 0,
}

-- default variables
gfx.r, gfx.g, gfx.b, gfx.a, gfx.a2   = 1, 1, 1, 1, 1
gfx.w, gfx.h, gfx.x, gfx.y, gfx.mode = 0, 0, 0, 0, 0
gfx.ext_retina, gfx.dest, gfx.texth  = 0, -1, DEFAULT_FONT_SIZE
gfx.mouse_x, gfx.mouse_y, gfx.clear  = 0, 0, 0
gfx.mouse_wheel, gfx.mouse_hwheel    = 0, 0
gfx.mouse_cap                        = 0

-- internal functions
local function tobool(v, default)
  if default ~= nil and v == nil then return default end
  return v ~= false and v ~= 0 and v ~= nil
end

local function toint(v)
  if not v or v ~= v or math.abs(v) == math.huge then return 0 end
  return v // 1 -- faster than floor
end

local function ringInsert(buffer, value)
  buffer[buffer.ptr] = value
  buffer.ptr = (buffer.ptr + 1) % buffer.max_size
  buffer.size = buffer.size + 1
  if buffer.size > buffer.max_size then buffer.size = buffer.max_size end
end

local function ringEnum(buffer)
  if buffer.size < 1 then return function() end end

  local i = 0
  return function()
    local j = (buffer.ptr + i) % buffer.size
    if i < buffer.size then
      local value = buffer[j]
      i = i + 1
      return value
    end
  end
end

local function drawCall(command)
  local list = global_state.commands[gfx.dest]
  if not list then
    list = { ptr=0, size=0, max_size=1<<12 }
    global_state.commands[gfx.dest] = list
  elseif list.want_clear then
    list.size, list.ptr, list.want_clear = 0, 0, false
  end

  ringInsert(list, command)
end

local function render(commands, draw_list, screen_x, screen_y, blit_opts)
  for command in ringEnum(commands) do
    command(draw_list, screen_x, screen_y, blit_opts)
  end
  commands.want_clear = true
end

local function color(r, g, b, a)
  r, g, b, a = r or gfx.r or 0, g or gfx.g or 0,
               b or gfx.b or 0, a or gfx.a or 0
  if r > 1 then r = 1 elseif r < 0 then r = 0 end
  if g > 1 then g = 1 elseif g < 0 then g = 0 end
  if b > 1 then b = 1 elseif b < 0 then b = 0 end
  -- gfx does not clamp alpha (it wraps around)
  return toint(r * 0xFF) << 24 |
         toint(g * 0xFF) << 16 |
         toint(b * 0xFF) << 8  |
        (toint(a * 0xFF) & 0xFF)
end

local function transformColor(c, blit_opts)
  if not blit_opts.mode then
    return (c & ~0xff) | ((c & 0xff) * blit_opts.alpha // 1 & 0xFF)
  end

  -- premultiply alpha when rendering from an offscreen buffer
  local a, a_blend = (c & 0xFF) / 0xFF
  if (blit_opts.mode & BLIT_NO_SOURCE_ALPHA) ~= 0 then
    a, a_blend = a * blit_opts.alpha, blit_opts.alpha
  else
    a_blend = a * blit_opts.alpha
  end

  local mask_r, mask_g, mask_b = 0xFF000000, 0x00FF0000, 0x0000FF00
  return ((c & mask_r) * a // 1 & mask_r) |
         ((c & mask_g) * a // 1 & mask_g) |
         ((c & mask_b) * a // 1 & mask_b) |
         ((0xFF * a_blend) // 1 & 0xFF)
end

local function transformPoint(x, y, blit_opts)
  return x * blit_opts.scale_x // 1, y * blit_opts.scale_y // 1
end

local function clip(x1, y1, x2, y2, blit_opts)
  return x1 > blit_opts.x2 or y1 > blit_opts.y2 or
         x2 < blit_opts.x1 or y2 < blit_opts.y1
end

local function mergeBlitOpts(src, dst)
  return {
    alpha   = src.alpha * dst.alpha,
    mode    = src.mode,
    scale_x = src.scale_x * dst.scale_x,
    scale_y = src.scale_y * dst.scale_y,
  }
end

local function updateMouse()
  if reaper.ImGui_IsWindowHovered(state.ctx, HOVERED_FLAGS) then -- not over Log window
    for button, flag in pairs(MOUSE_BTNS) do
      if reaper.ImGui_IsMouseClicked(state.ctx, button) then
        state.mouse_cap = state.mouse_cap | flag
      end
    end

    local wheel_v, wheel_h = reaper.ImGui_GetMouseWheel(state.ctx)
    gfx.mouse_wheel  = gfx.mouse_wheel  + (wheel_v * MW_TICK)
    gfx.mouse_hwheel = gfx.mouse_hwheel + (wheel_h * MW_TICK)

    if state.want_cursor then
      reaper.ImGui_SetMouseCursor(state.ctx, state.want_cursor)
    end
  end

  for button, flag in pairs(MOUSE_BTNS) do
    if reaper.ImGui_IsMouseReleased(state.ctx, button) then
      state.mouse_cap = state.mouse_cap & ~flag
    end
  end

  gfx.mouse_cap = state.mouse_cap

  for mod, flag in pairs(KEY_MODS) do
    if reaper.ImGui_IsKeyDown(state.ctx, mod) then
      gfx.mouse_cap = gfx.mouse_cap | flag
    end
  end

  gfx.mouse_x, gfx.mouse_y = reaper.ImGui_GetMousePos(state.ctx)
  gfx.mouse_x, gfx.mouse_y = gfx.mouse_x - state.screen_x,
                             gfx.mouse_y - state.screen_y
end

local function updateKeyboard()
  -- simulate gfx's behavior of eating shortcut keys in the global scope
  reaper.ImGui_SetNextFrameWantCaptureKeyboard(state.ctx, true)

  -- flags for gfx.getchar(65536)
  state.wnd_flags = 1
  if reaper.ImGui_IsWindowFocused(state.ctx, FOCUSED_FLAGS) then
    state.wnd_flags = state.wnd_flags | 2
  end
  if not reaper.ImGui_IsWindowCollapsed(state.ctx) then
    state.wnd_flags = state.wnd_flags | 4
  end

  for k, c in pairs(KEYS) do
    if reaper.ImGui_IsKeyPressed(state.ctx, k) then
      ringInsert(state.charqueue, c)
    end
  end

  local mods = reaper.ImGui_GetKeyMods(state.ctx) & CHAR_MOD_MASK
  for flags, mod_base in pairs(CHAR_MOD_BASE) do
    if flags == mods then
      for k = KEY_A, KEY_Z do
        if reaper.ImGui_IsKeyPressed(state.ctx, k) then
          ringInsert(state.charqueue, mod_base + (k - KEY_A))
        end
      end
      return -- break + bypass the character input queue
    end
  end

  local i = 0
  while true do
    local rv, char = reaper.ImGui_GetInputQueueCharacter(state.ctx, i)
    if not rv then break end
    ringInsert(state.charqueue, char)
    i = i + 1
  end
end

local function updateDropFiles()
  state.drop_files = {}
  if reaper.ImGui_BeginChild(state.ctx, 'drop_target', -FLT_MIN, -FLT_MIN, 0, CHILD_FLAGS) then
    reaper.ImGui_EndChild(state.ctx)
    if reaper.ImGui_BeginDragDropTarget(state.ctx) then
      local rv, count = reaper.ImGui_AcceptDragDropPayloadFiles(state.ctx)
      if rv then
        for i = 0, count - 1 do
          local filename
          rv, filename = reaper.ImGui_GetDragDropPayloadFile(state.ctx, i)
          state.drop_files[i] = filename
        end
      end
      reaper.ImGui_EndDragDropTarget(state.ctx)
    end
  end
end

local function warn(message, ...)
  if GFX2IMGUI_NO_LOG then return end

  local funcInfo = debug.getinfo(2, 'nSl')
  local warnLine = funcInfo.currentline

  -- don't print duplicate messages
  for _, line in ipairs(global_state.log_lines) do
    if line == warnLine then return end
  end
  global_state.log_lines[#global_state.log_lines + 1] = warnLine

  local depth, callerInfo = 3, nil
  repeat
    callerInfo = debug.getinfo(depth, 'nSl')
    depth = depth + 1
  until not callerInfo or callerInfo.source ~= funcInfo.source

  -- get the gfx call in case the warning comes from deeper in gfx2imgui
  funcInfo = debug.getinfo(depth - 2, 'nSl')

  if not callerInfo or not funcInfo.short_src:match('gfx2imgui.lua$') then
    -- tail calls
    callerInfo = funcInfo
    funcInfo = { name = '<optimized out>', currentline = 0 }
  end

  message = ('gfx.%s[%d]: %s [%s@%s:%d]')
    :format(funcInfo.name, warnLine, message:format(...),
            callerInfo.name, callerInfo.short_src, callerInfo.currentline)
  ringInsert(global_state.log, message)
  print(message)
end

local function showLog()
  reaper.ImGui_SetConfigVar(state.ctx, NO_DECORATION, 1)
  reaper.ImGui_SetNextWindowSize(state.ctx, 800, 300, reaper.ImGui_Cond_Once())
  local visible, open = reaper.ImGui_Begin(state.ctx, 'gfx2imgui [Log]', true, LOG_WND_FLAGS)
  reaper.ImGui_SetConfigVar(state.ctx, NO_DECORATION, 0)
  if not visible then return end
  local scroll_bottom = reaper.ImGui_GetScrollY(state.ctx) == reaper.ImGui_GetScrollMaxY(state.ctx)
  for line in ringEnum(global_state.log) do
    reaper.ImGui_TextWrapped(state.ctx, line)
  end
  if scroll_bottom then reaper.ImGui_SetScrollHereY(state.ctx, 1) end
  if not open then global_state.log.size = 0 end
  reaper.ImGui_End(state.ctx)
end

local function setDock(v)
  global_state.dock = v & 0xf01
  state.want_dock = (v & 1) == 1 and ~(v >> 8 & 0xf) or 0
end

local function hasValue(array, needle)
  for _, v in pairs(array) do
    if v == needle then
      return true
    end
  end

  return false
end

local function dig(array, ...)
  for i = 1, select('#', ...) do
    if type(array) ~= 'table' then return end
    array = array[select(i, ...)]
  end

  return array
end

local function put(array, ...)
  local n = select('#', ...)
  for i = 1, n - 2 do
    local k = select(i, ...)
    local v = array[k]
    if type(v) ~= 'table' then
      assert(not v)
      v = {}
      array[k] = v
    end
    array = v
  end
  array[select(n - 1, ...)] = select(n, ...)
end

local function nearest(array, target_key)
  local best_value, best_score

  for key, value in pairs(array) do
    local score = math.abs(key - target_key)
    if best_score and score > best_score then break end
    best_value, best_score = value, score
  end

  return best_value, best_score or 0
end

local function getCachedFont(font)
  return dig(state.fontmap, font.family, font.flags, font.size)
end

local function warnUnavailableFont(font)
  warn("font '%s'@%d[%x] temporarily unavailable: frame already started (falling back to nearest match for up to %d frames)",
    font.family, font.size, font.flags, THROTTLE_FONT_LOADING_FRAMES)
end

local function getNearestCachedFont(font)
  if not font then return nil, nil, 0 end

  local sizes = dig(state.fontmap, font.family, font.flags)
  if not sizes then
    warnUnavailableFont(font)
    return nil, nil, DEFAULT_FONT_SIZE - font.size
  end

  local match, score = sizes[font.size], 0
  if not match then
    warnUnavailableFont(font)
    match, score = nearest(sizes, font.size)
  end

  if match then
    match.last_use = state.frame_count
    return match, match.instance, score
  end

  return nil, nil, DEFAULT_FONT_SIZE - font.size
end

local function unloadUnusedFonts()
  local garbage = {}

  for family, styles in pairs(state.fontmap) do
    for style, sizes in pairs(styles) do
      for size, cache in pairs(sizes) do
        if not cache.keep_alive and cache.last_use + 1 < state.frame_count then
          garbage[#garbage + 1] =
            { cache = sizes, cache_key = size, cache_val = cache }
        end
      end
    end
  end

  table.sort(garbage, function(a, b)
    return a.cache_val.last_use > b.cache_val.last_use
  end)

  for i = UNUSED_FONTS_CACHE_SIZE + 1, #garbage do
    local old_font = garbage[i]
    -- print(('DetachFont() size=%d'):format(old_font.cache_key))
    reaper.ImGui_DetachFont(state.ctx, old_font.cache_val.instance)
    old_font.cache_val.attached = false
    old_font.cache[old_font.cache_key] = nil
  end
end

local function loadRequestedFonts()
  local throttled = false

  for _, font in ipairs(state.fontqueue) do
    if not getCachedFont(font) then
      if not throttled then
        if state.font_frame and state.frame_count > 4 and
           state.frame_count - state.font_frame < THROTTLE_FONT_LOADING_FRAMES then
          return
        else
          state.font_frame = state.frame_count
        end
        throttled = true
      end

      -- print(('AttachFont() %s@%d[%d]'):format(font.family, font.size, font.flags))
      local instance = reaper.ImGui_CreateFont(font.family, font.size, font.flags)
      local keep_alive = hasValue(global_state.fonts, font)
      reaper.ImGui_AttachFont(state.ctx, instance)
      put(state.fontmap, font.family, font.flags, font.size, {
        attached   = true,
        last_use   = state.frame_count,
        keep_alive = keep_alive,
        instance   = instance,
      })
    end
  end

  if #state.fontqueue > 0 then
    state.fontqueue = {}
  end
end

local function beginFrame()
  -- disable everything if called from an reaper.atexit callback while REAPER
  -- is exiting (reaimgui has unloaded at that point)
  if not reaper.EnumProjects(0) then return false end

  assert(reaper.ImGui_ValidatePtr(state.ctx, 'ImGui_Context*'),
    'reaimgui context got garbage-collected: was gfx.update called every defer cycle?')

  -- protect against scripts calling gfx.update more than once per defer cycle
  -- or before the first defer timer tick
  local this_frame = reaper.ImGui_GetFrameCount(state.canary)
  if state.frame_count == this_frame then return true end
  state.frame_count = this_frame

  unloadUnusedFonts()
  loadRequestedFonts()

  -- reaper.ImGui_ShowMetricsWindow(state.ctx)

  return true
end

local function center2D(points)
  local n_coords = #points
  local center_x, center_y, n_points = 0, 0, n_coords / 2
  for i = 1, n_coords, 2 do
    center_x, center_y = center_x + points[i], center_y + points[i + 1]
  end
  return center_x / n_points, center_y / n_points
end

local function sort2D(points, center_x, center_y)
  local atan2 = math.atan
  for i = 1, #points, 2 do
    local x, y, j = points[i], points[i + 1], i - 2
    local angle = atan2(y - center_y, x - center_x)
    while j >= 1 and
        atan2(points[j + 1] - center_y, points[j + 0] - center_x) > angle do
      points[j + 2], points[j + 3] = points[j + 0], points[j + 1]
      j = j - 2
    end
    points[j + 2], points[j + 3] = x, y
  end
end

local function uniq2D(points)
  local j, n_points = 3, #points
  for i = j, n_points, 2 do
    local x, y = points[i], points[i + 1]
    if x ~= points[j - 2] or y ~= points[j - 1] then
      points[j], points[j + 1] = x, y
      j = j + 2
    end
  end
  points.resize(j - 1)
  return j - 1
end

local function drawPixel(x, y, c)
  local AddRectFilled = reaper.ImGui_DrawList_AddRectFilled
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local c = transformColor(c, blit_opts)
    local x, y = transformPoint(x, y, blit_opts)
    x, y = screen_x + x, screen_y + y
    local w, h = transformPoint(1, 1, blit_opts)
    AddRectFilled(draw_list, x, y, x + w, y + h, c)
  end)
end

local function drawLine(x1, y1, x2, y2, c)
  local AddLine = reaper.ImGui_DrawList_AddLine
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local c = transformColor(c, blit_opts)
    local x1, y1 = transformPoint(x1, y1, blit_opts)
    local x2, y2 = transformPoint(x2, y2, blit_opts)
    -- FIXME: scale thickness
    x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
    AddLine(draw_list, x1, y1, x2, y2, c)
  end)
end

-- translation functions
function gfx.arc(x, y, r, ang1, ang2, antialias)
  -- if antialias then warn('ignoring parameter antialias') end
  local c, quarter = color(), math.pi / 2
  x, y, ang1, ang2 = toint(x) + 1, toint(y), ang1 - quarter, ang2 - quarter
  local PathArcTo  = reaper.ImGui_DrawList_PathArcTo
  local PathStroke = reaper.ImGui_DrawList_PathStroke
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local c = transformColor(c, blit_opts)
    local r = r * blit_opts.scale_y -- FIXME: scale_x
    local x, y = transformPoint(x, y, blit_opts)
    PathArcTo(draw_list, screen_x + x, screen_y + y, r, ang1, ang2)
    PathStroke(draw_list, c)
  end)
  return 0
end

function gfx.blit(source, ...)
  local n_args = select('#', ...)
  if n_args < 2 then return 0 end

  local scale, rotation, srcx, srcy, srcw, srch,
        destx, desty, destw, desth, rotxoffs, rotyoffs = ...

  source = toint(source)
  srcx, srcy, srcw, srch, destx, desty, destw, desth =
    toint(srcx),  toint(srcy),  toint(srcw),  toint(srch),
    toint(destx), toint(desty), toint(destw), toint(desth)
  rotxoffs, rotyoffs = rotxoffs or 0.0, rotyoffs or 0.0

  local dim = global_state.imgdim[source]

  if n_args <  1 then scale = 1            end
  if n_args <  5 and dim then srcw = dim.w end
  if n_args <  6 and dim then srch = dim.h end
  if n_args <  7 then destx = gfx.x        end
  if n_args <  8 then desty = gfx.y        end
  if n_args <  9 then destw = srcw * scale end
  if n_args < 10 then desth = srch * scale end

  if rotation ~= 0 then warn('ignoring parameter rotation') end
  if rotxoffs ~= 0 then warn('ignoring parameter rotxoffs') end
  if rotyoffs ~= 0 then warn('ignoring parameter rotyoffs') end

  if gfx.mode ~= 0 and (gfx.mode & ~BLIT_NO_SOURCE_ALPHA) ~= 0 then
    warn('mode %d not implemented', gfx.mode)
  end

  local sourceCommands = global_state.commands[source]
  if not sourceCommands then
    warn('source buffer is empty, nothing to blit')
    return 0
  end

  local commands = { ptr=0, size=0, max_size=sourceCommands.size }
  for c in ringEnum(sourceCommands) do
    ringInsert(commands, c)
  end

  local src_blit = {
    alpha   = gfx.a,
    mode    = gfx.mode,
    scale_x = srcw ~= 0 and destw / srcw or 1,
    scale_y = srch ~= 0 and desth / srch or 1,
  }

  drawCall(function(draw_list, screen_x, screen_y, dst_blit)
    local destx, desty = transformPoint(destx, desty, dst_blit)
    local destw, desth = transformPoint(destw, desth, dst_blit)

    if not clip(destx, desty, destx + destw, desty + desth, dst_blit) then
      local merged_blit = mergeBlitOpts(src_blit, dst_blit)
      local srcx, srcy  = transformPoint(srcx,  srcy,  src_blit)
      merged_blit.x1, merged_blit.y1 = srcx, srcy
      merged_blit.x2, merged_blit.y2 = srcx + destw, srcy + desth

      local x1, y1 = screen_x + destx, screen_y + desty
      local x2, y2 = x1 + destw, y1 + desth
      reaper.ImGui_DrawList_PushClipRect(draw_list, x1, y1, x2, y2, true)
      render(commands, draw_list, x1 - srcx, y1 - srcy, merged_blit)
      reaper.ImGui_DrawList_PopClipRect(draw_list)
    end

    sourceCommands.want_clear = true
  end)

  return source
end

function gfx.blitext()
  warn('not implemented')
  -- return img
end

function gfx.blurto()
  warn('not supported')
  -- return x
end

function gfx.circle(x, y, r, fill, antialias)
  -- if antialias then warn('ignoring parameter antialias') end
  local c = color()
  x, y, r = toint(x), toint(y), toint(r)
  local AddCircle = tobool(fill, false) and
    reaper.ImGui_DrawList_AddCircleFilled or reaper.ImGui_DrawList_AddCircle
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local c = transformColor(c, blit_opts)
    local x, y = transformPoint(x, y, blit_opts)
    local r = r * blit_opts.scale_y -- FIXME: draw ellipse if x/y scale mismatch
    AddCircle(draw_list, screen_x + x + .5, screen_y + y + .5, r + .5, c)
  end)
  return 0
end

function gfx.clienttoscreen(x, y)
  if not state then return x, y end
  return global_state.pos_x + x, global_state.pos_y + y
end

function gfx.deltablit()
  warn('not implemented')
  return 0
end

function gfx.dock(v, ...) -- v[,wx,wy,ww,wh]
  v = tonumber(v)

  if v >= 0 then
    setDock(v)
    return global_state.dock
  end

  local n, rv = select('#', ...), {}
  if n >= 1 then rv[1] = global_state.pos_x end
  if n >= 2 then rv[2] = global_state.pos_y end
  if n >= 3 then rv[3] = gfx.w              end
  if n >= 4 then rv[4] = gfx.h              end

  return global_state.dock, table.unpack(rv)
end

function gfx.drawchar(char)
  gfx.drawstr(char or '')
  return char
end

function gfx.drawnumber(n, ndigits)
  ndigits = math.floor((tonumber(ndigits) or 0) + 0.5)
  gfx.drawstr(('%%.%df'):format(ndigits):format(n))
  return n
end

function gfx.drawstr(str, flags, right, bottom)
  if not state then return end
  str = str or '<bad string>'

  local x, y, c = toint(gfx.x), toint(gfx.y), color()
  local w, h = gfx.measurestr(str) -- calls beginFrame()
  local f = global_state.fonts[state.font]
  local f_sz = f and f.size or DEFAULT_FONT_SIZE
  local f_cache, f_inst = getNearestCachedFont(f)
  if right  then right  = toint(right) end
  if bottom then bottom = toint(bottom) end

  if flags then
    if (flags & 1) ~= 0 and right then -- center horizontally
      local diff = (right - (x + w)) / 2
      if diff > 0 then
        x = x + diff
        right = right - diff
      end
    elseif (flags & 2) ~= 0 and right then -- right justify
      x = right - w
    end
    if (flags & 4) ~= 0 and bottom then -- center vertically
      local diff = (bottom - (y + h)) / 2
      if diff > 0 then
        y = y + diff
        bottom = bottom - diff
      end
    elseif (flags & 4) ~= 0 and right then -- bottom justify
      y = bottom - h
    end
    if (flags & 256) ~= 0 then right, bottom = nil, nil end -- disable clipping
  end

  gfx.x = gfx.x + w

  local AddTextEx = reaper.ImGui_DrawList_AddTextEx
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    -- search for a new font as the draw call may have been stored for a
    -- long time in an offscreen buffer while the font instance got detached
    -- or the script may have re-created the context with gfx.quit+gfx.init
    if f_cache and not f_cache.attached then
      f_cache, f_inst = getNearestCachedFont(f)
    end

    -- keep the font alive while the draw call is still in use (eg. from a blit)
    if f_cache then
      f_cache.last_use = state.frame_count
    end

    local c = transformColor(c, blit_opts)
    local x, y = transformPoint(x, y, blit_opts)
    local f_sz = f_sz * blit_opts.scale_y -- height only, cannot stretch width
    AddTextEx(draw_list, f_inst, f_sz,
      screen_x + x, screen_y + y, c, str, 0, 0,
      right and right - x or nil, bottom and bottom - y or nil)
  end)
  return 0
end

function gfx.getchar(char)
  if not state then return -1 end
  if not char or char <= 0 then
    if state.want_close then
      return -1
    end

    local wptr
    wptr, state.charqueue.ptr = state.charqueue.ptr, state.charqueue.rptr
    if wptr == state.charqueue.rptr then return 0 end

    local char = ringEnum(state.charqueue)()
    state.charqueue.rptr = (state.charqueue.rptr + 1) % state.charqueue.max_size
    state.charqueue.ptr = wptr
    return char
  elseif char == 65536 then
    return state.wnd_flags
  elseif type(char) == 'string' then
    char = string.byte(char)
  end

  if not beginFrame() then return -1 end
  return reaper.ImGui_IsKeyDown(state.ctx, char)
end

function gfx.getdropfile(idx)
  if not state then return end
  if idx < 0 then
    state.drop_files = {}
    return
  end
  local file = state.drop_files[idx]
  return file ~= nil and 1 or 0, file
end

function gfx.getfont()
  if not state then return -1 end
  local font = global_state.fonts[state.font]
  return state.font - 1, font and font.family
end

function gfx.getimgdim(image)
  image = toint(image)
  local dim = global_state.imgdim[image]
  if not dim then return 0, 0 end
  return dim.w, dim.h
end

function gfx.getpixel()
  warn('not supported')
  return 0
end

function gfx.gradrect(x, y, w, h, r, g, b, a, drdx, dgdx, dbdx, dadx, drdy, dgdy, dbdy, dady)
  -- FIXME: support colors growing to > 1 or < 0 before the end of the rect
  x, y, w, h = toint(x), toint(y), toint(w), toint(h)
  drdx, dgdx, dbdx, dadx = w * (drdx or 0), w * (dgdx or 0),
                           w * (dbdx or 0), w * (dadx or 0)
  drdy, dgdy, dbdy, dady = h * (drdy or 0), h * (dgdy or 0),
                           h * (dbdy or 0), h * (dady or 0)
  local ctl = color(r, g, b, a)
  local ctr = color(r + drdx, g + dgdx, b + dbdx, a + dadx)
  local cbl = color(r + drdy, g + dgdy, b + dbdy, a + dady)
  local cbr = color(r + drdx + drdy, g + dgdx + dgdy,
                    b + dbdx + dbdy, a + dadx + dady)
  local AddRectFilledMultiColor = reaper.ImGui_DrawList_AddRectFilledMultiColor
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local ctl, ctr, cbr, cbl = transformColor(ctl, blit_opts),
                               transformColor(ctr, blit_opts),
                               transformColor(cbr, blit_opts),
                               transformColor(cbl, blit_opts)
    local x1, y1 = transformPoint(x,     y,     blit_opts)
    local x2, y2 = transformPoint(x + w, y + h, blit_opts)
    x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
    AddRectFilledMultiColor(draw_list, x1, y1, x2, y2, ctl, ctr, cbr, cbl)
  end)
  return 0
end

function gfx.imgui(callback)
  local x, y = toint(gfx.x), toint(gfx.y)
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    reaper.ImGui_SetCursorScreenPos(state.ctx, screen_x + x, screen_y + y)
    callback(state.ctx, draw_list, screen_x, screen_y, blit_opts)
  end)
end

function gfx.init(name, width, height, dockstate, xpos, ypos)
  if state then
    if name ~= state.name and
        not width and not height and not dockstate and not xpos and not ypos then
      state.name = name
    else
      warn('ignoring repeated call to init')
    end
    return 0
  end

  local ctx_name = name
  if ctx_name:len() < 1 then ctx_name = 'gfx2imgui' end

  state = {
    name        = name,
    ctx         = reaper.ImGui_CreateContext(ctx_name, CTX_FLAGS),
    canary      = reaper.ImGui_CreateContext(ctx_name, CANARY_FLAGS),
    wnd_flags   = 1,
    want_close  = false,
    font        = 0,
    fontmap     = {},
    fontqueue   = {},
    frame_count = -1,
    charqueue   = { ptr=0, rptr=0, size=0, max_size=16 },
    drop_files  = {},
    mouse_cap   = 0,
  }

  reaper.ImGui_SetConfigVar(state.ctx, NO_DECORATION, 0)

  for _, font in ipairs(global_state.fonts) do
    state.fontqueue[#state.fontqueue + 1] = font
  end

  if width and height then
    gfx.w, gfx.h = math.floor(width), math.floor(height)
    gfx.w, gfx.h = math.max(16, gfx.w), math.max(16, gfx.h)
    state.want_size = { w=gfx.w, h=gfx.h }
  end
  if dockstate and (dockstate & 1) == 1 then
    setDock(dockstate)
  end
  if xpos and ypos then
    state.want_pos = { x=math.floor(xpos), y=math.floor(ypos) }
  end

  gfx.ext_retina = 1 -- ReaImGui scales automatically

  return 1
end

function gfx.line(x1, y1, x2, y2, aa)
  -- if aa then warn('ignoring parameter aa') end
  x1, y1, x2, y2 = toint(x1), toint(y1), toint(x2), toint(y2)

  -- gfx.line(10, 30, 10, 30)
  if x1 == x2 and y1 == y2 then
    drawPixel(x1, y1, color()) -- faster than 1px lines according to dear imgui
  else
    drawLine(x1, y1, x2, y2, color())
  end

  return 0
end

function gfx.lineto(x, y, aa)
  gfx.line(gfx.x, gfx.y, x, y, aa)
  gfx.x, gfx.y = x, y
  return x
end

function gfx.loadimg(image, filename)
  local rv = ogfx.loadimg(1, filename)
  if rv < 0 then return rv end

  local w, h = ogfx.getimgdim(1)
  gfx.setimgdim(image, w, h)

  local dest_backup = gfx.dest
  gfx.dest = image

  warn('bitmap images not implemented (placeholder pattern)')

  local AddRect, AddLine = reaper.ImGui_DrawList_AddRect,
                           reaper.ImGui_DrawList_AddLine
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local c = transformColor(0xFF00FFFF, blit_opts)
    local w, h = transformPoint(w, h, blit_opts)
    local x, y = screen_x + blit_opts.x1, screen_y + blit_opts.y1
    local max_w, max_h = blit_opts.x2 - blit_opts.x1,
                         blit_opts.y2 - blit_opts.y1
    if w > max_w then w = max_w end if h > max_h then h = max_h end

    AddRect(draw_list, x, y, x + w, y + h, c)
    local pitch = 10 * blit_opts.scale_x
    for i = pitch, w * 2, pitch do
      AddLine(draw_list, x + i, y, (x + i) - w, y + h, c)
      AddLine(draw_list, (x + i) - w, y, x + i, y + h, c)
    end
  end)

  gfx.dest = dest_backup

  return rv
end

function gfx.measurechar(char)
  if not state then return gfx.texth, gfx.texth end
  return gfx.measurestr(utf8.char(char))
end

function gfx.measurestr(str)
  if not state or not beginFrame() then
    return gfx.texth * utf8.len(str), gfx.texth
  end
  local _, font_inst, size_error =
    getNearestCachedFont(global_state.fonts[state.font])
  local correction_factor = gfx.texth / (gfx.texth + size_error)
  reaper.ImGui_PushFont(state.ctx, font_inst)
  local w, h = reaper.ImGui_CalcTextSize(state.ctx, str)
  reaper.ImGui_PopFont(state.ctx)
  return w * correction_factor, h * correction_factor
end

function gfx.muladdrect()
  warn('not implemented')
  return 0
end

function gfx.printf(format, ...)
  if not state then return end
  return gfx.drawstr(format:format(...))
end

function gfx.quit()
  if not state then return end
  -- context will already have been destroyed when calling quit() from atexit()
  if reaper.ImGui_ValidatePtr(state.ctx, 'ImGui_Context*') then
    reaper.ImGui_DestroyContext(state.ctx)
  end
  for family, styles in pairs(state.fontmap) do
    for style, sizes in pairs(styles) do
      for size, cache in pairs(sizes) do
        cache.attached = false
      end
    end
  end
  state = nil
  return 0
end

function gfx.rect(x, y, w, h, filled)
  local c = color()
  local AddRect = tobool(filled, true) and
    reaper.ImGui_DrawList_AddRectFilled or reaper.ImGui_DrawList_AddRect
  x, y, w, h = toint(x), toint(y), toint(w), toint(h)
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local c = transformColor(c, blit_opts)
    local x1, y1 = transformPoint(x,     y,     blit_opts)
    local x2, y2 = transformPoint(x + w, y + h, blit_opts)
    -- FIXME: scale thickness
    x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
    AddRect(draw_list, x1, y1, x2, y2, c)
  end)
  return 0
end

function gfx.rectto(x, y)
  gfx.rect(gfx.x, gfx.y, x - gfx.x, y - gfx.y)
  gfx.x, gfx.y = x, y
  return x
end

function gfx.roundrect(x, y, w, h, radius, antialias)
  -- if antialias then warn('ignoring parameter antialias') end
  local c = color()
  x, y, w, h = toint(x), toint(y), toint(w), toint(h)
  local AddRect = reaper.ImGui_DrawList_AddRect
  drawCall(function(draw_list, screen_x, screen_y, blit_opts)
    local c = transformColor(c, blit_opts)
    local radius = radius * blit_opts.scale_y -- FIXME: scale_x
    local x1, y1 = transformPoint(x,     y,     blit_opts)
    local x2, y2 = transformPoint(x + w, y + h, blit_opts)
    -- FIXME: scale thickness
    x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
    AddRect(draw_list, x1, y1, x2 + 1, y2 + 1, c, radius, ROUND_CORNERS)
  end)
  return 0
end

function gfx.screentoclient(x, y)
  if not state then return x, y end
  return x - global_state.pos_x, y - global_state.pos_y
end

function gfx.set(...)
  local n = select('#', ...)
  if n < 1 then return end

  local r, g, b, a, mode, dest, a2 = ...
  if n < 2 then g = r end
  if n < 3 then b = r end

  gfx.r, gfx.g, gfx.b = r, g, b
  if n >= 4 then gfx.a    = a    end
  if n >= 5 then gfx.mode = mode end
  if n >= 6 then gfx.dest = dest end
  if n >= 7 then gfx.a2   = a2   end

  return 0
end

function gfx.setcursor(resource_id, custom_cursor_name)
  if not state then return end
  if custom_cursor_name then warn('ignoring parameter custom_cursor_name') end
  state.want_cursor = CURSORS[resource_id]
  if not state.want_cursor then warn("unknown cursor '%s'", resource_id) end
  return 0
end

function gfx.setfont(idx, fontface, sz, flag)
  idx = tonumber(idx) -- Default_6.0_theme_adjuster.lua gives a string sometimes

  local font = global_state.fonts[idx]

  if idx > 0 and (fontface or sz) then
    -- gfx does this
    if not fontface or fontface:len() == 0 then
      fontface = 'Arial'
    end
    sz = toint(sz)
    if sz < 2 then
      sz = 10
    end

    local flags = FONT_FLAGS[flag or 0]
    if not flags then
      flags = 0
      warn("unknown font flag '%s'", flag)
    end

    local is_new

    if font then
      is_new = font.family ~= fontface or font.size ~= sz or font.flags ~= flags
      if is_new and state then
        local cache = getCachedFont(font)
        if cache then cache.keep_alive = false end
      end
    else
      is_new = true
    end

    if is_new then
      font = { family = fontface, size = sz, flags = flags }
      global_state.fonts[idx] = font
    end

    if state and not getCachedFont(font) then
      state.fontqueue[#state.fontqueue + 1] = font
    end
  end

  if state then
    state.font = font and idx or 0
  end

  gfx.texth = idx ~= 0 and ((font and font.size) or sz) or DEFAULT_FONT_SIZE

  return 1
end

function gfx.setimgdim(image, w, h)
  image = toint(image)
  local dim = { w=math.max(0, toint(w)), h=math.max(0, toint(h)) }
  global_state.imgdim[image] = dim

  local commands = global_state.commands[image]
  if commands and dim.w == 0 and dim.h == 0 then
    commands.want_clear = true
  end

  return 1
end

function gfx.setpixel(r, g, b)
  drawPixel(gfx.x, gfx.y, color(r, g, b, 1))
  return r
end

function gfx.showmenu(str)
  -- cannot use a ImGui menu because the host script expect gfx.showmenu to be blocking
  if not WINDOWS then return ogfx.showmenu(str) end

  -- Using hidden gfx window menu code by amagalma
  -- https://forum.cockos.com/showthread.php?t=239556
  local title = reaper.genGuid()
  ogfx.init(title, 0, 0, 0, 0, 0)
  ogfx.x, ogfx.y = ogfx.mouse_x, ogfx.mouse_y

  if reaper.JS_Window_Show then
    local hwnd = reaper.JS_Window_Find(title, true)
    if hwnd then
      reaper.JS_Window_Show(hwnd, 'HIDE')
    end
  end

  local value = ogfx.showmenu(str)
  ogfx.quit()
  return value
end

function gfx.transformblit()
  warn('not implemented')
  return 0
end

function gfx.triangle(...)
  local c, n_coords = color(), select('#', ...)
  assert(n_coords >= 6, 'gfx.triangle requires 6 or more parameters')

  -- rounding up to nearest even point count
  local has_even = (n_coords & 1) == 0
  local points = reaper.new_array(has_even and n_coords or n_coords + 1)
  for i = 1, n_coords, 2 do
    points[i], points[i + 1] = toint(select(i, ...)), toint(select(i + 1, ...))
  end
  if not has_even then
    n_coords = n_coords + 1
    points[n_coords] = points[2]
  end

  local center_x, center_y = center2D(points)
  sort2D(points, center_x, center_y) -- sort clockwise for antialiasing
  n_coords = uniq2D(points)

  if n_coords == 2 then
    -- gfx.triangle(0,33, 0,33, 0,33, 0,33)
    drawPixel(points[1], points[2], c)
  elseif n_coords == 4 then
    -- gfx.triangle(0,33, 0,0, 0,33, 0,33)
    drawLine(points[1], points[2], points[3], points[4], c)
  elseif n_coords == 6 then
    local AddTriangleFilled = reaper.ImGui_DrawList_AddTriangleFilled
    drawCall(function(draw_list, screen_x, screen_y, blit_opts)
      local c = transformColor(c, blit_opts)
      local x1, y1 = transformPoint(points[1], points[2], blit_opts)
      local x2, y2 = transformPoint(points[3], points[4], blit_opts)
      local x3, y3 = transformPoint(points[5], points[6], blit_opts)
      if points[1] > center_x then x1 = x1 + 1 end
      if points[2] > center_y then y1 = y1 + 1 end
      if points[3] > center_x then x2 = x2 + 1 end
      if points[4] > center_y then y2 = y2 + 1 end
      if points[5] > center_x then x3 = x3 + 1 end
      if points[6] > center_y then y3 = y3 + 1 end
      AddTriangleFilled(draw_list,
        screen_x + x1, screen_y + y1,
        screen_x + x2, screen_y + y2,
        screen_x + x3, screen_y + y3, c)
    end)
  else
    local AddConvexPolyFilled = reaper.ImGui_DrawList_AddConvexPolyFilled
    local screen_points = reaper.new_array(n_coords)
    drawCall(function(draw_list, screen_x, screen_y, blit_opts)
      local c = transformColor(c, blit_opts)
      for i = 1, n_coords, 2 do
        screen_points[i], screen_points[i + 1] =
          transformPoint(points[i], points[i + 1], blit_opts)
        screen_points[i], screen_points[i + 1] =
          screen_x + screen_points[i], screen_y + screen_points[i + 1]
        if points[i]     > center_x then screen_points[i]     = screen_points[i]     + 1 end
        if points[i + 1] > center_y then screen_points[i + 1] = screen_points[i + 1] + 1 end
      end
      AddConvexPolyFilled(draw_list, screen_points, c)
    end)
  end

  return 0
end

function gfx.update()
  if not state or not beginFrame() then return end

  if global_state.log.size > 0 then showLog() end

  if state.want_dock then
    reaper.ImGui_SetNextWindowDockID(state.ctx, state.want_dock)
    -- keep position and size when using gfx.init with dock=0
    if state.want_dock ~= 0 then
      state.want_pos, state.want_size = nil, nil
    end
    state.want_dock = nil
  end
  if state.want_pos then
    local x, y = reaper.ImGui_PointConvertNative(state.ctx, state.want_pos.x, state.want_pos.y)
    if MACOS then y = y - (state.want_size and state.want_size.h or gfx.h) end
    reaper.ImGui_SetNextWindowPos(state.ctx, x, y)
    state.want_pos = nil
  end
  if state.want_size then
    reaper.ImGui_SetNextWindowSize(state.ctx, state.want_size.w, state.want_size.h)
    state.want_size = nil
  end

  -- start window
  local col_clear = math.max(0, gfx.clear)
  local bg = (col_clear >> 8  & 0x0000ff00) |
             (col_clear << 8  & 0x00ff0000) |
             (col_clear << 24 & 0xff000000) |
             0xff
  reaper.ImGui_PushStyleColor(state.ctx, WINDOW_BG, bg)
  reaper.ImGui_PushStyleVar(state.ctx, WINDOW_PADDING, 0, 0)
  reaper.ImGui_PushStyleVar(state.ctx, CHILD_BORDER_SIZE, 0) -- no border when docked
  local wnd_label = ('%s###gfx2imgui'):format(state.name)
  local visible, open = reaper.ImGui_Begin(state.ctx, wnd_label, true, WND_FLAGS)
  reaper.ImGui_PopStyleVar(state.ctx, 2)
  reaper.ImGui_PopStyleColor(state.ctx)

  if not visible then
    for _, commands in pairs(global_state.commands) do
      commands.want_clear = true
    end
    return 0
  end

  -- update variables
  gfx.w, gfx.h = reaper.ImGui_GetWindowSize(state.ctx)
  state.want_close = state.want_close or not open
  state.screen_x, state.screen_y = reaper.ImGui_GetWindowPos(state.ctx)
  global_state.pos_x, global_state.pos_y = state.screen_x, state.screen_y
  if MACOS then global_state.pos_y = global_state.pos_y + gfx.h end
  global_state.pos_x, global_state.pos_y = reaper.ImGui_PointConvertNative(state.ctx,
    global_state.pos_x, global_state.pos_y, true)

  if reaper.ImGui_IsWindowDocked(state.ctx) then
    global_state.dock = 1 | (~reaper.ImGui_GetWindowDockID(state.ctx) << 8)
  else
    global_state.dock = global_state.dock & ~1 -- preserve previous docker ID
  end

  updateMouse()
  updateKeyboard()
  updateDropFiles()

  -- draw contents
  local commands = global_state.commands[-1]
  if commands then
    local draw_list = reaper.ImGui_GetWindowDrawList(state.ctx)
    -- mode=nil tells transformColor it's not outputting to an offscreen buffer
    local blit_opts = {
      alpha=1, mode=nil, scale_x=1, scale_y=1,
      x1=0, y1=0, x2=gfx.w, y2=gfx.h,
    }
    render(commands, draw_list, state.screen_x, state.screen_y, blit_opts)
  end

  reaper.ImGui_End(state.ctx)
  return 0
end

return gfx
