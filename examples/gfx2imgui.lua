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
--     reaper.defer(loop)
--   end
--   gfx.update()
-- end
-- reaper.defer(loop)

local reaper, ogfx, print = reaper, gfx, print
local string, table, utf8 = string, table, utf8

local FLT_MIN, FLT_MAX = reaper.ImGui_NumericLimits_Float()
local CTX_FLAGS = reaper.ImGui_ConfigFlags_NoSavedSettings() |
                  reaper.ImGui_ConfigFlags_DockingEnable()
local CANARY_FLAGS = reaper.ImGui_ConfigFlags_NoSavedSettings()
local WND_FLAGS = reaper.ImGui_WindowFlags_NoScrollbar() |
                  reaper.ImGui_WindowFlags_NoScrollWithMouse() |
                  reaper.ImGui_WindowFlags_NoMove()
local LOG_WND_FLAGS = reaper.ImGui_WindowFlags_NoDocking()
local WINDOW_PADDING, WINDOW_BG =
  reaper.ImGui_StyleVar_WindowPadding(), reaper.ImGui_Col_WindowBg()
local MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE =
  reaper.ImGui_MouseButton_Left(), reaper.ImGui_MouseButton_Right(),
  reaper.ImGui_MouseButton_Middle()
local MOD_CTRL, MOD_SHIFT, MOD_ALT, MOD_SUPER =
  reaper.ImGui_Key_ModCtrl(), reaper.ImGui_Key_ModShift(),
  reaper.ImGui_Key_ModAlt(),  reaper.ImGui_Key_ModSuper()
local ROUND_CORNERS = reaper.ImGui_DrawFlags_RoundCornersAll()
local MACOS, WINDOWS = reaper.GetOS():find('OSX') ~= nil,
                       reaper.GetOS():find('Win') == 1
local MW_TICK = 6 -- gfx.mouse_[h]wheel increments per wheel tick
local CURSORS = {
  [32512] = reaper.ImGui_MouseCursor_Arrow(),
  [32649] = reaper.ImGui_MouseCursor_Hand(),
  [32648] = reaper.ImGui_MouseCursor_NotAllowed(),
  [32646] = reaper.ImGui_MouseCursor_ResizeAll(),
  [32644] = reaper.ImGui_MouseCursor_ResizeEW(),
  [32643] = reaper.ImGui_MouseCursor_ResizeNESW(),
  [32645] = reaper.ImGui_MouseCursor_ResizeNS(),
  [32642] = reaper.ImGui_MouseCursor_ResizeNWSE(),
  [32513] = reaper.ImGui_MouseCursor_TextInput(),
}
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
local FONT_FLAGS = {
  [0]                = reaper.ImGui_FontFlags_None(),
  [string.byte('b')] = reaper.ImGui_FontFlags_Bold(),
  [string.byte('i')] = reaper.ImGui_FontFlags_Italic(),
}

local gfx, global_state, state = {}, {
  commands   = {},
  fonts      = {},
  log        = { ptr=0, size=0, max_size=64 },
  log_lines  = {},
  imgdim     = {},
  pos        = { x=0, y=0 },
  dock       = 0,
}

-- default variables
gfx.r, gfx.g, gfx.b, gfx.a, gfx.a2   = 1, 1, 1, 1, 1
gfx.w, gfx.h, gfx.x, gfx.y, gfx.mode = 0, 0, 0, 0, 0
gfx.ext_retina, gfx.dest, gfx.texth  = 0, -1, 13 -- gfx default texth is 8
gfx.mouse_x, gfx.mouse_y, gfx.clear  = 0, 0, 0
gfx.mouse_wheel, gfx.mouse_hwheel    = 0, 0
gfx.mouse_cap                        = 0

-- internal functions
local function ringInsert(buffer, value)
  buffer[buffer.ptr] = value
  buffer.ptr = (buffer.ptr + 1) % buffer.max_size
  buffer.size = math.min(buffer.size + 1, buffer.max_size)
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
    list = { ptr=0, size=0, max_size=4096 }
    global_state.commands[gfx.dest] = list
  elseif list.want_clear then
    list.size = 0
    list.ptr = 0
    list.want_clear = false
  end

  ringInsert(list, command)
