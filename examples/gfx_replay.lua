-- Record and replay gfx code
--
-- GFXREPLAY_RECORD = true
-- gfx = dofile('/path/to/gfx_replay.lua')

GFX2IMGUI_DEBUG               = GFX2IMGUI_DEBUG or false
GFX2IMGUI_MAX_DRAW_CALLS      = GFX2IMGUI_MAX_DRAW_CALLS or 1<<13
GFX2IMGUI_NO_BLIT_PREMULTIPLY = GFX2IMGUI_NO_BLIT_PREMULTIPLY or false
GFX2IMGUI_NO_LOG              = GFX2IMGUI_NO_LOG or false

package.path = reaper.ImGui_GetBuiltinPath() .. '/?.lua'
local ImGui = require 'imgui' '0.8.7'

local FILE_EXT = 'gfx'
local FLT_MIN = ImGui.NumericLimits_Float()
local SCRIPT_NAME = 'GFX Replay'

local ogfx, rgfx = gfx

local colors = {
  error  = 0xff3232ff,
  active = 0x32ff32ff,
}

local ctx = ImGui.CreateContext(SCRIPT_NAME)
local clipper = ImGui.CreateListClipper(ctx)
ImGui.Attach(ctx, clipper)

local record, play = GFXREPLAY_RECORD or false
local frames, sel_frame, zero_frame, next_frame = {}, 1, true
local want_close, dirty, filename, basename = false, false

local runtime_gfx, mocks = {}, {
  showmenu = function() end,
  loadimg  = function(img)
    local r, g, b, a, d =
      play.gfx.r, play.gfx.g, play.gfx.b, play.gfx.a, play.gfx.dest
    play.gfx.setimgdim(img, 1<<8, 1<<8)
    play.gfx.dest = img
    play.gfx.set(1, 0, 1, 1)
    play.gfx.rect(0, 0, 1<<8, 1<<8)
    play.gfx.set(r, g, b, a)
    play.gfx.dest = d
  end,
}

setmetatable(runtime_gfx, {
  __index = function(gfx, key)
    local mock = mocks[key]
    return mock or play.gfx[key]
  end,
  __newindex = function(gfx, key, value)
    play.gfx[key] = value
  end,
})

local function startPlay(gfx)
  if #frames < 1 then return end
  play = { gfx = gfx, frame_n = frames[1].n, frame_i = 1, start_time = os.clock() }
  if GFX2IMGUI_PROFILER then GFX2IMGUI_PROFILER.clear() end
  gfx.quit()
end

local function stopPlay()
  play.gfx.quit()
  play = nil
end

local function playFrame()
  local frame = frames[play.frame_i]
  if frame.func then
    frame.err = nil
  else
    local env = { gfx = runtime_gfx }
    frame.func, frame.err = load(frame.code, 'frame'..frame.n, 't', env)
  end

  if play.frame_n < frame.n then
    play.gfx.update()
  elseif not frame.err then
    if GFX2IMGUI_PROFILER then
      GFX2IMGUI_PROFILER.start()
    end
    local ok, err = pcall(frame.func)
    if GFX2IMGUI_PROFILER then
      GFX2IMGUI_PROFILER.stop()
      if ok then
        GFX2IMGUI_PROFILER.frame()
      else
        GFX2IMGUI_PROFILER.clear()
      end
    end
    if not ok then frame.err = err end
  end

  if frame.err then
    sel_frame = play.frame_i
    return stopPlay()
  end

  play.frame_n = play.frame_n + 1
  if play.frame_n > frame.n + frame.repeat_count then
    play.frame_i = play.frame_i + 1
  end
  if play.frame_i > #frames then
    stopPlay()
  end
end

local function setFilename(new_filename)
  filename = new_filename
  basename = filename:match('[^/\\]+$')
end

local function new()
  frames, next_frame, sel_frame, dirty = {}, nil, 1, false
  filename, basename, play, record = nil, nil, nil, false
end

