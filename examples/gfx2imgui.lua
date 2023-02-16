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
--
-- Configuration variables (set before including gfx2imgui)
-- GFX2IMGUI_DEBUG = false
-- GFX2IMGUI_MAX_DRAW_CALLS = 8192
-- GFX2IMGUI_NO_BLIT_PREMULTIPLY = false
-- GFX2IMGUI_NO_LOG = false
-- GFX2IMGUI_UNUSED_FONTS_CACHE_SIZE = 8

local ImGui = {}
for name, func in pairs(reaper) do
  name = name:match('^ImGui_(.+)$')
  if name then
    ImGui[name] = func
  end
end

local reaper, ogfx, print = reaper, gfx, print
local debug, math, string, table, utf8 = debug, math, string, table, utf8

local FLT_MIN, FLT_MAX = ImGui.NumericLimits_Float()
local CTX_FLAGS = ImGui.ConfigFlags_NoSavedSettings() |
                  ImGui.ConfigFlags_DockingEnable()
local CANARY_FLAGS = ImGui.ConfigFlags_NoSavedSettings()
local WND_FLAGS = ImGui.WindowFlags_NoScrollbar() |
                  ImGui.WindowFlags_NoScrollWithMouse() |
                  ImGui.WindowFlags_NoMove()
local CHILD_FLAGS = ImGui.WindowFlags_NoMouseInputs()
local LOG_WND_FLAGS = ImGui.WindowFlags_NoDocking() |
                      ImGui.WindowFlags_NoFocusOnAppearing()
local HOVERED_FLAGS = ImGui.HoveredFlags_ChildWindows()
local FOCUSED_FLAGS = ImGui.FocusedFlags_RootAndChildWindows()
local WINDOW_PADDING, WINDOW_BG, CHILD_BORDER_SIZE =
  ImGui.StyleVar_WindowPadding(), ImGui.Col_WindowBg(),
  ImGui.StyleVar_ChildBorderSize()
local ROUND_CORNERS = ImGui.DrawFlags_RoundCornersAll()
local NO_DECORATION = ImGui.ConfigVar_ViewportsNoDecoration()
local MACOS, WINDOWS = reaper.GetOS():find('OSX') ~= nil,
                       reaper.GetOS():find('Win') == 1
local CURSORS = {
  [0x7f00] = ImGui.MouseCursor_Arrow(),
  [0x7f01] = ImGui.MouseCursor_TextInput(),
  [0x7f82] = ImGui.MouseCursor_ResizeNWSE(),
  [0x7f83] = ImGui.MouseCursor_ResizeNESW(),
  [0x7f84] = ImGui.MouseCursor_ResizeEW(),
  [0x7f85] = ImGui.MouseCursor_ResizeNS(),
  [0x7f86] = ImGui.MouseCursor_ResizeAll(),
  [0x7f88] = ImGui.MouseCursor_NotAllowed(),
  [0x7f89] = ImGui.MouseCursor_Hand(),
}
local MOUSE_BTNS = {
  [ImGui.MouseButton_Left()  ] = 1<<0,
  [ImGui.MouseButton_Right() ] = 1<<1,
  [ImGui.MouseButton_Middle()] = 1<<6,
}
local KEY_MODS = {
  [ImGui.Mod_Ctrl() ] = 1<<2,
  [ImGui.Mod_Shift()] = 1<<3,
  [ImGui.Mod_Alt()  ] = 1<<4,
  [ImGui.Mod_Super()] = 1<<5,
}
local CHAR_MOD_MASK = ImGui.Mod_Ctrl() | ImGui.Mod_Alt()
local CHAR_MOD_BASE = {
  [ImGui.Mod_Ctrl()] = 0x001,
  [CHAR_MOD_MASK   ] = 0x101,
  [ImGui.Mod_Alt() ] = 0x141,
}
local MW_TICK = 6 -- gfx.mouse_[h]wheel increments per wheel tick
local KEYS = {
  [ImGui.Key_Backspace()]   = 0x00000008,
  [ImGui.Key_Delete()]      = 0x0064656c,
  [ImGui.Key_DownArrow()]   = 0x646f776e,
  [ImGui.Key_End()]         = 0x00656e64,
  [ImGui.Key_Enter()]       = 0x0000000d,
  [ImGui.Key_Escape()]      = 0x0000001b,
  [ImGui.Key_F1()]          = 0x00006631,
  [ImGui.Key_F2()]          = 0x00006632,
  [ImGui.Key_F3()]          = 0x00006633,
  [ImGui.Key_F4()]          = 0x00006634,
  [ImGui.Key_F5()]          = 0x00006635,
  [ImGui.Key_F6()]          = 0x00006636,
  [ImGui.Key_F7()]          = 0x00006637,
  [ImGui.Key_F8()]          = 0x00006638,
  [ImGui.Key_F9()]          = 0x00006639,
  [ImGui.Key_F10()]         = 0x00663130,
  [ImGui.Key_F11()]         = 0x00663131,
  [ImGui.Key_F12()]         = 0x00663132,
  [ImGui.Key_Home()]        = 0x686f6d65,
  [ImGui.Key_Insert()]      = 0x00696e73,
  [ImGui.Key_KeypadEnter()] = 0x0000000d,
  [ImGui.Key_LeftArrow()]   = 0x6c656674,
  [ImGui.Key_PageDown()]    = 0x7067646e,
  [ImGui.Key_PageUp()]      = 0x70677570,
  [ImGui.Key_RightArrow()]  = 0x72676874,
  [ImGui.Key_Tab()]         = 0x00000009,
  [ImGui.Key_UpArrow()]     = 0x00007570,
}
local KEY_A, KEY_Z = ImGui.Key_A(), ImGui.Key_Z()
local FONT_FLAGS = {
  [0]                = ImGui.FontFlags_None(),
  [string.byte('b')] = ImGui.FontFlags_Bold(),
  [string.byte('i')] = ImGui.FontFlags_Italic(),
}
local FALLBACK_STRING = '<bad string>'
local DEFAULT_FONT_SIZE = 13 -- gfx default texth is 8
local QUARTER_CIRCLE, INFINITY = math.pi / 2, math.huge