end

local function render(commands, draw_list, screen_x, screen_y)
  for command in ringEnum(commands) do
    command(draw_list, screen_x, screen_y)
  end
  commands.want_clear = true
end

local function color(r, g, b, a)
  return math.floor((a or gfx.a) * 0xFF)       |
         math.floor((b or gfx.b) * 0xFF) << 8  |
         math.floor((g or gfx.g) * 0xFF) << 16 |
         math.floor((r or gfx.r) * 0xFF) << 24
end

local function updateMouse()
  gfx.mouse_cap = 0
  if reaper.ImGui_IsWindowHovered(state.ctx) then -- not over Log window
    if reaper.ImGui_IsMouseDown(state.ctx, MOUSE_LEFT)   then gfx.mouse_cap = gfx.mouse_cap | 1  end
    if reaper.ImGui_IsMouseDown(state.ctx, MOUSE_RIGHT)  then gfx.mouse_cap = gfx.mouse_cap | 2  end
    if reaper.ImGui_IsMouseDown(state.ctx, MOUSE_MIDDLE) then gfx.mouse_cap = gfx.mouse_cap | 64 end
  end
  if reaper.ImGui_IsKeyDown(state.ctx, MOD_CTRL)  then gfx.mouse_cap = gfx.mouse_cap | 4  end
  if reaper.ImGui_IsKeyDown(state.ctx, MOD_SHIFT) then gfx.mouse_cap = gfx.mouse_cap | 8  end
  if reaper.ImGui_IsKeyDown(state.ctx, MOD_ALT)   then gfx.mouse_cap = gfx.mouse_cap | 16 end
  if reaper.ImGui_IsKeyDown(state.ctx, MOD_SUPER) then gfx.mouse_cap = gfx.mouse_cap | 32 end

  if state.want_cursor and reaper.ImGui_IsWindowHovered(state.ctx) then
    reaper.ImGui_SetMouseCursor(state.ctx, state.want_cursor)
  end

  if reaper.ImGui_IsMousePosValid(state.ctx) then
    gfx.mouse_x, gfx.mouse_y = reaper.ImGui_GetMousePos(state.ctx)
    gfx.mouse_x, gfx.mouse_y = gfx.mouse_x - state.screen_x, gfx.mouse_y - state.screen_y
  end

  gfx.mouse_wheel, gfx.mouse_hwheel = reaper.ImGui_GetMouseWheel(state.ctx)
  gfx.mouse_wheel, gfx.mouse_hwheel = gfx.mouse_wheel * MW_TICK, gfx.mouse_hwheel * MW_TICK
end

local function updateKeyboard()
  -- simulate gfx's behavior of eating shortcut keys in the global scope
  reaper.ImGui_SetNextFrameWantCaptureKeyboard(state.ctx, true)

  -- flags for gfx.getchar(65536)
  state.wnd_flags = 1
  if reaper.ImGui_IsWindowFocused(state.ctx) then
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

  -- TODO ctrl/alt altering the character values
  local i = 0
  while true do
    local rv, char = reaper.ImGui_GetInputQueueCharacter(state.ctx, i)
    if not rv then break end
    ringInsert(state.charqueue, char)
    i = i + 1
  end
end