local function open()
  local ok, new_filename = reaper.GetUserFileNameForRead(
    filename or '', SCRIPT_NAME, FILE_EXT)
  if not ok then return end

  local new_frames = {}
  local file, err = io.open(new_filename, 'rb')
  if not file then error(err) end
  if file:read(9) ~= 'GFXREPLAY' then error('not a GFX replay file') end
  local header_size = ('I'):unpack(file:read(4))
  file:seek('cur', header_size)
  while true do
    local frame_size = file:read(4)
    if not frame_size then break end
    frame_size = ('I'):unpack(frame_size)
    local n, repeat_count, code_size = ('IIT'):unpack(file:read(frame_size))
    local code = file:read(code_size)
    new_frames[#new_frames + 1] = { n = n, repeat_count = repeat_count, code = code }
  end
  file:close()

  new()
  setFilename(new_filename)
  frames = new_frames
end

local function save(prompt)
  if not filename or prompt then
    local ok, new_filename = reaper.JS_Dialog_BrowseForSaveFile(SCRIPT_NAME, '', filename, ('%s files\0*.%s\0\0'):format(SCRIPT_NAME, FILE_EXT))
    if not ok or new_filename:len() < 1 then return end

    setFilename(new_filename)
  end

  local file, err = io.open(filename, 'wb')
  if not file then error(err) end
  file:write('GFXREPLAY')
  file:write(('I'):pack(0)) -- no header for now
  for i, frame in ipairs(frames) do
    local frame_fmt = 'IIT'
    file:write(('I'):pack(frame_fmt:packsize()))
    file:write((frame_fmt):pack(frame.n, frame.repeat_count, #frame.code))
    file:write(frame.code)
  end
  file:close()

  dirty = false
end

local function loadProfiler()
  local file = reaper.GetResourcePath() .. '/Scripts/ReaTeam Scripts/Development/cfillion_Lua profiler.lua'
  if not reaper.file_exists(file) then
    reaper.MB("cfillion's Lua profiler is not installed. Install it using ReaPack to enable this feature.", SCRIPT_NAME, 0)
    return
  end
  GFX2IMGUI_PROFILER = dofile(file)
  GFX2IMGUI_PROFILER.attachTo('gfx')
end

local function unloadProfiler()
  GFX2IMGUI_PROFILER.detachFromWorld()
  GFX2IMGUI_PROFILER = nil
end

local function menuBar()
  if not ImGui.BeginMenuBar(ctx) then return end
  if ImGui.BeginMenu(ctx, 'File') then
    if ImGui.MenuItem(ctx, 'New') then new() end
    if ImGui.MenuItem(ctx, 'Open...') then open() end
    if ImGui.MenuItem(ctx, 'Save', nil, nil, dirty) then save(false) end
    if ImGui.MenuItem(ctx, 'Save as...') then save(true) end
    if ImGui.MenuItem(ctx, 'Close') then want_close = true end
    ImGui.EndMenu(ctx)
  end
  if ImGui.BeginMenu(ctx, 'Options') then
    if ImGui.MenuItem(ctx, 'Enable profiler', nil, GFX2IMGUI_PROFILER ~= nil) then
      if GFX2IMGUI_PROFILER then unloadProfiler() else loadProfiler() end
    end
    ImGui.Separator(ctx)
    ImGui.MenuItem(ctx, 'Disable gfx.showmenu', nil, true, false)
    ImGui.MenuItem(ctx, 'Disable gfx.loadimg', nil, true, false)
    ImGui.Separator(ctx)
    local gfx2imgui_opts = {
      'DEBUG', 'MAX_DRAW_CALLS', 'NO_BLIT_PREMULTIPLY', 'NO_LOG',
    }
    for i, opt in ipairs(gfx2imgui_opts) do
      local var = ('GFX2IMGUI_%s'):format(opt)
      if type(_G[var]) == 'number' then
        if ImGui.BeginMenu(ctx, var) then
          _G[var] = select(2, ImGui.SliderInt(ctx, '##'..opt, _G[var],
            1<<10, 1<<18, nil, ImGui.SliderFlags_Logarithmic))
          ImGui.EndMenu(ctx)
        end
      else
        _G[var] = select(2, ImGui.MenuItem(ctx, var, nil, _G[var]))
      end
    end
    ImGui.EndMenu(ctx)
  end
  ImGui.EndMenuBar(ctx)
end

local function dots()
  ImGui.SameLine(ctx, nil, 0)
  ImGui.Text(ctx, ('%-3s'):format(('.'):rep(ImGui.GetTime(ctx) // 1 % 3 + 1)))
end

local function statusBar()
  local last_frame = frames[#frames]
  local last_frame_n = last_frame and last_frame.n + last_frame.repeat_count + frames[1].n - 1 or 0

  ImGui.AlignTextToFramePadding(ctx)
  ImGui.Text(ctx, ('%04.01f FPS | Length: %.01fs | Status:'):format(ImGui.GetFramerate(ctx), last_frame_n * 0.033))
  if record then
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, 'Recording')
    dots()
    ImGui.SameLine(ctx)
    if ImGui.Button(ctx, 'Stop') then record = false end
  elseif play then
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, 'Playing')
    dots()
    ImGui.SameLine(ctx)
    if ImGui.Button(ctx, 'Stop') then return stopPlay() end
    ImGui.SameLine(ctx)
    ImGui.ProgressBar(ctx, play.frame_n / last_frame_n, nil, nil,
      ('%d / %d'):format(play.frame_n, last_frame_n))
  else
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, 'Ready')
    ImGui.SameLine(ctx)
    if ImGui.Button(ctx, 'Play with gfx') then startPlay(ogfx) end
    ImGui.SameLine(ctx)
    if ImGui.Button(ctx, 'Play with gfx2imgui') then
      startPlay(dofile(reaper.GetResourcePath() ..
        '/Scripts/ReaTeam Extensions/API/gfx2imgui.lua'))
    end
  end
end

local function frameList()
  if not ImGui.BeginListBox(ctx, '##frames', 150, -FLT_MIN) then return end
  local scroll_bottom = ImGui.GetScrollY(ctx) == ImGui.GetScrollMaxY(ctx)
  ImGui.ListClipper_Begin(clipper, #frames)
  while ImGui.ListClipper_Step(clipper) do
    local display_start, display_end = ImGui.ListClipper_GetDisplayRange(clipper)
    for i = display_start + 1, display_end do
      local frame = frames[i]
      local label = ('Frame %d'):format(frame.n)
      if frame.repeat_count > 1 then
        label = label .. '-' .. (frame.n + frame.repeat_count - 1)
      end
      local pop_color = false
      if frame.err then
        ImGui.PushStyleColor(ctx, ImGui.Col_Text, colors.error)
        pop_color = true
      elseif play and play.frame_i == i then
        ImGui.PushStyleColor(ctx, ImGui.Col_Text, colors.active)
        pop_color = true
      end
      if ImGui.Selectable(ctx, label, i == sel_frame) then
        sel_frame = i
      end
      if pop_color then
        ImGui.PopStyleColor(ctx)
      end
    end
  end
  if scroll_bottom then ImGui.SetScrollHereY(ctx) end
  ImGui.EndListBox(ctx)
end

local function editFrame(frame)
  ImGui.BeginGroup(ctx)

  if frame.err then
    ImGui.PushStyleColor(ctx, ImGui.Col_Text, colors.error)
    ImGui.TextWrapped(ctx, frame.err)
    ImGui.PopStyleColor(ctx)
  end

  local changed
  changed, frame.code = ImGui.InputTextMultiline(ctx, '##code', frame.code, -FLT_MIN, -FLT_MIN)
  if changed then dirty, frame.func = true, nil end

  ImGui.EndGroup(ctx)
end

local function profiler()
  if not GFX2IMGUI_PROFILER then
    ImGui.Text(ctx, 'The profiler is disabled. Profiling adds some CPU overhead.')
    if ImGui.Button(ctx, 'Enable profiler') then
      loadProfiler()
    end
    return
  end

  GFX2IMGUI_PROFILER.showProfile(ctx, 'profile', -FLT_MIN, -FLT_MIN)
end

local function loop()
  zero_frame = false

  if play then playFrame() end

  local title = basename and ('%s - %s'):format(basename, SCRIPT_NAME) or SCRIPT_NAME
  local label = ('%s###main_window'):format(title)
  local flags = ImGui.WindowFlags_MenuBar
  if dirty then flags = flags | ImGui.WindowFlags_UnsavedDocument end
  ImGui.SetNextWindowSize(ctx, 800, 500, ImGui.Cond_FirstUseEver)
  local visible, open = ImGui.Begin(ctx, label, true, flags)
  if visible then
    menuBar()
    statusBar()

    if ImGui.BeginTabBar(ctx, 'main_tabs') then
      if ImGui.BeginTabItem(ctx, 'Code') then
        frameList()
        ImGui.SameLine(ctx)
        local frame = frames[sel_frame]
        if frame then editFrame(frame) end
        ImGui.EndTabItem(ctx)
      end
      if ImGui.BeginTabItem(ctx, 'Profiler') then
        profiler()
        ImGui.EndTabItem(ctx)
      end
      ImGui.EndTabBar(ctx)
    end

    ImGui.End(ctx)
  end
  if open and not want_close then
    reaper.defer(loop)
  end
end

reaper.defer(loop)

local function write(line)
  if not record then return end

  local frame_n = zero_frame and 0 or ImGui.GetFrameCount(ctx)
  if not next_frame then
    next_frame = {}
  elseif next_frame.n ~= frame_n then
    local code = table.concat(next_frame, '\n')
    local prev_frame = frames[#frames]
    if prev_frame and prev_frame.code == code then
      prev_frame.repeat_count = prev_frame.repeat_count + 1
    else
      frames[#frames + 1] = {
        n = next_frame.n,
        code = table.concat(next_frame, '\n'),
        repeat_count = 1,
      }
    end
    next_frame = {}
    dirty = true
  end

  next_frame.n = frame_n
  next_frame[#next_frame + 1] = line
end

local function logCall(func, ...)
  local line = ('gfx.%s('):format(func)
  for i = 1, select('#', ...) do
    if i > 1 then line = line .. ', ' end
    line = ('%s%q'):format(line, select(i, ...))
  end
  line = line .. ')'
  write(line)
end

local function logWrite(key, value)
  write(('gfx.%s = %q'):format(key, value))
end

local gfx = {}

for key, value in pairs(ogfx) do
  gfx[key] = function(...)
    logCall(key, ...)
    return value(...)
  end
end

setmetatable(gfx, {
  __index = function(gfx, key)
    return ogfx[key]
  end,
  __newindex = function(gfx, key, value)
    logWrite(key, value)
    ogfx[key] = value
  end,
})

return gfx