-- settings
local BLIT_NO_PREMULTIPLY          = GFX2IMGUI_NO_BLIT_PREMULTIPLY or false
local DEBUG                        = GFX2IMGUI_DEBUG               or false
local NO_LOG                       = GFX2IMGUI_NO_LOG              or false
local MAX_DRAW_CALLS               = GFX2IMGUI_MAX_DRAW_CALLS      or 1<<13
local PROFILE                      = GFX2IMGUI_PROFILE             or false
local THROTTLE_FONT_LOADING_FRAMES = 16
local UNUSED_FONTS_CACHE_SIZE      = GFX2IMGUI_UNUSED_FONTS_CACHE_SIZE or 8

-- gfx.mode bits
local BLIT_NO_SOURCE_ALPHA = 2

local profiler
if PROFILE then
  -- https://github.com/charlesmallah/lua-profiler
  profiler = dofile(reaper.GetResourcePath() .. '/Scripts/profiler.lua')
  profiler.attachPrintFunction(reaper.ShowConsoleMsg)
  profiler.configuration({ fW = 60 })
end

local DL_AddCircle               = ImGui.DrawList_AddCircle
local DL_AddCircleFilled         = ImGui.DrawList_AddCircleFilled
local DL_AddConvexPolyFilled     = ImGui.DrawList_AddConvexPolyFilled
local DL_AddImage                = ImGui.DrawList_AddImage
local DL_AddLine                 = ImGui.DrawList_AddLine
local DL_AddRect                 = ImGui.DrawList_AddRect
local DL_AddRectFilled           = ImGui.DrawList_AddRectFilled
local DL_AddRectFilledMultiColor = ImGui.DrawList_AddRectFilledMultiColor
local DL_AddTextEx               = ImGui.DrawList_AddTextEx
local DL_AddTriangleFilled       = ImGui.DrawList_AddTriangleFilled
local DL_PathArcTo               = ImGui.DrawList_PathArcTo
local DL_PathStroke              = ImGui.DrawList_PathStroke
local DL_PopClipRect             = ImGui.DrawList_PopClipRect
local DL_PushClipRect            = ImGui.DrawList_PushClipRect

local gfx, global_state, state = {}, {
  commands   = {},
  fonts      = {},
  images     = {},
  log        = { ptr=0, size=0, max_size=64 },
  log_lines  = {},
  dock       = 0,
  pos_x = 0, pos_y = 0,
}

-- default variables (see also gfx_vars_initializers)
local gfx_vars = {
  r = 1.0, g = 1.0, b = 1.0,
  w = 0, h = 0, x = 0, y = 0, mode = 0,
  ext_retina = 0, dest = -1, texth = DEFAULT_FONT_SIZE,
  mouse_x = 0, mouse_y = 0, clear = 0,
  mouse_wheel = 0, mouse_hwheel = 0,
}

-- internal functions
local function tobool(v, default)
  if default ~= nil and v == nil then return default end
  return v ~= false and v ~= 0 and v ~= nil
end

local function toint(v)
  if not v or v ~= v or v == INFINITY or -v == INFINITY then return 0 end
  return v // 1 -- faster than floor
end

local function ringReserve(buffer)
  local ptr = buffer.ptr
  buffer.ptr = (ptr + 1) % buffer.max_size
  buffer.size = buffer.size + 1
  if buffer.size > buffer.max_size then buffer.size = buffer.max_size end
  return ptr + 1
end

local function ringInsert(buffer, value)
  buffer[ringReserve(buffer)] = value
end

local function ringEnum(buffer)
  if buffer.size < 1 then return function() end end

  local i, ptr, size = 0, buffer.ptr, buffer.size
  return function()
    if i >= size then return end
    local j = (ptr + i) % size
    i = i + 1
    return buffer[j + 1]
  end
end

local function drawCall(...)
  local list = global_state.commands[gfx_vars.dest]
  if not list then
    list = { ptr=0, size=0, max_size=MAX_DRAW_CALLS }
    global_state.commands[gfx_vars.dest] = list
  elseif list.want_clear then
    list.size, list.ptr, list.rendered_frame, list.want_clear = 0, 0, 0, false
  end

  local ptr = ringReserve(list)
  local c = list[ptr]

  if not c then
    -- pre-allocate the maximum size w/o nil gaps
    -- IF SIZE CHANGES: also update copy code in gfx.blit!
    c = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }
    list[ptr] = c
  end

  -- faster than looping over select('#', ...) or creating a new table
  c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9], c[10], c[11] = ...

  if DEBUG then
    assert(type((...)) == 'function', 'uncallable draw command')
    assert(select('#', ...) <= 11)
  end

  return 0
end

local function render(commands, draw_list, screen_x, screen_y, blit_opts)
  local ptr, size = commands.ptr, commands.size
  for i = 0, size - 1 do
    local j = (ptr + i) % size
    local c = commands[j + 1]
    c[1](draw_list, screen_x, screen_y, blit_opts,
         c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9], c[10], c[11])
  end
  commands.want_clear = true
end