local function warn(message, ...)
  if GFX2IMGUI_NO_LOG then return end

  local funcInfo = debug.getinfo(2, 'nSl')

  -- don't print duplicate messages
  for _, line in ipairs(global_state.log_lines) do
    if line == funcInfo.currentline then return end
  end
  global_state.log_lines[#global_state.log_lines + 1] = funcInfo.currentline

  local depth, callerInfo = 3, nil
  repeat
    callerInfo = debug.getinfo(depth, 'nSl')
    depth = depth + 1
  until not callerInfo or callerInfo.source ~= funcInfo.source

  if not callerInfo or not funcInfo.short_src:match('gfx2imgui.lua$') then
    -- tail calls
    callerInfo = funcInfo
    funcInfo = { name = '<optimized out>', currentline = 0 }
  end

  message = ('gfx.%s[%d]: %s [%s@%s:%d]')
    :format(funcInfo.name, funcInfo.currentline, message:format(...),
            callerInfo.name, callerInfo.short_src, callerInfo.currentline)
  ringInsert(global_state.log, message)
  print(message)
end

local function showLog()
  reaper.ImGui_SetConfigVar(state.ctx, reaper.ImGui_ConfigVar_ViewportsNoDecoration(), 1)
  reaper.ImGui_SetNextWindowSize(state.ctx, 800, 300, reaper.ImGui_Cond_Once())
  local visible, open = reaper.ImGui_Begin(state.ctx, 'gfx2imgui [Log]', true, LOG_WND_FLAGS)
  reaper.ImGui_SetConfigVar(state.ctx, reaper.ImGui_ConfigVar_ViewportsNoDecoration(), 0)
  if not visible then return end
  local scroll_bottom = reaper.ImGui_GetScrollY(state.ctx) == reaper.ImGui_GetScrollMaxY(state.ctx)
  for line in ringEnum(global_state.log) do
    reaper.ImGui_TextWrapped(state.ctx, line)
  end
  if scroll_bottom then reaper.ImGui_SetScrollHereY(state.ctx, 1) end
  if not open then global_state.log.size = 0 end
  reaper.ImGui_End(state.ctx)
end

local function tobool(v, default)
  if default ~= nil and v == nil then return default end
  return v ~= false and v ~= 0 and v ~= nil
end

local function gfxdo(callback)
  if not WINDOWS then return callback() end

  local curx, cury = reaper.GetMousePosition()
  ogfx.init('', 0, 0, 0, curx, cury)

  if reaper.JS_Window_SetStyle then
    local window = reaper.JS_Window_GetFocus()
    local winx, winy = reaper.JS_Window_ClientToScreen(window, 0, 0)
    gfx.x = gfx.x - (winx - curx)
    gfx.y = gfx.y - (winy - cury)
    reaper.JS_Window_SetStyle(window, 'POPUP')
    reaper.JS_Window_SetOpacity(window, 'ALPHA', 0)
  end

  local value = callback()
  ogfx.quit()
  return value
end

local function setDock(v)
  global_state.dock = v -- keep original value for scripts that check it
  state.want_dock = (v & 1) == 1 and ~(v >> 1 & 0x1f) or 0
end

local function toInt(v)
  return math.floor(v or 0)
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

  return array
end

local function nearest(array, target_key)
  local best_value, best_score

  for key, value in pairs(array) do
    local score = key - target_key
    if best_score and score > best_score then break end
    best_value, best_score = value, score
  end

  return best_value, best_score or 0
end

local function getFontInstance(font)
  if not font then return nil, 0 end

  local sizes = dig(state.fontmap, font.family, font.flags)
  if not sizes then return nil, 0 end

  local match, score = sizes[font.size], 0
  if not match then
    match, score = nearest(sizes, font.size)
  end

  if match then
    match.used = true
    return match.instance, score
  end

  return nil, 0
end

local function beginFrame()
  -- protect against scripts calling gfx.update more than once per defer cycle
  -- or before the first defer timer tick
  local this_frame = reaper.ImGui_GetFrameCount(state.canary)
  if state.last_frame == this_frame then return end
  state.last_frame = this_frame

  for family, styles in pairs(state.fontmap) do
    for style, sizes in pairs(styles) do
      for size, font in pairs(sizes) do
        if font.used then
          font.used = false
        elseif not font.keep_alive then
          reaper.ImGui_DetachFont(state.ctx, font.instance)
          put(state.fontmap, family, style, size, nil)
        end
      end
    end
  end

  for _, font in ipairs(state.fontqueue) do
    if not dig(state.fontmap, font.family, font.flags, font.size) then
      local instance = reaper.ImGui_CreateFont(font.family, font.size, font.flags)
      local keep_alive = hasValue(global_state.fonts, font)
      reaper.ImGui_AttachFont(state.ctx, instance)
      put(state.fontmap, font.family, font.flags, font.size,
        { used = true, keep_alive = keep_alive, instance = instance })
    end
  end

  state.fontqueue = {}
end

-- translation functions
function gfx.arc(x, y, r, ang1, ang2, antialias)
  if antialias then warn('ignoring parameter antialias') end
  local c, quarter = color(), math.pi / 2
  x, y, ang1, ang2 = toInt(x), toInt(y), ang1 - quarter, ang2 - quarter
  drawCall(function(draw_list, screen_x, screen_y)
    local x, y = screen_x + x, screen_y + y
    reaper.ImGui_DrawList_PathArcTo(draw_list, x, y, r, ang1, ang2)
    reaper.ImGui_DrawList_PathStroke(draw_list, c)
  end)
end

function gfx.blit(source, scale, rotation, srcx, srcy, srcw, srch, destx, desty, destw, desth, rotxoffs, rotyoffs)
  local dim = global_state.imgdim[source]

  if not scale or not destw or not desth then scale = 1 end
  if not rotation then rotation = 0 end
  if not srcx  then srcx = 0 end
  if not srcy  then srcy = 0 end
  if not srcw  then srcw = dim and dim.w or 0
  else warn('ignoring parameter srcw') end
  if not srch  then srch = dim and dim.h or 0
  else warn('ignoring parameter srch') end
  if not destx then destx = gfx.x end
  if not desty then desty = gfx.y end
  if not destw then destw = srcw * scale end
  if not desth then desth = srch * scale end

  srcx, srcy, srcw, srch, destx, desty, destw, desth =
    toInt(srcx), toInt(srcy), toInt(srcw), toInt(srch),
    toInt(destx), toInt(desty), toInt(destw), toInt(desth)

  if scale    ~= 1 then warn('ignoring parameter scale')    end
  if rotation ~= 0 then warn('ignoring parameter rotation') end
  if rotxoffs      then warn('ignoring parameter rotxoffs') end
  if rotyoffs      then warn('ignoring parameter rotyoffs') end

  local sourceCommands = global_state.commands[source]
  if not sourceCommands then return warn('source buffer is empty, nothing to blit') end

  local commands = { ptr=0, size=0, max_size=sourceCommands.size }
  for c in ringEnum(sourceCommands) do
    ringInsert(commands, c)
  end

  drawCall(function(draw_list, screen_x, screen_y)
    local x, y = screen_x + destx, screen_y + desty
    reaper.ImGui_DrawList_PushClipRect(draw_list, x, y, x + destw, y + desth, true)
    render(commands, draw_list, x - srcx, y - srcy)
    reaper.ImGui_DrawList_PopClipRect(draw_list)

    sourceCommands.want_clear = true
  end)
end

function gfx.blitext()
  warn('not implemented')
end

function gfx.blurto()
  warn('not implemented')
end

function gfx.circle(x, y, r, fill, antialias)
  if antialias then warn('ignoring parameter antialias') end
  local c = color()
  x, y = toInt(x), toInt(y)
  local AddCircle = tobool(fill, true) and
    reaper.ImGui_DrawList_AddCircleFilled or reaper.ImGui_DrawList_AddCircle
  drawCall(function(draw_list, screen_x, screen_y)
    AddCircle(draw_list, screen_x + x, screen_y + y, r, c)
  end)
end

function gfx.clienttoscreen(x, y)
  if not state then return x, y end
  return global_state.pos.x + x, global_state.pos.y + y
end

function gfx.deltablit()
  warn('not implemented')
end

function gfx.dock(...)
  local args = {...}
  args[1] = tonumber(args[1])
  if args[1] < 0 then
    local n_args = select('#', ...)
    args[1] = global_state.dock
    if args[2] then args[2] = global_state.pos.x end
    if args[3] then args[3] = global_state.pos.y end
    if args[4] then args[4] = gfx.w end
    if args[5] then args[5] = gfx.h end
    return table.unpack(args)
  else
    setDock(args[1])
  end
end

function gfx.drawchar(char)
  gfx.drawstr(char)
end

function gfx.drawnumber()
  warn('not implemented')
end

function gfx.drawstr(str, flags, right, bottom)
  if not state then return end

  local x, y, c, f = toInt(gfx.x), toInt(gfx.y), color(), state.font
  local w, h = gfx.measurestr(str) -- calls beginFrame()
  if right  then right  = toInt(right) end
  if bottom then bottom = toInt(bottom) end

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
  drawCall(function(draw_list, screen_x, screen_y)
    local font_instance, font_size = getFontInstance(f), f and f.size or 0
    reaper.ImGui_DrawList_AddTextEx(draw_list, font_instance, font_size,
      screen_x + x, screen_y + y, c, str, 0, 0,
      right and right - x or nil, bottom and bottom - y or nil)
  end)
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

  beginFrame()
  return reaper.ImGui_IsKeyDown(state.ctx, char)
end

function gfx.getdropfile()
  if not state then return end
  warn('FIXME: not implemented')
end

function gfx.getfont()
  warn('not implemented')
end

function gfx.getimgdim(image)
  local dim = global_state.imgdim[image]
  if not dim then return 0, 0 end
  return dim.w, dim.h
end

function gfx.getpixel()
  warn('not implemented')
end

function gfx.gradrect(x, y, w, h, r, g, b, a, drdx, dgdx, dbdx, dadx, drdy, dgdy, dbdy, dady)
  x, y, w, h = toInt(x), toInt(y), toInt(w), toInt(h)
  local c = color(r, g, b, a)
  warn('FIXME: gradient parameters not computed')
  drawCall(function(draw_list, screen_x, screen_y)
    local x1, y1 = screen_x + x, screen_y + y
    local x2, y2 = x1 + w, y1 + h
    reaper.ImGui_DrawList_AddRectFilledMultiColor(draw_list,
      x1, y1, x2, y2, c, c, c, c)
  end)
end

function gfx.imgui(callback)
  local x, y = toInt(gfx.x), toInt(gfx.y)
  drawCall(function(draw_list, screen_x, screen_y)
    reaper.ImGui_SetCursorScreenPos(state.ctx, screen_x + x, screen_y + y)
    callback(state.ctx, draw_list, screen_x, screen_y)
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
    return
  end

  local ctx_name = name
  if ctx_name:len() < 1 then ctx_name = 'gfx2imgui' end

  state = {
    name       = name,
    ctx        = reaper.ImGui_CreateContext(ctx_name, CTX_FLAGS),
    canary     = reaper.ImGui_CreateContext(ctx_name, CANARY_FLAGS),
    wnd_flags  = 1,
    want_close = false,
    font       = nil,
    fontmap    = {},
    fontqueue  = {},
    last_frame = -1,
    charqueue  = { ptr=0, rptr=0, size=0, max_size=16 },
  }

  reaper.ImGui_SetConfigVar(state.ctx, reaper.ImGui_ConfigVar_ViewportsNoDecoration(), 0)

  if global_state.commands[0] then
    global_state.commands[0].want_clear = true
  end

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
end

function gfx.line(x1, y1, x2, y2, aa)
  if aa then warn('ignoring parameter aa') end
  local c = color()
  x1, y1, x2, y2 = toInt(x1), toInt(y1), toInt(x2), toInt(y2)
  drawCall(function(draw_list, screen_x, screen_y)
    local x1, y1 = screen_x + x1, screen_y + y1
    local x2, y2 = screen_x + x2, screen_y + y2
    reaper.ImGui_DrawList_AddLine(draw_list, x1, y1, x2, y2, c)
  end)
end

function gfx.lineto(x, y, aa)
  gfx.line(gfx.x, gfx.y, x, y, aa)
  gfx.x, gfx.y = x, y
end

function gfx.loadimg(image, filename)
  local rv = ogfx.loadimg(1, filename)
  if rv < 0 then return rv end

  local w, h = ogfx.getimgdim(1)
  gfx.setimgdim(image, w, h)

  local dest_backup = gfx.dest
  gfx.dest = image

  warn('placeholder pattern')

  drawCall(function(draw_list, screen_x, screen_y)
    reaper.ImGui_DrawList_AddRectFilled(draw_list,
      screen_x, screen_y, screen_x + w, screen_y + h, 0xFF00FFff)
  end)

  gfx.dest = dest_backup

  return rv
end

function gfx.measurechar(char)
  if not state then return 13, 13 end
  return gfx.measurestr(char)
end

function gfx.measurestr(str)
  if not state then return 13 * utf8.len(str), 13 end
  local font, size_error = getFontInstance(state.font)
  local correction_factor = gfx.texth / (gfx.texth + size_error)
  beginFrame()
  reaper.ImGui_PushFont(state.ctx, font)
  local w, h = reaper.ImGui_CalcTextSize(state.ctx, str)
  reaper.ImGui_PopFont(state.ctx)
  return w * correction_factor, h * correction_factor
end

function gfx.muladdrect()
  warn('not implemented')
end

function gfx.printf(format, ...)
  if not state then return end
  gfx.drawstr(format:format(...))
end

function gfx.quit()
  if not state then return end
  if reaper.ImGui_ValidatePtr(state.ctx, 'ImGui_Context*') then
    reaper.ImGui_DestroyContext(state.ctx)
  end
  state = nil
end

function gfx.rect(x, y, w, h, filled)
  local c = color()
  local AddRect = tobool(filled, true) and
    reaper.ImGui_DrawList_AddRectFilled or reaper.ImGui_DrawList_AddRect
  x, y, w, h = toInt(x), toInt(y), toInt(w), toInt(h)
  drawCall(function(draw_list, screen_x, screen_y)
    local x1, y1 = screen_x + x, screen_y + y
    local x2, y2 = x1 + w, y1 + h
    AddRect(draw_list, x1, y1, x2, y2, c)
  end)
end

function gfx.rectto(x, y)
  gfx.rect(gfx.x, gfx.y, x - gfx.x, y - gfx.y)
  gfx.x, gfx.y = x, y
end

function gfx.roundrect(x, y, w, h, radius, antialias)
  if antialias then warn('ignoring parameter antialias') end
  local c = color()
  x, y, w, h = toInt(x), toInt(y), toInt(w), toInt(h)
  drawCall(function(draw_list, screen_x, screen_y)
    local x1, y1 = screen_x + x, screen_y + y
    local x2, y2 = x1 + w, y1 + h
    reaper.ImGui_DrawList_AddRectFilled(draw_list, x1, y1, x2, y2, c,
      radius, ROUND_CORNERS)
  end)
end

function gfx.screentoclient(x, y)
  if not state then return x, y end
  return x - global_state.pos.x, y - global_state.pos.y
end

function gfx.set(r, ...) -- g, b, a, mode, dest, a2
  gfx.r = r
  local args = {...}
  if args[1] then gfx.g = args[1] end
  if args[2] then gfx.b = args[2] end
  if args[3] then gfx.a = args[3] end
  if args[4] then gfx.mode = args[4] end
  if args[6] then
    gfx.a2 = args[6]
    if args[5] then args.dest = args[5] end
  elseif args[5] then args.a2 = args[5] end
end

function gfx.setcursor(resource_id, custom_cursor_name)
  if not state then return end
  if custom_cursor_name then warn('ignoring parameter custom_cursor_name') end
  state.want_cursor = CURSORS[resource_id]
  if not state.want_cursor then warn("unknown cursor '%s'", resource_id) end
end

function gfx.setfont(idx, fontface, sz, flag)
  if not state then return warn('ignored setfont before init') end

  idx = tonumber(idx) -- Default_6.0_theme_adjuster.lua gives a string sometimes

  if idx > 0 and fontface then
    sz = toInt(sz)

    if sz < 1 then
      warn('requested font size is smaller than 1px, clamping')
      sz = 1
    end

    local flags = FONT_FLAGS[flag or 0]
    if not flags then
      flags = 0
      warn("unknown font flag '%s'", flag)
    end

    local font, is_new = global_state.fonts[idx], true

    if font then
      is_new = font.family ~= fontface or font.size ~= sz or font.flags ~= flags
      if is_new then
        local attached = dig(state.fontmap, font.family, font.flags, font.size)
        if attached then attached.keep_alive = false end
      end
    end

    if is_new then
      font = { family = fontface, size = sz, flags = flags }
      state.fontqueue[#state.fontqueue + 1] = font
      global_state.fonts[idx] = font
    end

    state.font = font
  else
    state.font = global_state.fonts[idx]
  end

  gfx.texth = idx ~= 0 and ((state and state.font and state.font.size) or sz) or 13
end

function gfx.setimgdim(image, w, h)
  global_state.imgdim[image] = { w=toInt(w), h=toInt(h) }
end

function gfx.setpixel(r, g, b)
  local c = color(r, g, b, 1)
  local x, y = gfx.x, gfx.y
  drawCall(function(draw_list, screen_x, screen_y)
    local x, y = screen_x + x, screen_y + y
    reaper.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + 1, y + 1, c)
  end)
end

function gfx.showmenu(str)
  -- cannot use a ImGui menu because the host script expect gfx.showmenu to be blocking
  return gfxdo(function() return ogfx.showmenu(str) end)
end

function gfx.transformblit()
  warn('not implemented')
end

function gfx.triangle(...)
  local points = {...}
  local n_points = #points
  local c = color()

  if n_points > 6 then -- convex polygon
    drawCall(function(draw_list, screen_x, screen_y)
      local points = reaper.new_array(points)
      for i = 1, n_points, 2 do
        points[i] = screen_x + math.floor(points[i])
        points[i + 1] = screen_y + math.floor(points[i + 1])
      end
      reaper.ImGui_DrawList_AddConvexPolyFilled(draw_list, points, c)
    end)
    return
  end

  drawCall(function(draw_list, screen_x, screen_y)
    reaper.ImGui_DrawList_AddTriangleFilled(draw_list,
      screen_x + math.floor(points[1]), screen_y + math.floor(points[2]),
      screen_x + math.floor(points[3]), screen_y + math.floor(points[4]),
      screen_x + math.floor(points[5]), screen_y + math.floor(points[6]), c)
  end)
end

function gfx.update()
  if not state then return end

  beginFrame()

  if global_state.log.size > 0 then showLog() end

  if state.want_dock then
    reaper.ImGui_SetNextWindowDockID(state.ctx, state.want_dock)
    state.want_dock = nil
    if reaper.ImGui_GetFrameCount(state.ctx) > 1 then
      -- keep position and size when using gfx.init with dock=0
      state.want_pos, state.want_size = nil, nil
    end
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
  reaper.ImGui_PushStyleVar(state.ctx, WINDOW_PADDING, 0, 0)
  local col_clear = math.max(0, gfx.clear)
  local bg = (col_clear >> 8  & 0x0000ff00) |
             (col_clear << 8  & 0x00ff0000) |
             (col_clear << 24 & 0xff000000) |
             0xff
  reaper.ImGui_PushStyleColor(state.ctx, WINDOW_BG, bg)
  local wnd_label = ('%s###gfx2imgui'):format(state.name)
  local visible, open = reaper.ImGui_Begin(state.ctx, wnd_label, true, WND_FLAGS)
  reaper.ImGui_PopStyleColor(state.ctx)
  reaper.ImGui_PopStyleVar(state.ctx)

  if not visible then
    for _, commands in pairs(global_state.commands) do
      commands.want_clear = true
    end
    return
  end

  -- draw contents
  state.screen_x, state.screen_y = reaper.ImGui_GetCursorScreenPos(state.ctx)
  local commands = global_state.commands[-1]
  if commands then
    local draw_list = reaper.ImGui_GetWindowDrawList(state.ctx)
    render(commands, draw_list, state.screen_x, state.screen_y)
  end

  -- update variables
  gfx.w, gfx.h = reaper.ImGui_GetWindowSize(state.ctx)
  updateMouse()
  updateKeyboard()

  state.want_close = state.want_close or not open
  global_state.pos.x, global_state.pos.y = reaper.ImGui_GetWindowPos(state.ctx)
  if MACOS then global_state.pos.y = global_state.pos.y + gfx.h end
  global_state.pos.x, global_state.pos.y = reaper.ImGui_PointConvertNative(state.ctx,
    global_state.pos.x, global_state.pos.y, true)

  if reaper.ImGui_IsWindowDocked(state.ctx) then
    -- keep extra bits set, for scripts that check it
    global_state.dock = 1 | (global_state.dock & ~0x3e) |
      (~reaper.ImGui_GetWindowDockID(state.ctx) << 1)
  else
    global_state.dock = global_state.dock & ~1 -- preserve previous docker ID
  end

  reaper.ImGui_End(state.ctx)
end

return gfx