local function makeColor(r, g, b, a)
  return ((r * 0xFF) // 1) << 24 |
         ((g * 0xFF) // 1) << 16 |
         ((b * 0xFF) // 1) << 8  |
        (((a * 0xFF) // 1) & 0xFF)
end

local function color()
  -- gfx.a is reset to nil every frame (initialized to 1 on access)
  -- this is a hot path, accessing gfx.a (calls __index) is too slow here
  local r, g, b, a = gfx_vars.r, gfx_vars.g, gfx_vars.b, gfx_vars.a or 1
  if r > 1 then r = 1 elseif r < 0 then r = 0 end
  if g > 1 then g = 1 elseif g < 0 then g = 0 end
  if b > 1 then b = 1 elseif b < 0 then b = 0 end
  if gfx_vars.dest == -1 then
    -- gfx does not clamp alpha when blitting (it wraps around)
    if a > 1 then a = 1 elseif a < 0 then a = 0 end
  end
  return makeColor(r, g, b, a)
end

local function transformColor(c, blit_opts)
  if not blit_opts.mode or BLIT_NO_PREMULTIPLY then
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

local function alignText(flags, pos, size, limit)
  local offset = 0
  if flags == 0 then return pos, offset end

  local diff = limit - (pos + size)
  if flags & 1 ~= 0 then diff = diff / 2 end -- center

  if diff > 0 then
    pos, limit = pos + diff
  else
    offset = diff
  end

  return pos, offset
end

local function updateMouse()
  state.hovered = ImGui.IsWindowHovered(state.ctx, HOVERED_FLAGS)
  if state.hovered then -- not over Log window
    local wheel_v, wheel_h = ImGui.GetMouseWheel(state.ctx)
    gfx_vars.mouse_wheel  = gfx_vars.mouse_wheel  + (wheel_v * MW_TICK)
    gfx_vars.mouse_hwheel = gfx_vars.mouse_hwheel + (wheel_h * MW_TICK)

    if state.want_cursor then
      ImGui.SetMouseCursor(state.ctx, state.want_cursor)
    end
  end

  gfx_vars.mouse_x, gfx_vars.mouse_y = ImGui.GetMousePos(state.ctx)
  gfx_vars.mouse_x, gfx_vars.mouse_y = gfx_vars.mouse_x - state.screen_x,
                                       gfx_vars.mouse_y - state.screen_y
end

local function updateKeyboard()
  -- simulate gfx's behavior of eating shortcut keys in the global scope
  ImGui.SetNextFrameWantCaptureKeyboard(state.ctx, true)

  -- flags for gfx.getchar(65536)
  state.wnd_flags = 1
  if ImGui.IsWindowFocused(state.ctx, FOCUSED_FLAGS) then
    state.wnd_flags = state.wnd_flags | 2
  end
  -- if not ImGui.IsWindowCollapsed(state.ctx) then
  if state.collapsed then
    state.wnd_flags = state.wnd_flags | 4
  end

  for k, c in pairs(KEYS) do
    if ImGui.IsKeyPressed(state.ctx, k) then
      ringInsert(state.charqueue, c)
    end
  end

  local mods = ImGui.GetKeyMods(state.ctx) & CHAR_MOD_MASK
  for flags, mod_base in pairs(CHAR_MOD_BASE) do
    if flags == mods then
      for k = KEY_A, KEY_Z do
        if ImGui.IsKeyPressed(state.ctx, k) then
          ringInsert(state.charqueue, mod_base + (k - KEY_A))
        end
      end
      return -- break + bypass the character input queue
    end
  end

  local i = 0
  while true do
    local rv, char = ImGui.GetInputQueueCharacter(state.ctx, i)
    if not rv then break end
    ringInsert(state.charqueue, char)
    i = i + 1
  end
end

local function updateDropFiles()
  state.drop_files = {}
  if ImGui.BeginChild(state.ctx, 'drop_target', -FLT_MIN, -FLT_MIN, 0, CHILD_FLAGS) then
    ImGui.EndChild(state.ctx)

    -- reset cursor pos for when gfx.update() is run more than once per frame
    ImGui.SetCursorScreenPos(state.ctx, state.screen_x, state.screen_y)

    if ImGui.BeginDragDropTarget(state.ctx) then
      local rv, count = ImGui.AcceptDragDropPayloadFiles(state.ctx)
      if rv then
        for i = 0, count - 1 do
          local filename
          rv, filename = ImGui.GetDragDropPayloadFile(state.ctx, i)
          state.drop_files[i] = filename
        end
      end
      ImGui.EndDragDropTarget(state.ctx)
    end
  end
end

local function warn(message, ...)
  if NO_LOG then return end

  local funcInfo = debug.getinfo(2, 'nSl')
  local warnLine = funcInfo.currentline

  -- don't print duplicate messages
  for _, line in ipairs(global_state.log_lines) do
    if line == warnLine then return end
  end
  global_state.log_lines[#global_state.log_lines + 1] = warnLine

  local depth, callerInfo = 3, nil
  if DEBUG then depth = depth + 1 end -- skip xpcall()
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
  ImGui.SetConfigVar(state.ctx, NO_DECORATION, 1)
  ImGui.SetNextWindowSize(state.ctx, 800, 300, ImGui.Cond_Once())
  local visible, open = ImGui.Begin(state.ctx, 'gfx2imgui [Log]', true, LOG_WND_FLAGS)
  ImGui.SetConfigVar(state.ctx, NO_DECORATION, 0)
  if not visible then return end
  local scroll_bottom = ImGui.GetScrollY(state.ctx) == ImGui.GetScrollMaxY(state.ctx)
  local copy = false
  if ImGui.BeginPopupContextWindow(state.ctx) then
    if ImGui.MenuItem(state.ctx, 'Copy') then copy = true end
    ImGui.EndPopup(state.ctx)
  end
  if copy then ImGui.LogToClipboard(state.ctx) end
  for line in ringEnum(global_state.log) do
    ImGui.TextWrapped(state.ctx, line)
  end
  if copy then ImGui.LogFinish(state.ctx) end
  if scroll_bottom then ImGui.SetScrollHereY(state.ctx, 1) end
  if not open then global_state.log.size = 0 end
  ImGui.End(state.ctx)
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
    -- print(('Detach() size=%d'):format(old_font.cache_key))
    ImGui.Detach(state.ctx, old_font.cache_val.instance)
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

      -- print(('Attach() %s@%d[%d]'):format(font.family, font.size, font.flags))
      local instance = ImGui.CreateFont(font.family, font.size, font.flags)
      local keep_alive = hasValue(global_state.fonts, font)
      ImGui.Attach(state.ctx, instance)
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

  assert(ImGui.ValidatePtr(state.ctx, 'ImGui_Context*'),
    'reaimgui context got garbage-collected: was gfx.update called every defer cycle?')

  -- protect against scripts calling gfx.update more than once per defer cycle
  -- or before the first defer timer tick
  local this_frame = ImGui.GetFrameCount(state.canary)
  if state.frame_count == this_frame then return true end
  state.frame_count = this_frame

  unloadUnusedFonts()
  loadRequestedFonts()

  -- ImGui.ShowMetricsWindow(state.ctx)
  if global_state.log.size > 0 then showLog() end

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

local function drawPixel(draw_list, screen_x, screen_y, blit_opts, x, y, c)
  c = transformColor(c, blit_opts)
  x, y = transformPoint(x, y, blit_opts)
  x, y = screen_x + x, screen_y + y
  w, h = transformPoint(1, 1, blit_opts)
  DL_AddRectFilled(draw_list, x, y, x + w, y + h, c)
end

local function addPixel(x, y, c)
  return drawCall(drawPixel, x, y, c)
end

local function drawLine(draw_list, screen_x, screen_y, blit_opts,
                        x1, y1, x2, y2, c)
  c = transformColor(c, blit_opts)

  -- workarounds to avoid gaps due to rounding in vertical/horizontal lines
  local scaled = blit_opts.scale_x ~= 1 and blit_opts.scale_y ~= 1
  if scaled and (x1 == x2 or y1 == y2) then
    x1, y1 = screen_x + (x1 * blit_opts.scale_x),
                  screen_y + (y1 * blit_opts.scale_y)
    x2, y2 = screen_x + (x2 * blit_opts.scale_x),
                  screen_y + (y2 * blit_opts.scale_y)
    if x1 == x2 then
      x2 = x2 + blit_opts.scale_x
    elseif y1 == y2 then
      y2 = y2 + blit_opts.scale_y
    end

    DL_AddRectFilled(draw_list, x1, y1, x2, y2, c)
    return
  end

  local x1, y1 = transformPoint(x1, y1, blit_opts)
  local x2, y2 = transformPoint(x2, y2, blit_opts)
  x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
  DL_AddLine(draw_list, x1, y1, x2, y2, c,
            (blit_opts.scale_x + blit_opts.scale_y) / 2)
end

local function addLine(x1, y1, x2, y2, c)
  return drawCall(drawLine, x1, y1, x2, y2, c)
end

-- variables to reset on the first access of every frame via gfx.__index
local gfx_vars_initializers = {
  a  = function() return 1.0 end,
  a2 = function() return 1.0 end,

  mouse_cap = function()
    if not state or not beginFrame() then return 0 end

    if state.hovered then -- not over Log window
      for button, flag in pairs(MOUSE_BTNS) do
        if ImGui.IsMouseClicked(state.ctx, button) then
          state.mouse_cap = state.mouse_cap | flag
        end
      end
    end

    for button, flag in pairs(MOUSE_BTNS) do
      -- IsMouseReleased is not emitted when buttons are cleared due to focus loss
      -- also the user might not access mouse_cap every frame
      if not ImGui.IsMouseDown(state.ctx, button) then
        state.mouse_cap = state.mouse_cap & ~flag
      end
    end

    local mouse_cap = state.mouse_cap

    for mod, flag in pairs(KEY_MODS) do
      if ImGui.IsKeyDown(state.ctx, mod) then
        mouse_cap = mouse_cap | flag
      end
    end

    return mouse_cap
  end,
}

setmetatable(gfx, {
  __index = function(gfx, key)
    local val = gfx_vars[key]
    if val then return val end

    local init = gfx_vars_initializers[key]
    if init then
      val = init()
      gfx_vars[key] = val
      return val
    end

    return rawget(gfx, key)
  end,
  __newindex = function(gfx, key, value)
    local t = type(value)
    if t == 'function' then
      return rawset(gfx, key, value)
    elseif t ~= 'number' then
      -- same behavior as gfx
      error(('bad argument: expected number, got %s'):format(t))
    end

    if value ~= value or value == INFINITY or -value == INFINITY then
      gfx_vars[key] = 0
    else
      gfx_vars[key] = value
    end
  end,
})

-- translation functions
local function drawArc(draw_list, screen_x, screen_y, blit_opts,
                       x, y, r, c, ang1, ang2)
  local c = transformColor(c, blit_opts)
  local r = r * blit_opts.scale_y -- FIXME: scale_x
  local x, y = transformPoint(x, y, blit_opts)
  DL_PathArcTo(draw_list, screen_x + x, screen_y + y, r, ang1, ang2)
  DL_PathStroke(draw_list, c)
end

function gfx.arc(x, y, r, ang1, ang2, antialias)
  -- if antialias then warn('ignoring parameter antialias') end
  return drawCall(drawArc, toint(x) + 1, toint(y), r, color(),
                  ang1 - QUARTER_CIRCLE, ang2 - QUARTER_CIRCLE)
end

local function drawBlit(draw_list, screen_x, screen_y, dst_blit,
                        srcx, srcy, destx, desty, destw, desth,
                        src_blit, commands, sourceCommands)
  destx, desty = transformPoint(destx, desty, dst_blit)
  destw, desth = transformPoint(destw, desth, dst_blit)

  sourceCommands.want_clear = true

  if clip(destx, desty, destx + destw, desty + desth, dst_blit) then
    return
  end

  local merged_blit = mergeBlitOpts(src_blit, dst_blit)
  srcx, srcy = transformPoint(srcx,  srcy,  src_blit)
  merged_blit.x1, merged_blit.y1 = srcx, srcy
  merged_blit.x2, merged_blit.y2 = srcx + destw, srcy + desth

  local x1, y1 = screen_x + destx, screen_y + desty
  local x2, y2 = x1 + destw, y1 + desth
  DL_PushClipRect(draw_list, x1, y1, x2, y2, true)
  render(commands, draw_list, x1 - srcx, y1 - srcy, merged_blit)
  DL_PopClipRect(draw_list)
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

  local dim = global_state.images[source]

  if n_args <  1 then scale = 1            end
  if n_args <  5 and dim then srcw = dim.w end
  if n_args <  6 and dim then srch = dim.h end
  if n_args <  7 then destx = gfx_vars.x   end
  if n_args <  8 then desty = gfx_vars.y   end
  if n_args <  9 then destw = srcw * scale end
  if n_args < 10 then desth = srch * scale end

  if rotation ~= 0 then warn('ignoring parameter rotation') end
  if rotxoffs ~= 0 then warn('ignoring parameter rotxoffs') end
  if rotyoffs ~= 0 then warn('ignoring parameter rotyoffs') end

  if gfx_vars.mode ~= 0 and (gfx_vars.mode & ~BLIT_NO_SOURCE_ALPHA) ~= 0 then
    warn('mode %d not implemented', gfx_vars.mode)
  end

  local sourceCommands = global_state.commands[source]
  if not sourceCommands then
    warn('source buffer is empty, nothing to blit')
    return 0
  end

  local size = sourceCommands.size
  local commands = { ptr=sourceCommands.ptr, size=size, max_size=size }
  for i = 1, size do
    -- make an immutable copy
    local c = sourceCommands[i]
    commands[i] =
      { c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9], c[10], c[11] }
  end

  local src_blit = {
    alpha   = gfx.a,
    mode    = gfx_vars.mode,
    scale_x = srcw ~= 0 and destw / srcw or 1,
    scale_y = srch ~= 0 and desth / srch or 1,
  }

  drawCall(drawBlit, srcx, srcy, destx, desty, destw, desth,
           src_blit, commands, sourceCommands)

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

local function drawCircle(draw_list, screen_x, screen_y, blit_opts,
                          circleFunc, x, y, r, c)
  c = transformColor(c, blit_opts)
  x, y = transformPoint(x, y, blit_opts)
  r = r * blit_opts.scale_y -- FIXME: draw ellipse if x/y scale mismatch
  circleFunc(draw_list, screen_x + x + .5, screen_y + y + .5, r + .5, c)
end

function gfx.circle(x, y, r, fill, antialias)
  -- if antialias then warn('ignoring parameter antialias') end
  local circleFunc = tobool(fill, false) and DL_AddCircleFilled or DL_AddCircle
  return drawCall(drawCircle, circleFunc, toint(x), toint(y), toint(r), color())
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
  if n >= 3 then rv[3] = gfx_vars.w         end
  if n >= 4 then rv[4] = gfx_vars.h         end

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

local function drawString(draw_list, screen_x, screen_y, blit_opts,
                          c, str, size, x, x_off, y, y_off, right, bottom, font)
  -- search for a new font as the draw call may have been stored for a
  -- long time in an offscreen buffer while the font instance got detached
  -- or the script may have re-created the context with gfx.quit+gfx.init
  if font.cache and not font.cache.attached then
    font.cache, font.inst = getNearestCachedFont(f)
  end

  -- keep the font alive while the draw call is still in use (eg. from a blit)
  if font.cache then
    font.cache.last_use = state.frame_count
  end

  c = transformColor(c, blit_opts)
  x, y = transformPoint(x, y, blit_opts)
  x_off, y_off = transformPoint(x_off, y_off, blit_opts)
  size = size * blit_opts.scale_y -- height only, cannot stretch width
  DL_AddTextEx(draw_list, font.inst, size,
               screen_x + x + x_off, screen_y + y + y_off, c, str, 0,
               screen_x + x, screen_y + y,
               right and screen_x + (right * blit_opts.scale_x) // 1,
               bottom and screen_y + (bottom * blit_opts.scale_y) // 1)
end

function gfx.drawstr(str, flags, right, bottom)
  if not state then return end
  str = str or FALLBACK_STRING

  local x, y, c = toint(gfx_vars.x), toint(gfx_vars.y), color()
  local w, h = gfx.measurestr(str) -- calls beginFrame()
  local f = global_state.fonts[state.font]
  local f_sz = f and f.size or DEFAULT_FONT_SIZE
  local f_cache, f_inst = getNearestCachedFont(f)
  if right  then right  = toint(right) end
  if bottom then bottom = toint(bottom) end

  gfx_vars.x = gfx_vars.x + w

  local x_off, y_off = 0, 0
  if flags and right and bottom then
    x, x_off = alignText(flags        & 3, x, w, right)
    y, y_off = alignText((flags >> 2) & 3, y, h, bottom)
    if (flags & 256) ~= 0 then right, bottom = nil, nil end -- disable clipping
  end

  -- passing f_{cache,inst} as a table to be read/writeable
  return drawCall(drawString, c, str, f_sz, x, x_off, y, y_off, right, bottom,
                  { cache = f_cache, inst = f_inst })
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
  return ImGui.IsKeyDown(state.ctx, char)
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
  image = global_state.images[toint(image)]
  if not image then return 0, 0 end
  return image.w, image.h
end

function gfx.getpixel()
  warn('not supported')
  return 0
end

local function drawGradRect(draw_list, screen_x, screen_y, blit_opts,
                            x1, y1, x2, y2, ctl, ctr, cbr, cbl)
  ctl, ctr, cbr, cbl = transformColor(ctl, blit_opts),
                       transformColor(ctr, blit_opts),
                       transformColor(cbr, blit_opts),
                       transformColor(cbl, blit_opts)
  x1, y1 = transformPoint(x1,     y1,     blit_opts)
  x2, y2 = transformPoint(x2, y2, blit_opts)
  x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
  DL_AddRectFilledMultiColor(draw_list, x1, y1, x2, y2, ctl, ctr, cbr, cbl)
end

function gfx.gradrect(x, y, w, h, r, g, b, a, drdx, dgdx, dbdx, dadx, drdy, dgdy, dbdy, dady)
  -- FIXME: support colors growing to > 1 or < 0 before the end of the rect
  x, y, w, h = toint(x), toint(y), toint(w), toint(h)
  drdx, dgdx, dbdx, dadx = w * (drdx or 0), w * (dgdx or 0),
                           w * (dbdx or 0), w * (dadx or 0)
  drdy, dgdy, dbdy, dady = h * (drdy or 0), h * (dgdy or 0),
                           h * (dbdy or 0), h * (dady or 0)
  local ctl = makeColor(r, g, b, a)
  local ctr = makeColor(r + drdx, g + dgdx, b + dbdx, a + dadx)
  local cbl = makeColor(r + drdy, g + dgdy, b + dbdy, a + dady)
  local cbr = makeColor(r + drdx + drdy, g + dgdx + dgdy,
                        b + dbdx + dbdy, a + dadx + dady)
  return drawCall(drawGradRect, x, y, x + w, y + h, ctl, ctr, cbr, cbl)
end

local function drawImGui(draw_list, screen_x, screen_y, blit_opts, callback, x, y)
  ImGui.SetCursorScreenPos(state.ctx, screen_x + x, screen_y + y)
  callback(state.ctx, draw_list, screen_x, screen_y, blit_opts)
end

function gfx.imgui(callback)
  return drawCall(drawImGui, callback, toint(gfx_vars.x), toint(gfx_vars.y))
end

function gfx.init(name, width, height, dockstate, xpos, ypos)
  local is_new = not state

  if is_new then
    local ctx_name = name
    if ctx_name:len() < 1 then ctx_name = 'gfx2imgui' end

    state = {
      name        = name,
      ctx         = ImGui.CreateContext(ctx_name, CTX_FLAGS),
      canary      = ImGui.CreateContext(ctx_name, CANARY_FLAGS),
      wnd_flags   = 1,
      collapsed   = false,
      want_close  = false,
      font        = 0,
      fontmap     = {},
      fontqueue   = {},
      frame_count = -1,
      charqueue   = { ptr=0, rptr=0, size=0, max_size=16 },
      drop_files  = {},
      mouse_cap   = 0,
    }

    ImGui.SetConfigVar(state.ctx, NO_DECORATION, 0)

    -- using pairs (not ipairs) to support gaps in requested font slots
    for _, font in pairs(global_state.fonts) do
      state.fontqueue[#state.fontqueue + 1] = font
    end

    -- always update global_state.dock with the current value
    setDock(toint(tonumber(dockstate)))

    gfx_vars.ext_retina = 1 -- ReaImGui scales automatically
  elseif name and name:len() > 0 then
    state.name = name
    return 1
  end

  if width and height then
    gfx_vars.w, gfx_vars.h = math.max(16, toint(tonumber(width))),
                             math.max(16, toint(tonumber(height)))
    state.want_size = { w=gfx_vars.w, h=gfx_vars.h }
  end

  if xpos and ypos then
    global_state.pos_x, global_state.pos_y =
      toint(tonumber(xpos)), toint(tonumber(ypos))
    state.want_pos = { x=global_state.pos_x, y=global_state.pos_y }
  end

  return 1
end

function gfx.line(x1, y1, x2, y2, aa)
  -- if aa then warn('ignoring parameter aa') end
  x1, y1, x2, y2 = toint(x1), toint(y1), toint(x2), toint(y2)

  -- gfx.line(10, 30, 10, 30)
  if x1 == x2 and y1 == y2 then
    -- faster than 1px lines according to dear imgui
    return addPixel(x1, y1, color())
  else
    return addLine(x1, y1, x2, y2, color())
  end
end

function gfx.lineto(x, y, aa)
  gfx.line(gfx_vars.x, gfx_vars.y, x, y, aa)
  gfx_vars.x, gfx_vars.y = x, y
  return x
end

local function drawImage(draw_list, screen_x, screen_y, blit_opts,
                         filename, imageState, x, y, w, h)
  if not imageState.attached then
    -- could not attach before in loadimg, as it can be called before gfx.init
    if not ImGui.ValidatePtr(imageState.inst, 'ImGui_Image*') then
      imageState.inst = ImGui.CreateImage(filename)
    end
    ImGui.Attach(state.ctx, imageState.inst)
    imageState.attached = true
  end

  w, h = transformPoint(w, h, blit_opts)
  local uv0_x, uv0_y = blit_opts.x1 / w, blit_opts.y1 / h
  local uv1_x, uv1_y = blit_opts.x2 / w, blit_opts.y2 / h

  DL_AddImage(draw_list, imageState.inst,
    screen_x + blit_opts.x1, screen_y + blit_opts.y1,
    screen_x + blit_opts.x2, screen_y + blit_opts.y2,
    uv0_x, uv0_y, uv1_x, uv1_y)
end

function gfx.loadimg(image, filename)
  image = toint(image)

  local bitmap
  if not pcall(function() bitmap = ImGui.CreateImage(filename) end) then
    return -1
  end

  local w, h = ImGui.Image_GetSize(bitmap)
  gfx.setimgdim(image, w, h)

  local imageState = global_state.images[image] -- initialized by setimgdim
  if imageState.inst and state then
    ImGui.Detach(state.ctx, imageState.inst)
  end

  imageState.inst = bitmap
  if state then
    ImGui.Attach(state.ctx, imageState.inst)
    imageState.attached = true
  else
    imageState.attached = false
  end

  local dest_backup = gfx_vars.dest
  gfx_vars.dest = image
  local commands = global_state.commands[gfx_vars.dest]
  if commands then commands.want_clear = true end
  drawCall(drawImage, filename, imageState, x, y, w, h)
  gfx_vars.dest = dest_backup

  return image
end

function gfx.measurechar(char)
  if not state then return gfx_vars.texth, gfx_vars.texth end
  return gfx.measurestr(utf8.char(char))
end

function gfx.measurestr(str)
  str = str or FALLBACK_STRING
  if not state or not beginFrame() then
    return gfx_vars.texth * utf8.len(str), gfx_vars.texth
  end
  local _, font_inst, size_error =
    getNearestCachedFont(global_state.fonts[state.font])
  local correction_factor = gfx_vars.texth / (gfx_vars.texth + size_error)
  ImGui.PushFont(state.ctx, font_inst)
  local w, h = ImGui.CalcTextSize(state.ctx, str)
  ImGui.PopFont(state.ctx)
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
  if ImGui.ValidatePtr(state.ctx, 'ImGui_Context*') then
    ImGui.DestroyContext(state.ctx)
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

local function drawRect(draw_list, screen_x, screen_y, blit_opts,
                        rectFunc, x1, y1, x2, y2, c)
  c = transformColor(c, blit_opts)
  x1, y1 = transformPoint(x1, y1, blit_opts)
  x2, y2 = transformPoint(x2, y2, blit_opts)
  -- FIXME: scale thickness
  x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
  rectFunc(draw_list, x1, y1, x2, y2, c)
end

function gfx.rect(x, y, w, h, filled)
  x, y, w, h = toint(x), toint(y), toint(w), toint(h)
  local rectFunc = tobool(filled, true) and DL_AddRectFilled or DL_AddRect
  return drawCall(drawRect, rectFunc, x, y, x + w, y + h, color())
end

function gfx.rectto(x, y)
  gfx.rect(gfx_vars.x, gfx_vars.y, x - gfx_vars.x, y - gfx_vars.y)
  gfx_vars.x, gfx_vars.y = x, y
  return x
end

local function drawRoundRect(draw_list, screen_x, screen_y, blit_opts,
                    x1, y1, x2, y2, c, radius)
  c = transformColor(c, blit_opts)
  radius = radius * blit_opts.scale_y -- FIXME: scale_x
  x1, y1 = transformPoint(x1, y1, blit_opts)
  x2, y2 = transformPoint(x2, y2, blit_opts)
  -- FIXME: scale thickness
  x1, y1, x2, y2 = screen_x + x1, screen_y + y1, screen_x + x2, screen_y + y2
  DL_AddRect(draw_list, x1, y1, x2 + 1, y2 + 1, c, radius, ROUND_CORNERS)
end

function gfx.roundrect(x, y, w, h, radius, antialias)
  -- if antialias then warn('ignoring parameter antialias') end
  x, y, w, h = toint(x), toint(y), toint(w), toint(h)
  return drawCall(drawRoundRect, x, y, x + w, y + h, color(), radius)
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

  -- write thorough gfx's metadatable for sanitization
  gfx.r, gfx.g, gfx.b = tonumber(r) or 0, tonumber(g) or 0, tonumber(b) or 0
  if n >= 4 then gfx.a    = tonumber(a)    or 0 end
  if n >= 5 then gfx.mode = tonumber(mode) or 0 end
  if n >= 6 then gfx.dest = tonumber(dest) or 0 end
  if n >= 7 then gfx.a2   = tonumber(a2)   or 0 end

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

  local dim = global_state.images[image]
  if not dim then
    dim = {}
    global_state.images[image] = dim
  end

  dim.w, dim.h = math.max(0, toint(w)), math.max(0, toint(h))

  local commands = global_state.commands[image]
  if commands and dim.w == 0 and dim.h == 0 then
    commands.want_clear = true
  end

  return 1
end

function gfx.setpixel(r, g, b)
  addPixel(gfx_vars.x, gfx_vars.y, makeColor(r, g, b, 1))
  return r
end

function gfx.showmenu(str)
  -- cannot use a ImGui menu because the host script expect gfx.showmenu to be blocking
  if not WINDOWS then return ogfx.showmenu(str) end

  -- Using hidden gfx window menu code by amagalma
  -- https://forum.cockos.com/showthread.php?t=239556
  local foreground = reaper.JS_Window_GetForeground and
                     reaper.JS_Window_GetForeground()
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

  if foreground then
    reaper.JS_Window_SetForeground(foreground)
  end

  return value
end

function gfx.transformblit()
  warn('not implemented')
  return 0
end

local function drawTriangle6(draw_list, screen_x, screen_y, blit_opts,
                             points, center_x, center_y, c)
  c = transformColor(c, blit_opts)
  local x1, y1 = transformPoint(points[1], points[2], blit_opts)
  local x2, y2 = transformPoint(points[3], points[4], blit_opts)
  local x3, y3 = transformPoint(points[5], points[6], blit_opts)
  if points[1] > center_x then x1 = x1 + 1 end
  if points[2] > center_y then y1 = y1 + 1 end
  if points[3] > center_x then x2 = x2 + 1 end
  if points[4] > center_y then y2 = y2 + 1 end
  if points[5] > center_x then x3 = x3 + 1 end
  if points[6] > center_y then y3 = y3 + 1 end
  DL_AddTriangleFilled(draw_list,
    screen_x + x1, screen_y + y1,
    screen_x + x2, screen_y + y2,
    screen_x + x3, screen_y + y3, c)
end

local function drawTriangleN(draw_list, screen_x, screen_y, blit_opts,
                             points, screen_points, n_coords,
                             center_x, center_y, c)
  c = transformColor(c, blit_opts)
  for i = 1, n_coords, 2 do
    screen_points[i], screen_points[i + 1] =
      transformPoint(points[i], points[i + 1], blit_opts)
    screen_points[i], screen_points[i + 1] =
      screen_x + screen_points[i], screen_y + screen_points[i + 1]
    if points[i]     > center_x then screen_points[i]     = screen_points[i]     + 1 end
    if points[i + 1] > center_y then screen_points[i + 1] = screen_points[i + 1] + 1 end
  end
  DL_AddConvexPolyFilled(draw_list, screen_points, c)
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
    return addPixel(points[1], points[2], c)
  elseif n_coords == 4 then
    -- gfx.triangle(0,33, 0,0, 0,33, 0,33)
    return addLine(points[1], points[2], points[3], points[4], c)
  elseif n_coords == 6 then
    return drawCall(drawTriangle6, points, center_x, center_y, c)
  else
    local screen_points = reaper.new_array(n_coords)
    return drawCall(drawTriangleN, points, screen_points, n_coords,
                    center_x, center_y, c)
  end
end

function gfx.update()
  if not state or not beginFrame() then return end

  if state.want_dock then
    ImGui.SetNextWindowDockID(state.ctx, state.want_dock)
    -- keep position and size when using gfx.init with dock=0
    if state.want_dock ~= 0 then
      state.want_pos, state.want_size = nil, nil
    end
    state.want_dock = nil
  end
  if state.want_pos then
    local x, y = ImGui.PointConvertNative(state.ctx, state.want_pos.x, state.want_pos.y)
    if MACOS then y = y - (state.want_size and state.want_size.h or gfx_vars.h) end
    ImGui.SetNextWindowPos(state.ctx, x, y)
    state.want_pos = nil
  end
  if state.want_size then
    ImGui.SetNextWindowSize(state.ctx, state.want_size.w, state.want_size.h)
    state.want_size = nil
  end

  -- start window
  local col_clear = math.max(0, gfx_vars.clear)
  local bg = (col_clear >> 8  & 0x0000ff00) |
             (col_clear << 8  & 0x00ff0000) |
             (col_clear << 24 & 0xff000000) |
             0xff
  ImGui.PushStyleColor(state.ctx, WINDOW_BG, bg)
  ImGui.PushStyleVar(state.ctx, WINDOW_PADDING, 0, 0)
  ImGui.PushStyleVar(state.ctx, CHILD_BORDER_SIZE, 0) -- no border when docked
  local wnd_label = ('%s###gfx2imgui'):format(state.name)
  local visible, open = ImGui.Begin(state.ctx, wnd_label, true, WND_FLAGS)
  state.collapsed = not visible
  ImGui.PopStyleVar(state.ctx, 2)
  ImGui.PopStyleColor(state.ctx)

  if not visible then
    for _, commands in pairs(global_state.commands) do
      commands.want_clear = true
    end
    return 0
  end

  -- update variables
  gfx_vars.w, gfx_vars.h = ImGui.GetWindowSize(state.ctx)
  state.want_close = state.want_close or not open
  state.screen_x, state.screen_y = ImGui.GetCursorScreenPos(state.ctx)
  global_state.pos_x, global_state.pos_y = state.screen_x, state.screen_y
  if MACOS then global_state.pos_y = global_state.pos_y + gfx_vars.h end
  global_state.pos_x, global_state.pos_y = ImGui.PointConvertNative(state.ctx,
    global_state.pos_x, global_state.pos_y, true)

  -- remove space taken by the window titlebar or docker tabbar
  local pos_x, pos_y = ImGui.GetWindowPos(state.ctx)
  gfx_vars.w, gfx_vars.h = gfx_vars.w - (state.screen_x - pos_x),
                           gfx_vars.h - (state.screen_y - pos_y)

  if ImGui.IsWindowDocked(state.ctx) then
    global_state.dock = 1 | (~ImGui.GetWindowDockID(state.ctx) << 8)
  else
    global_state.dock = global_state.dock & ~1 -- preserve previous docker ID
  end

  updateMouse()
  updateKeyboard()
  updateDropFiles()

  for key, _ in pairs(gfx_vars_initializers) do
    gfx_vars[key] = nil -- re-initialize at the next defer cycle (frame)
  end

  -- draw contents
  local commands = global_state.commands[-1]
  if commands and commands.rendered_frame ~= state.frame_count then
    local draw_list = ImGui.GetWindowDrawList(state.ctx)
    -- mode=nil tells transformColor it's not outputting to an offscreen buffer
    local blit_opts = {
      alpha=1, mode=nil, scale_x=1, scale_y=1,
      x1=0, y1=0, x2=gfx_vars.w, y2=gfx_vars.h,
    }
    render(commands, draw_list, state.screen_x, state.screen_y, blit_opts)

    -- Allow calling gfx.update muliple times per frame without re-rendering
    -- everything from the top. Keep the existing commands in case they aren't
    -- re-filled every frame (eg. rtk).
    -- FIXME: Flickering if some update() calls only happen in some frames.
    commands.rendered_frame = state.frame_count
  end

  if PROFILE then
    ImGui.SetCursorPos(state.ctx, 0, 0)
    if type(PROFILE) == 'number' then
      local label = ('Profiling (%d)...'):format(PROFILE - state.frame_count)
      if ImGui.Button(state.ctx, label) or PROFILE == state.frame_count then
        profiler.stop()
        profiler.report(reaper.GetResourcePath() .. '/Scripts/profiler.log')
        PROFILE = true
      end
    elseif ImGui.Button(state.ctx, 'Start profiler') then
      PROFILE = state.frame_count + 60
      profiler.start()
    end
  end

  ImGui.End(state.ctx)
  return 0
end

if DEBUG then
  local function errorHandler(status, err, ...)
    if not status then error(err) end
    return err, ...
  end

  local function wrapFuncs(list)
    for key, value in pairs(list) do
      list[key] = function(...)
        return errorHandler(xpcall(value, debug.traceback, ...))
      end
    end
  end

  wrapFuncs(gfx)
  wrapFuncs(getmetatable(gfx))
end

return gfx
