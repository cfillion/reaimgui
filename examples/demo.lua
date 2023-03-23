-- Lua/ReaImGui port of Dear ImGui's C++ demo code (v1.89.4)

--[[
This file can be imported in other scripts to help during development:

local demo = dofile(reaper.GetResourcePath() .. '/Scripts/ReaTeam Extensions/API/ReaImGui_Demo.lua')
local ctx = reaper.ImGui_CreateContext('My script')
local function loop()
  demo.PushStyle(ctx)
  demo.ShowDemoWindow(ctx)
  if reaper.ImGui_Begin(ctx, 'Dear ImGui Style Editor') then
    demo.ShowStyleEditor(ctx)
    reaper.ImGui_End(ctx)
  end
  demo.PopStyle(ctx)
  reaper.defer(loop)
end
reaper.defer(loop)
--]]

--[[
Index of this file:

// [SECTION] Helpers
// [SECTION] Demo Window / ShowDemoWindow()
// - ShowDemoWindow()
// - sub section: ShowDemoWindowWidgets()
// - sub section: ShowDemoWindowLayout()
// - sub section: ShowDemoWindowPopups()
// - sub section: ShowDemoWindowTables()
// - sub section: ShowDemoWindowInputs()
-- [SECTION] Style Editor / ShowStyleEditor()
-- [SECTION] User Guide / ShowUserGuide()
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
// [SECTION] Example App: Manipulating window titles / ShowExampleAppWindowTitles()
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
--]]

local ImGui = {}
for name, func in pairs(reaper) do
  name = name:match('^ImGui_(.+)$')
  if name then ImGui[name] = func end
end

local ctx
local FLT_MIN, FLT_MAX = ImGui.NumericLimits_Float()
local IMGUI_VERSION, IMGUI_VERSION_NUM, REAIMGUI_VERSION = ImGui.GetVersion()
local demo = {
  open = true,

  menu = {
    enabled = true,
    f = 0.5,
    n = 0,
    b = true,
  },

  -- Window flags (accessible from the "Configuration" section)
  no_titlebar       = false,
  no_scrollbar      = false,
  no_menu           = false,
  no_move           = false,
  no_resize         = false,
  no_collapse       = false,
  no_close          = false,
  no_nav            = false,
  no_background     = false,
  -- no_bring_to_front = false,
  unsaved_document  = false,
  no_docking        = false,
}
local show_app = {
  -- Examples Apps (accessible from the "Examples" menu)
  -- main_menu_bar      = false,
  -- dockspace          = false,
  documents          = false,
  console            = false,
  log                = false,
  layout             = false,
  property_editor    = false,
  long_text          = false,
  auto_resize        = false,
  constrained_resize = false,
  simple_overlay     = false,
  fullscreen         = false,
  window_titles      = false,
  custom_rendering   = false,

  -- Dear ImGui Tools/Apps (accessible from the "Tools" menu)
  metrics      = false,
  debug_log    = false,
  stack_tool   = false,
  style_editor = false,
  about        = false,
}

local config  = {}
local widgets = {}
local layout  = {}
local popups  = {}
local tables  = {}
local misc    = {}
local app     = {}
local cache   = {}

function demo.loop()
  demo.PushStyle()
  demo.open = demo.ShowDemoWindow(true)
  demo.PopStyle()

  if demo.open then
    reaper.defer(demo.loop)
  end
end

if select(2, reaper.get_action_context()) == debug.getinfo(1, 'S').source:sub(2) then
  -- show global storage in the IDE for convenience
  _G.demo    = demo
  _G.widgets = widgets
  _G.layout  = layout
  _G.popups  = popups
  _G.tables  = tables
  _G.misc    = misc
  _G.app     = app

  -- hajime!
  ctx = ImGui.CreateContext('ReaImGui Demo', ImGui.ConfigFlags_DockingEnable())
  reaper.defer(demo.loop)
end

-------------------------------------------------------------------------------
-- [SECTION] Helpers
-------------------------------------------------------------------------------

-- Helper to display a little (?) mark which shows a tooltip when hovered.
-- In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
function demo.HelpMarker(desc)
  ImGui.TextDisabled(ctx, '(?)')
  if ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_DelayShort()) and ImGui.BeginTooltip(ctx) then
    ImGui.PushTextWrapPos(ctx, ImGui.GetFontSize(ctx) * 35.0)
    ImGui.Text(ctx, desc)
    ImGui.PopTextWrapPos(ctx)
    ImGui.EndTooltip(ctx)
  end
end

function demo.RgbaToArgb(rgba)
  return (rgba >> 8 & 0x00FFFFFF) | (rgba << 24 & 0xFF000000)
end

function demo.ArgbToRgba(argb)
  return (argb << 8 & 0xFFFFFF00) | (argb >> 24 & 0xFF)
end

function demo.round(n)
  return math.floor(n + .5)
end

function demo.clamp(v, mn, mx)
  if v < mn then return mn end
  if v > mx then return mx end
  return v
end

function demo.Link(url)
  if not reaper.CF_ShellExecute then
    ImGui.Text(ctx, url)
    return
  end

  local color = ImGui.GetStyleColor(ctx, ImGui.Col_CheckMark())
  ImGui.TextColored(ctx, color, url)
  if ImGui.IsItemClicked(ctx) then
    reaper.CF_ShellExecute(url)
  elseif ImGui.IsItemHovered(ctx) then
    ImGui.SetMouseCursor(ctx, ImGui.MouseCursor_Hand())
  end
end

function demo.HSV(h, s, v, a)
  local r, g, b = ImGui.ColorConvertHSVtoRGB(h, s, v)
  return ImGui.ColorConvertDouble4ToU32(r, g, b, a or 1.0)
end

function demo.EachEnum(enum)
  local enum_cache = cache[enum]
  if not enum_cache then
    enum_cache = {}
    cache[enum] = enum_cache

    for func_name, func in pairs(reaper) do
      local enum_name = func_name:match(('^ImGui_%s_(.+)$'):format(enum))
      if enum_name then
        table.insert(enum_cache, { func(), enum_name })
      end
    end
    table.sort(enum_cache, function(a, b) return a[1] < b[1] end)
  end

  local i = 0
  return function()
    i = i + 1
    if not enum_cache[i] then return end
    return table.unpack(enum_cache[i])
  end
end

function demo.DockName(dock_id)
  if dock_id == 0 then
    return 'Floating'
  elseif dock_id > 0 then
    return ('ImGui docker %d'):format(dock_id)
  end

  -- reaper.DockGetPosition was added in v6.02
  local positions = {
    [0]='Bottom', [1]='Left', [2]='Top', [3]='Right', [4]='Floating'
  }
  local position = reaper.DockGetPosition and
    positions[reaper.DockGetPosition(~dock_id)] or 'Unknown'
  return ('REAPER docker %d (%s)'):format(-dock_id, position)
end

-------------------------------------------------------------------------------
-- [SECTION] Demo Window / ShowDemoWindow()
-------------------------------------------------------------------------------
-- - ShowDemoWindowWidgets()
-- - ShowDemoWindowLayout()
-- - ShowDemoWindowPopups()
-- - ShowDemoWindowTables()
-- - ShowDemoWindowColumns()
-- - ShowDemoWindowInputs()
-------------------------------------------------------------------------------

-- Demonstrate most Dear ImGui features (this is big function!)
-- You may execute this function to experiment with the UI and understand what it does.
-- You may then search for keywords in the code when you are interested by a specific feature.
function demo.ShowDemoWindow(open)
  local rv = nil

  -- if show_app.main_menu_bar      then                               demo.ShowExampleAppMainMenuBar()       end
  -- if show_app.dockspace          then show_app.dockspace          = demo.ShowExampleAppDockSpace()         end -- Process the Docking app first, as explicit DockSpace() nodes needs to be submitted early (read comments near the DockSpace function)
  if show_app.documents          then show_app.documents          = demo.ShowExampleAppDocuments()         end -- Process the Document app next, as it may also use a DockSpace()
  if show_app.console            then show_app.console            = demo.ShowExampleAppConsole()           end
  if show_app.log                then show_app.log                = demo.ShowExampleAppLog()               end
  if show_app.layout             then show_app.layout             = demo.ShowExampleAppLayout()            end
  if show_app.property_editor    then show_app.property_editor    = demo.ShowExampleAppPropertyEditor()    end
  if show_app.long_text          then show_app.long_text          = demo.ShowExampleAppLongText()          end
  if show_app.auto_resize        then show_app.auto_resize        = demo.ShowExampleAppAutoResize()        end
  if show_app.constrained_resize then show_app.constrained_resize = demo.ShowExampleAppConstrainedResize() end
  if show_app.simple_overlay     then show_app.simple_overlay     = demo.ShowExampleAppSimpleOverlay()     end
  if show_app.fullscreen         then show_app.fullscreen         = demo.ShowExampleAppFullscreen()        end
  if show_app.window_titles      then                               demo.ShowExampleAppWindowTitles()      end
  if show_app.custom_rendering   then show_app.custom_rendering   = demo.ShowExampleAppCustomRendering()   end

  if show_app.metrics    then show_app.metrics    = ImGui.ShowMetricsWindow(ctx,   show_app.metrics)    end
  if show_app.debug_log  then show_app.debug_log  = ImGui.ShowDebugLogWindow(ctx,  show_app.debug_log)  end
  if show_app.stack_tool then show_app.stack_tool = ImGui.ShowStackToolWindow(ctx, show_app.stack_tool) end
  if show_app.about      then show_app.about      = ImGui.ShowAboutWindow(ctx,     show_app.about)      end
  if show_app.style_editor then
    rv, show_app.style_editor = ImGui.Begin(ctx, 'Dear ImGui Style Editor', true)
    if rv then
      demo.ShowStyleEditor()
      ImGui.End(ctx)
    end
  end

  -- Demonstrate the various window flags. Typically you would just use the default!
  local window_flags = ImGui.WindowFlags_None()
  if demo.no_titlebar       then window_flags = window_flags | ImGui.WindowFlags_NoTitleBar()            end
  if demo.no_scrollbar      then window_flags = window_flags | ImGui.WindowFlags_NoScrollbar()           end
  if not demo.no_menu       then window_flags = window_flags | ImGui.WindowFlags_MenuBar()               end
  if demo.no_move           then window_flags = window_flags | ImGui.WindowFlags_NoMove()                end
  if demo.no_resize         then window_flags = window_flags | ImGui.WindowFlags_NoResize()              end
  if demo.no_collapse       then window_flags = window_flags | ImGui.WindowFlags_NoCollapse()            end
  if demo.no_nav            then window_flags = window_flags | ImGui.WindowFlags_NoNav()                 end
  if demo.no_background     then window_flags = window_flags | ImGui.WindowFlags_NoBackground()          end
  -- if demo.no_bring_to_front then window_flags = window_flags | ImGui.WindowFlags_NoBringToFrontOnFocus() end
  if demo.no_docking        then window_flags = window_flags | ImGui.WindowFlags_NoDocking()             end
  if demo.topmost           then window_flags = window_flags | ImGui.WindowFlags_TopMost()               end
  if demo.unsaved_document  then window_flags = window_flags | ImGui.WindowFlags_UnsavedDocument()       end
  if demo.no_close          then open = false end -- disable the close button

  -- We specify a default position/size in case there's no data in the .ini file.
  -- We only do it to make the demo applications a little more welcoming, but typically this isn't required.
  local main_viewport = ImGui.GetMainViewport(ctx)
  local work_pos = {ImGui.Viewport_GetWorkPos(main_viewport)}
  ImGui.SetNextWindowPos(ctx, work_pos[1] + 20, work_pos[2] + 20, ImGui.Cond_FirstUseEver())
  ImGui.SetNextWindowSize(ctx, 550, 680, ImGui.Cond_FirstUseEver())

  if demo.set_dock_id then
    ImGui.SetNextWindowDockID(ctx, demo.set_dock_id)
    demo.set_dock_id = nil
  end

  -- Main body of the Demo window starts here.
  rv,open = ImGui.Begin(ctx, 'Dear ImGui Demo', open, window_flags)
  -- Early out if the window is collapsed
  if not rv then return open end

  -- Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.

  -- e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)
  --ImGui.PushItemWidth(ctx, -ImGui.GetWindowWidth(ctx) * 0.35)

  -- e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
  ImGui.PushItemWidth(ctx, ImGui.GetFontSize(ctx) * -12)

  -- Menu Bar
  if ImGui.BeginMenuBar(ctx) then
    if ImGui.BeginMenu(ctx, 'Menu') then
      demo.ShowExampleMenuFile()
      ImGui.EndMenu(ctx)
    end
    if ImGui.BeginMenu(ctx, 'Examples') then
      -- rv,show_app.main_menu_bar =
      --   ImGui.MenuItem(ctx, 'Main menu bar', nil, show_app.main_menu_bar)
      rv,show_app.console =
        ImGui.MenuItem(ctx, 'Console', nil, show_app.console)
      rv,show_app.log =
        ImGui.MenuItem(ctx, 'Log', nil, show_app.log)
      rv,show_app.layout =
        ImGui.MenuItem(ctx, 'Simple layout', nil, show_app.layout)
      rv,show_app.property_editor =
        ImGui.MenuItem(ctx, 'Property editor', nil, show_app.property_editor)
      rv,show_app.long_text =
        ImGui.MenuItem(ctx, 'Long text display', nil, show_app.long_text)
      rv,show_app.auto_resize =
        ImGui.MenuItem(ctx, 'Auto-resizing window', nil, show_app.auto_resize)
      rv,show_app.constrained_resize =
        ImGui.MenuItem(ctx, 'Constrained-resizing window', nil, show_app.constrained_resize)
      rv,show_app.simple_overlay =
        ImGui.MenuItem(ctx, 'Simple overlay', nil, show_app.simple_overlay)
      rv,show_app.fullscreen =
        ImGui.MenuItem(ctx, 'Fullscreen window', nil, show_app.fullscreen)
      rv,show_app.window_titles =
        ImGui.MenuItem(ctx, 'Manipulating window titles', nil, show_app.window_titles)
      rv,show_app.custom_rendering =
        ImGui.MenuItem(ctx, 'Custom rendering', nil, show_app.custom_rendering)
      -- rv,show_app.dockspace =
      --   ImGui.MenuItem(ctx, 'Dockspace', nil, show_app.dockspace, false)
      rv,show_app.documents =
        ImGui.MenuItem(ctx, 'Documents', nil, show_app.documents, false)
      ImGui.EndMenu(ctx)
    end
    -- if ImGui.MenuItem(ctx, 'MenuItem') then end -- You can also use MenuItem() inside a menu bar!
    if ImGui.BeginMenu(ctx, 'Tools') then
      rv,show_app.metrics      = ImGui.MenuItem(ctx, 'Metrics/Debugger', nil, show_app.metrics)
      rv,show_app.debug_log    = ImGui.MenuItem(ctx, 'Debug Log',        nil, show_app.debug_log)
      rv,show_app.stack_tool   = ImGui.MenuItem(ctx, 'Stack Tool',       nil, show_app.stack_tool)
      rv,show_app.style_editor = ImGui.MenuItem(ctx, 'Style Editor',     nil, show_app.style_editor)
      rv,show_app.about        = ImGui.MenuItem(ctx, 'About Dear ImGui', nil, show_app.about)
      ImGui.EndMenu(ctx)
    end
    if ImGui.SmallButton(ctx, 'Documentation') then
      local doc = ('%s/Data/reaper_imgui_doc.html'):format(reaper.GetResourcePath())
      if reaper.CF_ShellExecute then
        reaper.CF_ShellExecute(doc)
      else
        reaper.MB(doc, 'ReaImGui Documentation', 0)
      end
    end
    ImGui.EndMenuBar(ctx)
  end

  ImGui.Text(ctx, ('dear imgui says hello. (%s) (%d) (ReaImGui %s)'):format(IMGUI_VERSION, IMGUI_VERSION_NUM, REAIMGUI_VERSION))
  ImGui.Spacing(ctx)

  if ImGui.CollapsingHeader(ctx, 'Help') then
    ImGui.Text(ctx, 'ABOUT THIS DEMO:')
    ImGui.BulletText(ctx, 'Sections below are demonstrating many aspects of the library.')
    ImGui.BulletText(ctx, 'The "Examples" menu above leads to more demo contents.')
    ImGui.BulletText(ctx, 'The "Tools" menu above gives access to: About Box, Style Editor,\n' ..
                            'and Metrics/Debugger (general purpose Dear ImGui debugging tool).')
    ImGui.Separator(ctx)

    ImGui.Text(ctx, 'PROGRAMMER GUIDE:')
    ImGui.BulletText(ctx, 'See the ShowDemoWindow() code in ReaImGui_Demo.lua. <- you are here!')
    -- ImGui.BulletText(ctx, 'See comments in imgui.cpp.')
    ImGui.BulletText(ctx, 'See example scripts in the examples/ folder.')
    ImGui.Indent(ctx); demo.Link('https://github.com/cfillion/reaimgui/tree/master/examples'); ImGui.Unindent(ctx)
    ImGui.BulletText(ctx, 'Read the FAQ at '); ImGui.SameLine(ctx, 0, 0); demo.Link('https://www.dearimgui.org/faq/')
    -- ImGui.BulletText(ctx, "Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.")
    -- ImGui.BulletText(ctx, "Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.")
    ImGui.Separator(ctx)

    ImGui.Text(ctx, 'USER GUIDE:')
    demo.ShowUserGuide()
  end

  if ImGui.CollapsingHeader(ctx, 'Configuration') then
    if ImGui.TreeNode(ctx, 'Configuration##2') then
      local function configVarCheckbox(name)
        local var = assert(reaper[('ImGui_%s'):format(name)], 'unknown var')()
        local rv,val = ImGui.Checkbox(ctx, name, ImGui.GetConfigVar(ctx, var))
        if rv then ImGui.SetConfigVar(ctx, var, val and 1 or 0) end
      end
      config.flags = ImGui.GetConfigVar(ctx, ImGui.ConfigVar_Flags())

      ImGui.SeparatorText(ctx, 'General')
      rv,config.flags = ImGui.CheckboxFlags(ctx, 'ConfigFlags_NavEnableKeyboard', config.flags, ImGui.ConfigFlags_NavEnableKeyboard())
      ImGui.SameLine(ctx); demo.HelpMarker('Enable keyboard controls.')
      -- ImGui.CheckboxFlags("io.ConfigFlags: NavEnableGamepad",     &io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad)
      -- ImGui.SameLine(ctx); demo.HelpMarker("Enable gamepad controls. Require backend to set io.BackendFlags |= ImGuiBackendFlags_HasGamepad.\n\nRead instructions in imgui.cpp for details.")
      rv,config.flags = ImGui.CheckboxFlags(ctx, 'ConfigFlags_NavEnableSetMousePos', config.flags, ImGui.ConfigFlags_NavEnableSetMousePos())
      ImGui.SameLine(ctx); demo.HelpMarker('Instruct navigation to move the mouse cursor.')
      rv,config.flags = ImGui.CheckboxFlags(ctx, 'ConfigFlags_NoMouse', config.flags, ImGui.ConfigFlags_NoMouse())
      if (config.flags & ImGui.ConfigFlags_NoMouse()) ~= 0 then
        -- The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to fix it:
        if ImGui.GetTime(ctx) % 0.40 < 0.20 then
          ImGui.SameLine(ctx)
          ImGui.Text(ctx, '<<PRESS SPACE TO DISABLE>>')
        end
        if ImGui.IsKeyPressed(ctx, ImGui.Key_Space()) then
          config.flags = config.flags & ~ImGui.ConfigFlags_NoMouse()
        end
      end
      rv,config.flags = ImGui.CheckboxFlags(ctx, 'ConfigFlags_NoMouseCursorChange', config.flags, ImGui.ConfigFlags_NoMouseCursorChange())
      ImGui.SameLine(ctx); demo.HelpMarker('Instruct backend to not alter mouse cursor shape and visibility.')
      rv,config.flags = ImGui.CheckboxFlags(ctx, 'ConfigFlags_NoSavedSettings', config.flags, ImGui.ConfigFlags_NoSavedSettings())
      ImGui.SameLine(ctx); demo.HelpMarker('Globally disable loading and saving state to an .ini file')

      rv,config.flags = ImGui.CheckboxFlags(ctx, 'ConfigFlags_DockingEnable', config.flags, ImGui.ConfigFlags_DockingEnable())
      ImGui.SameLine(ctx)
      if ImGui.GetConfigVar(ctx, ImGui.ConfigVar_DockingWithShift()) then
        demo.HelpMarker('Drag from window title bar or their tab to dock/undock. Hold SHIFT to enable docking.\n\nDrag from window menu button (upper-left button) to undock an entire node (all windows).')
      else
          demo.HelpMarker('Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking.\n\nDrag from window menu button (upper-left button) to undock an entire node (all windows).')
      end
      if config.flags & ImGui.ConfigFlags_DockingEnable() ~= 0 then
        ImGui.Indent(ctx)
        configVarCheckbox('ConfigVar_DockingNoSplit')
        ImGui.SameLine(ctx); demo.HelpMarker('Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.')
        configVarCheckbox('ConfigVar_DockingWithShift')
        ImGui.SameLine(ctx); demo.HelpMarker('Enable docking when holding Shift only (allow to drop in wider space, reduce visual noise)')
        -- ImGui.Checkbox(ctx, 'io.ConfigDockingAlwaysTabBar', &io.ConfigDockingAlwaysTabBar)
        -- ImGui.SameLine(ctx); demo.HelpMarker('Create a docking node and tab-bar on single floating windows.')
        configVarCheckbox('ConfigVar_DockingTransparentPayload')
        ImGui.SameLine(ctx); demo.HelpMarker('Make window or viewport transparent when docking and only display docking boxes on the target viewport.')
        ImGui.Unindent(ctx)
      end

      -- ImGui::CheckboxFlags("io.ConfigFlags: ViewportsEnable", &io.ConfigFlags, ImGuiConfigFlags_ViewportsEnable);
      -- ImGui::SameLine(); HelpMarker("[beta] Enable beta multi-viewports support. See ImGuiPlatformIO for details.");
      -- if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
      -- {
      --     ImGui::Indent();
      --     ImGui::Checkbox("io.ConfigViewportsNoAutoMerge", &io.ConfigViewportsNoAutoMerge);
      --     ImGui::SameLine(); HelpMarker("Set to make all floating imgui windows always create their own viewport. Otherwise, they are merged into the main host viewports when overlapping it.");
      --     ImGui::Checkbox("io.ConfigViewportsNoTaskBarIcon", &io.ConfigViewportsNoTaskBarIcon);
      --     ImGui::SameLine(); HelpMarker("Toggling this at runtime is normally unsupported (most platform backends won't refresh the task bar icon state right away).");
      configVarCheckbox('ConfigVar_ViewportsNoDecoration')
      --     ImGui::Checkbox("io.ConfigViewportsNoDefaultParent", &io.ConfigViewportsNoDefaultParent);
      --     ImGui::SameLine(); HelpMarker("Toggling this at runtime is normally unsupported (most platform backends won't refresh the parenting right away).");
      --     ImGui::Unindent();
      -- }

      configVarCheckbox('ConfigVar_InputTrickleEventQueue')
      ImGui.SameLine(ctx); demo.HelpMarker('Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.')
      -- ImGui.Checkbox(ctx, 'io.MouseDrawCursor', &io.MouseDrawCursor)
      -- ImGui.SameLine(ctx); HelpMarker('Instruct Dear ImGui to render a mouse cursor itself. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).')

      ImGui.SeparatorText(ctx, 'Widgets')
      configVarCheckbox('ConfigVar_InputTextCursorBlink')
      ImGui.SameLine(ctx); demo.HelpMarker('Enable blinking cursor (optional as some users consider it to be distracting).')
      configVarCheckbox('ConfigVar_InputTextEnterKeepActive')
      ImGui.SameLine(ctx); demo.HelpMarker('Pressing Enter will keep item active and select contents (single-line only).')
      configVarCheckbox('ConfigVar_DragClickToInputText')
      ImGui.SameLine(ctx); demo.HelpMarker("Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving).")
      configVarCheckbox('ConfigVar_WindowsResizeFromEdges')
      ImGui.SameLine(ctx); demo.HelpMarker('Enable resizing of windows from their edges and from the lower-left corner.')
      configVarCheckbox('ConfigVar_WindowsMoveFromTitleBarOnly')
      ImGui.SameLine(ctx); demo.HelpMarker('Does not apply to windows without a title bar.')
      configVarCheckbox('ConfigVar_MacOSXBehaviors')
      ImGui.Text(ctx, "Also see Style->Rendering for rendering options.")

      ImGui.SeparatorText(ctx, 'Debug')
      ImGui.BeginDisabled(ctx)
      configVarCheckbox('ConfigVar_DebugBeginReturnValueOnce')
      ImGui.EndDisabled(ctx)
      ImGui.SameLine(ctx); demo.HelpMarker('First calls to Begin()/BeginChild() will return false.\n\nTHIS OPTION IS DISABLED because it needs to be set at application boot-time to make sense. Showing the disabled option is a way to make this feature easier to discover')
      configVarCheckbox('ConfigVar_DebugBeginReturnValueLoop')
      ImGui.SameLine(ctx); demo.HelpMarker('Some calls to Begin()/BeginChild() will return false.\n\nWill cycle through window depths then repeat. Windows should be flickering while running.')

      ImGui.SetConfigVar(ctx, ImGui.ConfigVar_Flags(), config.flags)
      ImGui.TreePop(ctx)
      ImGui.Spacing(ctx)
    end

--         if (ImGui.TreeNode("Backend Flags"))
--         {
--             HelpMarker(
--                 "Those flags are set by the backends (imgui_impl_xxx files) to specify their capabilities.\n"
--                 "Here we expose then as read-only fields to avoid breaking interactions with your backend.");
--
--             // Make a local copy to avoid modifying actual backend flags.
--             // FIXME: We don't use BeginDisabled() to keep label bright, maybe we need a BeginReadonly() equivalent..
--             ImGuiBackendFlags backend_flags = io.BackendFlags;
--             ImGui::CheckboxFlags("io.BackendFlags: HasGamepad",             &backend_flags, ImGuiBackendFlags_HasGamepad);
--             ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors",        &backend_flags, ImGuiBackendFlags_HasMouseCursors);
--             ImGui::CheckboxFlags("io.BackendFlags: HasSetMousePos",         &backend_flags, ImGuiBackendFlags_HasSetMousePos);
--             ImGui::CheckboxFlags("io.BackendFlags: PlatformHasViewports",   &backend_flags, ImGuiBackendFlags_PlatformHasViewports);
--             ImGui::CheckboxFlags("io.BackendFlags: HasMouseHoveredViewport",&backend_flags, ImGuiBackendFlags_HasMouseHoveredViewport);
--             ImGui::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset",   &backend_flags, ImGuiBackendFlags_RendererHasVtxOffset);
--             ImGui::CheckboxFlags("io.BackendFlags: RendererHasViewports",   &backend_flags, ImGuiBackendFlags_RendererHasViewports);
--             ImGui.TreePop();
--             ImGui.Spacing();
--         }

    if ImGui.TreeNode(ctx, 'Style') then
      demo.HelpMarker("The same contents can be accessed in 'Tools->Style Editor'.")
      demo.ShowStyleEditor()
      ImGui.TreePop(ctx)
      ImGui.Spacing(ctx)
    end

    if ImGui.TreeNode(ctx, 'Capture/Logging') then
      if not config.logging then
        config.logging = {
          auto_open_depth = 2,
        }
      end

      demo.HelpMarker(
        'The logging API redirects all text output so you can easily capture the content of \z
         a window or a block. Tree nodes can be automatically expanded.\n\z
         Try opening any of the contents below in this window and then click one of the "Log To" button.')
      ImGui.PushID(ctx, 'LogButtons')
      local log_to_tty = ImGui.Button(ctx, 'Log To TTY'); ImGui.SameLine(ctx)
      local log_to_file = ImGui.Button(ctx, 'Log To File'); ImGui.SameLine(ctx)
      local log_to_clipboard = ImGui.Button(ctx, 'Log To Clipboard'); ImGui.SameLine(ctx)
      ImGui.PushTabStop(ctx, false)
      ImGui.SetNextItemWidth(ctx, 80.0)
      rv,config.logging.auto_open_depth =
        ImGui.SliderInt(ctx, 'Open Depth', config.logging.auto_open_depth, 0, 9)
      ImGui.PopTabStop(ctx)
      ImGui.PopID(ctx)

      -- Start logging at the end of the function so that the buttons don't appear in the log
      local depth = config.logging.auto_open_depth
      if log_to_tty       then ImGui.LogToTTY(ctx, depth)       end
      if log_to_file      then ImGui.LogToFile(ctx, depth)      end
      if log_to_clipboard then ImGui.LogToClipboard(ctx, depth) end

      demo.HelpMarker('You can also call ImGui.LogText() to output directly to the log without a visual output.')
      if ImGui.Button(ctx, 'Copy "Hello, world!" to clipboard') then
        ImGui.LogToClipboard(ctx, depth)
        ImGui.LogText(ctx, 'Hello, world!')
        ImGui.LogFinish(ctx)
      end
      ImGui.TreePop(ctx)
    end
  end

  if ImGui.CollapsingHeader(ctx, 'Window options') then
    if ImGui.BeginTable(ctx, 'split', 3) then
      ImGui.TableNextColumn(ctx); rv,demo.topmost           = ImGui.Checkbox(ctx, 'Always on top', demo.topmost)
      ImGui.TableNextColumn(ctx); rv,demo.no_titlebar       = ImGui.Checkbox(ctx, 'No titlebar', demo.no_titlebar)
      ImGui.TableNextColumn(ctx); rv,demo.no_scrollbar      = ImGui.Checkbox(ctx, 'No scrollbar', demo.no_scrollbar)
      ImGui.TableNextColumn(ctx); rv,demo.no_menu           = ImGui.Checkbox(ctx, 'No menu', demo.no_menu)
      ImGui.TableNextColumn(ctx); rv,demo.no_move           = ImGui.Checkbox(ctx, 'No move', demo.no_move)
      ImGui.TableNextColumn(ctx); rv,demo.no_resize         = ImGui.Checkbox(ctx, 'No resize', demo.no_resize)
      ImGui.TableNextColumn(ctx); rv,demo.no_collapse       = ImGui.Checkbox(ctx, 'No collapse', demo.no_collapse)
      ImGui.TableNextColumn(ctx); rv,demo.no_close          = ImGui.Checkbox(ctx, 'No close', demo.no_close)
      ImGui.TableNextColumn(ctx); rv,demo.no_nav            = ImGui.Checkbox(ctx, 'No nav', demo.no_nav)
      ImGui.TableNextColumn(ctx); rv,demo.no_background     = ImGui.Checkbox(ctx, 'No background', demo.no_background)
      -- ImGui.TableNextColumn(ctx); rv,demo.no_bring_to_front = ImGui.Checkbox(ctx, 'No bring to front', demo.no_bring_to_front)
      ImGui.TableNextColumn(ctx); rv,demo.no_docking        = ImGui.Checkbox(ctx, 'No docking', demo.no_docking)
      ImGui.TableNextColumn(ctx); rv,demo.unsaved_document  = ImGui.Checkbox(ctx, 'Unsaved document', demo.unsaved_document)
      ImGui.EndTable(ctx)
    end

    local flags = ImGui.GetConfigVar(ctx, ImGui.ConfigVar_Flags())
    local docking_disabled = demo.no_docking or (flags & ImGui.ConfigFlags_DockingEnable()) == 0

    ImGui.Spacing(ctx)
    if docking_disabled then
      ImGui.BeginDisabled(ctx)
    end

    local dock_id = ImGui.GetWindowDockID(ctx)
    ImGui.AlignTextToFramePadding(ctx)
    ImGui.Text(ctx, 'Dock in docker:')
    ImGui.SameLine(ctx)
    ImGui.SetNextItemWidth(ctx, 222)
    if ImGui.BeginCombo(ctx, '##docker', demo.DockName(dock_id)) then
      if ImGui.Selectable(ctx, 'Floating', dock_id == 0) then
        demo.set_dock_id = 0
      end
      for id = -1, -16, -1 do
        if ImGui.Selectable(ctx, demo.DockName(id), dock_id == id) then
          demo.set_dock_id = id
        end
      end
      ImGui.EndCombo(ctx)
    end

    if docking_disabled then
      ImGui.SameLine(ctx)
      ImGui.Text(ctx, ('Disabled via %s'):format(demo.no_docking and 'WindowFlags' or 'ConfigFlags'))
      ImGui.EndDisabled(ctx)
    end
  end

  -- All demo contents
  demo.ShowDemoWindowWidgets()
  demo.ShowDemoWindowLayout()
  demo.ShowDemoWindowPopups()
  demo.ShowDemoWindowTables()
  demo.ShowDemoWindowInputs()

  -- End of ShowDemoWindow()
  ImGui.PopItemWidth(ctx)
  ImGui.End(ctx)

  return open
end

function demo.ShowDemoWindowWidgets()
  if not ImGui.CollapsingHeader(ctx, 'Widgets') then
    return
  end

  if widgets.disable_all then
    ImGui.BeginDisabled(ctx)
  end

  local rv

  if ImGui.TreeNode(ctx, 'Basic') then
    if not widgets.basic then
      widgets.basic = {
        clicked = 0,
        check   = true,
        radio   = 0,
        counter = 0,
        tooltip = reaper.new_array({ 0.6, 0.1, 1.0, 0.5, 0.92, 0.1, 0.2 }),
        curitem = 0,
        str0    = 'Hello, world!',
        str1    = '',
        vec4a   = reaper.new_array({ 0.10, 0.20, 0.30, 0.44 }),
        i0      = 123,
        i1      = 50,
        i2      = 42,
        i3      = 0,
        d0      = 999999.00000001,
        d1      = 1e10,
        d2      = 1.00,
        d3      = 0.0067,
        d4      = 0.123,
        d5      = 0.0,
        angle   = 0.0,
        elem    = 1,
        col1    = 0xff0033,   -- 0xRRGGBB
        col2    = 0x66b2007f, -- 0xRRGGBBAA
        listcur = 0,
      }
    end

    ImGui.SeparatorText(ctx, 'General')
    if ImGui.Button(ctx, 'Button') then
      widgets.basic.clicked = widgets.basic.clicked + 1
    end
    if widgets.basic.clicked & 1 ~= 0 then
      ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Thanks for clicking me!')
    end

    rv,widgets.basic.check = ImGui.Checkbox(ctx, 'checkbox', widgets.basic.check)

    rv,widgets.basic.radio = ImGui.RadioButtonEx(ctx, 'radio a', widgets.basic.radio, 0); ImGui.SameLine(ctx)
    rv,widgets.basic.radio = ImGui.RadioButtonEx(ctx, 'radio b', widgets.basic.radio, 1); ImGui.SameLine(ctx)
    rv,widgets.basic.radio = ImGui.RadioButtonEx(ctx, 'radio c', widgets.basic.radio, 2)

    -- Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
    for i = 0, 6 do
     if i > 0 then
      ImGui.SameLine(ctx)
     end
     ImGui.PushID(ctx, i)
     ImGui.PushStyleColor(ctx, ImGui.Col_Button(),        demo.HSV(i / 7.0, 0.6, 0.6, 1.0))
     ImGui.PushStyleColor(ctx, ImGui.Col_ButtonHovered(), demo.HSV(i / 7.0, 0.7, 0.7, 1.0))
     ImGui.PushStyleColor(ctx, ImGui.Col_ButtonActive(),  demo.HSV(i / 7.0, 0.8, 0.8, 1.0))
     ImGui.Button(ctx, 'Click')
     ImGui.PopStyleColor(ctx, 3)
     ImGui.PopID(ctx)
    end

    -- Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
    -- (otherwise a Text+SameLine+Button sequence will have the text a little too high by default!)
    -- See 'Demo->Layout->Text Baseline Alignment' for details.
    ImGui.AlignTextToFramePadding(ctx)
    ImGui.Text(ctx, 'Hold to repeat:')
    ImGui.SameLine(ctx)

    -- Arrow buttons with Repeater
    local spacing = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())
    ImGui.PushButtonRepeat(ctx, true)
    if ImGui.ArrowButton(ctx, '##left', ImGui.Dir_Left()) then
      widgets.basic.counter = widgets.basic.counter - 1
    end
    ImGui.SameLine(ctx, 0.0, spacing)
    if ImGui.ArrowButton(ctx, '##right', ImGui.Dir_Right()) then
      widgets.basic.counter = widgets.basic.counter + 1
    end
    ImGui.PopButtonRepeat(ctx)
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, ('%d'):format(widgets.basic.counter))

    do
      -- Tooltips
      -- ImGui.AlignTextToFramePadding(ctx)
      ImGui.Text(ctx, 'Tooltips:')

      ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Button')
      if ImGui.IsItemHovered(ctx) then
        ImGui.SetTooltip(ctx, 'I am a tooltip')
      end

      ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Fancy')
      if ImGui.IsItemHovered(ctx) and ImGui.BeginTooltip(ctx) then
        ImGui.Text(ctx, 'I am a fancy tooltip')
        ImGui.PlotLines(ctx, 'Curve', widgets.basic.tooltip)
        ImGui.Text(ctx, ('Sin(time) = %f'):format(math.sin(ImGui.GetTime(ctx))))
        ImGui.EndTooltip(ctx)
      end

      ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Delayed')
      if ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_DelayNormal()) then -- With a delay
        ImGui.SetTooltip(ctx, 'I am a tooltip with a delay.')
      end

      ImGui.SameLine(ctx)
      demo.HelpMarker('Tooltip are created by using the IsItemHovered() function over any kind of item.')
    end

    ImGui.LabelText(ctx, 'label', 'Value')

    ImGui.SeparatorText(ctx, 'Inputs')

    do
      rv,widgets.basic.str0 = ImGui.InputText(ctx, 'input text', widgets.basic.str0)
      ImGui.SameLine(ctx); demo.HelpMarker(
       'USER:\n\z
        Hold SHIFT or use mouse to select text.\n\z
        CTRL+Left/Right to word jump.\n\z
        CTRL+A or double-click to select all.\n\z
        CTRL+X,CTRL+C,CTRL+V clipboard.\n\z
        CTRL+Z,CTRL+Y undo/redo.\n\z
        ESCAPE to revert.\n\n')

      rv,widgets.basic.str1 = ImGui.InputTextWithHint(ctx, 'input text (w/ hint)', 'enter text here', widgets.basic.str1)

      rv,widgets.basic.i0 = ImGui.InputInt(ctx, 'input int', widgets.basic.i0)

      rv,widgets.basic.d0 = ImGui.InputDouble(ctx, 'input double', widgets.basic.d0, 0.01, 1.0, '%.8f')
      rv,widgets.basic.d1 = ImGui.InputDouble(ctx, 'input scientific', widgets.basic.d1, 0.0, 0.0, '%e')
      ImGui.SameLine(ctx); demo.HelpMarker(
        'You can input value using the scientific notation,\n\z
        e.g. "1e+8" becomes "100000000".')

      ImGui.InputDoubleN(ctx, 'input reaper.array', widgets.basic.vec4a)
    end

    ImGui.SeparatorText(ctx, 'Drags')

    do
      rv,widgets.basic.i1 = ImGui.DragInt(ctx, 'drag int', widgets.basic.i1, 1)
      ImGui.SameLine(ctx); demo.HelpMarker(
        'Click and drag to edit value.\n\z
        Hold SHIFT/ALT for faster/slower edit.\n\z
        Double-click or CTRL+click to input value.')

      rv,widgets.basic.i2 = ImGui.DragInt(ctx, 'drag int 0..100', widgets.basic.i2, 1, 0, 100, '%d%%', ImGui.SliderFlags_AlwaysClamp())

      rv,widgets.basic.d2 = ImGui.DragDouble(ctx, 'drag double', widgets.basic.d2, 0.005)
      rv,widgets.basic.d3 = ImGui.DragDouble(ctx, 'drag small double', widgets.basic.d3, 0.0001, 0.0, 0.0, '%.06f ns')
    end

    ImGui.SeparatorText(ctx, 'Sliders')

    do
      rv,widgets.basic.i3 = ImGui.SliderInt(ctx, 'slider int', widgets.basic.i3, -1, 3)
      ImGui.SameLine(ctx); demo.HelpMarker('CTRL+click to input value.')

      rv,widgets.basic.d4 = ImGui.SliderDouble(ctx, 'slider double', widgets.basic.d4, 0.0, 1.0, 'ratio = %.3f')
      rv,widgets.basic.d5 = ImGui.SliderDouble(ctx, 'slider double (log)', widgets.basic.d5, -10.0, 10.0, '%.4f', ImGui.SliderFlags_Logarithmic())

      rv,widgets.basic.angle = ImGui.SliderAngle(ctx, 'slider angle', widgets.basic.angle)

      -- Using the format string to display a name instead of an integer.
      -- Here we completely omit '%d' from the format string, so it'll only display a name.
      -- This technique can also be used with DragInt().
      local elements = { 'Fire', 'Earth', 'Air', 'Water' }
      local current_elem = elements[widgets.basic.elem] or 'Unknown'
      rv,widgets.basic.elem = ImGui.SliderInt(ctx, 'slider enum', widgets.basic.elem, 1, #elements, current_elem)
      ImGui.SameLine(ctx)
      demo.HelpMarker(
        'Using the format string parameter to display a name instead \z
        of the underlying integer.')
    end

    ImGui.SeparatorText(ctx, 'Selectors/Pickers')

    do
      foo = widgets.basic.col1
      rv,widgets.basic.col1 = ImGui.ColorEdit3(ctx, 'color 1', widgets.basic.col1)
      ImGui.SameLine(ctx); demo.HelpMarker(
        'Click on the color square to open a color picker.\n\z
        Click and hold to use drag and drop.\n\z
        Right-click on the color square to show options.\n\z
        CTRL+click on individual component to input value.')

      rv, widgets.basic.col2 = ImGui.ColorEdit4(ctx, 'color 2', widgets.basic.col2)
    end

    do
      -- Using the _simplified_ one-liner Combo() api here
      -- See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
      local items = 'AAAA\0BBBB\0CCCC\0DDDD\0EEEE\0FFFF\0GGGG\0HHHH\0IIIIIII\0JJJJ\0KKKKKKK\0'
      rv,widgets.basic.curitem = ImGui.Combo(ctx, 'combo', widgets.basic.curitem, items)
      ImGui.SameLine(ctx); demo.HelpMarker(
        'Using the simplified one-liner Combo API here.\n' ..
        'Refer to the "Combo" section below for an explanation of how to use the more flexible and general BeginCombo/EndCombo API.')
    end

    do
      -- Using the _simplified_ one-liner ListBox() api here
      -- See "List boxes" section for examples of how to use the more flexible BeginListBox()/EndListBox() api.
      local items = 'Apple\0Banana\0Cherry\0Kiwi\0Mango\0Orange\0Pineapple\0Strawberry\0Watermelon\0'
      rv,widgets.basic.listcur = ImGui.ListBox(ctx, 'listbox\n(single select)', widgets.basic.listcur, items, 4)
      ImGui.SameLine(ctx)
      demo.HelpMarker(
        'Using the simplified one-liner ListBox API here.\n\z
        Refer to the "List boxes" section below for an explanation of how to use\z
        the more flexible and general BeginListBox/EndListBox API.')
    end

    ImGui.TreePop(ctx)
  end

--     // Testing ImGuiOnceUponAFrame helper.
--     //static ImGuiOnceUponAFrame once;
--     //for (int i = 0; i < 5; i++)
--     //    if (once)
--     //        ImGui.Text("This will be displayed only once.");

  if ImGui.TreeNode(ctx, 'Trees') then
    if not widgets.trees then
      widgets.trees = {
        base_flags = ImGui.TreeNodeFlags_OpenOnArrow() |
                     ImGui.TreeNodeFlags_OpenOnDoubleClick() |
                     ImGui.TreeNodeFlags_SpanAvailWidth(),
        align_label_with_current_x_position = false,
        test_drag_and_drop = false,
        selection_mask = 1 << 2,
      }
    end

    if ImGui.TreeNode(ctx, 'Basic trees') then
      for i = 0, 4 do
        -- Use SetNextItemOpen() so set the default state of a node to be open. We could
        -- also use TreeNodeEx() with the ImGui_TreeNodeFlags_DefaultOpen flag to achieve the same thing!
        if i == 0 then
          ImGui.SetNextItemOpen(ctx, true, ImGui.Cond_Once())
        end

        if ImGui.TreeNodeEx(ctx, i, ('Child %d'):format(i)) then
          ImGui.Text(ctx, 'blah blah')
          ImGui.SameLine(ctx)
          if ImGui.SmallButton(ctx, 'button') then end
          ImGui.TreePop(ctx)
        end
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Advanced, with Selectable nodes') then
      demo.HelpMarker(
        'This is a more typical looking tree with selectable nodes.\n\z
         Click to select, CTRL+Click to toggle, click on arrows or double-click to open.')
      rv,widgets.trees.base_flags = ImGui.CheckboxFlags(ctx, 'ImGui_TreeNodeFlags_OpenOnArrow',       widgets.trees.base_flags, ImGui.TreeNodeFlags_OpenOnArrow())
      rv,widgets.trees.base_flags = ImGui.CheckboxFlags(ctx, 'ImGui_TreeNodeFlags_OpenOnDoubleClick', widgets.trees.base_flags, ImGui.TreeNodeFlags_OpenOnDoubleClick())
      rv,widgets.trees.base_flags = ImGui.CheckboxFlags(ctx, 'ImGui_TreeNodeFlags_SpanAvailWidth',    widgets.trees.base_flags, ImGui.TreeNodeFlags_SpanAvailWidth()); ImGui.SameLine(ctx); demo.HelpMarker('Extend hit area to all available width instead of allowing more items to be laid out after the node.')
      rv,widgets.trees.base_flags = ImGui.CheckboxFlags(ctx, 'ImGuiTreeNodeFlags_SpanFullWidth',      widgets.trees.base_flags, ImGui.TreeNodeFlags_SpanFullWidth())
      rv,widgets.trees.align_label_with_current_x_position = ImGui.Checkbox(ctx, 'Align label with current X position', widgets.trees.align_label_with_current_x_position)
      rv,widgets.trees.test_drag_and_drop = ImGui.Checkbox(ctx, 'Test tree node as drag source',      widgets.trees.test_drag_and_drop)
      ImGui.Text(ctx, 'Hello!')
      if widgets.trees.align_label_with_current_x_position then
        ImGui.Unindent(ctx, ImGui.GetTreeNodeToLabelSpacing(ctx))
      end

      -- 'selection_mask' is dumb representation of what may be user-side selection state.
      --  You may retain selection state inside or outside your objects in whatever format you see fit.
      -- 'node_clicked' is temporary storage of what node we have clicked to process selection at the end
      -- of the loop. May be a pointer to your own node type, etc.
      local node_clicked = -1

      for i = 0, 5 do
        -- Disable the default "open on single-click behavior" + set Selected flag according to our selection.
        -- To alter selection we use IsItemClicked() && !IsItemToggledOpen(), so clicking on an arrow doesn't alter selection.
        local node_flags = widgets.trees.base_flags
        local is_selected = (widgets.trees.selection_mask & (1 << i)) ~= 0
        if is_selected then
          node_flags = node_flags | ImGui.TreeNodeFlags_Selected()
        end
        if i < 3 then
          -- Items 0..2 are Tree Node
          local node_open = ImGui.TreeNodeEx(ctx, i, ('Selectable Node %d'):format(i), node_flags)
          if ImGui.IsItemClicked(ctx) and not ImGui.IsItemToggledOpen(ctx) then
            node_clicked = i
          end
          if widgets.trees.test_drag_and_drop and ImGui.BeginDragDropSource(ctx) then
            ImGui.SetDragDropPayload(ctx, '_TREENODE', nil, 0)
            ImGui.Text(ctx, 'This is a drag and drop source')
            ImGui.EndDragDropSource(ctx)
          end
          if node_open then
            ImGui.BulletText(ctx, 'Blah blah\nBlah Blah')
            ImGui.TreePop(ctx)
          end
        else
          -- Items 3..5 are Tree Leaves
          -- The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we can
          -- use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
          node_flags = node_flags | ImGui.TreeNodeFlags_Leaf() | ImGui.TreeNodeFlags_NoTreePushOnOpen() -- | ImGui.TreeNodeFlags_Bullet()
          ImGui.TreeNodeEx(ctx, i, ('Selectable Leaf %d'):format(i), node_flags)
          if ImGui.IsItemClicked(ctx) and not ImGui.IsItemToggledOpen(ctx) then
            node_clicked = i
          end
          if widgets.trees.test_drag_and_drop and ImGui.BeginDragDropSource(ctx) then
            ImGui.SetDragDropPayload(ctx, '_TREENODE', nil, 0)
            ImGui.Text(ctx, 'This is a drag and drop source')
            ImGui.EndDragDropSource(ctx)
          end
        end
      end

      if node_clicked ~= -1 then
        -- Update selection state
        -- (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
        if ImGui.IsKeyDown(ctx, ImGui.Mod_Ctrl()) then -- CTRL+click to toggle
          widgets.trees.selection_mask = widgets.trees.selection_mask ~ (1 << node_clicked)
        elseif widgets.trees.selection_mask & (1 << node_clicked) == 0 then -- Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
          widgets.trees.selection_mask = (1 << node_clicked)                -- Click to single-select
        end
      end

      if widgets.trees.align_label_with_current_x_position then
        ImGui.Indent(ctx, ImGui.GetTreeNodeToLabelSpacing(ctx))
      end

      ImGui.TreePop(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Collapsing Headers') then
    if not widgets.cheads then
      widgets.cheads = {
        closable_group = true,
      }
    end

    rv,widgets.cheads.closable_group = ImGui.Checkbox(ctx, 'Show 2nd header', widgets.cheads.closable_group)

    if ImGui.CollapsingHeader(ctx, 'Header', nil, ImGui.TreeNodeFlags_None()) then
      ImGui.Text(ctx, ('IsItemHovered: %s'):format(ImGui.IsItemHovered(ctx)))
      for i = 0, 4 do
        ImGui.Text(ctx, ('Some content %s'):format(i))
      end
    end

    if widgets.cheads.closable_group then
      rv,widgets.cheads.closable_group = ImGui.CollapsingHeader(ctx, 'Header with a close button', true)
      if rv then
        ImGui.Text(ctx, ('IsItemHovered: %s'):format(ImGui.IsItemHovered(ctx)))
        for i = 0, 4 do
          ImGui.Text(ctx, ('More content %d'):format(i))
        end
      end
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Bullets') then
    ImGui.BulletText(ctx, 'Bullet point 1')
    ImGui.BulletText(ctx, 'Bullet point 2\nOn multiple lines')
    if ImGui.TreeNode(ctx, 'Tree node') then
      ImGui.BulletText(ctx, 'Another bullet point')
      ImGui.TreePop(ctx)
    end
    ImGui.Bullet(ctx); ImGui.Text(ctx, 'Bullet point 3 (two calls)')
    ImGui.Bullet(ctx); ImGui.SmallButton(ctx, 'Button')
    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Text') then
    if not widgets.text then
      widgets.text = {
        wrap_width = 200.0,
        utf8 = '日本語',
      }
    end

    if ImGui.TreeNode(ctx, 'Colorful Text') then
      -- Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
      ImGui.TextColored(ctx, 0xFF00FFFF, 'Pink')
      ImGui.TextColored(ctx, 0xFFFF00FF, 'Yellow')
      ImGui.TextDisabled(ctx, 'Disabled')
      ImGui.SameLine(ctx); demo.HelpMarker('The TextDisabled color is stored in ImGuiStyle.')
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Word Wrapping') then
      -- Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
      ImGui.TextWrapped(ctx,
        'This text should automatically wrap on the edge of the window. The current implementation ' ..
        'for text wrapping follows simple rules suitable for English and possibly other languages.')
      ImGui.Spacing(ctx)

      rv,widgets.text.wrap_width = ImGui.SliderDouble(ctx, 'Wrap width', widgets.text.wrap_width, -20, 600, '%.0f')

      local draw_list = ImGui.GetWindowDrawList(ctx)
      for n = 0, 1 do
        ImGui.Text(ctx, ('Test paragraph %d:'):format(n))

        local screen_x, screen_y = ImGui.GetCursorScreenPos(ctx)
        local marker_min_x, marker_min_y = screen_x + widgets.text.wrap_width, screen_y
        local marker_max_x, marker_max_y = screen_x + widgets.text.wrap_width + 10, screen_y + ImGui.GetTextLineHeight(ctx)

        local window_x, window_y = ImGui.GetCursorPos(ctx)
        ImGui.PushTextWrapPos(ctx, window_x + widgets.text.wrap_width)

        if n == 0 then
          ImGui.Text(ctx, ('The lazy dog is a good dog. This paragraph should fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.'):format(widgets.text.wrap_width))
        else
          ImGui.Text(ctx, 'aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh')
        end

        -- Draw actual text bounding box, following by marker of our expected limit (should not overlap!)
        local text_min_x, text_min_y = ImGui.GetItemRectMin(ctx)
        local text_max_x, text_max_y = ImGui.GetItemRectMax(ctx)
        ImGui.DrawList_AddRect(draw_list, text_min_x, text_min_y, text_max_x, text_max_y, 0xFFFF00FF)
        ImGui.DrawList_AddRectFilled(draw_list, marker_min_x, marker_min_y, marker_max_x, marker_max_y, 0xFF00FFFF)

        ImGui.PopTextWrapPos(ctx)
      end

      ImGui.TreePop(ctx)
    end

    -- Not supported by the default built-in font TODO
    if ImGui.TreeNode(ctx, 'UTF-8 Text') then
      -- UTF-8 test with Japanese characters
      -- (Needs a suitable font? Try "Google Noto" or "Arial Unicode". See docs/FONTS.md for details.)
      -- so you can safely copy & paste garbled characters into another application.
      ImGui.TextWrapped(ctx,
        'CJK text cannot be rendered due to current limitations regarding font rasterization. \z
        It is however safe to copy & paste from/into another application.')
      demo.Link('https://github.com/cfillion/reaimgui/issues/5')
      ImGui.Spacing(ctx)
      -- ImGui.TextWrapped(ctx,
      --   'CJK text will only appear if the font was loaded with the appropriate CJK character ranges. \z
      --    Call io.Fonts->AddFontFromFileTTF() manually to load extra character ranges. \z
      --    Read docs/FONTS.md for details.')
      ImGui.Text(ctx, 'Hiragana: かきくけこ (kakikukeko)')
      ImGui.Text(ctx, 'Kanjis: 日本語 (nihongo)')
      rv,widgets.text.utf8 = ImGui.InputText(ctx, 'UTF-8 input', widgets.text.utf8)

      ImGui.TreePop(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Images') then
    if not widgets.images then
      widgets.images = {
        pressed_count = 0,
        use_text_color_for_tint = false,
      }
    end
    if not ImGui.ValidatePtr(widgets.images.bitmap, 'ImGui_Image*') then
      widgets.images.bitmap = ImGui.CreateImageFromMem(
       "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52\z
        \x00\x00\x01\x9D\x00\x00\x00\x45\x08\x00\x00\x00\x00\xB4\xAE\x64\z
        \x88\x00\x00\x06\x2D\x49\x44\x41\x54\x78\xDA\xED\x9D\xBF\x6E\xE3\z
        \x36\x1C\xC7\xBF\xC3\x01\x77\xC3\x19\x47\x2F\x09\x70\x68\x21\x08\z
        \x87\x03\x32\x14\x08\x24\x20\x1E\x3A\xA4\x03\x81\x2B\xD0\xB1\x30\z
        \xF4\x06\xEA\x18\x64\xE2\xD4\xB1\x83\xF3\x00\x1D\xB8\x76\xF4\xD0\z
        \x17\xE0\x2B\xE8\x15\xF4\x0A\x7A\x85\x5F\x07\x4A\xB2\x9D\x88\xB4\z
        \xA4\x23\x6D\xDA\xE5\x6F\x49\x22\x3A\x24\xCD\x8F\xF9\xFB\x4B\xC9\z
        \xA0\x21\x51\x14\x25\x04\xC1\xD0\xC5\x12\xDB\xB8\x32\xA1\xD2\x29\z
        \x01\xD6\xC4\xA5\x09\x93\x8E\x00\x80\x32\x2E\x4D\x90\x74\x14\x00\z
        \x00\x75\x5C\x9B\x10\xE9\xA4\x9A\x4E\xDC\x3C\x21\xD2\xD9\x02\x00\z
        \x44\x89\x68\x79\x02\xA4\xB3\x06\xC0\x14\x29\x48\x07\xBD\xF3\xFF\z
        \xDD\x7A\x4A\xE9\x95\x0E\x00\x56\x11\x11\xB8\x8F\xDE\xAF\x5E\x84\z
        \xF0\x49\xA7\x02\x74\xB0\x03\x17\xAA\x2D\xD2\x71\xBB\x7E\x0A\xED\z
        \xA6\x01\x54\xA4\x13\x1A\x1D\xD1\x51\x01\x44\xA4\x13\x1E\x9D\xB4\z
        \xB3\x3F\x65\xA4\x13\x1A\x9D\x6D\xBB\x65\x6A\xB8\x70\x0B\x22\x1D\z
        \xD7\x76\x47\x74\x61\x4F\xA4\x13\x20\x1D\x6D\x76\x4A\x60\x1D\xE9\z
        \x9C\x9D\x4E\xC5\x0F\x24\x43\xC6\x39\xE7\x8F\xEF\x80\x84\x7F\xB7\z
        \x80\xFB\x96\xC7\xEC\xF1\xF0\xEF\x2F\x4B\xE0\x43\xC2\x4F\x20\xD9\z
        \xCF\x03\x17\x13\x17\x43\x3F\x3C\x70\x9E\x65\x59\xF6\xD7\xEB\x4F\z
        \x77\xAD\x35\x9B\xC0\x85\x78\xD4\xEA\x70\x9A\x1B\x76\xBA\x2C\xE1\z
        \xA0\x53\xEB\x64\xEF\x70\x4E\x04\x00\x02\x83\x63\x36\x0C\x60\x74\z
        \x71\x74\x9A\x35\x3A\x69\xAE\x93\x4E\xBE\x26\x9D\x6C\x2B\x2F\x8F\z
        \x0E\xEF\xE1\xA0\xBA\x4E\x3A\x65\xDA\xE6\xA9\xEB\x8B\xA3\x23\x77\z
        \x70\x18\x5D\x27\x1D\x89\xBA\x66\x70\x92\x29\x70\x4C\xA7\xD9\x70\z
        \x20\x15\x66\x3A\xF9\x8E\x8E\x18\xDD\xC1\x65\xD1\xA9\xB1\xC9\x01\z
        \xA4\x4D\x68\x74\x86\x0D\xFE\x3E\x9D\x1D\x9C\xBC\x19\xDD\xC1\x65\z
        \xD1\x69\x6B\xA3\x6E\xCE\x4C\xB9\xA3\x63\x32\xF8\xFB\x74\xDE\xD9\z
        \xE0\xB8\xF7\x18\xCE\x42\x87\xC3\x95\x5E\x73\x49\xC7\x64\xF0\xF7\z
        \xE9\x24\x9D\x5A\x6B\x26\x74\x10\x36\x9D\x46\x1D\xCA\x33\x00\x64\z
        \xCA\x2A\xFF\x3E\x17\x8F\x59\xF1\xA7\x3A\x2A\x50\x8E\xE4\x79\xB7\z
        \xB6\x1F\x0F\x1A\x5E\xF0\xD2\xFF\x9E\xFD\xFE\x15\xF8\x5A\xFC\x33\z
        \xA9\x83\xF9\x82\x62\xE0\x62\x51\x38\xE8\x39\xCB\x94\x02\x80\x3F\z
        \x50\x8B\x03\xF9\x06\x00\xB7\x4F\xC2\x22\x4F\xF7\x78\xFF\xAD\x28\z
        \x8A\xD5\x6D\x21\x8E\x08\xC4\x44\x79\xFA\x25\x01\x3E\xAD\xDE\x5C\z
        \xBF\xDD\x2D\xEE\x61\x63\x81\xDD\x24\x12\xCB\x7C\x8C\x1D\x1C\x19\z
        \xD8\xF6\xE6\x86\x5E\xBD\x5A\x89\xEF\x97\x24\x11\x02\x00\x0A\xBC\z
        \x76\x81\x8E\x3A\xD3\x15\x03\xD3\x2F\x68\x72\xE9\x58\xB3\x19\x0D\z
        \xB7\xD1\xE0\xEF\x6B\x36\x6E\x31\x96\x1E\x3C\x86\x93\xDB\x9D\x8A\z
        \x1D\xA5\x53\xB3\xCE\x2A\x35\x22\x67\xB5\x4B\x3A\x66\xC3\xAD\x8C\z
        \x06\x7F\x2C\x1D\x0F\x1E\xC3\xA9\xE9\x34\x29\x00\xB6\xC5\xE6\x88\z
        \x79\xDE\xF6\xE1\x45\xE9\x92\x8E\xD9\x70\x2B\xA3\xC1\x1F\x4B\xC7\z
        \x83\xC7\x30\x99\xCE\x61\xAA\x73\x32\x1D\x0E\x00\x92\x52\x4B\xED\z
        \xA0\xEA\xDD\xED\x2D\x80\xAE\x90\x3A\x89\x8E\x29\x28\xB4\x84\xFA\z
        \x8A\xCB\x1C\xC8\x45\x6D\xCD\x15\xD8\xE8\xF0\x67\x43\x07\x23\x72\z
        \x0C\x86\x09\x4F\xA6\x83\x03\x99\x4A\x47\x68\x38\x54\x5A\xFE\x53\z
        \xF4\x1F\x30\x79\x6C\x0C\x03\x1D\xA3\x8A\xB7\x84\xFA\x8A\x8F\xC9\z
        \xE4\x58\xE9\x58\xDA\x8E\xE4\x18\x4C\x13\x3E\x2D\x9D\xAA\x9B\xC2\z
        \xD6\x12\x8B\xF2\xBE\xE3\x1A\x38\x5A\x3F\xC5\x04\x15\x5F\x59\x0C\z
        \xB7\x5F\x3A\x56\x8F\xC1\x3C\xE1\xD3\xD2\xE1\x00\x72\x22\xA2\xC6\z
        \x12\x8C\xEE\x75\x2C\x19\xF2\x6A\x2A\x1D\x8B\x6D\xB1\x18\x6E\xBF\z
        \x74\xAC\x1E\x83\x79\xC2\x27\xA5\x23\x81\xCE\x55\xCE\xF9\x18\x3A\z
        \x73\x72\x05\x36\xDB\x62\x31\xDC\x7E\xE9\xD8\x3C\x06\xCB\x84\x4F\z
        \x41\xA7\xAF\x5C\x2F\x00\x7C\xD1\x85\xD3\x05\x96\x9F\x93\x87\xE1\z
        \x5A\x34\x80\x11\xF5\xE8\xB6\x80\xFC\xE6\x95\x8B\xDD\xEC\x5E\x97\z
        \x78\xB3\xE5\xDD\x02\x58\x24\x43\xF5\xE0\x6C\x69\x2C\x1F\xEB\x4A\z
        \x3B\xE7\x9C\xF3\x65\x66\x9E\x90\xB5\xED\x47\xE3\xC0\xB6\x09\x1F\z
        \x5E\x78\xBC\xBB\x59\x00\xF8\xF8\x83\x83\xCA\xF5\x72\xA9\x17\xFA\z
        \x57\xEC\xAB\x7D\x4E\x44\x54\x75\x7B\x39\x55\xB3\xF7\x4E\x67\x48\z
        \x31\x41\xC5\x2B\x8B\x0D\xF3\xBB\x77\x66\xDA\xA4\x83\xBD\x23\x18\z
        \x00\x51\xD3\xFD\xBD\x97\x78\xA7\x3B\x49\x50\x31\xAB\x07\xC3\x47\z
        \xD0\xB1\x04\x77\x98\x63\x5B\xCE\x49\xC7\x62\x93\xF6\x96\xA7\xC9\z
        \x01\xF0\x9A\x88\x9E\xEF\xBC\xD0\xE1\xDA\x25\xA8\xD9\xBE\x6E\x2C\z
        \x2D\x1E\x35\x11\x11\x95\x6A\x5A\x70\x87\x39\xB6\xE5\x9C\x74\x2C\z
        \x36\x69\x8F\xCE\x1A\x60\x3A\x44\x17\x6B\x2F\x74\x72\x40\xEA\x71\z
        \x90\x0B\xCE\x64\xC9\xD0\x67\x05\x06\xA3\x51\x22\xA2\x86\x4D\x33\z
        \xA4\x04\x69\x0C\x0A\xC3\xA4\x63\x89\x62\x77\x74\x14\xB0\x6E\xDA\z
        \x0F\xEF\xC6\x0B\x1D\x9D\x5E\xAB\x01\x26\x89\x04\x1A\xAD\x9F\x52\z
        \x4B\x26\x87\x88\x36\x7C\x62\x70\x87\x39\x04\xCE\x4A\x47\x8D\xC8\z
        \xE4\x94\xED\xAD\x68\x55\xEE\xE6\x66\xF5\x21\x3A\xA9\x4E\xCF\x6C\z
        \xFA\xF7\x5C\xEA\xD4\x81\x29\x0B\x4A\x54\xA5\xF5\xC4\xE0\xEE\x3A\z
        \xE9\xA4\xB2\xD7\xFA\x4E\xCA\x96\x43\x76\x87\x53\x7F\x0B\x42\x1B\z
        \x8F\xF2\xA1\xE3\xBA\xBB\x0A\x42\x95\x0E\x06\xA3\xCC\x12\xDC\x5D\z
        \x27\x1D\x10\x11\xA9\xDC\xD9\x99\x85\xB7\x74\xD6\x2D\x1D\xBD\x0E\z
        \x3A\x11\x5A\x0F\xA6\x39\x9B\x12\x4C\x2A\xB5\x2D\x79\x65\x4E\xC5\z
        \x19\x82\xBB\xEB\xA4\x93\x6F\xA8\x2A\x01\xA4\x8E\x6E\x1A\x7D\x1B\z
        \x8D\x26\x58\x72\xCE\x33\xE8\xB8\xEF\xE6\x43\x17\x8B\x0D\xC6\x9A\z
        \x77\xC9\xCD\xD2\x10\xAE\x72\xCE\xCD\x51\xA5\xED\x60\xB5\x39\xE2\z
        \xF4\x1E\x8D\xCE\x6B\xDB\x45\xA3\x0F\x4B\x00\xCB\xCF\x3F\xB9\x3A\z
        \xA1\x3D\x14\x8D\xE6\x44\x44\x4C\x9B\x35\x81\xF6\x87\xE3\xF3\x82\z
        \xD7\xB9\x77\x5C\xCB\x40\x9E\x2D\x47\x4D\x44\x42\x8F\xD9\xBE\x69\z
        \xA1\x91\x45\x3A\x67\xA7\x23\xF5\x68\x39\x24\x11\xD5\xDA\x59\x2B\z
        \x5D\x1F\xE6\x8F\x74\x66\xD2\xA1\x1C\x15\x11\x35\x1C\x65\x3F\x74\z
        \x0A\x15\xE9\x84\x41\xA7\x6A\x5D\xE0\x8D\x68\x5F\x40\x12\xAE\x1F\z
        \xD6\x11\xE9\xCC\xA5\x43\x72\x2F\x42\x59\xE7\x44\x0D\x63\x55\xA4\z
        \x13\x0A\x1D\x92\x5D\x2E\x8F\x48\x80\xEA\x1C\x92\x22\x9D\x60\xE8\z
        \x90\x4A\xC1\x65\xDD\xFA\x08\x8C\xB9\x7F\x3C\x68\xA4\x33\x8D\xCE\z
        \xE1\x49\xDD\xA7\xD5\x27\x00\xC9\x2D\x80\xF7\xAB\x27\xE1\x5C\x2C\z
        \x47\x77\x8B\x64\x4E\xDB\xD8\x93\xBA\x1E\xDA\xB0\x12\x9E\x64\x77\z
        \x52\xF7\xF5\x29\x77\xF5\x77\x51\x64\x45\xF1\xDB\xDE\xE9\x71\x87\z
        \x62\x39\xF6\xFE\x92\xCD\x6B\x1B\x79\x70\xFC\xC5\x7D\x9B\x77\xA9\z
        \x4D\xAA\xA6\x71\x6F\x73\xBC\x68\xB6\xD1\xCA\xC2\x83\xD6\xF3\x2F\z
        \x38\xAD\x56\x8D\x74\xE6\x2F\x57\xB3\x11\x9B\x26\xD2\x09\x93\x8E\z
        \x62\xDD\x83\x0D\x23\x9D\xE0\xE8\x34\xBA\x6C\xC6\x03\xA4\xE3\x57\z
        \x2E\x82\xCE\x06\x7B\x4F\xA2\xAE\x83\xF2\x0A\x22\x9D\xBE\xA6\xD9\z
        \xDE\xFF\xA1\x22\x9D\x00\xF7\x8E\x3E\xE5\x54\x32\x8A\x74\x42\xA2\z
        \xD3\xDE\xA2\x21\xB5\x0D\x2A\x4F\x4D\x87\xE4\xB9\xD6\x40\xD2\x05\z
        \xD0\x21\x8E\x94\xAF\xF5\x6C\xD6\x9E\xBE\x07\xE1\xE2\x1E\xA7\x17\z
        \x0E\x9D\xAA\xAF\x18\x94\xBE\x32\x7C\x91\xCE\xFC\xE5\x92\x0C\xA2\z
        \x22\xDA\x72\x6F\x4F\x9F\xBB\xB8\x47\xEF\x0B\x0A\x86\x4E\x77\x77\z
        \x48\x1A\xBF\xBA\x2A\x0C\x79\xF3\xE4\x49\x29\x44\xFC\xD6\xB7\x50\z
        \xE4\x3F\xB8\xA9\x68\x06\x1B\x45\x77\x96\x00\x00\x00\x00\x49\x45\z
        \x4E\x44\xAE\x42\x60\x82")
    end

    ImGui.TextWrapped(ctx, 'Hover the texture for a zoomed view!')

    -- Consider using the lower-level Draw List API, via ImGui.DrawList_AddImage(ImGui.GetWindowDrawList()).
    local my_tex_w, my_tex_h = ImGui.Image_GetSize(widgets.images.bitmap)
    do
      rv,widgets.images.use_text_color_for_tint =
        ImGui.Checkbox(ctx, 'Use Text Color for Tint', widgets.images.use_text_color_for_tint)
      ImGui.Text(ctx, ('%.0fx%.0f'):format(my_tex_w, my_tex_h))
      local pos_x, pos_y = ImGui.GetCursorScreenPos(ctx)
      local uv_min_x, uv_min_y = 0.0, 0.0 -- Top-left
      local uv_max_x, uv_max_y = 1.0, 1.0 -- Lower-right
      local tint_col   = widgets.images.use_text_color_for_tint and ImGui.GetStyleColor(ctx, ImGui.Col_Text()) or 0xFFFFFFFF -- No tint
      local border_col = ImGui.GetStyleColor(ctx, ImGui.Col_Border())
      ImGui.Image(ctx, widgets.images.bitmap, my_tex_w, my_tex_h,
        uv_min_x, uv_min_y, uv_max_x, uv_max_y, tint_col, border_col)
      if ImGui.IsItemHovered(ctx) and ImGui.BeginTooltip(ctx) then
        local region_sz = 32.0
        local mouse_x, mouse_y = ImGui.GetMousePos(ctx)
        local region_x = mouse_x - pos_x - region_sz * 0.5
        local region_y = mouse_y - pos_y - region_sz * 0.5
        local zoom = 4.0
        if region_x < 0.0 then region_x = 0.0
        elseif region_x > my_tex_w - region_sz then region_x = my_tex_w - region_sz end
        if region_y < 0.0 then region_y = 0.0
        elseif region_y > my_tex_h - region_sz then region_y = my_tex_h - region_sz end
        ImGui.Text(ctx, ('Min: (%.2f, %.2f)'):format(region_x, region_y))
        ImGui.Text(ctx, ('Max: (%.2f, %.2f)'):format(region_x + region_sz, region_y + region_sz))
        local uv0_x, uv0_y = region_x / my_tex_w, region_y / my_tex_h
        local uv1_x, uv1_y = (region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h
        ImGui.Image(ctx, widgets.images.bitmap, region_sz * zoom, region_sz * zoom,
          uv0_x, uv0_y, uv1_x, uv1_y, tint_col, border_col)
        ImGui.EndTooltip(ctx)
      end
    end
    ImGui.TextWrapped(ctx, 'And now some textured buttons...')
    -- static int pressed_count = 0;
    for i = 0, 8 do
      -- UV coordinates are (0.0, 0.0) and (1.0, 1.0) to display an entire textures.
      -- Here we are trying to display only a 32x32 pixels area of the texture, hence the UV computation.
      -- Read about UV coordinates here: https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
      if i > 0 then
        ImGui.PushStyleVar(ctx, ImGui.StyleVar_FramePadding(), i - 1, i - 1)
      end
      local size_w, size_h = 32.0, 32.0                     -- Size of the image we want to make visible
      local uv0_x, uv0_y = 0.0, 0.0                         -- UV coordinates for lower-left
      local uv1_x, uv1_y = 32.0 / my_tex_w, 32.0 / my_tex_h -- UV coordinates for (32,32) in our texture
      local bg_col = 0x000000FF   -- Black background
      local tint_col = 0xFFFFFFFF -- No tint
      if ImGui.ImageButton(ctx, i, widgets.images.bitmap, size_w, size_h,
                             uv0_x, uv0_y, uv1_x, uv1_y, bg_col, tint_col) then
        widgets.images.pressed_count = widgets.images.pressed_count + 1
      end
      if i > 0 then
        ImGui.PopStyleVar(ctx)
      end
      ImGui.SameLine(ctx)
    end
    ImGui.NewLine(ctx)
    ImGui.Text(ctx, ('Pressed %d times.'):format(widgets.images.pressed_count))
    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Combo') then
    if not widgets.combos then
      widgets.combos = {
        flags = ImGui.ComboFlags_None(),
        current_item1 = 1,
        current_item2 = 0,
        current_item3 = -1,
      }
    end

    -- Combo Boxes are also called "Dropdown" in other systems
    -- Expose flags as checkbox for the demo
    rv,widgets.combos.flags = ImGui.CheckboxFlags(ctx, 'ImGuiComboFlags_PopupAlignLeft', widgets.combos.flags, ImGui.ComboFlags_PopupAlignLeft())
    ImGui.SameLine(ctx); demo.HelpMarker('Only makes a difference if the popup is larger than the combo')

    rv,widgets.combos.flags = ImGui.CheckboxFlags(ctx, 'ImGuiComboFlags_NoArrowButton', widgets.combos.flags, ImGui.ComboFlags_NoArrowButton())
    if rv then
      widgets.combos.flags = widgets.combos.flags & ~ImGui.ComboFlags_NoPreview() -- Clear the other flag, as we cannot combine both
    end

    rv,widgets.combos.flags = ImGui.CheckboxFlags(ctx, 'ImGuiComboFlags_NoPreview', widgets.combos.flags, ImGui.ComboFlags_NoPreview())
    if rv then
      widgets.combos.flags = widgets.combos.flags & ~ImGui.ComboFlags_NoArrowButton() -- Clear the other flag, as we cannot combine both
    end

    -- Using the generic BeginCombo() API, you have full control over how to display the combo contents.
    -- (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
    -- stored in the object itself, etc.)
    local combo_items = { 'AAAA', 'BBBB', 'CCCC', 'DDDD', 'EEEE', 'FFFF', 'GGGG', 'HHHH', 'IIII', 'JJJJ', 'KKKK', 'LLLLLLL', 'MMMM', 'OOOOOOO' }
    local combo_preview_value = combo_items[widgets.combos.current_item1] -- Pass in the preview value visible before opening the combo (it could be anything)
    if ImGui.BeginCombo(ctx, 'combo 1', combo_preview_value, widgets.combos.flags) then
      for i,v in ipairs(combo_items) do
        local is_selected = widgets.combos.current_item1 == i
        if ImGui.Selectable(ctx, combo_items[i], is_selected) then
          widgets.combos.current_item1 = i
        end

        -- Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if is_selected then
          ImGui.SetItemDefaultFocus(ctx)
        end
      end
      ImGui.EndCombo(ctx)
    end

    -- Simplified one-liner Combo() API, using values packed in a single constant string
    -- This is a convenience for when the selection set is small and known when writing the script.
    combo_items = 'aaaa\0bbbb\0cccc\0dddd\0eeee\0'
    rv,widgets.combos.current_item2 = ImGui.Combo(ctx, 'combo 2 (one-liner)', widgets.combos.current_item2, combo_items)

    -- Simplified one-liner Combo() using an array of const char*
    -- If the selection isn't within 0..count, Combo won't display a preview
    rv,widgets.combos.current_item3 = ImGui.Combo(ctx, 'combo 3 (out of range)', widgets.combos.current_item3, combo_items)

--         // Simplified one-liner Combo() using an accessor function
--         struct Funcs { static bool ItemGetter(void* data, int n, const char** out_str) { *out_str = ((const char**)data)[n]; return true; } };
--         static int item_current_4 = 0;
--         ImGui.Combo("combo 4 (function)", &item_current_4, &Funcs::ItemGetter, items, IM_ARRAYSIZE(items));

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'List boxes') then
    if not widgets.lists then
      widgets.lists = { current_idx = 1 }
    end

    -- Using the generic BeginListBox() API, you have full control over how to display the combo contents.
    -- (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
    -- stored in the object itself, etc.)
    local items = { 'AAAA', 'BBBB', 'CCCC', 'DDDD', 'EEEE', 'FFFF', 'GGGG', 'HHHH', 'IIII', 'JJJJ', 'KKKK', 'LLLLLLL', 'MMMM', 'OOOOOOO' }
    if ImGui.BeginListBox(ctx, 'listbox 1') then
      for n,v in ipairs(items) do
        local is_selected = widgets.lists.current_idx == n
        if ImGui.Selectable(ctx, v, is_selected) then
          widgets.lists.current_idx = n
        end

        -- Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if is_selected then
          ImGui.SetItemDefaultFocus(ctx)
        end
      end
      ImGui.EndListBox(ctx)
    end

    -- Custom size: use all width, 5 items tall
    ImGui.Text(ctx, 'Full-width:')
    if ImGui.BeginListBox(ctx, '##listbox 2', -FLT_MIN, 5 * ImGui.GetTextLineHeightWithSpacing(ctx)) then
      for n,v in ipairs(items) do
        local is_selected = widgets.lists.current_idx == n
        if ImGui.Selectable(ctx, v, is_selected) then
          widgets.lists.current_idx = n
        end

        -- Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if is_selected then
          ImGui.SetItemDefaultFocus(ctx)
        end
      end
      ImGui.EndListBox(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Selectables') then
    if not widgets.selectables then
      widgets.selectables = {
        basic    = { false, false, false, false, false },
        single   = -1,
        multiple = { false, false, false, false, false },
        sameline = { false, false, false },
        columns  = { false, false, false, false, false, false, false, false, false, false },
        grid     = {
          { true,  false, false, false },
          { false, true,  false, false },
          { false, false, true,  false },
          { false, false, false, true  },
        },
        align    = {
          { true,  false, true  },
          { false, true , false },
          { true,  false, true  },
        },
      }
    end

    -- Selectable() has 2 overloads:
    -- - The one taking "bool selected" as a read-only selection information.
    --   When Selectable() has been clicked it returns true and you can alter selection state accordingly.
    -- - The one taking "bool* p_selected" as a read-write selection information (convenient in some cases)
    -- The earlier is more flexible, as in real application your selection may be stored in many different ways
    -- and not necessarily inside a bool value (e.g. in flags within objects, as an external list, etc).
    if ImGui.TreeNode(ctx, 'Basic') then
      rv,widgets.selectables.basic[1] = ImGui.Selectable(ctx, '1. I am selectable', widgets.selectables.basic[1])
      rv,widgets.selectables.basic[2] = ImGui.Selectable(ctx, '2. I am selectable', widgets.selectables.basic[2])
      ImGui.Text(ctx, '(I am not selectable)')
      rv,widgets.selectables.basic[4] = ImGui.Selectable(ctx, '4. I am selectable', widgets.selectables.basic[4])
      if ImGui.Selectable(ctx, '5. I am double clickable', widgets.selectables.basic[5], ImGui.SelectableFlags_AllowDoubleClick()) then
        if ImGui.IsMouseDoubleClicked(ctx, 0) then
          widgets.selectables.basic[5] = not widgets.selectables.basic[5]
        end
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Selection State: Single Selection') then
      for i = 0, 4 do
        if ImGui.Selectable(ctx, ('Object %d'):format(i), widgets.selectables.single == i) then
          widgets.selectables.single = i
        end
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Selection State: Multiple Selection') then
      demo.HelpMarker('Hold CTRL and click to select multiple items.')
      for i,sel in ipairs(widgets.selectables.multiple) do
        if ImGui.Selectable(ctx, ('Object %d'):format(i-1), sel) then
          if not ImGui.IsKeyDown(ctx, ImGui.Mod_Ctrl()) then -- Clear selection when CTRL is not held
            for j = 1, #widgets.selectables.multiple do
              widgets.selectables.multiple[j] = false
            end
          end
          widgets.selectables.multiple[i] = not sel
        end
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Rendering more text into the same line') then
      rv,widgets.selectables.sameline[1] = ImGui.Selectable(ctx, 'main.c',    widgets.selectables.sameline[1]); ImGui.SameLine(ctx, 300); ImGui.Text(ctx, ' 2,345 bytes')
      rv,widgets.selectables.sameline[2] = ImGui.Selectable(ctx, 'Hello.cpp', widgets.selectables.sameline[2]); ImGui.SameLine(ctx, 300); ImGui.Text(ctx, '12,345 bytes')
      rv,widgets.selectables.sameline[3] = ImGui.Selectable(ctx, 'Hello.h',   widgets.selectables.sameline[3]); ImGui.SameLine(ctx, 300); ImGui.Text(ctx, ' 2,345 bytes')
      ImGui.TreePop(ctx)
    end
    if ImGui.TreeNode(ctx, 'In columns') then
      if ImGui.BeginTable(ctx, 'split1', 3, ImGui.TableFlags_Resizable() | ImGui.TableFlags_NoSavedSettings() | ImGui.TableFlags_Borders()) then
        for i,sel in ipairs(widgets.selectables.columns) do
          ImGui.TableNextColumn(ctx)
          rv,widgets.selectables.columns[i] = ImGui.Selectable(ctx, ('Item %d'):format(i-1), sel)
        end
        ImGui.EndTable(ctx)
      end
      ImGui.Spacing(ctx)
      if ImGui.BeginTable(ctx, 'split2', 3, ImGui.TableFlags_Resizable() | ImGui.TableFlags_NoSavedSettings() | ImGui.TableFlags_Borders()) then
        for i,sel in ipairs(widgets.selectables.columns) do
          ImGui.TableNextRow(ctx)
          ImGui.TableNextColumn(ctx)
          rv,widgets.selectables.columns[i] = ImGui.Selectable(ctx, ('Item %d'):format(i-1), sel, ImGui.SelectableFlags_SpanAllColumns())
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, 'Some other contents')
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, '123456')
        end
        ImGui.EndTable(ctx)
      end
      ImGui.TreePop(ctx)
    end

    -- Add in a bit of silly fun...
    if ImGui.TreeNode(ctx, 'Grid') then
      local winning_state = true -- If all cells are selected...
      for ri,row in ipairs(widgets.selectables.grid) do
        for ci,sel in ipairs(row) do
          if not sel then
            winning_state = false
            break
          end
        end
      end
      if winning_state then
        local time = ImGui.GetTime(ctx)
        ImGui.PushStyleVar(ctx, ImGui.StyleVar_SelectableTextAlign(),
          0.5 + 0.5 * math.cos(time * 2.0), 0.5 + 0.5 * math.sin(time * 3.0))
      end

      for ri,row in ipairs(widgets.selectables.grid) do
        for ci,col in ipairs(row) do
          if ci > 1 then
            ImGui.SameLine(ctx)
          end
          ImGui.PushID(ctx, ri * #widgets.selectables.grid + ci)
          if ImGui.Selectable(ctx, 'Sailor', col, 0, 50, 50) then
            -- Toggle clicked cell + toggle neighbors
            row[ci] = not row[ci]
            if ci > 1 then row[ci - 1] = not row[ci - 1]; end
            if ci < 4 then row[ci + 1] = not row[ci + 1]; end
            if ri > 1 then widgets.selectables.grid[ri - 1][ci] = not widgets.selectables.grid[ri - 1][ci]; end
            if ri < 4 then widgets.selectables.grid[ri + 1][ci] = not widgets.selectables.grid[ri + 1][ci]; end
          end
          ImGui.PopID(ctx)
        end
      end

      if winning_state then
        ImGui.PopStyleVar(ctx)
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Alignment') then
      demo.HelpMarker(
        "By default, Selectables uses style.SelectableTextAlign but it can be overridden on a per-item \z
         basis using PushStyleVar(). You'll probably want to always keep your default situation to \z
         left-align otherwise it becomes difficult to layout multiple items on a same line")

      for y = 1, 3 do
        for x = 1, 3 do
          local align_x, align_y = (x-1) / 2.0, (y-1) / 2.0
          local name = ('(%.1f,%.1f)'):format(align_x, align_y)
          if x > 1 then ImGui.SameLine(ctx); end
          ImGui.PushStyleVar(ctx, ImGui.StyleVar_SelectableTextAlign(), align_x, align_y)
          local row = widgets.selectables.align[y]
          rv,row[x] = ImGui.Selectable(ctx, name, row[x], ImGui.SelectableFlags_None(), 80, 80)
          ImGui.PopStyleVar(ctx)
        end
      end

      ImGui.TreePop(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Text Input') then
    if not widgets.input then
      widgets.input = {
        multiline = {
          text = [[/*
 The Pentium F00F bug, shorthand for F0 0F C7 C8,
 the hexadecimal encoding of one offending instruction,
 more formally, the invalid operand with locked CMPXCHG8B
 instruction bug, is a design flaw in the majority of
 Intel Pentium, Pentium MMX, and Pentium OverDrive
 processors (all in the P5 microarchitecture).
*/

label:
	lock cmpxchg8b eax
]],
        },
        flags = ImGui.InputTextFlags_AllowTabInput(),
        buf = { '', '', '', '', '', '', '', '', '' },
        password = 'hunter2',
      }
    end

    if ImGui.TreeNode(ctx, 'Multi-line Text Input') then
      rv,widgets.input.multiline.flags = ImGui.CheckboxFlags(ctx, 'ImGuiInputTextFlags_ReadOnly', widgets.input.multiline.flags, ImGui.InputTextFlags_ReadOnly());
      rv,widgets.input.multiline.flags = ImGui.CheckboxFlags(ctx, 'ImGuiInputTextFlags_AllowTabInput', widgets.input.multiline.flags, ImGui.InputTextFlags_AllowTabInput());
      rv,widgets.input.multiline.flags = ImGui.CheckboxFlags(ctx, 'ImGuiInputTextFlags_CtrlEnterForNewLine', widgets.input.multiline.flags, ImGui.InputTextFlags_CtrlEnterForNewLine());
      rv,widgets.input.multiline.text = ImGui.InputTextMultiline(ctx, '##source', widgets.input.multiline.text, -FLT_MIN, ImGui.GetTextLineHeight(ctx) * 16, widgets.input.multiline.flags)
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Filtered Text Input') then
      if not ImGui.ValidatePtr(widgets.input.filterImGuiLetters, 'ImGui_Function*') then
        -- Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i'
        widgets.input.filterImGuiLetters = ImGui.CreateFunctionFromEEL([[
        eat = 1; i = strlen(#allowed);
        while(
          i -= 1;
          str_getchar(#allowed, i) == EventChar ? eat = 0;
          eat && i;
        );
        eat ? EventChar = 0;
        ]])
        ImGui.Function_SetValue_String(widgets.input.filterImGuiLetters, '#allowed', 'imgui')
      end

      rv,widgets.input.buf[1] = ImGui.InputText(ctx, 'default',     widgets.input.buf[1])
      rv,widgets.input.buf[2] = ImGui.InputText(ctx, 'decimal',     widgets.input.buf[2], ImGui.InputTextFlags_CharsDecimal())
      rv,widgets.input.buf[3] = ImGui.InputText(ctx, 'hexadecimal', widgets.input.buf[3], ImGui.InputTextFlags_CharsHexadecimal() | ImGui.InputTextFlags_CharsUppercase())
      rv,widgets.input.buf[4] = ImGui.InputText(ctx, 'uppercase',   widgets.input.buf[4], ImGui.InputTextFlags_CharsUppercase())
      rv,widgets.input.buf[5] = ImGui.InputText(ctx, 'no blank',    widgets.input.buf[5], ImGui.InputTextFlags_CharsNoBlank())
      rv,widgets.input.buf[6] = ImGui.InputText(ctx, '"imgui" letters', widgets.input.buf[6], ImGui.InputTextFlags_CallbackCharFilter(), widgets.input.filterImGuiLetters)
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Password Input') then
      rv,widgets.input.password = ImGui.InputText(ctx, 'password', widgets.input.password, ImGui.InputTextFlags_Password())
      ImGui.SameLine(ctx); demo.HelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n")
      rv,widgets.input.password = ImGui.InputTextWithHint(ctx, 'password (w/ hint)', '<password>', widgets.input.password, ImGui.InputTextFlags_Password())
      rv,widgets.input.password = ImGui.InputText(ctx, 'password (clear)', widgets.input.password)
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Completion, History, Edit Callbacks') then
      if not ImGui.ValidatePtr(widgets.input.callback, 'ImGui_Function*') then
        widgets.input.callback = ImGui.CreateFunctionFromEEL([[
        EventFlag == InputTextFlags_CallbackCompletion ?
          InputTextCallback_InsertChars(CursorPos, "..");
        EventFlag == InputTextFlags_CallbackHistory ? (
          EventKey == Key_UpArrow ? (
            InputTextCallback_DeleteChars(0, strlen(#Buf));
            InputTextCallback_InsertChars(0, "Pressed Up!");
            InputTextCallback_SelectAll();
          ) : EventKey == Key_DownArrow ? (
            InputTextCallback_DeleteChars(0, strlen(#Buf));
            InputTextCallback_InsertChars(0, "Pressed Down!");
            InputTextCallback_SelectAll();
          );
        );
        EventFlag == InputTextFlags_CallbackEdit ? (
          // Toggle casing of first character
          c = str_getchar(#Buf, 0);
          (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ? (
            str_setchar(#first, 0, c ~ 32);
            InputTextCallback_DeleteChars(0, 1);
            InputTextCallback_InsertChars(0, #first);
          );

          // Increment a counter
          edit_count += 1;
        );
        ]])
        local consts = {
          'InputTextFlags_CallbackCompletion',
          'InputTextFlags_CallbackEdit',
          'InputTextFlags_CallbackHistory',
          'Key_UpArrow',
          'Key_DownArrow',
        }
        for _, const in ipairs(consts) do
          ImGui.Function_SetValue(widgets.input.callback, const, ImGui[const]())
        end
      end

      rv,widgets.input.buf[7] = ImGui.InputText(ctx, 'Completion', widgets.input.buf[7], ImGui.InputTextFlags_CallbackCompletion(), widgets.input.callback)
      ImGui.SameLine(ctx); demo.HelpMarker("Here we append \"..\" each time Tab is pressed. See 'Examples>Console' for a more meaningful demonstration of using this callback.")

      rv,widgets.input.buf[8] = ImGui.InputText(ctx, 'History', widgets.input.buf[8], ImGui.InputTextFlags_CallbackHistory(), widgets.input.callback)
      ImGui.SameLine(ctx); demo.HelpMarker("Here we replace and select text each time Up/Down are pressed. See 'Examples>Console' for a more meaningful demonstration of using this callback.")

      rv,widgets.input.buf[9] = ImGui.InputText(ctx, 'Edit', widgets.input.buf[9], ImGui.InputTextFlags_CallbackEdit(), widgets.input.callback)
      ImGui.SameLine(ctx); demo.HelpMarker('Here we toggle the casing of the first character on every edit + count edits.')
      local edit_count = ImGui.Function_GetValue(widgets.input.callback, 'edit_count')
      ImGui.SameLine(ctx); ImGui.Text(ctx, ('(%d)'):format(edit_count))

      ImGui.TreePop(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Tabs') then
    if not widgets.tabs then
      widgets.tabs = {
        flags1  = ImGui.TabBarFlags_Reorderable(),
        opened  = { true, true, true, true },
        flags2  = ImGui.TabBarFlags_AutoSelectNewTabs() |
                  ImGui.TabBarFlags_Reorderable() |
                  ImGui.TabBarFlags_FittingPolicyResizeDown(),
        active  = { 1, 2, 3 },
        next_id = 4,
        show_leading_button  = true,
        show_trailing_button = true,
      }
    end

    local fitting_policy_mask = ImGui.TabBarFlags_FittingPolicyResizeDown() |
                                ImGui.TabBarFlags_FittingPolicyScroll()

    if ImGui.TreeNode(ctx, 'Basic') then
      if ImGui.BeginTabBar(ctx, 'MyTabBar', ImGui.TabBarFlags_None()) then
        if ImGui.BeginTabItem(ctx, 'Avocado') then
          ImGui.Text(ctx, 'This is the Avocado tab!\nblah blah blah blah blah')
          ImGui.EndTabItem(ctx)
        end
        if ImGui.BeginTabItem(ctx, 'Broccoli') then
          ImGui.Text(ctx, 'This is the Broccoli tab!\nblah blah blah blah blah')
          ImGui.EndTabItem(ctx)
        end
        if ImGui.BeginTabItem(ctx, 'Cucumber') then
          ImGui.Text(ctx, 'This is the Cucumber tab!\nblah blah blah blah blah')
          ImGui.EndTabItem(ctx)
        end
        ImGui.EndTabBar(ctx)
      end
      ImGui.Separator(ctx)
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Advanced & Close Button') then
      -- Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
      rv,widgets.tabs.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_Reorderable', widgets.tabs.flags1, ImGui.TabBarFlags_Reorderable())
      rv,widgets.tabs.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_AutoSelectNewTabs', widgets.tabs.flags1, ImGui.TabBarFlags_AutoSelectNewTabs())
      rv,widgets.tabs.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_TabListPopupButton', widgets.tabs.flags1, ImGui.TabBarFlags_TabListPopupButton())
      rv,widgets.tabs.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_NoCloseWithMiddleMouseButton', widgets.tabs.flags1, ImGui.TabBarFlags_NoCloseWithMiddleMouseButton())

      if widgets.tabs.flags1 & fitting_policy_mask == 0 then
        widgets.tabs.flags1 = widgets.tabs.flags1 | ImGui.TabBarFlags_FittingPolicyResizeDown() -- was FittingPolicyDefault_
      end
      if ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyResizeDown', widgets.tabs.flags1, ImGui.TabBarFlags_FittingPolicyResizeDown()) then
        widgets.tabs.flags1 = widgets.tabs.flags1 & ~fitting_policy_mask | ImGui.TabBarFlags_FittingPolicyResizeDown()
      end
      if ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyScroll', widgets.tabs.flags1, ImGui.TabBarFlags_FittingPolicyScroll()) then
        widgets.tabs.flags1 = widgets.tabs.flags1 & ~fitting_policy_mask | ImGui.TabBarFlags_FittingPolicyScroll()
      end

      -- Tab Bar
      local names = { 'Artichoke', 'Beetroot', 'Celery', 'Daikon' }
      for n,opened in ipairs(widgets.tabs.opened) do
        if n > 1 then ImGui.SameLine(ctx); end
        rv,widgets.tabs.opened[n] = ImGui.Checkbox(ctx, names[n], opened)
      end

      -- Passing a bool* to BeginTabItem() is similar to passing one to Begin():
      -- the underlying bool will be set to false when the tab is closed.
      if ImGui.BeginTabBar(ctx, 'MyTabBar', widgets.tabs.flags1) then
        for n,opened in ipairs(widgets.tabs.opened) do
          if opened then
            rv,widgets.tabs.opened[n] = ImGui.BeginTabItem(ctx, names[n], true, ImGui.TabItemFlags_None())
            if rv then
              ImGui.Text(ctx, ('This is the %s tab!'):format(names[n]))
              if n & 1 == 0 then
                ImGui.Text(ctx, 'I am an odd tab.')
              end
              ImGui.EndTabItem(ctx)
            end
          end
        end
        ImGui.EndTabBar(ctx)
      end
      ImGui.Separator(ctx)
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'TabItemButton & Leading/Trailing flags') then
      -- TabItemButton() and Leading/Trailing flags are distinct features which we will demo together.
      -- (It is possible to submit regular tabs with Leading/Trailing flags, or TabItemButton tabs without Leading/Trailing flags...
      -- but they tend to make more sense together)
      rv,widgets.tabs.show_leading_button = ImGui.Checkbox(ctx, 'Show Leading TabItemButton()', widgets.tabs.show_leading_button)
      rv,widgets.tabs.show_trailing_button = ImGui.Checkbox(ctx, 'Show Trailing TabItemButton()', widgets.tabs.show_trailing_button)

      -- Expose some other flags which are useful to showcase how they interact with Leading/Trailing tabs
      rv,widgets.tabs.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_TabListPopupButton', widgets.tabs.flags2, ImGui.TabBarFlags_TabListPopupButton())
      if ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyResizeDown', widgets.tabs.flags2, ImGui.TabBarFlags_FittingPolicyResizeDown()) then
        widgets.tabs.flags2 = widgets.tabs.flags2 & ~fitting_policy_mask | ImGui.TabBarFlags_FittingPolicyResizeDown()
      end
      if ImGui.CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyScroll', widgets.tabs.flags2, ImGui.TabBarFlags_FittingPolicyScroll()) then
        widgets.tabs.flags2 = widgets.tabs.flags2 & ~fitting_policy_mask | ImGui.TabBarFlags_FittingPolicyScroll()
      end

      if ImGui.BeginTabBar(ctx, 'MyTabBar', widgets.tabs.flags2) then
        -- Demo a Leading TabItemButton(): click the '?' button to open a menu
        if widgets.tabs.show_leading_button then
          if ImGui.TabItemButton(ctx, '?', ImGui.TabItemFlags_Leading() | ImGui.TabItemFlags_NoTooltip()) then
            ImGui.OpenPopup(ctx, 'MyHelpMenu')
          end
        end
        if ImGui.BeginPopup(ctx, 'MyHelpMenu') then
          ImGui.Selectable(ctx, 'Hello!')
          ImGui.EndPopup(ctx)
        end

        -- Demo Trailing Tabs: click the "+" button to add a new tab (in your app you may want to use a font icon instead of the "+")
        -- Note that we submit it before the regular tabs, but because of the ImGuiTabItemFlags_Trailing flag it will always appear at the end.
        if widgets.tabs.show_trailing_button then
          if ImGui.TabItemButton(ctx, '+', ImGui.TabItemFlags_Trailing() | ImGui.TabItemFlags_NoTooltip()) then
            -- add new tab
            table.insert(widgets.tabs.active, widgets.tabs.next_id)
            widgets.tabs.next_id = widgets.tabs.next_id + 1
          end
        end

        -- Submit our regular tabs
        local n = 1
        while n <= #widgets.tabs.active do
          local name = ('%04d'):format(widgets.tabs.active[n]-1)
          local open
          rv,open = ImGui.BeginTabItem(ctx, name, true, ImGui.TabItemFlags_None())
          if rv then
            ImGui.Text(ctx, ('This is the %s tab!'):format(name))
            ImGui.EndTabItem(ctx)
          end

          if open then
            n = n + 1
          else
            table.remove(widgets.tabs.active, n)
          end
        end

        ImGui.EndTabBar(ctx)
      end
      ImGui.Separator(ctx)
      ImGui.TreePop(ctx)
    end
    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Plotting') then
    local PLOT1_SIZE = 90
    local plot2_funcs   = {
      function(i) return math.sin(i * 0.1) end, -- sin
      function(i) return (i & 1) == 1 and 1.0 or -1.0 end, --saw
    }

    if not widgets.plots then
      widgets.plots = {
        animate = true,
        frame_times = reaper.new_array({ 0.6, 0.1, 1.0, 0.5, 0.92, 0.1, 0.2 }),
        plot1 = {
          offset       = 1,
          refresh_time = 0.0,
          phase        = 0.0,
          data         = reaper.new_array(PLOT1_SIZE),
        },
        plot2 = {
          func = 0,
          size = 70,
          fill = true,
          data = reaper.new_array(1),
        },
        progress     = 0.0,
        progress_dir = 1,
      }
      widgets.plots.plot1.data.clear()
    end

    rv,widgets.plots.animate = ImGui.Checkbox(ctx, 'Animate', widgets.plots.animate)

    -- Plot as lines and plot as histogram
    ImGui.PlotLines(ctx, 'Frame Times', widgets.plots.frame_times)
    ImGui.PlotHistogram(ctx, 'Histogram', widgets.plots.frame_times, 0, nil, 0.0, 1.0, 0, 80.0)

    -- Fill an array of contiguous float values to plot
    if not widgets.plots.animate or widgets.plots.plot1.refresh_time == 0.0 then
      widgets.plots.plot1.refresh_time = ImGui.GetTime(ctx)
    end
    while widgets.plots.plot1.refresh_time < ImGui.GetTime(ctx) do -- Create data at fixed 60 Hz rate for the demo
      widgets.plots.plot1.data[widgets.plots.plot1.offset] = math.cos(widgets.plots.plot1.phase)
      widgets.plots.plot1.offset = (widgets.plots.plot1.offset % PLOT1_SIZE) + 1
      widgets.plots.plot1.phase = widgets.plots.plot1.phase + (0.10 * widgets.plots.plot1.offset)
      widgets.plots.plot1.refresh_time = widgets.plots.plot1.refresh_time + (1.0 / 60.0)
    end

    -- Plots can display overlay texts
    -- (in this example, we will display an average value)
    do
      local average = 0.0
      for n = 1, PLOT1_SIZE do
        average = average + widgets.plots.plot1.data[n]
      end
      average = average / PLOT1_SIZE

      local overlay = ('avg %f'):format(average)
      ImGui.PlotLines(ctx, 'Lines', widgets.plots.plot1.data, widgets.plots.plot1.offset - 1, overlay, -1.0, 1.0, 0, 80.0)
    end

    ImGui.SeparatorText(ctx, 'Functions')
    ImGui.SetNextItemWidth(ctx, ImGui.GetFontSize(ctx) * 8)
    rv,widgets.plots.plot2.func = ImGui.Combo(ctx, 'func', widgets.plots.plot2.func, 'Sin\0Saw\0')
    local funcChanged = rv
    ImGui.SameLine(ctx)
    rv,widgets.plots.plot2.size = ImGui.SliderInt(ctx, 'Sample count', widgets.plots.plot2.size, 1, 400)

    -- Use functions to generate output
    if funcChanged or rv or widgets.plots.plot2.fill then
      widgets.plots.plot2.fill = false -- fill the first time
      widgets.plots.plot2.data = reaper.new_array(widgets.plots.plot2.size)
      for n = 1, widgets.plots.plot2.size do
        widgets.plots.plot2.data[n] = plot2_funcs[widgets.plots.plot2.func + 1](n - 1)
      end
    end

    ImGui.PlotLines(ctx, 'Lines', widgets.plots.plot2.data, 0, nil, -1.0, 1.0, 0, 80)
    ImGui.PlotHistogram(ctx, 'Histogram', widgets.plots.plot2.data, 0, nil, -1.0, 1.0, 0, 80)
    ImGui.Separator(ctx)

    -- Animate a simple progress bar
    if widgets.plots.animate then
      widgets.plots.progress = widgets.plots.progress +
        (widgets.plots.progress_dir * 0.4 * ImGui.GetDeltaTime(ctx))
      if widgets.plots.progress >= 1.1 then
        widgets.plots.progress = 1.1
        widgets.plots.progress_dir = widgets.plots.progress_dir * -1
      elseif widgets.plots.progress <= -0.1 then
        widgets.plots.progress = -0.1
        widgets.plots.progress_dir = widgets.plots.progress_dir * -1
      end
    end

    -- Typically we would use (-1.0,0.0) or (-FLT_MIN,0.0) to use all available width,
    -- or (width,0.0) for a specified width. (0.0,0.0) uses ItemWidth.
    ImGui.ProgressBar(ctx, widgets.plots.progress, 0.0, 0.0)
    ImGui.SameLine(ctx, 0.0, (ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())))
    ImGui.Text(ctx, 'Progress Bar')

    local progress_saturated = demo.clamp(widgets.plots.progress, 0.0, 1.0);
    local buf = ('%d/%d'):format(math.floor(progress_saturated * 1753), 1753)
    ImGui.ProgressBar(ctx, widgets.plots.progress, 0.0, 0.0, buf);

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Color/Picker Widgets') then
    if not widgets.colors then
      widgets.colors = {
        rgba               = 0x72909ac8,
        alpha_preview      = true,
        alpha_half_preview = false,
        drag_and_drop      = true,
        options_menu       = true,
        saved_palette      = nil, -- filled later
        backup_color       = nil,
        no_border          = false,
        alpha              = true,
        alpha_bar          = true,
        side_preview       = true,
        ref_color          = false,
        ref_color_rgba     = 0xff00ff80,
        display_mode       = 0,
        picker_mode        = 0,
        hsva               = 0x3bffffff,
        raw_hsv            = reaper.new_array(4),
      }
    end

    -- static bool hdr = false;
    ImGui.SeparatorText(ctx, 'Options')
    rv,widgets.colors.alpha_preview      = ImGui.Checkbox(ctx, 'With Alpha Preview',      widgets.colors.alpha_preview)
    rv,widgets.colors.alpha_half_preview = ImGui.Checkbox(ctx, 'With Half Alpha Preview', widgets.colors.alpha_half_preview)
    rv,widgets.colors.drag_and_drop      = ImGui.Checkbox(ctx, 'With Drag and Drop',      widgets.colors.drag_and_drop)
    rv,widgets.colors.options_menu       = ImGui.Checkbox(ctx, 'With Options Menu',       widgets.colors.options_menu)
    ImGui.SameLine(ctx); demo.HelpMarker('Right-click on the individual color widget to show options.')
    -- ImGui.Checkbox("With HDR", &hdr); ImGui.SameLine(); HelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.")
    local misc_flags = --(widgets.colors.hdr and ImGui.ColorEditFlags_HDR() or 0) |
    (widgets.colors.drag_and_drop and 0 or ImGui.ColorEditFlags_NoDragDrop()) |
    (widgets.colors.alpha_half_preview and ImGui.ColorEditFlags_AlphaPreviewHalf()
    or (widgets.colors.alpha_preview and ImGui.ColorEditFlags_AlphaPreview() or 0)) |
    (widgets.colors.options_menu  and 0 or ImGui.ColorEditFlags_NoOptions())

    ImGui.SeparatorText(ctx, 'Inline color editor')
    ImGui.Text(ctx, 'Color widget:')
    ImGui.SameLine(ctx); demo.HelpMarker(
      'Click on the color square to open a color picker.\n\z
       CTRL+click on individual component to input value.\n')
    local argb = demo.RgbaToArgb(widgets.colors.rgba)
    rv,argb = ImGui.ColorEdit3(ctx, 'MyColor##1', argb, misc_flags)
    if rv then
      widgets.colors.rgba = demo.ArgbToRgba(argb)
    end

    ImGui.Text(ctx, 'Color widget HSV with Alpha:')
    rv,widgets.colors.rgba = ImGui.ColorEdit4(ctx, 'MyColor##2', widgets.colors.rgba, ImGui.ColorEditFlags_DisplayHSV() | misc_flags)

    ImGui.Text(ctx, 'Color widget with Float Display:')
    rv,widgets.colors.rgba = ImGui.ColorEdit4(ctx, 'MyColor##2f', widgets.colors.rgba, ImGui.ColorEditFlags_Float() | misc_flags)

    ImGui.Text(ctx, 'Color button with Picker:')
    ImGui.SameLine(ctx); demo.HelpMarker(
      'With the ImGuiColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\n\z
       With the ImGuiColorEditFlags_NoLabel flag you can pass a non-empty label which will only \z
       be used for the tooltip and picker popup.')
    rv,widgets.colors.rgba = ImGui.ColorEdit4(ctx, 'MyColor##3', widgets.colors.rgba, ImGui.ColorEditFlags_NoInputs() | ImGui.ColorEditFlags_NoLabel() | misc_flags)

    ImGui.Text(ctx, 'Color button with Custom Picker Popup:')

    -- Generate a default palette. The palette will persist and can be edited.
    if not widgets.colors.saved_palette then
      widgets.colors.saved_palette = {}
      for n = 0, 31 do
        table.insert(widgets.colors.saved_palette, demo.HSV(n / 31.0, 0.8, 0.8))
      end
    end

    local open_popup = ImGui.ColorButton(ctx, 'MyColor##3b', widgets.colors.rgba, misc_flags)
    ImGui.SameLine(ctx, 0, (ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())))
    open_popup = ImGui.Button(ctx, 'Palette') or open_popup
    if open_popup then
      ImGui.OpenPopup(ctx, 'mypicker')
      widgets.colors.backup_color = widgets.colors.rgba
    end
    if ImGui.BeginPopup(ctx, 'mypicker') then
      ImGui.Text(ctx, 'MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!')
      ImGui.Separator(ctx)
      rv,widgets.colors.rgba = ImGui.ColorPicker4(ctx, '##picker', widgets.colors.rgba, misc_flags | ImGui.ColorEditFlags_NoSidePreview() | ImGui.ColorEditFlags_NoSmallPreview())
      ImGui.SameLine(ctx)

      ImGui.BeginGroup(ctx) -- Lock X position
      ImGui.Text(ctx, 'Current')
      ImGui.ColorButton(ctx, '##current', widgets.colors.rgba,
        ImGui.ColorEditFlags_NoPicker() |
        ImGui.ColorEditFlags_AlphaPreviewHalf(), 60, 40)
      ImGui.Text(ctx, 'Previous')
      if ImGui.ColorButton(ctx, '##previous', widgets.colors.backup_color,
          ImGui.ColorEditFlags_NoPicker() |
          ImGui.ColorEditFlags_AlphaPreviewHalf(), 60, 40) then
        widgets.colors.rgba = widgets.colors.backup_color
      end
      ImGui.Separator(ctx)
      ImGui.Text(ctx, 'Palette')
      local palette_button_flags = ImGui.ColorEditFlags_NoAlpha()  |
                                   ImGui.ColorEditFlags_NoPicker() |
                                   ImGui.ColorEditFlags_NoTooltip()
      for n,c in ipairs(widgets.colors.saved_palette) do
        ImGui.PushID(ctx, n)
        if ((n - 1) % 8) ~= 0 then
          ImGui.SameLine(ctx, 0.0, select(2, ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemSpacing())))
        end

        if ImGui.ColorButton(ctx, '##palette', c, palette_button_flags, 20, 20) then
          widgets.colors.rgba = (c << 8) | (widgets.colors.rgba & 0xFF) -- Preserve alpha!
        end

        -- Allow user to drop colors into each palette entry. Note that ColorButton() is already a
        -- drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
        if ImGui.BeginDragDropTarget(ctx) then
          local drop_color
          rv,drop_color = ImGui.AcceptDragDropPayloadRGB(ctx)
          if rv then
            widgets.colors.saved_palette[n] = drop_color
          end
          rv,drop_color = ImGui.AcceptDragDropPayloadRGBA(ctx)
          if rv then
            widgets.colors.saved_palette[n] = drop_color >> 8
          end
          ImGui.EndDragDropTarget(ctx)
        end

        ImGui.PopID(ctx)
      end
      ImGui.EndGroup(ctx)
      ImGui.EndPopup(ctx)
    end

    ImGui.Text(ctx, 'Color button only:')
    rv,widgets.colors.no_border = ImGui.Checkbox(ctx, 'ImGuiColorEditFlags_NoBorder', widgets.colors.no_border)
    ImGui.ColorButton(ctx, 'MyColor##3c', widgets.colors.rgba,
      misc_flags | (widgets.colors.no_border and ImGui.ColorEditFlags_NoBorder() or 0),
      80, 80)

    ImGui.SeparatorText(ctx, 'Color picker')
    rv,widgets.colors.alpha = ImGui.Checkbox(ctx, 'With Alpha', widgets.colors.alpha)
    rv,widgets.colors.alpha_bar = ImGui.Checkbox(ctx, 'With Alpha Bar', widgets.colors.alpha_bar)
    rv,widgets.colors.side_preview = ImGui.Checkbox(ctx, 'With Side Preview', widgets.colors.side_preview)
    if widgets.colors.side_preview then
      ImGui.SameLine(ctx)
      rv,widgets.colors.ref_color = ImGui.Checkbox(ctx, 'With Ref Color', widgets.colors.ref_color)
      if widgets.colors.ref_color then
        ImGui.SameLine(ctx)
        rv,widgets.colors.ref_color_rgba = ImGui.ColorEdit4(ctx, '##RefColor',
          widgets.colors.ref_color_rgba, ImGui.ColorEditFlags_NoInputs() | misc_flags)
      end
    end
    rv,widgets.colors.display_mode = ImGui.Combo(ctx, 'Display Mode', widgets.colors.display_mode,
      'Auto/Current\0None\0RGB Only\0HSV Only\0Hex Only\0')
    ImGui.SameLine(ctx); demo.HelpMarker(
      "ColorEdit defaults to displaying RGB inputs if you don't specify a display mode, \z
       but the user can change it with a right-click on those inputs.\n\nColorPicker defaults to displaying RGB+HSV+Hex \z
       if you don't specify a display mode.\n\nYou can change the defaults using SetColorEditOptions().")
    rv,widgets.colors.picker_mode = ImGui.Combo(ctx, 'Picker Mode', widgets.colors.picker_mode,
      'Auto/Current\0Hue bar + SV rect\0Hue wheel + SV triangle\0')
    ImGui.SameLine(ctx); demo.HelpMarker('When not specified explicitly (Auto/Current mode), user can right-click the picker to change mode.')
    local flags = misc_flags
    if not widgets.colors.alpha         then flags = flags | ImGui.ColorEditFlags_NoAlpha()        end
    if widgets.colors.alpha_bar         then flags = flags | ImGui.ColorEditFlags_AlphaBar()       end
    if not widgets.colors.side_preview  then flags = flags | ImGui.ColorEditFlags_NoSidePreview()  end
    if widgets.colors.picker_mode  == 1 then flags = flags | ImGui.ColorEditFlags_PickerHueBar()   end
    if widgets.colors.picker_mode  == 2 then flags = flags | ImGui.ColorEditFlags_PickerHueWheel() end
    if widgets.colors.display_mode == 1 then flags = flags | ImGui.ColorEditFlags_NoInputs()       end -- Disable all RGB/HSV/Hex displays
    if widgets.colors.display_mode == 2 then flags = flags | ImGui.ColorEditFlags_DisplayRGB()     end -- Override display mode
    if widgets.colors.display_mode == 3 then flags = flags | ImGui.ColorEditFlags_DisplayHSV()     end
    if widgets.colors.display_mode == 4 then flags = flags | ImGui.ColorEditFlags_DisplayHex()     end

    local color = widgets.colors.alpha and widgets.colors.rgba or demo.RgbaToArgb(widgets.colors.rgba)
    local ref_color = widgets.colors.alpha and widgets.colors.ref_color_rgba or demo.RgbaToArgb(widgets.colors.ref_color_rgba)
    rv,color = ImGui.ColorPicker4(ctx, 'MyColor##4', color, flags,
      widgets.colors.ref_color and ref_color or nil)
    if rv then
      widgets.colors.rgba = widgets.colors.alpha and color or demo.ArgbToRgba(color)
    end

    ImGui.Text(ctx, 'Set defaults in code:')
    ImGui.SameLine(ctx); demo.HelpMarker(
      "SetColorEditOptions() is designed to allow you to set boot-time default.\n\z
       We don't have Push/Pop functions because you can force options on a per-widget basis if needed,\z
       and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid\z
       encouraging you to persistently save values that aren't forward-compatible.")
    if ImGui.Button(ctx, 'Default: Uint8 + HSV + Hue Bar') then
      ImGui.SetColorEditOptions(ctx, ImGui.ColorEditFlags_Uint8() | ImGui.ColorEditFlags_DisplayHSV() | ImGui.ColorEditFlags_PickerHueBar())
    end
    if ImGui.Button(ctx, 'Default: Float + Hue Wheel') then -- (NOTE: removed HDR for ReaImGui as we use uint32 for color i/o)
      ImGui.SetColorEditOptions(ctx, ImGui.ColorEditFlags_Float() | ImGui.ColorEditFlags_PickerHueWheel())
    end

    -- Always both a small version of both types of pickers (to make it more visible in the demo to people who are skimming quickly through it)
    local color = demo.RgbaToArgb(widgets.colors.rgba)
    ImGui.Text(ctx, 'Both types:')
    local w = (ImGui.GetContentRegionAvail(ctx) - select(2, ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemSpacing()))) * 0.40
    ImGui.SetNextItemWidth(ctx, w)
    rv,color = ImGui.ColorPicker3(ctx, '##MyColor##5', color, ImGui.ColorEditFlags_PickerHueBar() | ImGui.ColorEditFlags_NoSidePreview() | ImGui.ColorEditFlags_NoInputs() | ImGui.ColorEditFlags_NoAlpha())
    if rv then widgets.colors.rgba = demo.ArgbToRgba(color) end
    ImGui.SameLine(ctx)
    ImGui.SetNextItemWidth(ctx, w)
    rv,color = ImGui.ColorPicker3(ctx, '##MyColor##6', color, ImGui.ColorEditFlags_PickerHueWheel() | ImGui.ColorEditFlags_NoSidePreview() | ImGui.ColorEditFlags_NoInputs() | ImGui.ColorEditFlags_NoAlpha())
    if rv then widgets.colors.rgba = demo.ArgbToRgba(color) end

    -- HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
    ImGui.Spacing(ctx)
    ImGui.Text(ctx, 'HSV encoded colors')
    ImGui.SameLine(ctx); demo.HelpMarker(
      'By default, colors are given to ColorEdit and ColorPicker in RGB, but ImGuiColorEditFlags_InputHSV \z
       allows you to store colors as HSV and pass them to ColorEdit and ColorPicker as HSV. This comes with the \z
       added benefit that you can manipulate hue values with the picker even when saturation or value are zero.')
    ImGui.Text(ctx, 'Color widget with InputHSV:')
    rv,widgets.colors.hsva = ImGui.ColorEdit4(ctx, 'HSV shown as RGB##1', widgets.colors.hsva,
      ImGui.ColorEditFlags_DisplayRGB() | ImGui.ColorEditFlags_InputHSV() | ImGui.ColorEditFlags_Float())
    rv,widgets.colors.hsva = ImGui.ColorEdit4(ctx, 'HSV shown as HSV##1', widgets.colors.hsva,
      ImGui.ColorEditFlags_DisplayHSV() | ImGui.ColorEditFlags_InputHSV() | ImGui.ColorEditFlags_Float())

    local raw_hsv = widgets.colors.raw_hsv
    raw_hsv[1] = (widgets.colors.hsva >> 24 & 0xFF) / 255.0 -- H
    raw_hsv[2] = (widgets.colors.hsva >> 16 & 0xFF) / 255.0 -- S
    raw_hsv[3] = (widgets.colors.hsva >>  8 & 0xFF) / 255.0 -- V
    raw_hsv[4] = (widgets.colors.hsva       & 0xFF) / 255.0 -- A
    if ImGui.DragDoubleN(ctx, 'Raw HSV values', raw_hsv, 0.01, 0.0, 1.0) then
      widgets.colors.hsva =
        (demo.round(raw_hsv[1] * 0xFF) << 24) |
        (demo.round(raw_hsv[2] * 0xFF) << 16) |
        (demo.round(raw_hsv[3] * 0xFF) <<  8) |
        (demo.round(raw_hsv[4] * 0xFF)      )
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Drag/Slider Flags') then
    if not widgets.sliders then
      widgets.sliders = {
        flags    = ImGui.SliderFlags_None(),
        drag_d   = 0.5,
        drag_i   = 50,
        slider_d = 0.5,
        slider_i = 50,
      }
    end

    -- Demonstrate using advanced flags for DragXXX and SliderXXX functions. Note that the flags are the same!
    rv,widgets.sliders.flags = ImGui.CheckboxFlags(ctx, 'ImGuiSliderFlags_AlwaysClamp', widgets.sliders.flags, ImGui.SliderFlags_AlwaysClamp())
    ImGui.SameLine(ctx); demo.HelpMarker('Always clamp value to min/max bounds (if any) when input manually with CTRL+Click.')
    rv,widgets.sliders.flags = ImGui.CheckboxFlags(ctx, 'ImGuiSliderFlags_Logarithmic', widgets.sliders.flags, ImGui.SliderFlags_Logarithmic())
    ImGui.SameLine(ctx); demo.HelpMarker('Enable logarithmic editing (more precision for small values).')
    rv,widgets.sliders.flags = ImGui.CheckboxFlags(ctx, 'ImGuiSliderFlags_NoRoundToFormat', widgets.sliders.flags, ImGui.SliderFlags_NoRoundToFormat())
    ImGui.SameLine(ctx); demo.HelpMarker('Disable rounding underlying value to match precision of the format string (e.g. %.3f values are rounded to those 3 digits).')
    rv,widgets.sliders.flags = ImGui.CheckboxFlags(ctx, 'ImGuiSliderFlags_NoInput', widgets.sliders.flags, ImGui.SliderFlags_NoInput())
    ImGui.SameLine(ctx); demo.HelpMarker('Disable CTRL+Click or Enter key allowing to input text directly into the widget.')

    local DBL_MIN, DBL_MAX = 2.22507e-308, 1.79769e+308

    -- Drags
    ImGui.Text(ctx, ('Underlying double value: %f'):format(widgets.sliders.drag_d))
    rv,widgets.sliders.drag_d = ImGui.DragDouble(ctx, 'DragDouble (0 -> 1)', widgets.sliders.drag_d, 0.005, 0.0, 1.0, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_d = ImGui.DragDouble(ctx, 'DragDouble (0 -> +inf)', widgets.sliders.drag_d, 0.005, 0.0, DBL_MAX, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_d = ImGui.DragDouble(ctx, 'DragDouble (-inf -> 1)', widgets.sliders.drag_d, 0.005, -DBL_MAX, 1.0, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_d = ImGui.DragDouble(ctx, 'DragDouble (-inf -> +inf)', widgets.sliders.drag_d, 0.005, -DBL_MAX, DBL_MAX, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_i = ImGui.DragInt(ctx, 'DragInt (0 -> 100)', widgets.sliders.drag_i, 0.5, 0, 100, '%d', widgets.sliders.flags)

    -- Sliders
    ImGui.Text(ctx, ('Underlying float value: %f'):format(widgets.sliders.slider_d))
    rv,widgets.sliders.slider_d = ImGui.SliderDouble(ctx, 'SliderDouble (0 -> 1)', widgets.sliders.slider_d, 0.0, 1.0, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.slider_i = ImGui.SliderInt(ctx, 'SliderInt (0 -> 100)', widgets.sliders.slider_i, 0, 100, '%d', widgets.sliders.flags)

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Range Widgets') then
    if not widgets.range then
      widgets.range = {
        begin_f = 10.0,
        end_f   = 90.0,
        begin_i = 100,
        end_i   = 1000,
      }
    end

    rv,widgets.range.begin_f,widgets.range.end_f = ImGui.DragFloatRange2(ctx, 'range float', widgets.range.begin_f, widgets.range.end_f, 0.25, 0.0, 100.0, 'Min: %.1f %%', 'Max: %.1f %%', ImGui.SliderFlags_AlwaysClamp())
    rv,widgets.range.begin_i,widgets.range.end_i = ImGui.DragIntRange2(ctx, 'range int', widgets.range.begin_i, widgets.range.end_i, 5, 0, 1000, 'Min: %d units', 'Max: %d units')
    rv,widgets.range.begin_i,widgets.range.end_i = ImGui.DragIntRange2(ctx, 'range int (no bounds)', widgets.range.begin_i, widgets.range.end_i, 5, 0, 0, 'Min: %d units', 'Max: %d units')
    ImGui.TreePop(ctx)
  end

--     if (ImGui.TreeNode("Data Types"))
--     {
--         // DragScalar/InputScalar/SliderScalar functions allow various data types
--         // - signed/unsigned
--         // - 8/16/32/64-bits
--         // - integer/float/double
--         // To avoid polluting the public API with all possible combinations, we use the ImGuiDataType enum
--         // to pass the type, and passing all arguments by pointer.
--         // This is the reason the test code below creates local variables to hold "zero" "one" etc. for each type.
--         // In practice, if you frequently use a given type that is not covered by the normal API entry points,
--         // you can wrap it yourself inside a 1 line function which can take typed argument as value instead of void*,
--         // and then pass their address to the generic function. For example:
--         //   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0, const char* format = "%lld")
--         //   {
--         //      return SliderScalar(label, ImGuiDataType_U64, value, &min, &max, format);
--         //   }
--
--         // Setup limits (as helper variables so we can take their address, as explained above)
--         // Note: SliderScalar() functions have a maximum usable range of half the natural type maximum, hence the /2.
--         #ifndef LLONG_MIN
--         ImS64 LLONG_MIN = -9223372036854775807LL - 1;
--         ImS64 LLONG_MAX = 9223372036854775807LL;
--         ImU64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
--         #endif
--         const char    s8_zero  = 0,   s8_one  = 1,   s8_fifty  = 50, s8_min  = -128,        s8_max = 127;
--         const ImU8    u8_zero  = 0,   u8_one  = 1,   u8_fifty  = 50, u8_min  = 0,           u8_max = 255;
--         const short   s16_zero = 0,   s16_one = 1,   s16_fifty = 50, s16_min = -32768,      s16_max = 32767;
--         const ImU16   u16_zero = 0,   u16_one = 1,   u16_fifty = 50, u16_min = 0,           u16_max = 65535;
--         const ImS32   s32_zero = 0,   s32_one = 1,   s32_fifty = 50, s32_min = INT_MIN/2,   s32_max = INT_MAX/2,    s32_hi_a = INT_MAX/2 - 100,    s32_hi_b = INT_MAX/2;
--         const ImU32   u32_zero = 0,   u32_one = 1,   u32_fifty = 50, u32_min = 0,           u32_max = UINT_MAX/2,   u32_hi_a = UINT_MAX/2 - 100,   u32_hi_b = UINT_MAX/2;
--         const ImS64   s64_zero = 0,   s64_one = 1,   s64_fifty = 50, s64_min = LLONG_MIN/2, s64_max = LLONG_MAX/2,  s64_hi_a = LLONG_MAX/2 - 100,  s64_hi_b = LLONG_MAX/2;
--         const ImU64   u64_zero = 0,   u64_one = 1,   u64_fifty = 50, u64_min = 0,           u64_max = ULLONG_MAX/2, u64_hi_a = ULLONG_MAX/2 - 100, u64_hi_b = ULLONG_MAX/2;
--         const float   f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f, f32_hi_a = +10000000000.0f;
--         const double  f64_zero = 0.,  f64_one = 1.,  f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;
--
--         // State
--         static char   s8_v  = 127;
--         static ImU8   u8_v  = 255;
--         static short  s16_v = 32767;
--         static ImU16  u16_v = 65535;
--         static ImS32  s32_v = -1;
--         static ImU32  u32_v = (ImU32)-1;
--         static ImS64  s64_v = -1;
--         static ImU64  u64_v = (ImU64)-1;
--         static float  f32_v = 0.123f;
--         static double f64_v = 90000.01234567890123456789;
--
--         const float drag_speed = 0.2f;
--         static bool drag_clamp = false;
--         ImGui.SeparatorText("Drags");
--         ImGui.Checkbox("Clamp integers to 0..50", &drag_clamp);
--         ImGui.SameLine(); HelpMarker(
--             "As with every widget in dear imgui, we never modify values unless there is a user interaction.\n"
--             "You can override the clamping limits by using CTRL+Click to input a value.");
--         ImGui.DragScalar("drag s8",        ImGuiDataType_S8,     &s8_v,  drag_speed, drag_clamp ? &s8_zero  : NULL, drag_clamp ? &s8_fifty  : NULL);
--         ImGui.DragScalar("drag u8",        ImGuiDataType_U8,     &u8_v,  drag_speed, drag_clamp ? &u8_zero  : NULL, drag_clamp ? &u8_fifty  : NULL, "%u ms");
--         ImGui.DragScalar("drag s16",       ImGuiDataType_S16,    &s16_v, drag_speed, drag_clamp ? &s16_zero : NULL, drag_clamp ? &s16_fifty : NULL);
--         ImGui.DragScalar("drag u16",       ImGuiDataType_U16,    &u16_v, drag_speed, drag_clamp ? &u16_zero : NULL, drag_clamp ? &u16_fifty : NULL, "%u ms");
--         ImGui.DragScalar("drag s32",       ImGuiDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL);
--         ImGui.DragScalar("drag s32 hex",   ImGuiDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL, "0x%08X");
--         ImGui.DragScalar("drag u32",       ImGuiDataType_U32,    &u32_v, drag_speed, drag_clamp ? &u32_zero : NULL, drag_clamp ? &u32_fifty : NULL, "%u ms");
--         ImGui.DragScalar("drag s64",       ImGuiDataType_S64,    &s64_v, drag_speed, drag_clamp ? &s64_zero : NULL, drag_clamp ? &s64_fifty : NULL);
--         ImGui.DragScalar("drag u64",       ImGuiDataType_U64,    &u64_v, drag_speed, drag_clamp ? &u64_zero : NULL, drag_clamp ? &u64_fifty : NULL);
--         ImGui.DragScalar("drag float",     ImGuiDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f");
--         ImGui.DragScalar("drag float log", ImGuiDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f", ImGuiSliderFlags_Logarithmic);
--         ImGui.DragScalar("drag double",    ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, NULL,     "%.10f grams");
--         ImGui.DragScalar("drag double log",ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, &f64_one, "0 < %.10f < 1", ImGuiSliderFlags_Logarithmic);
--
--         ImGui.SeparatorText("Sliders");
--         ImGui.SliderScalar("slider s8 full",       ImGuiDataType_S8,     &s8_v,  &s8_min,   &s8_max,   "%d");
--         ImGui.SliderScalar("slider u8 full",       ImGuiDataType_U8,     &u8_v,  &u8_min,   &u8_max,   "%u");
--         ImGui.SliderScalar("slider s16 full",      ImGuiDataType_S16,    &s16_v, &s16_min,  &s16_max,  "%d");
--         ImGui.SliderScalar("slider u16 full",      ImGuiDataType_U16,    &u16_v, &u16_min,  &u16_max,  "%u");
--         ImGui.SliderScalar("slider s32 low",       ImGuiDataType_S32,    &s32_v, &s32_zero, &s32_fifty,"%d");
--         ImGui.SliderScalar("slider s32 high",      ImGuiDataType_S32,    &s32_v, &s32_hi_a, &s32_hi_b, "%d");
--         ImGui.SliderScalar("slider s32 full",      ImGuiDataType_S32,    &s32_v, &s32_min,  &s32_max,  "%d");
--         ImGui.SliderScalar("slider s32 hex",       ImGuiDataType_S32,    &s32_v, &s32_zero, &s32_fifty, "0x%04X");
--         ImGui.SliderScalar("slider u32 low",       ImGuiDataType_U32,    &u32_v, &u32_zero, &u32_fifty,"%u");
--         ImGui.SliderScalar("slider u32 high",      ImGuiDataType_U32,    &u32_v, &u32_hi_a, &u32_hi_b, "%u");
--         ImGui.SliderScalar("slider u32 full",      ImGuiDataType_U32,    &u32_v, &u32_min,  &u32_max,  "%u");
--         ImGui.SliderScalar("slider s64 low",       ImGuiDataType_S64,    &s64_v, &s64_zero, &s64_fifty,"%I64d");
--         ImGui.SliderScalar("slider s64 high",      ImGuiDataType_S64,    &s64_v, &s64_hi_a, &s64_hi_b, "%I64d");
--         ImGui.SliderScalar("slider s64 full",      ImGuiDataType_S64,    &s64_v, &s64_min,  &s64_max,  "%I64d");
--         ImGui.SliderScalar("slider u64 low",       ImGuiDataType_U64,    &u64_v, &u64_zero, &u64_fifty,"%I64u ms");
--         ImGui.SliderScalar("slider u64 high",      ImGuiDataType_U64,    &u64_v, &u64_hi_a, &u64_hi_b, "%I64u ms");
--         ImGui.SliderScalar("slider u64 full",      ImGuiDataType_U64,    &u64_v, &u64_min,  &u64_max,  "%I64u ms");
--         ImGui.SliderScalar("slider float low",     ImGuiDataType_Float,  &f32_v, &f32_zero, &f32_one);
--         ImGui.SliderScalar("slider float low log", ImGuiDataType_Float,  &f32_v, &f32_zero, &f32_one,  "%.10f", ImGuiSliderFlags_Logarithmic);
--         ImGui.SliderScalar("slider float high",    ImGuiDataType_Float,  &f32_v, &f32_lo_a, &f32_hi_a, "%e");
--         ImGui.SliderScalar("slider double low",    ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f grams");
--         ImGui.SliderScalar("slider double low log",ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f", ImGuiSliderFlags_Logarithmic);
--         ImGui.SliderScalar("slider double high",   ImGuiDataType_Double, &f64_v, &f64_lo_a, &f64_hi_a, "%e grams");
--
--         ImGui.SeparatorText("Sliders (reverse)");
--         ImGui.SliderScalar("slider s8 reverse",    ImGuiDataType_S8,   &s8_v,  &s8_max,    &s8_min, "%d");
--         ImGui.SliderScalar("slider u8 reverse",    ImGuiDataType_U8,   &u8_v,  &u8_max,    &u8_min, "%u");
--         ImGui.SliderScalar("slider s32 reverse",   ImGuiDataType_S32,  &s32_v, &s32_fifty, &s32_zero, "%d");
--         ImGui.SliderScalar("slider u32 reverse",   ImGuiDataType_U32,  &u32_v, &u32_fifty, &u32_zero, "%u");
--         ImGui.SliderScalar("slider s64 reverse",   ImGuiDataType_S64,  &s64_v, &s64_fifty, &s64_zero, "%I64d");
--         ImGui.SliderScalar("slider u64 reverse",   ImGuiDataType_U64,  &u64_v, &u64_fifty, &u64_zero, "%I64u ms");
--
--         static bool inputs_step = true;
--         ImGui.SeparatorText("Inputs");
--         ImGui.Checkbox("Show step buttons", &inputs_step);
--         ImGui.InputScalar("input s8",      ImGuiDataType_S8,     &s8_v,  inputs_step ? &s8_one  : NULL, NULL, "%d");
--         ImGui.InputScalar("input u8",      ImGuiDataType_U8,     &u8_v,  inputs_step ? &u8_one  : NULL, NULL, "%u");
--         ImGui.InputScalar("input s16",     ImGuiDataType_S16,    &s16_v, inputs_step ? &s16_one : NULL, NULL, "%d");
--         ImGui.InputScalar("input u16",     ImGuiDataType_U16,    &u16_v, inputs_step ? &u16_one : NULL, NULL, "%u");
--         ImGui.InputScalar("input s32",     ImGuiDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%d");
--         ImGui.InputScalar("input s32 hex", ImGuiDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%04X");
--         ImGui.InputScalar("input u32",     ImGuiDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%u");
--         ImGui.InputScalar("input u32 hex", ImGuiDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%08X");
--         ImGui.InputScalar("input s64",     ImGuiDataType_S64,    &s64_v, inputs_step ? &s64_one : NULL);
--         ImGui.InputScalar("input u64",     ImGuiDataType_U64,    &u64_v, inputs_step ? &u64_one : NULL);
--         ImGui.InputScalar("input float",   ImGuiDataType_Float,  &f32_v, inputs_step ? &f32_one : NULL);
--         ImGui.InputScalar("input double",  ImGuiDataType_Double, &f64_v, inputs_step ? &f64_one : NULL);
--
--         ImGui.TreePop();
--     }

  if ImGui.TreeNode(ctx, 'Multi-component Widgets') then
    if not widgets.multi_component then
      widgets.multi_component = {
        vec4d = { 0.10, 0.20, 0.30, 0.44 },
        vec4i = { 1, 5, 100, 255 },
        vec4a = reaper.new_array({ 0.10, 0.20, 0.30, 0.44 }),
      }
    end

    local vec4d = widgets.multi_component.vec4d
    local vec4i = widgets.multi_component.vec4i

    ImGui.SeparatorText(ctx, '2-wide')
    rv,vec4d[1],vec4d[2] = ImGui.InputDouble2(ctx, 'input double2', vec4d[1], vec4d[2])
    rv,vec4d[1],vec4d[2] = ImGui.DragDouble2(ctx, 'drag double2', vec4d[1], vec4d[2], 0.01, 0.0, 1.0)
    rv,vec4d[1],vec4d[2] = ImGui.SliderDouble2(ctx, 'slider double2', vec4d[1], vec4d[2], 0.0, 1.0)
    rv,vec4i[1],vec4i[2] = ImGui.InputInt2(ctx, 'input int2', vec4i[1], vec4i[2])
    rv,vec4i[1],vec4i[2] = ImGui.DragInt2(ctx, 'drag int2', vec4i[1], vec4i[2], 1, 0, 255)
    rv,vec4i[1],vec4i[2] = ImGui.SliderInt2(ctx, 'slider int2', vec4i[1], vec4i[2], 0, 255)

    ImGui.SeparatorText(ctx, '3-wide')
    rv,vec4d[1],vec4d[2],vec4d[3] = ImGui.InputDouble3(ctx, 'input double3', vec4d[1], vec4d[2], vec4d[3])
    rv,vec4d[1],vec4d[2],vec4d[3] = ImGui.DragDouble3(ctx, 'drag double3', vec4d[1], vec4d[2], vec4d[3], 0.01, 0.0, 1.0)
    rv,vec4d[1],vec4d[2],vec4d[3] = ImGui.SliderDouble3(ctx, 'slider double3', vec4d[1], vec4d[2], vec4d[3], 0.0, 1.0)
    rv,vec4i[1],vec4i[2],vec4i[3] = ImGui.InputInt3(ctx, 'input int3', vec4i[1], vec4i[2], vec4i[3])
    rv,vec4i[1],vec4i[2],vec4i[3] = ImGui.DragInt3(ctx, 'drag int3', vec4i[1], vec4i[2], vec4i[3], 1, 0, 255)
    rv,vec4i[1],vec4i[2],vec4i[3] = ImGui.SliderInt3(ctx, 'slider int3', vec4i[1], vec4i[2], vec4i[3], 0, 255)

    ImGui.SeparatorText(ctx, '4-wide')
    rv,vec4d[1],vec4d[2],vec4d[3],vec4d[4] = ImGui.InputDouble4(ctx, 'input double4', vec4d[1], vec4d[2], vec4d[3], vec4d[4])
    rv,vec4d[1],vec4d[2],vec4d[3],vec4d[4] = ImGui.DragDouble4(ctx, 'drag double4', vec4d[1], vec4d[2], vec4d[3], vec4d[4], 0.01, 0.0, 1.0)
    rv,vec4d[1],vec4d[2],vec4d[3],vec4d[4] = ImGui.SliderDouble4(ctx, 'slider double4', vec4d[1], vec4d[2], vec4d[3], vec4d[4], 0.0, 1.0)
    rv,vec4i[1],vec4i[2],vec4i[3],vec4i[4] = ImGui.InputInt4(ctx, 'input int4', vec4i[1], vec4i[2], vec4i[3], vec4i[4])
    rv,vec4i[1],vec4i[2],vec4i[3],vec4i[4] = ImGui.DragInt4(ctx, 'drag int4', vec4i[1], vec4i[2], vec4i[3], vec4i[4], 1, 0, 255)
    rv,vec4i[1],vec4i[2],vec4i[3],vec4i[4] = ImGui.SliderInt4(ctx, 'slider int4', vec4i[1], vec4i[2], vec4i[3], vec4i[4], 0, 255)
    ImGui.Spacing(ctx)

    ImGui.InputDoubleN(ctx, 'input reaper.array', widgets.multi_component.vec4a)
    ImGui.DragDoubleN(ctx, 'drag reaper.array', widgets.multi_component.vec4a, 0.01, 0.0, 1.0)
    ImGui.SliderDoubleN(ctx, 'slider reaper.array', widgets.multi_component.vec4a, 0.0, 1.0)

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Vertical Sliders') then
    if not widgets.vsliders then
      widgets.vsliders = {
        int_value = 0,
        values    = { 0.0,  0.60, 0.35, 0.9, 0.70, 0.20, 0.0 },
        values2   = { 0.20, 0.80, 0.40, 0.25 },
      }
    end

    local spacing = 4
    ImGui.PushStyleVar(ctx, ImGui.StyleVar_ItemSpacing(), spacing, spacing)

    rv,widgets.vsliders.int_value = ImGui.VSliderInt(ctx, '##int', 18, 160, widgets.vsliders.int_value, 0, 5)
    ImGui.SameLine(ctx)

    ImGui.PushID(ctx, 'set1')
    for i,v in ipairs(widgets.vsliders.values) do
      if i > 1 then ImGui.SameLine(ctx) end
      ImGui.PushID(ctx, i)
      ImGui.PushStyleColor(ctx, ImGui.Col_FrameBg(),        demo.HSV((i-1) / 7.0, 0.5, 0.5, 1.0))
      ImGui.PushStyleColor(ctx, ImGui.Col_FrameBgHovered(), demo.HSV((i-1) / 7.0, 0.6, 0.5, 1.0))
      ImGui.PushStyleColor(ctx, ImGui.Col_FrameBgActive(),  demo.HSV((i-1) / 7.0, 0.7, 0.5, 1.0))
      ImGui.PushStyleColor(ctx, ImGui.Col_SliderGrab(),     demo.HSV((i-1) / 7.0, 0.9, 0.9, 1.0))
      rv,widgets.vsliders.values[i] = ImGui.VSliderDouble(ctx, '##v', 18, 160, v, 0.0, 1.0, ' ')
      if ImGui.IsItemActive(ctx) or ImGui.IsItemHovered(ctx) then
        ImGui.SetTooltip(ctx, ('%.3f'):format(v))
      end
      ImGui.PopStyleColor(ctx, 4)
      ImGui.PopID(ctx)
    end
    ImGui.PopID(ctx)

    ImGui.SameLine(ctx)
    ImGui.PushID(ctx, 'set2')
    local rows = 3
    local small_slider_w, small_slider_h = 18, (160.0 - (rows - 1) * spacing) / rows
    for nx,v2 in ipairs(widgets.vsliders.values2) do
      if nx > 1 then ImGui.SameLine(ctx) end
      ImGui.BeginGroup(ctx)
      for ny = 0, rows - 1 do
        ImGui.PushID(ctx, nx * rows + ny)
        rv,v2 = ImGui.VSliderDouble(ctx, '##v', small_slider_w, small_slider_h, v2, 0.0, 1.0, ' ')
        if rv then
          widgets.vsliders.values2[nx] = v2
        end
        if ImGui.IsItemActive(ctx) or ImGui.IsItemHovered(ctx) then
          ImGui.SetTooltip(ctx, ('%.3f'):format(v2))
        end
        ImGui.PopID(ctx)
      end
      ImGui.EndGroup(ctx)
    end
    ImGui.PopID(ctx)

    ImGui.SameLine(ctx)
    ImGui.PushID(ctx, 'set3')
    for i = 1, 4 do
      local v = widgets.vsliders.values[i]
      if i > 1 then ImGui.SameLine(ctx) end
      ImGui.PushID(ctx, i)
      ImGui.PushStyleVar(ctx, ImGui.StyleVar_GrabMinSize(), 40)
      rv,widgets.vsliders.values[i] = ImGui.VSliderDouble(ctx, '##v', 40, 160, v, 0.0, 1.0, '%.2f\nsec')
      ImGui.PopStyleVar(ctx)
      ImGui.PopID(ctx)
    end
    ImGui.PopID(ctx)
    ImGui.PopStyleVar(ctx)
    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Drag and Drop') then
    if not widgets.dragdrop then
      widgets.dragdrop = {
        color1 = 0xFF0033,
        color2 = 0x66B30080,
        mode   = 0,
        names  = {
          'Bobby', 'Beatrice', 'Betty',
          'Brianna', 'Barry', 'Bernard',
          'Bibi', 'Blaine', 'Bryn',
        },
        items  = { 'Item One', 'Item Two', 'Item Three', 'Item Four', 'Item Five' },
        files = {},
      }
    end

    if ImGui.TreeNode(ctx, 'Drag and drop in standard widgets') then
      -- ColorEdit widgets automatically act as drag source and drag target.
      -- They are using standardized payload types accessible using
      -- ImGui_AcceptDragDropPayloadRGB or ImGui_AcceptDragDropPayloadRGBA
      -- to allow your own widgets to use colors in their drag and drop interaction.
      -- Also see 'Demo->Widgets->Color/Picker Widgets->Palette' demo.
      demo.HelpMarker('You can drag from the color squares.')
      rv,widgets.dragdrop.color1 = ImGui.ColorEdit3(ctx, 'color 1', widgets.dragdrop.color1)
      rv,widgets.dragdrop.color2 = ImGui.ColorEdit4(ctx, 'color 2', widgets.dragdrop.color2)
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Drag and drop to copy/swap items') then
      local mode_copy, mode_move, mode_swap = 0, 1, 2
      if ImGui.RadioButton(ctx, 'Copy', widgets.dragdrop.mode == mode_copy) then widgets.dragdrop.mode = mode_copy end ImGui.SameLine(ctx)
      if ImGui.RadioButton(ctx, 'Move', widgets.dragdrop.mode == mode_move) then widgets.dragdrop.mode = mode_move end ImGui.SameLine(ctx)
      if ImGui.RadioButton(ctx, 'Swap', widgets.dragdrop.mode == mode_swap) then widgets.dragdrop.mode = mode_swap end
      for n,name in ipairs(widgets.dragdrop.names) do
        ImGui.PushID(ctx, n)
        if ((n-1) % 3) ~= 0 then
          ImGui.SameLine(ctx)
        end
        ImGui.Button(ctx, name, 60, 60)

        -- Our buttons are both drag sources and drag targets here!
        if ImGui.BeginDragDropSource(ctx, ImGui.DragDropFlags_None()) then
          -- Set payload to carry the index of our item (could be anything)
          ImGui.SetDragDropPayload(ctx, 'DND_DEMO_CELL', tostring(n))

          -- Display preview (could be anything, e.g. when dragging an image we could decide to display
          -- the filename and a small preview of the image, etc.)
          if widgets.dragdrop.mode == mode_copy then ImGui.Text(ctx, ('Copy %s'):format(name)) end
          if widgets.dragdrop.mode == mode_move then ImGui.Text(ctx, ('Move %s'):format(name)) end
          if widgets.dragdrop.mode == mode_swap then ImGui.Text(ctx, ('Swap %s'):format(name)) end
          ImGui.EndDragDropSource(ctx)
        end
        if ImGui.BeginDragDropTarget(ctx) then
          local payload
          rv,payload = ImGui.AcceptDragDropPayload(ctx, 'DND_DEMO_CELL')
          if rv then
            local payload_n = tonumber(payload)
            if widgets.dragdrop.mode == mode_copy then
              widgets.dragdrop.names[n] = widgets.dragdrop.names[payload_n]
            end
            if widgets.dragdrop.mode == mode_move then
              widgets.dragdrop.names[n] = widgets.dragdrop.names[payload_n]
              widgets.dragdrop.names[payload_n] = ''
            end
            if widgets.dragdrop.mode == mode_swap then
              widgets.dragdrop.names[n] = widgets.dragdrop.names[payload_n]
              widgets.dragdrop.names[payload_n] = name
            end
          end
          ImGui.EndDragDropTarget(ctx)
        end
        ImGui.PopID(ctx)
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Drag to reorder items (simple)') then
      -- Simple reordering
      demo.HelpMarker(
        "We don't use the drag and drop api at all here! \z
         Instead we query when the item is held but not hovered, and order items accordingly.")
      for n,item in ipairs(widgets.dragdrop.items) do
        ImGui.Selectable(ctx, item)

        if ImGui.IsItemActive(ctx) and not ImGui.IsItemHovered(ctx) then
          local mouse_delta = select(2, ImGui.GetMouseDragDelta(ctx, ImGui.MouseButton_Left()))
          local n_next = n + (mouse_delta < 0 and -1 or 1)
          if n_next >= 1 and n_next <= #widgets.dragdrop.items then
            widgets.dragdrop.items[n] = widgets.dragdrop.items[n_next]
            widgets.dragdrop.items[n_next] = item
            ImGui.ResetMouseDragDelta(ctx, ImGui.MouseButton_Left())
          end
        end
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Drag and drop files') then
      if ImGui.BeginChildFrame(ctx, '##drop_files', -FLT_MIN, 100) then
        if #widgets.dragdrop.files == 0 then
          ImGui.Text(ctx, 'Drag and drop files here...')
        else
          ImGui.Text(ctx, ('Received %d file(s):'):format(#widgets.dragdrop.files))
          ImGui.SameLine(ctx)
          if ImGui.SmallButton(ctx, 'Clear') then
            widgets.dragdrop.files = {}
          end
        end
        for _, file in ipairs(widgets.dragdrop.files) do
          ImGui.Bullet(ctx)
          ImGui.TextWrapped(ctx, file)
        end
        ImGui.EndChildFrame(ctx)
      end

      if ImGui.BeginDragDropTarget(ctx) then
        local rv, count = ImGui.AcceptDragDropPayloadFiles(ctx)
        if rv then
          widgets.dragdrop.files = {}
          for i = 0, count - 1 do
            local filename
            rv,filename = ImGui.GetDragDropPayloadFile(ctx, i)
            table.insert(widgets.dragdrop.files, filename)
          end
        end
        ImGui.EndDragDropTarget(ctx)
      end

      ImGui.TreePop(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Querying Item Status (Edited/Active/Hovered etc.)') then
    if not widgets.query_item then
      widgets.query_item = {
        item_type   = 1,
        b           = false,
        color       = 0xFF8000FF,
        str         = '',
        current     = 1,
        d4a         = { 1.0, 0.5, 0.0, 1.0 },
      }
    end

    -- Select an item type
    rv,widgets.query_item.item_type = ImGui.Combo(ctx, 'Item Type', widgets.query_item.item_type,
      'Text\0Button\0Button (w/ repeat)\0Checkbox\0SliderDouble\0\z
       InputText\0InputTextMultiline\0InputDouble\0InputDouble3\0ColorEdit4\0\z
       Selectable\0MenuItem\0TreeNode\0TreeNode (w/ double-click)\0Combo\0ListBox\0')

    ImGui.SameLine(ctx)
    demo.HelpMarker(
      'Testing how various types of items are interacting with the IsItemXXX \z
       functions. Note that the bool return value of most ImGui function is \z
       generally equivalent to calling ImGui.IsItemHovered().')

    if widgets.query_item.item_disabled then
      ImGui.BeginDisabled(ctx, true)
    end

    -- Submit selected items so we can query their status in the code following it.
    local item_type = widgets.query_item.item_type
    if item_type == 0  then -- Testing text items with no identifier/interaction
      ImGui.Text(ctx, 'ITEM: Text')
    end
    if item_type == 1  then -- Testing button
      rv = ImGui.Button(ctx, 'ITEM: Button')
    end
    if item_type == 2  then -- Testing button (with repeater)
      ImGui.PushButtonRepeat(ctx, true)
      rv = ImGui.Button(ctx, 'ITEM: Button')
      ImGui.PopButtonRepeat(ctx)
    end
    if item_type == 3  then -- Testing checkbox
      rv,widgets.query_item.b = ImGui.Checkbox(ctx, 'ITEM: Checkbox', widgets.query_item.b)
    end
    if item_type == 4  then -- Testing basic item
      rv,widgets.query_item.d4a[1] = ImGui.SliderDouble(ctx, 'ITEM: SliderDouble', widgets.query_item.d4a[1], 0.0, 1.0)
    end
    if item_type == 5  then -- Testing input text (which handles tabbing)
      rv,widgets.query_item.str = ImGui.InputText(ctx, 'ITEM: InputText', widgets.query_item.str)
    end
    if item_type == 6  then -- Testing input text (which uses a child window)
      rv,widgets.query_item.str = ImGui.InputTextMultiline(ctx, 'ITEM: InputTextMultiline', widgets.query_item.str)
    end
    if item_type == 7  then -- Testing +/- buttons on scalar input
      rv,widgets.query_item.d4a[1] = ImGui.InputDouble(ctx, 'ITEM: InputDouble', widgets.query_item.d4a[1], 1.0)
    end
    if item_type == 8  then -- Testing multi-component items (IsItemXXX flags are reported merged)
      local d4a = widgets.query_item.d4a
      rv,d4a[1],d4a[2],d4a[3] = ImGui.InputDouble3(ctx, 'ITEM: InputDouble3', d4a[1], d4a[2], d4a[3])
    end
    if item_type == 9  then -- Testing multi-component items (IsItemXXX flags are reported merged)
      rv,widgets.query_item.color = ImGui.ColorEdit4(ctx, 'ITEM: ColorEdit', widgets.query_item.color)
    end
    if item_type == 10 then -- Testing selectable item
      rv = ImGui.Selectable(ctx, 'ITEM: Selectable')
    end
    if item_type == 11  then -- Testing menu item (they use ImGuiButtonFlags_PressedOnRelease button policy)
      rv = ImGui.MenuItem(ctx, 'ITEM: MenuItem')
    end
    if item_type == 12 then -- Testing tree node
      rv = ImGui.TreeNode(ctx, 'ITEM: TreeNode')
      if rv then ImGui.TreePop(ctx) end
    end
    if item_type == 13 then -- Testing tree node with ImGuiButtonFlags_PressedOnDoubleClick button policy.
      rv = ImGui.TreeNode(ctx, 'ITEM: TreeNode w/ ImGuiTreeNodeFlags_OpenOnDoubleClick',
        ImGui.TreeNodeFlags_OpenOnDoubleClick() | ImGui.TreeNodeFlags_NoTreePushOnOpen())
    end
    if item_type == 14 then
      rv,widgets.query_item.current = ImGui.Combo(ctx, 'ITEM: Combo', widgets.query_item.current, 'Apple\0Banana\0Cherry\0Kiwi\0')
    end
    if item_type == 15 then
      rv,widgets.query_item.current = ImGui.ListBox(ctx, 'ITEM: ListBox', widgets.query_item.current, 'Apple\0Banana\0Cherry\0Kiwi\0')
    end

    local hovered_delay_none = ImGui.IsItemHovered(ctx)
    local hovered_delay_short = ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_DelayShort())
    local hovered_delay_normal = ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_DelayNormal())

    -- Display the values of IsItemHovered() and other common item state functions.
    -- Note that the ImGuiHoveredFlags_XXX flags can be combined.
    -- Because BulletText is an item itself and that would affect the output of IsItemXXX functions,
    -- we query every state in a single call to avoid storing them and to simplify the code.
    ImGui.BulletText(ctx, ([[Return value = %s
IsItemFocused() = %s
IsItemHovered() = %s
IsItemHovered(_AllowWhenBlockedByPopup) = %s
IsItemHovered(_AllowWhenBlockedByActiveItem) = %s
IsItemHovered(_AllowWhenOverlapped) = %s
IsItemHovered(_AllowWhenDisabled) = %s
IsItemHovered(_RectOnly) = %s
IsItemActive() = %s
IsItemEdited() = %s
IsItemActivated() = %s
IsItemDeactivated() = %s
IsItemDeactivatedAfterEdit() = %s
IsItemVisible() = %s
IsItemClicked() = %s
IsItemToggledOpen() = %s
GetItemRectMin() = (%.1f, %.1f)
GetItemRectMax() = (%.1f, %.1f)
GetItemRectSize() = (%.1f, %.1f)]]):format(
      rv,
      ImGui.IsItemFocused(ctx),
      ImGui.IsItemHovered(ctx),
      ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_AllowWhenBlockedByPopup()),
      ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_AllowWhenBlockedByActiveItem()),
      ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_AllowWhenOverlapped()),
      ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_AllowWhenDisabled()),
      ImGui.IsItemHovered(ctx, ImGui.HoveredFlags_RectOnly()),
      ImGui.IsItemActive(ctx),
      ImGui.IsItemEdited(ctx),
      ImGui.IsItemActivated(ctx),
      ImGui.IsItemDeactivated(ctx),
      ImGui.IsItemDeactivatedAfterEdit(ctx),
      ImGui.IsItemVisible(ctx),
      ImGui.IsItemClicked(ctx),
      ImGui.IsItemToggledOpen(ctx),
      ImGui.GetItemRectMin(ctx), select(2, ImGui.GetItemRectMin(ctx)),
      ImGui.GetItemRectMax(ctx), select(2, ImGui.GetItemRectMax(ctx)),
      ImGui.GetItemRectSize(ctx), select(2, ImGui.GetItemRectSize(ctx))
    ))
    ImGui.BulletText(ctx,
      ('w/ Hovering Delay: None = %s, Fast = %s, Normal = %s'):format
      (hovered_delay_none, hovered_delay_short, hovered_delay_normal))

    if widgets.query_item.item_disabled then
      ImGui.EndDisabled(ctx)
    end

    ImGui.InputText(ctx, 'unused', '', ImGui.InputTextFlags_ReadOnly())
    ImGui.SameLine(ctx)
    demo.HelpMarker('This widget is only here to be able to tab-out of the widgets above and see e.g. Deactivated() status.')
    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Querying Window Status (Focused/Hovered etc.)') then
    if not widgets.query_window then
      widgets.query_window = {
        embed_all_inside_a_child_window = false,
        test_window = false,
      }
    end
    rv,widgets.query_window.embed_all_inside_a_child_window =
      ImGui.Checkbox(ctx, 'Embed everything inside a child window for testing _RootWindow flag.',
      widgets.query_window.embed_all_inside_a_child_window)
    local visible = true
    if widgets.query_window.embed_all_inside_a_child_window then
      visible = ImGui.BeginChild(ctx, 'outer_child', 0, ImGui.GetFontSize(ctx) * 20.0, true)
    end

    if visible then
      -- Testing IsWindowFocused() function with its various flags.
      ImGui.BulletText(ctx, ([[IsWindowFocused() = %s
  IsWindowFocused(_ChildWindows) = %s
  IsWindowFocused(_ChildWindows|_NoPopupHierarchy) = %s
  IsWindowFocused(_ChildWindows|_DockHierarchy) = %s
  IsWindowFocused(_ChildWindows|_RootWindow) = %s
  IsWindowFocused(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %s
  IsWindowFocused(_ChildWindows|_RootWindow|_DockHierarchy) = %s
  IsWindowFocused(_RootWindow) = %s
  IsWindowFocused(_RootWindow|_NoPopupHierarchy) = %s
  IsWindowFocused(_RootWindow|_DockHierarchy) = %s
  IsWindowFocused(_AnyWindow) = %s]]):format(
        ImGui.IsWindowFocused(ctx),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_ChildWindows()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_ChildWindows() | ImGui.FocusedFlags_NoPopupHierarchy()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_ChildWindows() | ImGui.FocusedFlags_DockHierarchy()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_ChildWindows() | ImGui.FocusedFlags_RootWindow()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_ChildWindows() | ImGui.FocusedFlags_RootWindow() | ImGui.FocusedFlags_NoPopupHierarchy()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_ChildWindows() | ImGui.FocusedFlags_RootWindow() | ImGui.FocusedFlags_DockHierarchy()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_RootWindow()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_RootWindow() | ImGui.FocusedFlags_NoPopupHierarchy()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_RootWindow() | ImGui.FocusedFlags_DockHierarchy()),
        ImGui.IsWindowFocused(ctx, ImGui.FocusedFlags_AnyWindow())))

      -- Testing IsWindowHovered() function with its various flags.
      ImGui.BulletText(ctx, ([[IsWindowHovered() = %s
  IsWindowHovered(_AllowWhenBlockedByPopup) = %s
  IsWindowHovered(_AllowWhenBlockedByActiveItem) = %s
  IsWindowHovered(_ChildWindows) = %s
  IsWindowHovered(_ChildWindows|_NoPopupHierarchy) = %s
  IsWindowHovered(_ChildWindows|_DockHierarchy) = %s
  IsWindowHovered(_ChildWindows|_RootWindow) = %s
  IsWindowHovered(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %s
  IsWindowHovered(_ChildWindows|_RootWindow|_DockHierarchy) = %s
  IsWindowHovered(_RootWindow) = %s
  IsWindowHovered(_RootWindow|_NoPopupHierarchy) = %s
  IsWindowHovered(_RootWindow|_DockHierarchy) = %s
  IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %s
  IsWindowHovered(_AnyWindow) = %s]]):format(
        ImGui.IsWindowHovered(ctx),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_AllowWhenBlockedByPopup()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_AllowWhenBlockedByActiveItem()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_ChildWindows()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_ChildWindows() | ImGui.HoveredFlags_NoPopupHierarchy()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_ChildWindows() | ImGui.HoveredFlags_DockHierarchy()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_ChildWindows() | ImGui.HoveredFlags_RootWindow()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_ChildWindows() | ImGui.HoveredFlags_RootWindow() | ImGui.HoveredFlags_NoPopupHierarchy()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_ChildWindows() | ImGui.HoveredFlags_RootWindow() | ImGui.HoveredFlags_DockHierarchy()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_RootWindow()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_RootWindow() | ImGui.HoveredFlags_NoPopupHierarchy()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_RootWindow() | ImGui.HoveredFlags_DockHierarchy()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_ChildWindows() | ImGui.HoveredFlags_AllowWhenBlockedByPopup()),
        ImGui.IsWindowHovered(ctx, ImGui.HoveredFlags_AnyWindow())))

      if ImGui.BeginChild(ctx, 'child', 0, 50, true) then
        ImGui.Text(ctx, 'This is another child window for testing the _ChildWindows flag.')
        ImGui.EndChild(ctx)
      end
      if widgets.query_window.embed_all_inside_a_child_window then
        ImGui.EndChild(ctx)
      end
    end

    -- Calling IsItemHovered() after begin returns the hovered status of the title bar.
    -- This is useful in particular if you want to create a context menu associated to the title bar of a window.
    -- This will also work when docked into a Tab (the Tab replace the Title Bar and guarantee the same properties).
    rv,widgets.query_window.test_window = ImGui.Checkbox(ctx, 'Hovered/Active tests after Begin() for title bar testing', widgets.query_window.test_window)
    if widgets.query_window.test_window then
      -- FIXME-DOCK: This window cannot be docked within the ImGui Demo window, this will cause a feedback loop and get them stuck.
      -- Could we fix this through an ImGuiWindowClass feature? Or an API call to tag our parent as "don't skip items"?
      rv,widgets.query_window.test_window = ImGui.Begin(ctx, 'Title bar Hovered/Active tests', true)
      if rv then
        if ImGui.BeginPopupContextItem(ctx) then -- <-- This is using IsItemHovered()
          if ImGui.MenuItem(ctx, 'Close') then widgets.query_window.test_window = false end
          ImGui.EndPopup(ctx)
        end
        ImGui.Text(ctx,
          ('IsItemHovered() after begin = %s (== is title bar hovered)\n\z
            IsItemActive() after begin = %s (== is window being clicked/moved)\n')
          :format(ImGui.IsItemHovered(ctx), ImGui.IsItemActive(ctx)))
        ImGui.End(ctx)
      end
    end

    ImGui.TreePop(ctx)
  end

  -- Demonstrate BeginDisabled/EndDisabled using a checkbox located at the bottom of the section (which is a bit odd:
  -- logically we'd have this checkbox at the top of the section, but we don't want this feature to steal that space)
  if widgets.disable_all then
    ImGui.EndDisabled(ctx)
  end

  if ImGui.TreeNode(ctx, 'Disable block') then
    rv,widgets.disable_all = ImGui.Checkbox(ctx, 'Disable entire section above', widgets.disable_all)
    ImGui.SameLine(ctx); demo.HelpMarker('Demonstrate using BeginDisabled()/EndDisabled() across this section.')
    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Text Filter') then
    -- Helper class to easy setup a text filter.
    -- You may want to implement a more feature-full filtering scheme in your own application.
    if not widgets.filter then
      widgets.filter = ImGui.CreateTextFilter()
      -- prevent the filter object from being destroyed once unused for one or more frames
      ImGui.Attach(ctx, widgets.filter)
    end

    demo.HelpMarker('Not a widget per-se, but ImGui_TextFilter is a helper to perform simple filtering on text strings.')
    ImGui.Text(ctx, [[Filter usage:
  ""         display all lines
  "xxx"      display lines containing "xxx"
  "xxx,yyy"  display lines containing "xxx" or "yyy"
  "-xxx"     hide lines containing "xxx"]])
    ImGui.TextFilter_Draw(widgets.filter, ctx)
    local lines = { 'aaa1.c', 'bbb1.c', 'ccc1.c', 'aaa2.cpp', 'bbb2.cpp', 'ccc2.cpp', 'abc.h', 'hello, world' }
    for i, line in ipairs(lines) do
      if ImGui.TextFilter_PassFilter(widgets.filter, line) then
        ImGui.BulletText(ctx, line)
      end
    end

    ImGui.TreePop(ctx)
  end
end

function demo.ShowDemoWindowLayout()
  if not ImGui.CollapsingHeader(ctx, 'Layout & Scrolling') then
    return
  end

  local rv

  if ImGui.TreeNode(ctx, 'Child windows') then
    if not layout.child then
      layout.child = {
        disable_mouse_wheel = false,
        disable_menu        = false,
        offset_x            = 0,
      }
    end

    ImGui.SeparatorText(ctx, 'Child windows')
    demo.HelpMarker('Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window.')
    rv,layout.child.disable_mouse_wheel = ImGui.Checkbox(ctx, 'Disable Mouse Wheel', layout.child.disable_mouse_wheel)
    rv,layout.child.disable_menu = ImGui.Checkbox(ctx, 'Disable Menu', layout.child.disable_menu)

    -- Child 1: no border, enable horizontal scrollbar
    do
      local window_flags = ImGui.WindowFlags_HorizontalScrollbar()
      if layout.child.disable_mouse_wheel then
        window_flags = window_flags | ImGui.WindowFlags_NoScrollWithMouse()
      end
      if ImGui.BeginChild(ctx, 'ChildL', ImGui.GetContentRegionAvail(ctx) * 0.5, 260, false, window_flags) then
        for i = 0, 99 do
          ImGui.Text(ctx, ('%04d: scrollable region'):format(i))
        end
        ImGui.EndChild(ctx)
      end
    end

    ImGui.SameLine(ctx)

    -- Child 2: rounded border
    do
      local window_flags = ImGui.WindowFlags_None()
      if layout.child.disable_mouse_wheel then
        window_flags = window_flags | ImGui.WindowFlags_NoScrollWithMouse()
      end
      if not layout.child.disable_menu then
        window_flags = window_flags | ImGui.WindowFlags_MenuBar()
      end
      ImGui.PushStyleVar(ctx, ImGui.StyleVar_ChildRounding(), 5.0)
      local visible = ImGui.BeginChild(ctx, 'ChildR', 0, 260, true, window_flags)
      if visible then
        if not layout.child.disable_menu and ImGui.BeginMenuBar(ctx) then
          if ImGui.BeginMenu(ctx, 'Menu') then
            demo.ShowExampleMenuFile()
            ImGui.EndMenu(ctx)
          end
          ImGui.EndMenuBar(ctx)
        end
        if ImGui.BeginTable(ctx, 'split', 2, ImGui.TableFlags_Resizable() | ImGui.TableFlags_NoSavedSettings()) then
          for i = 0, 99 do
            ImGui.TableNextColumn(ctx)
            ImGui.Button(ctx, ('%03d'):format(i), -FLT_MIN, 0.0)
          end
          ImGui.EndTable(ctx)
        end
        ImGui.EndChild(ctx)
      end
      ImGui.PopStyleVar(ctx)
    end

    ImGui.SeparatorText(ctx, 'Misc/Advanced')

    -- Demonstrate a few extra things
    -- - Changing ImGuiCol_ChildBg (which is transparent black in default styles)
    -- - Using SetCursorPos() to position child window (the child window is an item from the POV of parent window)
    --   You can also call SetNextWindowPos() to position the child window. The parent window will effectively
    --   layout from this position.
    -- - Using ImGui.GetItemRectMin/Max() to query the "item" state (because the child window is an item from
    --   the POV of the parent window). See 'Demo->Querying Status (Edited/Active/Hovered etc.)' for details.
    do
      ImGui.SetNextItemWidth(ctx, ImGui.GetFontSize(ctx) * 8)
      rv,layout.child.offset_x = ImGui.DragInt(ctx, 'Offset X', layout.child.offset_x, 1.0, -1000, 1000)

      ImGui.SetCursorPosX(ctx, ImGui.GetCursorPosX(ctx) + layout.child.offset_x)
      ImGui.PushStyleColor(ctx, ImGui.Col_ChildBg(), 0xFF000064)
      local visible = ImGui.BeginChild(ctx, 'Red', 200, 100, true, ImGui.WindowFlags_None())
      ImGui.PopStyleColor(ctx)
      if visible then
        for n = 0, 49 do
          ImGui.Text(ctx, ('Some test %d'):format(n))
        end
        ImGui.EndChild(ctx)
      end
      local child_is_hovered = ImGui.IsItemHovered(ctx)
      local child_rect_min_x,child_rect_min_y = ImGui.GetItemRectMin(ctx)
      local child_rect_max_x,child_rect_max_y = ImGui.GetItemRectMax(ctx)
      ImGui.Text(ctx, ('Hovered: %s'):format(child_is_hovered))
      ImGui.Text(ctx, ('Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)')
        :format(child_rect_min_x, child_rect_min_y, child_rect_max_x, child_rect_max_y))
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Widgets Width') then
    if not layout.width then
      layout.width = {
        d = 0.0,
        show_indented_items = true,
      }
    end

    -- Use SetNextItemWidth() to set the width of a single upcoming item.
    -- Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
    -- In real code use you'll probably want to choose width values that are proportional to your font size
    -- e.g. Using '20.0 * GetFontSize()' as width instead of '200.0', etc.

    rv,layout.width.show_indented_items = ImGui.Checkbox(ctx, 'Show indented items', layout.width.show_indented_items)

    ImGui.Text(ctx, 'SetNextItemWidth/PushItemWidth(100)')
    ImGui.SameLine(ctx); demo.HelpMarker('Fixed width.')
    ImGui.PushItemWidth(ctx, 100)
    rv,layout.width.d = ImGui.DragDouble(ctx, 'float##1b', layout.width.d)
    if layout.width.show_indented_items then
      ImGui.Indent(ctx)
      rv,layout.width.d = ImGui.DragDouble(ctx, 'float (indented)##1b', layout.width.d)
      ImGui.Unindent(ctx)
    end
    ImGui.PopItemWidth(ctx)

    ImGui.Text(ctx, 'SetNextItemWidth/PushItemWidth(-100)')
    ImGui.SameLine(ctx); demo.HelpMarker('Align to right edge minus 100')
    ImGui.PushItemWidth(ctx, -100)
    rv,layout.width.d = ImGui.DragDouble(ctx, 'float##2a', layout.width.d)
    if layout.width.show_indented_items then
      ImGui.Indent(ctx)
      rv,layout.width.d = ImGui.DragDouble(ctx, 'float (indented)##2b', layout.width.d)
      ImGui.Unindent(ctx)
    end
    ImGui.PopItemWidth(ctx)

    ImGui.Text(ctx, 'SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5)')
    ImGui.SameLine(ctx); demo.HelpMarker('Half of available width.\n(~ right-cursor_pos)\n(works within a column set)')
    ImGui.PushItemWidth(ctx, ImGui.GetContentRegionAvail(ctx) * 0.5)
    rv,layout.width.d = ImGui.DragDouble(ctx, 'float##3a', layout.width.d)
    if layout.width.show_indented_items then
      ImGui.Indent(ctx)
      rv,layout.width.d = ImGui.DragDouble(ctx, 'float (indented)##3b', layout.width.d)
      ImGui.Unindent(ctx)
    end
    ImGui.PopItemWidth(ctx)

    ImGui.Text(ctx, 'SetNextItemWidth/PushItemWidth(-GetContentRegionAvail().x * 0.5)')
    ImGui.SameLine(ctx); demo.HelpMarker('Align to right edge minus half')
    ImGui.PushItemWidth(ctx, -ImGui.GetContentRegionAvail(ctx) * 0.5)
    rv,layout.width.d = ImGui.DragDouble(ctx, 'float##4a', layout.width.d)
    if layout.width.show_indented_items then
      ImGui.Indent(ctx)
      rv,layout.width.d = ImGui.DragDouble(ctx, 'float (indented)##4b', layout.width.d)
      ImGui.Unindent(ctx)
    end
    ImGui.PopItemWidth(ctx)

    -- Demonstrate using PushItemWidth to surround three items.
    -- Calling SetNextItemWidth() before each of them would have the same effect.
    ImGui.Text(ctx, 'SetNextItemWidth/PushItemWidth(-FLT_MIN)')
    ImGui.SameLine(ctx); demo.HelpMarker('Align to right edge')
    ImGui.PushItemWidth(ctx, -FLT_MIN)
    rv,layout.width.d = ImGui.DragDouble(ctx, '##float5a', layout.width.d)
    if layout.width.show_indented_items then
      ImGui.Indent(ctx)
      rv,layout.width.d = ImGui.DragDouble(ctx, 'float (indented)##5b', layout.width.d)
      ImGui.Unindent(ctx)
    end
    ImGui.PopItemWidth(ctx)

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Basic Horizontal Layout') then
    if not layout.horizontal then
      layout.horizontal = {
        c1 = false, c2 = false, c3 = false, c4 = false,
        d0 = 1.0, d1 = 2.0, d2 = 3.0,
        item = -1,
        selection = { 0, 1, 2, 3 },
      }
    end

    ImGui.TextWrapped(ctx, '(Use ImGui.SameLine() to keep adding items to the right of the preceding item)')

    -- Text
    ImGui.Text(ctx, 'Two items: Hello'); ImGui.SameLine(ctx)
    ImGui.TextColored(ctx, 0xFFFF00FF, 'Sailor')

    -- Adjust spacing
    ImGui.Text(ctx, 'More spacing: Hello'); ImGui.SameLine(ctx, 0, 20)
    ImGui.TextColored(ctx, 0xFFFF00FF, 'Sailor')

    -- Button
    ImGui.AlignTextToFramePadding(ctx)
    ImGui.Text(ctx, 'Normal buttons'); ImGui.SameLine(ctx)
    ImGui.Button(ctx, 'Banana'); ImGui.SameLine(ctx)
    ImGui.Button(ctx, 'Apple'); ImGui.SameLine(ctx)
    ImGui.Button(ctx, 'Corniflower')

    -- Button
    ImGui.Text(ctx, 'Small buttons'); ImGui.SameLine(ctx)
    ImGui.SmallButton(ctx, 'Like this one'); ImGui.SameLine(ctx)
    ImGui.Text(ctx, 'can fit within a text block.')

    -- Aligned to arbitrary position. Easy/cheap column.
    ImGui.Text(ctx, 'Aligned')
    ImGui.SameLine(ctx, 150); ImGui.Text(ctx, 'x=150')
    ImGui.SameLine(ctx, 300); ImGui.Text(ctx, 'x=300')
    ImGui.Text(ctx, 'Aligned')
    ImGui.SameLine(ctx, 150); ImGui.SmallButton(ctx, 'x=150')
    ImGui.SameLine(ctx, 300); ImGui.SmallButton(ctx, 'x=300')

    -- Checkbox
    rv,layout.horizontal.c1 = ImGui.Checkbox(ctx, 'My',     layout.horizontal.c1); ImGui.SameLine(ctx)
    rv,layout.horizontal.c2 = ImGui.Checkbox(ctx, 'Tailor', layout.horizontal.c2); ImGui.SameLine(ctx)
    rv,layout.horizontal.c3 = ImGui.Checkbox(ctx, 'Is',     layout.horizontal.c3); ImGui.SameLine(ctx)
    rv,layout.horizontal.c4 = ImGui.Checkbox(ctx, 'Rich',   layout.horizontal.c4)

    -- Various
    ImGui.PushItemWidth(ctx, 80)
    local items = 'AAAA\0BBBB\0CCCC\0DDDD\0'
    rv,layout.horizontal.item = ImGui.Combo(ctx, 'Combo', layout.horizontal.item, items);   ImGui.SameLine(ctx)
    rv,layout.horizontal.d0 = ImGui.SliderDouble(ctx, 'X', layout.horizontal.d0, 0.0, 5.0); ImGui.SameLine(ctx)
    rv,layout.horizontal.d1 = ImGui.SliderDouble(ctx, 'Y', layout.horizontal.d1, 0.0, 5.0); ImGui.SameLine(ctx)
    rv,layout.horizontal.d2 = ImGui.SliderDouble(ctx, 'Z', layout.horizontal.d2, 0.0, 5.0)
    ImGui.PopItemWidth(ctx)

    ImGui.PushItemWidth(ctx, 80)
    ImGui.Text(ctx, 'Lists:')
    for i,sel in ipairs(layout.horizontal.selection) do
      if i > 1 then ImGui.SameLine(ctx) end
      ImGui.PushID(ctx, i)
      rv,layout.horizontal.selection[i] = ImGui.ListBox(ctx, '', sel, items)
      ImGui.PopID(ctx)
      --if ImGui.IsItemHovered(ctx) then ImGui.SetTooltip(ctx, ('ListBox %d hovered'):format(i)) end
    end
    ImGui.PopItemWidth(ctx)

    -- Dummy
    local button_sz = { 40, 40 }
    ImGui.Button(ctx, 'A', table.unpack(button_sz)); ImGui.SameLine(ctx)
    ImGui.Dummy(ctx, table.unpack(button_sz)); ImGui.SameLine(ctx)
    ImGui.Button(ctx,'B', table.unpack(button_sz))

    -- Manually wrapping
    -- (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
    ImGui.Text(ctx, 'Manual wrapping:')
    local item_spacing_x = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemSpacing())
    local buttons_count = 20
    local window_visible_x2 = ImGui.GetWindowPos(ctx) + ImGui.GetWindowContentRegionMax(ctx)
    for n = 0, buttons_count - 1 do
      ImGui.PushID(ctx, n)
      ImGui.Button(ctx, 'Box', table.unpack(button_sz))
      local last_button_x2 = ImGui.GetItemRectMax(ctx)
      local next_button_x2 = last_button_x2 + item_spacing_x + button_sz[1] -- Expected position if next button was on same line
      if n + 1 < buttons_count and next_button_x2 < window_visible_x2 then
        ImGui.SameLine(ctx)
      end
      ImGui.PopID(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Groups') then
    if not widgets.groups then
      widgets.groups = {
        values = reaper.new_array({ 0.5, 0.20, 0.80, 0.60, 0.25 }),
      }
    end

    demo.HelpMarker(
      'BeginGroup() basically locks the horizontal position for new line. \z
       EndGroup() bundles the whole group so that you can use "item" functions such as \z
       IsItemHovered()/IsItemActive() or SameLine() etc. on the whole group.')
    ImGui.BeginGroup(ctx)
    ImGui.BeginGroup(ctx)
    ImGui.Button(ctx, 'AAA')
    ImGui.SameLine(ctx)
    ImGui.Button(ctx, 'BBB')
    ImGui.SameLine(ctx)
    ImGui.BeginGroup(ctx)
    ImGui.Button(ctx, 'CCC')
    ImGui.Button(ctx, 'DDD')
    ImGui.EndGroup(ctx)
    ImGui.SameLine(ctx)
    ImGui.Button(ctx, 'EEE')
    ImGui.EndGroup(ctx)
    if ImGui.IsItemHovered(ctx) then
      ImGui.SetTooltip(ctx, 'First group hovered')
    end

    -- Capture the group size and create widgets using the same size
    local size = {ImGui.GetItemRectSize(ctx)}
    local item_spacing_x = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemSpacing())

    ImGui.PlotHistogram(ctx, '##values', widgets.groups.values, 0, nil, 0.0, 1.0, table.unpack(size))

    ImGui.Button(ctx, 'ACTION', (size[1] - item_spacing_x) * 0.5, size[2])
    ImGui.SameLine(ctx)
    ImGui.Button(ctx, 'REACTION', (size[1] - item_spacing_x) * 0.5, size[2])
    ImGui.EndGroup(ctx)
    ImGui.SameLine(ctx)

    ImGui.Button(ctx, 'LEVERAGE\nBUZZWORD', table.unpack(size))
    ImGui.SameLine(ctx)

    if ImGui.BeginListBox(ctx, 'List', table.unpack(size)) then
      ImGui.Selectable(ctx, 'Selected', true)
      ImGui.Selectable(ctx, 'Not Selected', false)
      ImGui.EndListBox(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Text Baseline Alignment') then
    do
      ImGui.BulletText(ctx, 'Text baseline:')
      ImGui.SameLine(ctx); demo.HelpMarker(
        'This is testing the vertical alignment that gets applied on text to keep it aligned with widgets. \z
        Lines only composed of text or "small" widgets use less vertical space than lines with framed widgets.')
      ImGui.Indent(ctx)

      ImGui.Text(ctx, 'KO Blahblah'); ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Some framed item'); ImGui.SameLine(ctx)
      demo.HelpMarker('Baseline of button will look misaligned with text..')

      -- If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
      -- (because we don't know what's coming after the Text() statement, we need to move the text baseline
      -- down by FramePadding.y ahead of time)
      ImGui.AlignTextToFramePadding(ctx)
      ImGui.Text(ctx, 'OK Blahblah'); ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Some framed item'); ImGui.SameLine(ctx)
      demo.HelpMarker('We call AlignTextToFramePadding() to vertically align the text baseline by +FramePadding.y')

      -- SmallButton() uses the same vertical padding as Text
      ImGui.Button(ctx, 'TEST##1'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'TEST'); ImGui.SameLine(ctx)
      ImGui.SmallButton(ctx, 'TEST##2')

      -- If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
      ImGui.AlignTextToFramePadding(ctx)
      ImGui.Text(ctx, 'Text aligned to framed item'); ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Item##1'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Item'); ImGui.SameLine(ctx)
      ImGui.SmallButton(ctx, 'Item##2'); ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Item##3')

      ImGui.Unindent(ctx)
    end

    ImGui.Spacing(ctx)

    do
      ImGui.BulletText(ctx, 'Multi-line text:')
      ImGui.Indent(ctx)
      ImGui.Text(ctx, 'One\nTwo\nThree'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Hello\nWorld'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Banana')

      ImGui.Text(ctx, 'Banana'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Hello\nWorld'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'One\nTwo\nThree')

      ImGui.Button(ctx, 'HOP##1'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Banana'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Hello\nWorld'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Banana')

      ImGui.Button(ctx, 'HOP##2'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Hello\nWorld'); ImGui.SameLine(ctx)
      ImGui.Text(ctx, 'Banana')
      ImGui.Unindent(ctx)
    end

    ImGui.Spacing(ctx)

    do
      ImGui.BulletText(ctx, 'Misc items:')
      ImGui.Indent(ctx)

      -- SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of previous Button.
      ImGui.Button(ctx, '80x80', 80, 80)
      ImGui.SameLine(ctx)
      ImGui.Button(ctx, '50x50', 50, 50)
      ImGui.SameLine(ctx)
      ImGui.Button(ctx, 'Button()')
      ImGui.SameLine(ctx)
      ImGui.SmallButton(ctx, 'SmallButton()')

      -- Tree
      local spacing = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())
      ImGui.Button(ctx, 'Button##1')
      ImGui.SameLine(ctx, 0.0, spacing)
      if ImGui.TreeNode(ctx, 'Node##1') then
        -- Placeholder tree data
        for i = 0, 5 do
          ImGui.BulletText(ctx, ('Item %d..'):format(i))
        end
        ImGui.TreePop(ctx)
      end

      -- Vertically align text node a bit lower so it'll be vertically centered with upcoming widget.
      -- Otherwise you can use SmallButton() (smaller fit).
      ImGui.AlignTextToFramePadding(ctx)

      -- Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add
      -- other contents below the node.
      local node_open = ImGui.TreeNode(ctx, 'Node##2')
      ImGui.SameLine(ctx, 0.0, spacing); ImGui.Button(ctx, 'Button##2')
      if node_open then
        -- Placeholder tree data
        for i = 0, 5 do
          ImGui.BulletText(ctx, ('Item %d..'):format(i))
        end
        ImGui.TreePop(ctx)
      end

      -- Bullet
      ImGui.Button(ctx, 'Button##3')
      ImGui.SameLine(ctx, 0.0, spacing)
      ImGui.BulletText(ctx, 'Bullet text')

      ImGui.AlignTextToFramePadding(ctx)
      ImGui.BulletText(ctx, 'Node')
      ImGui.SameLine(ctx, 0.0, spacing); ImGui.Button(ctx, 'Button##4')
      ImGui.Unindent(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Scrolling') then
    if not layout.scrolling then
      layout.scrolling = {
        track_item       = 50,
        enable_track     = true,
        enable_extra_decorations = false,
        scroll_to_off_px = 0.0,
        scroll_to_pos_px = 200.0,
        lines = 7,
        show_horizontal_contents_size_demo_window = false,
      }
    end

    -- Vertical scroll functions
    demo.HelpMarker('Use SetScrollHereY() or SetScrollFromPosY() to scroll to a given vertical position.')

    rv,layout.scrolling.enable_extra_decorations = ImGui.Checkbox(ctx, 'Decoration', layout.scrolling.enable_extra_decorations)

    rv,layout.scrolling.enable_track = ImGui.Checkbox(ctx, 'Track', layout.scrolling.enable_track)
    ImGui.PushItemWidth(ctx, 100)
    ImGui.SameLine(ctx, 140)
    rv,layout.scrolling.track_item = ImGui.DragInt(ctx, '##item', layout.scrolling.track_item, 0.25, 0, 99, 'Item = %d')
    if rv then
      layout.scrolling.enable_track = true
    end

    local scroll_to_off = ImGui.Button(ctx, 'Scroll Offset')
    ImGui.SameLine(ctx, 140)
    rv,layout.scrolling.scroll_to_off_px = ImGui.DragDouble(ctx, '##off', layout.scrolling.scroll_to_off_px, 1.00, 0, FLT_MAX, '+%.0f px')
    if rv then
      scroll_to_off = true
    end

    local scroll_to_pos = ImGui.Button(ctx, 'Scroll To Pos')
    ImGui.SameLine(ctx, 140)
    rv,layout.scrolling.scroll_to_pos_px = ImGui.DragDouble(ctx, '##pos', layout.scrolling.scroll_to_pos_px, 1.00, -10, FLT_MAX, 'X/Y = %.0f px')
    if rv then
      scroll_to_pos = true
    end
    ImGui.PopItemWidth(ctx)

    if scroll_to_off or scroll_to_pos then
      layout.scrolling.enable_track = false
    end

    local names = { 'Top', '25%', 'Center', '75%', 'Bottom' }
    local item_spacing_x = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemSpacing())
    local child_w = (ImGui.GetContentRegionAvail(ctx) - 4 * item_spacing_x) / #names
    local child_flags = layout.scrolling.enable_extra_decorations and ImGui.WindowFlags_MenuBar() or ImGui.WindowFlags_None()
    if child_w < 1.0 then
      child_w = 1.0
    end
    ImGui.PushID(ctx, '##VerticalScrolling')
    for i,name in ipairs(names) do
      if i > 1 then ImGui.SameLine(ctx) end
      ImGui.BeginGroup(ctx)
      ImGui.Text(ctx, name)

      if ImGui.BeginChild(ctx, i, child_w, 200.0, true, child_flags) then
        if ImGui.BeginMenuBar(ctx) then
          ImGui.Text(ctx, 'abc')
          ImGui.EndMenuBar(ctx)
        end
        if scroll_to_off then
          ImGui.SetScrollY(ctx, layout.scrolling.scroll_to_off_px)
        end
        if scroll_to_pos then
          ImGui.SetScrollFromPosY(ctx, select(2, ImGui.GetCursorStartPos(ctx)) + layout.scrolling.scroll_to_pos_px, (i - 1) * 0.25)
        end
        for item = 0, 99 do
          if layout.scrolling.enable_track and item == layout.scrolling.track_item then
            ImGui.TextColored(ctx, 0xFFFF00FF, ('Item %d'):format(item))
            ImGui.SetScrollHereY(ctx, (i - 1) * 0.25) -- 0.0:top, 0.5:center, 1.0:bottom
          else
            ImGui.Text(ctx, ('Item %d'):format(item))
          end
        end
        local scroll_y = ImGui.GetScrollY(ctx)
        local scroll_max_y = ImGui.GetScrollMaxY(ctx)
        ImGui.EndChild(ctx)
        ImGui.Text(ctx, ('%.0f/%.0f'):format(scroll_y, scroll_max_y))
      else
        ImGui.Text(ctx, 'N/A')
      end
      ImGui.EndGroup(ctx)
    end
    ImGui.PopID(ctx)

    -- Horizontal scroll functions
    ImGui.Spacing(ctx)
    demo.HelpMarker(
      "Use SetScrollHereX() or SetScrollFromPosX() to scroll to a given horizontal position.\n\n\z
       Because the clipping rectangle of most window hides half worth of WindowPadding on the \z
       left/right, using SetScrollFromPosX(+1) will usually result in clipped text whereas the \z
       equivalent SetScrollFromPosY(+1) wouldn't.")
    ImGui.PushID(ctx, '##HorizontalScrolling')
    local scrollbar_size = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ScrollbarSize())
    local window_padding_y = select(2, ImGui.GetStyleVar(ctx, ImGui.StyleVar_WindowPadding()))
    local child_height = ImGui.GetTextLineHeight(ctx) + scrollbar_size + window_padding_y * 2.0
    local child_flags = ImGui.WindowFlags_HorizontalScrollbar()
    if layout.scrolling.enable_extra_decorations then
      child_flags = child_flags | ImGui.WindowFlags_AlwaysVerticalScrollbar()
    end
    for i,name in ipairs(names) do
      local scroll_x, scroll_max_x = 0.0, 0.0
      if ImGui.BeginChild(ctx, i, -100, child_height, true, child_flags) then
        if scroll_to_off then
          ImGui.SetScrollX(ctx, layout.scrolling.scroll_to_off_px)
        end
        if scroll_to_pos then
          ImGui.SetScrollFromPosX(ctx, ImGui.GetCursorStartPos(ctx) + layout.scrolling.scroll_to_pos_px, (i - 1) * 0.25)
        end
        for item = 0, 99 do
          if item > 0 then
            ImGui.SameLine(ctx)
          end
          if layout.scrolling.enable_track and item == layout.scrolling.track_item then
            ImGui.TextColored(ctx, 0xFFFF00FF, ('Item %d'):format(item))
            ImGui.SetScrollHereX(ctx, (i - 1) * 0.25) -- 0.0:left, 0.5:center, 1.0:right
          else
            ImGui.Text(ctx, ('Item %d'):format(item))
          end
        end
        scroll_x = ImGui.GetScrollX(ctx)
        scroll_max_x = ImGui.GetScrollMaxX(ctx)
        ImGui.EndChild(ctx)
      end
      ImGui.SameLine(ctx)
      ImGui.Text(ctx, ('%s\n%.0f/%.0f'):format(name, scroll_x, scroll_max_x))
      ImGui.Spacing(ctx)
    end
    ImGui.PopID(ctx)

    -- Miscellaneous Horizontal Scrolling Demo
    demo.HelpMarker(
      'Horizontal scrolling for a window is enabled via the ImGuiWindowFlags_HorizontalScrollbar flag.\n\n\z
       You may want to also explicitly specify content width by using SetNextWindowContentWidth() before Begin().')
    rv,layout.scrolling.lines = ImGui.SliderInt(ctx, 'Lines', layout.scrolling.lines, 1, 15)
    ImGui.PushStyleVar(ctx, ImGui.StyleVar_FrameRounding(), 3.0)
    ImGui.PushStyleVar(ctx, ImGui.StyleVar_FramePadding(), 2.0, 1.0)
    local scrolling_child_width = ImGui.GetFrameHeightWithSpacing(ctx) * 7 + 30
    local scroll_x, scroll_max_x = 0.0, 0.0
    if ImGui.BeginChild(ctx, 'scrolling', 0, scrolling_child_width, true, ImGui.WindowFlags_HorizontalScrollbar()) then
      for line = 0, layout.scrolling.lines - 1 do
        -- Display random stuff. For the sake of this trivial demo we are using basic Button() + SameLine()
        -- If you want to create your own time line for a real application you may be better off manipulating
        -- the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets
        -- yourself. You may also want to use the lower-level ImDrawList API.
        local num_buttons = 10 + ((line & 1 ~= 0) and line * 9 or line * 3)
        for n = 0, num_buttons - 1 do
          if n > 0 then ImGui.SameLine(ctx) end
          ImGui.PushID(ctx, n + line * 1000)
          local label
          if n % 15 == 0 then
            label = 'FizzBuzz'
          elseif n % 3 == 0 then
            label = 'Fizz'
          elseif n % 5 == 0 then
            label = 'Buzz'
          else
            label = tostring(n)
          end
          local hue = n * 0.05
          ImGui.PushStyleColor(ctx, ImGui.Col_Button(), demo.HSV(hue, 0.6, 0.6))
          ImGui.PushStyleColor(ctx, ImGui.Col_ButtonHovered(), demo.HSV(hue, 0.7, 0.7))
          ImGui.PushStyleColor(ctx, ImGui.Col_ButtonActive(), demo.HSV(hue, 0.8, 0.8))
          ImGui.Button(ctx, label, 40.0 + math.sin(line + n) * 20.0, 0.0)
          ImGui.PopStyleColor(ctx, 3)
          ImGui.PopID(ctx)
        end
      end
      scroll_x = ImGui.GetScrollX(ctx)
      scroll_max_x = ImGui.GetScrollMaxX(ctx)
      ImGui.EndChild(ctx)
    end
    ImGui.PopStyleVar(ctx, 2)
    local scroll_x_delta = 0.0
    ImGui.SmallButton(ctx, '<<')
    if ImGui.IsItemActive(ctx) then
      scroll_x_delta = (0 - ImGui.GetDeltaTime(ctx)) * 1000.0
    end
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, 'Scroll from code'); ImGui.SameLine(ctx)
    ImGui.SmallButton(ctx, '>>')
    if ImGui.IsItemActive(ctx) then
      scroll_x_delta = ImGui.GetDeltaTime(ctx) * 1000.0
    end
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, ('%.0f/%.0f'):format(scroll_x, scroll_max_x))
    if scroll_x_delta ~= 0.0 then
      -- Demonstrate a trick: you can use Begin to set yourself in the context of another window
      -- (here we are already out of your child window)
      if ImGui.BeginChild(ctx, 'scrolling') then
        ImGui.SetScrollX(ctx, ImGui.GetScrollX(ctx) + scroll_x_delta)
        ImGui.EndChild(ctx)
      end
    end
    ImGui.Spacing(ctx)

    rv,layout.scrolling.show_horizontal_contents_size_demo_window =
      ImGui.Checkbox(ctx, 'Show Horizontal contents size demo window',
      layout.scrolling.show_horizontal_contents_size_demo_window)

    if layout.scrolling.show_horizontal_contents_size_demo_window then
      if not layout.horizontal_window then
        layout.horizontal_window = {
          show_h_scrollbar      = true,
          show_button           = true,
          show_tree_nodes       = true,
          show_text_wrapped     = false,
          show_columns          = true,
          show_tab_bar          = true,
          show_child            = false,
          explicit_content_size = false,
          contents_size_x       = 300.0,
        }
      end

      if layout.horizontal_window.explicit_content_size then
        ImGui.SetNextWindowContentSize(ctx, layout.horizontal_window.contents_size_x, 0.0)
      end
      rv,layout.scrolling.show_horizontal_contents_size_demo_window =
        ImGui.Begin(ctx, 'Horizontal contents size demo window', true,
          layout.horizontal_window.show_h_scrollbar and ImGui.WindowFlags_HorizontalScrollbar() or ImGui.WindowFlags_None())
      if rv then
        ImGui.PushStyleVar(ctx, ImGui.StyleVar_ItemSpacing(), 2, 0)
        ImGui.PushStyleVar(ctx, ImGui.StyleVar_FramePadding(), 2, 0)
        demo.HelpMarker("Test of different widgets react and impact the work rectangle growing when horizontal scrolling is enabled.\n\nUse 'Metrics->Tools->Show windows rectangles' to visualize rectangles.")
        rv,layout.horizontal_window.show_h_scrollbar =
          ImGui.Checkbox(ctx, 'H-scrollbar', layout.horizontal_window.show_h_scrollbar)
        rv,layout.horizontal_window.show_button =
          ImGui.Checkbox(ctx, 'Button', layout.horizontal_window.show_button)             -- Will grow contents size (unless explicitly overwritten)
        rv,layout.horizontal_window.show_tree_nodes =
          ImGui.Checkbox(ctx, 'Tree nodes', layout.horizontal_window.show_tree_nodes)     -- Will grow contents size and display highlight over full width
        rv,layout.horizontal_window.show_text_wrapped =
          ImGui.Checkbox(ctx, 'Text wrapped', layout.horizontal_window.show_text_wrapped) -- Will grow and use contents size
        rv,layout.horizontal_window.show_columns =
          ImGui.Checkbox(ctx, 'Columns', layout.horizontal_window.show_columns)           -- Will use contents size
        rv,layout.horizontal_window.show_tab_bar =
          ImGui.Checkbox(ctx, 'Tab bar', layout.horizontal_window.show_tab_bar)           -- Will use contents size
        rv,layout.horizontal_window.show_child =
          ImGui.Checkbox(ctx, 'Child', layout.horizontal_window.show_child)               -- Will grow and use contents size
        rv,layout.horizontal_window.explicit_content_size =
          ImGui.Checkbox(ctx, 'Explicit content size', layout.horizontal_window.explicit_content_size)
        ImGui.Text(ctx, ('Scroll %.1f/%.1f %.1f/%.1f'):format(ImGui.GetScrollX(ctx), ImGui.GetScrollMaxX(ctx), ImGui.GetScrollY(ctx), ImGui.GetScrollMaxY(ctx)))
        if layout.horizontal_window.explicit_content_size then
          ImGui.SameLine(ctx)
          ImGui.SetNextItemWidth(ctx, 100)
          rv,layout.horizontal_window.contents_size_x =
            ImGui.DragDouble(ctx, '##csx', layout.horizontal_window.contents_size_x)
          local x, y = ImGui.GetCursorScreenPos(ctx)
          local draw_list = ImGui.GetWindowDrawList(ctx)
          ImGui.DrawList_AddRectFilled(draw_list, x, y, x + 10, y + 10, 0xFFFFFFFF)
          ImGui.DrawList_AddRectFilled(draw_list, x + layout.horizontal_window.contents_size_x - 10, y, x + layout.horizontal_window.contents_size_x, y + 10, 0xFFFFFFFF)
          ImGui.Dummy(ctx, 0, 10)
        end
        ImGui.PopStyleVar(ctx, 2)
        ImGui.Separator(ctx)
        if layout.horizontal_window.show_button then
          ImGui.Button(ctx, 'this is a 300-wide button', 300, 0)
        end
        if layout.horizontal_window.show_tree_nodes then
          if ImGui.TreeNode(ctx, 'this is a tree node') then
            if ImGui.TreeNode(ctx, 'another one of those tree node...') then
              ImGui.Text(ctx, 'Some tree contents')
              ImGui.TreePop(ctx)
            end
            ImGui.TreePop(ctx)
          end
          ImGui.CollapsingHeader(ctx, 'CollapsingHeader', true)
        end
        if layout.horizontal_window.show_text_wrapped then
          ImGui.TextWrapped(ctx, 'This text should automatically wrap on the edge of the work rectangle.')
        end
        if layout.horizontal_window.show_columns then
          ImGui.Text(ctx, 'Tables:')
          if ImGui.BeginTable(ctx, 'table', 4, ImGui.TableFlags_Borders()) then
            for n = 0, 3 do
              ImGui.TableNextColumn(ctx)
              ImGui.Text(ctx, ('Width %.2f'):format(ImGui.GetContentRegionAvail(ctx)))
            end
            ImGui.EndTable(ctx)
          end
          -- ImGui.Text(ctx, 'Columns:')
          -- ImGui.Columns(ctx, 4)
          -- for n = 0, 3 do
          --   ImGui.Text(ctx, ('Width %.2f'):format(ImGui.GetColumnWidth()))
          --   ImGui.NextColumn(ctx)
          -- end
          -- ImGui.Columns(ctx, 1)
        end
        if layout.horizontal_window.show_tab_bar and ImGui.BeginTabBar(ctx, 'Hello') then
          if ImGui.BeginTabItem(ctx, 'OneOneOne') then ImGui.EndTabItem(ctx) end
          if ImGui.BeginTabItem(ctx, 'TwoTwoTwo') then ImGui.EndTabItem(ctx) end
          if ImGui.BeginTabItem(ctx, 'ThreeThreeThree') then ImGui.EndTabItem(ctx) end
          if ImGui.BeginTabItem(ctx, 'FourFourFour') then ImGui.EndTabItem(ctx) end
          ImGui.EndTabBar(ctx)
        end
        if layout.horizontal_window.show_child then
          if ImGui.BeginChild(ctx, 'child', 0, 0, true) then
            ImGui.EndChild(ctx)
          end
        end
        ImGui.End(ctx)
      end
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Clipping') then
    if not layout.clipping then
      layout.clipping = {
        size   = { 100.0, 100.0 },
        offset = {  30.0,  30.0 },
      }
    end

    rv,layout.clipping.size[1],layout.clipping.size[2] =
      ImGui.DragDouble2(ctx, 'size', layout.clipping.size[1], layout.clipping.size[2],
      0.5, 1.0, 200.0, '%.0f')
    ImGui.TextWrapped(ctx, '(Click and drag to scroll)')

    demo.HelpMarker(
      '(Left) Using ImGui_PushClipRect():\n\z
       Will alter ImGui hit-testing logic + DrawList rendering.\n\z
       (use this if you want your clipping rectangle to affect interactions)\n\n\z
       (Center) Using ImGui_DrawList_PushClipRect():\n\z
       Will alter DrawList rendering only.\n\z
       (use this as a shortcut if you are only using DrawList calls)\n\n\z
       (Right) Using ImGui_DrawList_AddText() with a fine ClipRect:\n\z
       Will alter only this specific ImGui_DrawList_AddText() rendering.\n\z
       This is often used internally to avoid altering the clipping rectangle and minimize draw calls.')

    for n = 0, 2 do
      if n > 0 then ImGui.SameLine(ctx) end

      ImGui.PushID(ctx, n)
      ImGui.InvisibleButton(ctx, '##canvas', table.unpack(layout.clipping.size))
      if ImGui.IsItemActive(ctx) and ImGui.IsMouseDragging(ctx, ImGui.MouseButton_Left()) then
        local mouse_delta = {ImGui.GetMouseDelta(ctx)}
        layout.clipping.offset[1] = layout.clipping.offset[1] + mouse_delta[1]
        layout.clipping.offset[2] = layout.clipping.offset[2] + mouse_delta[2]
      end
      ImGui.PopID(ctx)

      if ImGui.IsItemVisible(ctx) then -- Skip rendering as DrawList elements are not clipped.
        local p0_x, p0_y = ImGui.GetItemRectMin(ctx)
        local p1_x, p1_y = ImGui.GetItemRectMax(ctx)
        local text_str = 'Line 1 hello\nLine 2 clip me!'
        local text_pos = { p0_x + layout.clipping.offset[1], p0_y + layout.clipping.offset[2] }

        local draw_list = ImGui.GetWindowDrawList(ctx)
        if n == 0 then
          ImGui.PushClipRect(ctx, p0_x, p0_y, p1_x, p1_y, true)
          ImGui.DrawList_AddRectFilled(draw_list, p0_x, p0_y, p1_x, p1_y, 0x5a5a78ff)
          ImGui.DrawList_AddText(draw_list, text_pos[1], text_pos[2], 0xffffffff, text_str)
          ImGui.PopClipRect(ctx)
        elseif n == 1 then
          ImGui.DrawList_PushClipRect(draw_list, p0_x, p0_y, p1_x, p1_y, true)
          ImGui.DrawList_AddRectFilled(draw_list, p0_x, p0_y, p1_x, p1_y, 0x5a5a78ff)
          ImGui.DrawList_AddText(draw_list, text_pos[1], text_pos[2], 0xffffffff, text_str)
          ImGui.DrawList_PopClipRect(draw_list)
        elseif n == 2 then
          local clip_rect = { p0_x, p0_y, p1_x, p1_y }
          ImGui.DrawList_AddRectFilled(draw_list, p0_x, p0_y, p1_x, p1_y, 0x5a5a78ff)
          ImGui.DrawList_AddTextEx(draw_list, ImGui.GetFont(ctx), ImGui.GetFontSize(ctx),
            text_pos[1], text_pos[2], 0xffffffff, text_str, 0.0, table.unpack(clip_rect))
        end
      end
    end

    ImGui.TreePop(ctx)
  end
end

function demo.ShowDemoWindowPopups()
  if not ImGui.CollapsingHeader(ctx, 'Popups & Modal windows') then
    return
  end

  local rv

  -- The properties of popups windows are:
  -- - They block normal mouse hovering detection outside them. (*)
  -- - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
  -- - Their visibility state (~bool) is held internally by Dear ImGui instead of being held by the programmer as
  --   we are used to with regular Begin() calls. User can manipulate the visibility state by calling OpenPopup().
  -- (*) One can use IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) to bypass it and detect hovering even
  --     when normally blocked by a popup.
  -- Those three properties are connected. The library needs to hold their visibility state BECAUSE it can close
  -- popups at any time.

  -- Typical use for regular windows:
  --   bool my_tool_is_active = false; if (ImGui.Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
  -- Typical use for popups:
  --   if (ImGui.Button("Open")) ImGui.OpenPopup("MyPopup"); if (ImGui.BeginPopup("MyPopup") { [...] EndPopup(); }

  -- With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
  -- This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.

  if ImGui.TreeNode(ctx, 'Popups') then
    if not popups.popups then
      popups.popups = {
        selected_fish = -1,
        toggles = { true, false, false, false, false },
      }
    end

    ImGui.TextWrapped(ctx,
      'When a popup is active, it inhibits interacting with windows that are behind the popup. \z
       Clicking outside the popup closes it.')

    local names = { 'Bream', 'Haddock', 'Mackerel', 'Pollock', 'Tilefish' }

    -- Simple selection popup (if you want to show the current selection inside the Button itself,
    -- you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
    if ImGui.Button(ctx, 'Select..') then
      ImGui.OpenPopup(ctx, 'my_select_popup')
    end
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, names[popups.popups.selected_fish] or '<None>')
    if ImGui.BeginPopup(ctx, 'my_select_popup') then
      ImGui.SeparatorText(ctx, 'Aquarium')
      for i,fish in ipairs(names) do
        if ImGui.Selectable(ctx, fish) then
          popups.popups.selected_fish = i
        end
      end
      ImGui.EndPopup(ctx)
    end

    -- Showing a menu with toggles
    if ImGui.Button(ctx, 'Toggle..') then
      ImGui.OpenPopup(ctx, 'my_toggle_popup')
    end
    if ImGui.BeginPopup(ctx, 'my_toggle_popup') then
      for i,fish in ipairs(names) do
        rv,popups.popups.toggles[i] = ImGui.MenuItem(ctx, fish, '', popups.popups.toggles[i])
      end
      if ImGui.BeginMenu(ctx, 'Sub-menu') then
        ImGui.MenuItem(ctx, 'Click me')
        ImGui.EndMenu(ctx)
      end

      ImGui.Separator(ctx)
      ImGui.Text(ctx, 'Tooltip here')
      if ImGui.IsItemHovered(ctx) then
        ImGui.SetTooltip(ctx, 'I am a tooltip over a popup')
      end

      if ImGui.Button(ctx, 'Stacked Popup') then
        ImGui.OpenPopup(ctx, 'another popup')
      end
      if ImGui.BeginPopup(ctx, 'another popup') then
        for i,fish in ipairs(names) do
          rv,popups.popups.toggles[i] = ImGui.MenuItem(ctx, fish, '', popups.popups.toggles[i])
        end
        if ImGui.BeginMenu(ctx, 'Sub-menu') then
          ImGui.MenuItem(ctx, 'Click me')
          if ImGui.Button(ctx, 'Stacked Popup') then
            ImGui.OpenPopup(ctx, 'another popup')
          end
          if ImGui.BeginPopup(ctx, 'another popup') then
            ImGui.Text(ctx, 'I am the last one here.')
            ImGui.EndPopup(ctx)
          end
          ImGui.EndMenu(ctx)
        end
        ImGui.EndPopup(ctx)
      end
      ImGui.EndPopup(ctx)
    end

    -- Call the more complete ShowExampleMenuFile which we use in various places of this demo
    if ImGui.Button(ctx, 'With a menu..') then
      ImGui.OpenPopup(ctx, 'my_file_popup')
    end
    if ImGui.BeginPopup(ctx, 'my_file_popup', ImGui.WindowFlags_MenuBar()) then
      if ImGui.BeginMenuBar(ctx) then
        if ImGui.BeginMenu(ctx, 'File') then
          demo.ShowExampleMenuFile()
          ImGui.EndMenu(ctx)
        end
        if ImGui.BeginMenu(ctx, 'Edit') then
          ImGui.MenuItem(ctx, 'Dummy')
          ImGui.EndMenu(ctx)
        end
        ImGui.EndMenuBar(ctx)
      end
      ImGui.Text(ctx, 'Hello from popup!')
      ImGui.Button(ctx, 'This is a dummy button..')
      ImGui.EndPopup(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Context menus') then
    if not popups.context then
      popups.context = {
        value = 0.5,
        name  = 'Label1',
        selected = 0,
      }
    end

    demo.HelpMarker('"Context" functions are simple helpers to associate a Popup to a given Item or Window identifier.')

    -- BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
    --     if (id == 0)
    --         id = GetItemID(); // Use last item id
    --     if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
    --         OpenPopup(id);
    --     return BeginPopup(id);
    -- For advanced uses you may want to replicate and customize this code.
    -- See more details in BeginPopupContextItem().

    -- Example 1
    -- When used after an item that has an ID (e.g. Button), we can skip providing an ID to BeginPopupContextItem(),
    -- and BeginPopupContextItem() will use the last item ID as the popup ID.
    do
      local names = { 'Label1', 'Label2', 'Label3', 'Label4', 'Label5' }
      for n, name in ipairs(names) do
        if ImGui.Selectable(ctx, name, popups.context.selected == n) then
          popups.context.selected = n
        end
        if ImGui.BeginPopupContextItem(ctx) then -- use last item id as popup id
          popups.context.selected = n
          ImGui.Text(ctx, ('This a popup for "%s"!'):format(name))
          if ImGui.Button(ctx, 'Close') then
            ImGui.CloseCurrentPopup(ctx)
          end
          ImGui.EndPopup(ctx)
        end
        if ImGui.IsItemHovered(ctx) then
          ImGui.SetTooltip(ctx, 'Right-click to open popup')
        end
      end
    end

    -- Example 2
    -- Popup on a Text() element which doesn't have an identifier: we need to provide an identifier to BeginPopupContextItem().
    -- Using an explicit identifier is also convenient if you want to activate the popups from different locations.
    do
      demo.HelpMarker("Text() elements don't have stable identifiers so we need to provide one.")
      ImGui.Text(ctx, ('Value = %.6f <-- (1) right-click this text'):format(popups.context.value))
      if ImGui.BeginPopupContextItem(ctx, 'my popup') then
        if ImGui.Selectable(ctx, 'Set to zero') then popups.context.value = 0.0      end
        if ImGui.Selectable(ctx, 'Set to PI')   then popups.context.value = 3.141592 end
        ImGui.SetNextItemWidth(ctx, -FLT_MIN)
        rv,popups.context.value = ImGui.DragDouble(ctx, '##Value', popups.context.value, 0.1, 0.0, 0.0)
        ImGui.EndPopup(ctx)
      end

      -- We can also use OpenPopupOnItemClick() to toggle the visibility of a given popup.
      -- Here we make it that right-clicking this other text element opens the same popup as above.
      -- The popup itself will be submitted by the code above.
      ImGui.Text(ctx, '(2) Or right-click this text')
      ImGui.OpenPopupOnItemClick(ctx, 'my popup', ImGui.PopupFlags_MouseButtonRight())

      -- Back to square one: manually open the same popup.
      if ImGui.Button(ctx, '(3) Or click this button') then
        ImGui.OpenPopup(ctx, 'my popup')
      end
    end

    -- Example 3
    -- When using BeginPopupContextItem() with an implicit identifier (NULL == use last item ID),
    -- we need to make sure your item identifier is stable.
    -- In this example we showcase altering the item label while preserving its identifier, using the ### operator (see FAQ).
    do
      demo.HelpMarker('Showcase using a popup ID linked to item ID, with the item having a changing label + stable ID using the ### operator.')
      ImGui.Button(ctx, ('Button: %s###Button'):format(popups.context.name)) -- ### operator override ID ignoring the preceding label
      if ImGui.BeginPopupContextItem(ctx) then
        ImGui.Text(ctx, 'Edit name:')
        rv,popups.context.name = ImGui.InputText(ctx, '##edit', popups.context.name)
        if ImGui.Button(ctx, 'Close') then
          ImGui.CloseCurrentPopup(ctx)
        end
        ImGui.EndPopup(ctx)
      end
      ImGui.SameLine(ctx); ImGui.Text(ctx, '(<-- right-click here)')
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Modals') then
    if not popups.modal then
      popups.modal = {
        dont_ask_me_next_time = false,
        item  = 1,
        color = 0x66b30080,
      }
    end

    ImGui.TextWrapped(ctx, 'Modal windows are like popups but the user cannot close them by clicking outside.')

    if ImGui.Button(ctx, 'Delete..') then
      ImGui.OpenPopup(ctx, 'Delete?')
    end

    -- Always center this window when appearing
    local center = {ImGui.Viewport_GetCenter(ImGui.GetWindowViewport(ctx))}
    ImGui.SetNextWindowPos(ctx, center[1], center[2], ImGui.Cond_Appearing(), 0.5, 0.5)

    if ImGui.BeginPopupModal(ctx, 'Delete?', nil, ImGui.WindowFlags_AlwaysAutoResize()) then
      ImGui.Text(ctx, 'All those beautiful files will be deleted.\nThis operation cannot be undone!')
      ImGui.Separator(ctx)

      --static int unused_i = 0;
      --ImGui.Combo("Combo", &unused_i, "Delete\0Delete harder\0");

      ImGui.PushStyleVar(ctx, ImGui.StyleVar_FramePadding(), 0, 0)
      rv,popups.modal.dont_ask_me_next_time =
        ImGui.Checkbox(ctx, "Don't ask me next time", popups.modal.dont_ask_me_next_time)
      ImGui.PopStyleVar(ctx)

      if ImGui.Button(ctx, 'OK', 120, 0) then ImGui.CloseCurrentPopup(ctx) end
      ImGui.SetItemDefaultFocus(ctx)
      ImGui.SameLine(ctx)
      if ImGui.Button(ctx, 'Cancel', 120, 0) then ImGui.CloseCurrentPopup(ctx) end
      ImGui.EndPopup(ctx)
    end

    if ImGui.Button(ctx, 'Stacked modals..') then
      ImGui.OpenPopup(ctx, 'Stacked 1')
    end
    if ImGui.BeginPopupModal(ctx, 'Stacked 1', nil, ImGui.WindowFlags_MenuBar()) then
      if ImGui.BeginMenuBar(ctx) then
        if ImGui.BeginMenu(ctx, 'File') then
          if ImGui.MenuItem(ctx, 'Some menu item') then end
          ImGui.EndMenu(ctx)
        end
        ImGui.EndMenuBar(ctx)
      end
      ImGui.Text(ctx, 'Hello from Stacked The First\nUsing style.Colors[ImGuiCol_ModalWindowDimBg] behind it.')

      -- Testing behavior of widgets stacking their own regular popups over the modal.
      rv,popups.modal.item  = ImGui.Combo(ctx, 'Combo', popups.modal.item, 'aaaa\0bbbb\0cccc\0dddd\0eeee\0')
      rv,popups.modal.color = ImGui.ColorEdit4(ctx, 'color', popups.modal.color)

      if ImGui.Button(ctx, 'Add another modal..') then
        ImGui.OpenPopup(ctx, 'Stacked 2')
      end

      -- Also demonstrate passing p_open=true to BeginPopupModal(), this will create a regular close button which
      -- will close the popup.
      local unused_open = true
      if ImGui.BeginPopupModal(ctx, 'Stacked 2', unused_open) then
        ImGui.Text(ctx, 'Hello from Stacked The Second!')
        if ImGui.Button(ctx, 'Close') then
          ImGui.CloseCurrentPopup(ctx)
        end
        ImGui.EndPopup(ctx)
      end

      if ImGui.Button(ctx, 'Close') then
        ImGui.CloseCurrentPopup(ctx)
      end
      ImGui.EndPopup(ctx)
    end

    ImGui.TreePop(ctx)
  end

  if ImGui.TreeNode(ctx, 'Menus inside a regular window') then
    ImGui.TextWrapped(ctx, "Below we are testing adding menu items to a regular window. It's rather unusual but should work!")
    ImGui.Separator(ctx)

    ImGui.MenuItem(ctx, 'Menu item', 'CTRL+M')
    if ImGui.BeginMenu(ctx, 'Menu inside a regular window') then
      demo.ShowExampleMenuFile()
      ImGui.EndMenu(ctx)
    end
    ImGui.Separator(ctx)
    ImGui.TreePop(ctx)
  end
end

local MyItemColumnID_ID          = 4
local MyItemColumnID_Name        = 5
local MyItemColumnID_Quantity    = 6
local MyItemColumnID_Description = 7

function demo.CompareTableItems(a, b)
  for next_id = 0, math.huge do
    local ok, col_user_id, col_idx, sort_order, sort_direction = ImGui.TableGetColumnSortSpecs(ctx, next_id)
    if not ok then break end

    -- Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
    -- We could also choose to identify columns based on their index (col_idx), which is simpler!
    local key
    if col_user_id == MyItemColumnID_ID then
      key = 'id'
    elseif col_user_id == MyItemColumnID_Name then
      key = 'name'
    elseif col_user_id == MyItemColumnID_Quantity then
      key = 'quantity'
    elseif col_user_id == MyItemColumnID_Description then
      key = 'name'
    else
      error('unknown user column ID')
    end

    local is_ascending = sort_direction == ImGui.SortDirection_Ascending()
    if a[key] < b[key] then
      return is_ascending
    elseif a[key] > b[key] then
      return not is_ascending
    end
  end

  -- table.sort is unstable so always return a way to differentiate items.
  -- Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
  return a.id < b.id
end

-- Make the UI compact because there are so many fields
function demo.PushStyleCompact()
  local frame_padding_x, frame_padding_y = ImGui.GetStyleVar(ctx, ImGui.StyleVar_FramePadding())
  local item_spacing_x,  item_spacing_y  = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemSpacing())
  ImGui.PushStyleVar(ctx, ImGui.StyleVar_FramePadding(), frame_padding_x, math.floor(frame_padding_y * 0.60))
  ImGui.PushStyleVar(ctx, ImGui.StyleVar_ItemSpacing(),  item_spacing_x,  math.floor(item_spacing_y  * 0.60))
end

function demo.PopStyleCompact()
  ImGui.PopStyleVar(ctx, 2)
end

-- Show a combo box with a choice of sizing policies
function demo.EditTableSizingFlags(flags)
  local policies = {
    {
      value   = ImGui.TableFlags_None(),
      name    = 'Default',
      tooltip = 'Use default sizing policy:\n- ImGuiTableFlags_SizingFixedFit if ScrollX is on or if host window has ImGuiWindowFlags_AlwaysAutoResize.\n- ImGuiTableFlags_SizingStretchSame otherwise.',
    },
    {
      value   = ImGui.TableFlags_SizingFixedFit(),
      name    = 'ImGuiTableFlags_SizingFixedFit',
      tooltip = 'Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching contents width.',
    },
    {
      value   = ImGui.TableFlags_SizingFixedSame(),
      name    = 'ImGuiTableFlags_SizingFixedSame',
      tooltip = 'Columns are all the same width, matching the maximum contents width.\nImplicitly disable ImGuiTableFlags_Resizable and enable ImGuiTableFlags_NoKeepColumnsVisible.',
    },
    {
      value   = ImGui.TableFlags_SizingStretchProp(),
      name    = 'ImGuiTableFlags_SizingStretchProp',
      tooltip = 'Columns default to _WidthStretch with weights proportional to their widths.',
    },
    {
      value   = ImGui.TableFlags_SizingStretchSame(),
      name    = 'ImGuiTableFlags_SizingStretchSame',
      tooltip = 'Columns default to _WidthStretch with same weights.',
    },
  }

  local sizing_mask = ImGui.TableFlags_SizingFixedFit()    |
                      ImGui.TableFlags_SizingFixedSame()   |
                      ImGui.TableFlags_SizingStretchProp() |
                      ImGui.TableFlags_SizingStretchSame()
  local idx = 1
  while idx < #policies do
    if policies[idx].value == (flags & sizing_mask) then
      break
    end
    idx = idx + 1
  end
  local preview_text = ''
  if idx <= #policies then
    preview_text = policies[idx].name
    if idx > 1 then
      preview_text = preview_text:sub(('ImGuiTableFlags'):len() + 1)
    end
  end
  if ImGui.BeginCombo(ctx, 'Sizing Policy', preview_text) then
    for n,policy in ipairs(policies) do
      if ImGui.Selectable(ctx, policy.name, idx == n) then
        flags = (flags & ~sizing_mask) | policy.value
      end
    end
    ImGui.EndCombo(ctx)
  end
  ImGui.SameLine(ctx)
  ImGui.TextDisabled(ctx, '(?)')
  if ImGui.IsItemHovered(ctx) and ImGui.BeginTooltip(ctx) then
    ImGui.PushTextWrapPos(ctx, ImGui.GetFontSize(ctx) * 50.0)
    for m,policy in ipairs(policies) do
      ImGui.Separator(ctx)
      ImGui.Text(ctx, ('%s:'):format(policy.name))
      ImGui.Separator(ctx)
      local indent_spacing = ImGui.GetStyleVar(ctx, ImGui.StyleVar_IndentSpacing())
      ImGui.SetCursorPosX(ctx, ImGui.GetCursorPosX(ctx) + indent_spacing * 0.5)
      ImGui.Text(ctx, policy.tooltip)
    end
    ImGui.PopTextWrapPos(ctx)
    ImGui.EndTooltip(ctx)
  end

  return flags
end

function demo.EditTableColumnsFlags(flags)
  local rv
  local width_mask = ImGui.TableColumnFlags_WidthStretch() |
                     ImGui.TableColumnFlags_WidthFixed()

  rv,flags = ImGui.CheckboxFlags(ctx, '_Disabled', flags, ImGui.TableColumnFlags_Disabled()); ImGui.SameLine(ctx); demo.HelpMarker('Master disable flag (also hide from context menu)')
  rv,flags = ImGui.CheckboxFlags(ctx, '_DefaultHide', flags, ImGui.TableColumnFlags_DefaultHide())
  rv,flags = ImGui.CheckboxFlags(ctx, '_DefaultSort', flags, ImGui.TableColumnFlags_DefaultSort())
  rv,flags = ImGui.CheckboxFlags(ctx, '_WidthStretch', flags, ImGui.TableColumnFlags_WidthStretch())
  if rv then
    flags = flags & ~(width_mask ^ ImGui.TableColumnFlags_WidthStretch())
  end
  rv,flags = ImGui.CheckboxFlags(ctx, '_WidthFixed', flags, ImGui.TableColumnFlags_WidthFixed())
  if rv then
    flags = flags & ~(width_mask ^ ImGui.TableColumnFlags_WidthFixed())
  end
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoResize', flags, ImGui.TableColumnFlags_NoResize())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoReorder', flags, ImGui.TableColumnFlags_NoReorder())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoHide', flags, ImGui.TableColumnFlags_NoHide())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoClip', flags, ImGui.TableColumnFlags_NoClip())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoSort', flags, ImGui.TableColumnFlags_NoSort())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoSortAscending', flags, ImGui.TableColumnFlags_NoSortAscending())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoSortDescending', flags, ImGui.TableColumnFlags_NoSortDescending())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoHeaderLabel', flags, ImGui.TableColumnFlags_NoHeaderLabel())
  rv,flags = ImGui.CheckboxFlags(ctx, '_NoHeaderWidth', flags, ImGui.TableColumnFlags_NoHeaderWidth())
  rv,flags = ImGui.CheckboxFlags(ctx, '_PreferSortAscending', flags, ImGui.TableColumnFlags_PreferSortAscending())
  rv,flags = ImGui.CheckboxFlags(ctx, '_PreferSortDescending', flags, ImGui.TableColumnFlags_PreferSortDescending())
  rv,flags = ImGui.CheckboxFlags(ctx, '_IndentEnable', flags, ImGui.TableColumnFlags_IndentEnable()); ImGui.SameLine(ctx); demo.HelpMarker('Default for column 0')
  rv,flags = ImGui.CheckboxFlags(ctx, '_IndentDisable', flags, ImGui.TableColumnFlags_IndentDisable()); ImGui.SameLine(ctx); demo.HelpMarker('Default for column >0')

  return flags
end

function demo.ShowTableColumnsStatusFlags(flags)
  ImGui.CheckboxFlags(ctx, '_IsEnabled', flags, ImGui.TableColumnFlags_IsEnabled())
  ImGui.CheckboxFlags(ctx, '_IsVisible', flags, ImGui.TableColumnFlags_IsVisible())
  ImGui.CheckboxFlags(ctx, '_IsSorted',  flags, ImGui.TableColumnFlags_IsSorted())
  ImGui.CheckboxFlags(ctx, '_IsHovered', flags, ImGui.TableColumnFlags_IsHovered())
end

function demo.ShowDemoWindowTables()
  -- ImGui.SetNextItemOpen(ctx, true, ImGui.Cond_Once())
  if not ImGui.CollapsingHeader(ctx, 'Tables') then
    return
  end

  local rv

  -- Using those as a base value to create width/height that are factor of the size of our font
  local TEXT_BASE_WIDTH  = ImGui.CalcTextSize(ctx, 'A')
  local TEXT_BASE_HEIGHT = ImGui.GetTextLineHeightWithSpacing(ctx)

  ImGui.PushID(ctx, 'Tables')

  local open_action = -1
  if ImGui.Button(ctx, 'Open all') then
    open_action = 1
  end
  ImGui.SameLine(ctx)
  if ImGui.Button(ctx, 'Close all') then
    open_action = 0
  end
  ImGui.SameLine(ctx)

  if tables.disable_indent == nil then
    tables.disable_indent = false
  end

  -- Options
  rv,tables.disable_indent = ImGui.Checkbox(ctx, 'Disable tree indentation', tables.disable_indent)
  ImGui.SameLine(ctx)
  demo.HelpMarker('Disable the indenting of tree nodes so demo tables can use the full window width.')
  ImGui.Separator(ctx)
  if tables.disable_indent then
    ImGui.PushStyleVar(ctx, ImGui.StyleVar_IndentSpacing(), 0.0)
  end

  -- About Styling of tables
  -- Most settings are configured on a per-table basis via the flags passed to BeginTable() and TableSetupColumns APIs.
  -- There are however a few settings that a shared and part of the ImGuiStyle structure:
  --   style.CellPadding                          // Padding within each cell
  --   style.Colors[ImGuiCol_TableHeaderBg]       // Table header background
  --   style.Colors[ImGuiCol_TableBorderStrong]   // Table outer and header borders
  --   style.Colors[ImGuiCol_TableBorderLight]    // Table inner borders
  --   style.Colors[ImGuiCol_TableRowBg]          // Table row background when ImGuiTableFlags_RowBg is enabled (even rows)
  --   style.Colors[ImGuiCol_TableRowBgAlt]       // Table row background when ImGuiTableFlags_RowBg is enabled (odds rows)

  local function DoOpenAction()
    if open_action ~= -1 then
      ImGui.SetNextItemOpen(ctx, open_action ~= 0)
    end
  end

  -- Demos
  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Basic') then
    -- Here we will showcase three different ways to output a table.
    -- They are very simple variations of a same thing!

    -- [Method 1] Using TableNextRow() to create a new row, and TableSetColumnIndex() to select the column.
    -- In many situations, this is the most flexible and easy to use pattern.
    demo.HelpMarker('Using TableNextRow() + calling TableSetColumnIndex() _before_ each cell, in a loop.')
    if ImGui.BeginTable(ctx, 'table1', 3) then
      for row = 0, 3 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('Row %d Column %d'):format(row, column))
        end
      end
      ImGui.EndTable(ctx)
    end

    -- [Method 2] Using TableNextColumn() called multiple times, instead of using a for loop + TableSetColumnIndex().
    -- This is generally more convenient when you have code manually submitting the contents of each column.
    demo.HelpMarker('Using TableNextRow() + calling TableNextColumn() _before_ each cell, manually.')
    if ImGui.BeginTable(ctx, 'table2', 3) then
      for row = 0, 3 do
        ImGui.TableNextRow(ctx)
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, ('Row %d'):format(row))
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, 'Some contents')
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, '123.456')
      end
      ImGui.EndTable(ctx)
    end

    -- [Method 3] We call TableNextColumn() _before_ each cell. We never call TableNextRow(),
    -- as TableNextColumn() will automatically wrap around and create new rows as needed.
    -- This is generally more convenient when your cells all contains the same type of data.
    demo.HelpMarker(
      'Only using TableNextColumn(), which tends to be convenient for tables where every cell contains the same type of contents.\n\z
       This is also more similar to the old NextColumn() function of the Columns API, and provided to facilitate the Columns->Tables API transition.')
    if ImGui.BeginTable(ctx, 'table3', 3) then
      for item = 0, 13 do
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, ('Item %d'):format(item))
      end
      ImGui.EndTable(ctx)
    end

    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Borders, background') then
    if not tables.borders_bg then
      tables.borders_bg = {
        flags = ImGui.TableFlags_Borders() | ImGui.TableFlags_RowBg(),
        display_headers = false,
        contents_type = 0,
      }
    end
    -- Expose a few Borders related flags interactively

    demo.PushStyleCompact()
    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.borders_bg.flags, ImGui.TableFlags_RowBg())
    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Borders', tables.borders_bg.flags, ImGui.TableFlags_Borders())
    ImGui.SameLine(ctx); demo.HelpMarker('ImGuiTableFlags_Borders\n = ImGuiTableFlags_BordersInnerV\n | ImGuiTableFlags_BordersOuterV\n | ImGuiTableFlags_BordersInnerV\n | ImGuiTableFlags_BordersOuterH')
    ImGui.Indent(ctx)

    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersH', tables.borders_bg.flags, ImGui.TableFlags_BordersH())
    ImGui.Indent(ctx)
    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterH', tables.borders_bg.flags, ImGui.TableFlags_BordersOuterH())
    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerH', tables.borders_bg.flags, ImGui.TableFlags_BordersInnerH())
    ImGui.Unindent(ctx)

    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.borders_bg.flags, ImGui.TableFlags_BordersV())
    ImGui.Indent(ctx)
    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.borders_bg.flags, ImGui.TableFlags_BordersOuterV())
    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.borders_bg.flags, ImGui.TableFlags_BordersInnerV())
    ImGui.Unindent(ctx)

    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuter', tables.borders_bg.flags, ImGui.TableFlags_BordersOuter())
    rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInner', tables.borders_bg.flags, ImGui.TableFlags_BordersInner())
    ImGui.Unindent(ctx)

    ImGui.AlignTextToFramePadding(ctx); ImGui.Text(ctx, 'Cell contents:')
    ImGui.SameLine(ctx); rv,tables.borders_bg.contents_type = ImGui.RadioButtonEx(ctx, 'Text', tables.borders_bg.contents_type, 0)
    ImGui.SameLine(ctx); rv,tables.borders_bg.contents_type = ImGui.RadioButtonEx(ctx, 'FillButton', tables.borders_bg.contents_type, 1)
    rv,tables.borders_bg.display_headers = ImGui.Checkbox(ctx, 'Display headers', tables.borders_bg.display_headers)
    -- rv,tables.borders_bg.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBody', tables.borders_bg.flags, ImGui.TableFlags_NoBordersInBody()); ImGui.SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body (borders will always appear in Headers')
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table1', 3, tables.borders_bg.flags) then
      -- Display headers so we can inspect their interaction with borders.
      -- (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
      if tables.borders_bg.display_headers then
        ImGui.TableSetupColumn(ctx, 'One')
        ImGui.TableSetupColumn(ctx, 'Two')
        ImGui.TableSetupColumn(ctx, 'Three')
        ImGui.TableHeadersRow(ctx)
      end

      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          local buf = ('Hello %d,%d'):format(column, row)
          if tables.borders_bg.contents_type == 0 then
            ImGui.Text(ctx, buf)
          elseif tables.borders_bg.contents_type == 1 then
            ImGui.Button(ctx, buf, -FLT_MIN, 0.0)
          end
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Resizable, stretch') then
    if not tables.resz_stretch then
      tables.resz_stretch = {
        flags = ImGui.TableFlags_SizingStretchSame() |
                ImGui.TableFlags_Resizable() |
                ImGui.TableFlags_BordersOuter() |
                ImGui.TableFlags_BordersV() |
                ImGui.TableFlags_ContextMenuInBody(),
      }
    end

    -- By default, if we don't enable ScrollX the sizing policy for each column is "Stretch"
    -- All columns maintain a sizing weight, and they will occupy all available width.
    demo.PushStyleCompact()
    rv,tables.resz_stretch.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.resz_stretch.flags, ImGui.TableFlags_Resizable())
    rv,tables.resz_stretch.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.resz_stretch.flags, ImGui.TableFlags_BordersV())
    ImGui.SameLine(ctx); demo.HelpMarker('Using the _Resizable flag automatically enables the _BordersInnerV flag as well, this is why the resize borders are still showing when unchecking this.')
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table1', 3, tables.resz_stretch.flags) then
      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('Hello %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Resizable, fixed') then
    if not tables.resz_fixed then
      tables.resz_fixed = {
        flags = ImGui.TableFlags_SizingFixedFit() |
                ImGui.TableFlags_Resizable() |
                ImGui.TableFlags_BordersOuter() |
                ImGui.TableFlags_BordersV() |
                ImGui.TableFlags_ContextMenuInBody(),
      }
    end

    -- Here we use ImGuiTableFlags_SizingFixedFit (even though _ScrollX is not set)
    -- So columns will adopt the "Fixed" policy and will maintain a fixed width regardless of the whole available width (unless table is small)
    -- If there is not enough available width to fit all columns, they will however be resized down.
    -- FIXME-TABLE: Providing a stretch-on-init would make sense especially for tables which don't have saved settings
    demo.HelpMarker(
      'Using _Resizable + _SizingFixedFit flags.\n\z
       Fixed-width columns generally makes more sense if you want to use horizontal scrolling.\n\n\z
       Double-click a column border to auto-fit the column to its contents.')
    demo.PushStyleCompact()
    rv,tables.resz_fixed.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.resz_fixed.flags, ImGui.TableFlags_NoHostExtendX())
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table1', 3, tables.resz_fixed.flags) then
      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('Hello %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, "Resizable, mixed") then
    if not tables.resz_mixed then
      tables.resz_mixed = {
        flags = ImGui.TableFlags_SizingFixedFit() |
                ImGui.TableFlags_RowBg() | ImGui.TableFlags_Borders() |
                ImGui.TableFlags_Resizable() |
                ImGui.TableFlags_Reorderable() | ImGui.TableFlags_Hideable()
      }
    end
    demo.HelpMarker(
      'Using TableSetupColumn() to alter resizing policy on a per-column basis.\n\n\z
       When combining Fixed and Stretch columns, generally you only want one, maybe two trailing columns to use _WidthStretch.')

    if ImGui.BeginTable(ctx, 'table1', 3, tables.resz_mixed.flags) then
      ImGui.TableSetupColumn(ctx, 'AAA', ImGui.TableColumnFlags_WidthFixed())
      ImGui.TableSetupColumn(ctx, 'BBB', ImGui.TableColumnFlags_WidthFixed())
      ImGui.TableSetupColumn(ctx, 'CCC', ImGui.TableColumnFlags_WidthStretch())
      ImGui.TableHeadersRow(ctx)
      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('%s %d,%d'):format(column == 2 and 'Stretch' or 'Fixed', column, row))
        end
      end
      ImGui.EndTable(ctx)
    end
    if ImGui.BeginTable(ctx, 'table2', 6, tables.resz_mixed.flags) then
      ImGui.TableSetupColumn(ctx, 'AAA', ImGui.TableColumnFlags_WidthFixed())
      ImGui.TableSetupColumn(ctx, 'BBB', ImGui.TableColumnFlags_WidthFixed())
      ImGui.TableSetupColumn(ctx, 'CCC', ImGui.TableColumnFlags_WidthFixed() | ImGui.TableColumnFlags_DefaultHide())
      ImGui.TableSetupColumn(ctx, 'DDD', ImGui.TableColumnFlags_WidthStretch())
      ImGui.TableSetupColumn(ctx, 'EEE', ImGui.TableColumnFlags_WidthStretch())
      ImGui.TableSetupColumn(ctx, 'FFF', ImGui.TableColumnFlags_WidthStretch() | ImGui.TableColumnFlags_DefaultHide())
      ImGui.TableHeadersRow(ctx)
      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 5 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('%s %d,%d'):format(column >= 3 and 'Stretch' or 'Fixed', column, row))
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Reorderable, hideable, with headers') then
    if not tables.reorder then
      tables.reorder = {
        flags = ImGui.TableFlags_Resizable() |
                ImGui.TableFlags_Reorderable() |
                ImGui.TableFlags_Hideable() |
                ImGui.TableFlags_BordersOuter() |
                ImGui.TableFlags_BordersV()
      }
    end

    demo.HelpMarker(
      'Click and drag column headers to reorder columns.\n\n\z
       Right-click on a header to open a context menu.')
    demo.PushStyleCompact()
    rv,tables.reorder.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.reorder.flags, ImGui.TableFlags_Resizable())
    rv,tables.reorder.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Reorderable', tables.reorder.flags, ImGui.TableFlags_Reorderable())
    rv,tables.reorder.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Hideable', tables.reorder.flags, ImGui.TableFlags_Hideable())
    -- rv,tables.reorder.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBody', tables.reorder.flags, ImGui.TableFlags_NoBordersInBody())
    -- rv,tables.reorder.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBodyUntilResize', tables.reorder.flags, ImGui.TableFlags_NoBordersInBodyUntilResize()); ImGui.SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)')
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table1', 3, tables.reorder.flags) then
      -- Submit column names with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
      -- (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
      ImGui.TableSetupColumn(ctx, 'One')
      ImGui.TableSetupColumn(ctx, 'Two')
      ImGui.TableSetupColumn(ctx, 'Three')
      ImGui.TableHeadersRow(ctx)
      for row = 0, 5 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('Hello %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end

    -- Use outer_size.x == 0.0 instead of default to make the table as tight as possible (only valid when no scrolling and no stretch column)
    if ImGui.BeginTable(ctx, 'table2', 3, tables.reorder.flags | ImGui.TableFlags_SizingFixedFit(), 0.0, 0.0) then
      ImGui.TableSetupColumn(ctx, 'One')
      ImGui.TableSetupColumn(ctx, 'Two')
      ImGui.TableSetupColumn(ctx, 'Three')
      ImGui.TableHeadersRow(ctx)
      for row = 0, 5 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('Fixed %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Padding') then
    if not tables.padding then
      tables.padding = {
        flags1 = ImGui.TableFlags_BordersV(),
        show_headers = false,

        flags2 = ImGui.TableFlags_Borders() | ImGui.TableFlags_RowBg(),
        cell_padding = { 0.0, 0.0 },
        show_widget_frame_bg = true,
        text_bufs = {}, -- Mini text storage for 3x5 cells
      }

      for i = 1, 3*5 do
        tables.padding.text_bufs[i] = 'edit me'
      end
    end

    -- First example: showcase use of padding flags and effect of BorderOuterV/BorderInnerV on X padding.
    -- We don't expose BorderOuterH/BorderInnerH here because they have no effect on X padding.
    demo.HelpMarker(
      "We often want outer padding activated when any using features which makes the edges of a column visible:\n\z
       e.g.:\n\z
       - BorderOuterV\n\z
       - any form of row selection\n\z
       Because of this, activating BorderOuterV sets the default to PadOuterX. Using PadOuterX or NoPadOuterX you can override the default.\n\n\z
       Actual padding values are using style.CellPadding.\n\n\z
       In this demo we don't show horizontal borders to emphasize how they don't affect default horizontal padding.")

    demo.PushStyleCompact()
    rv,tables.padding.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_PadOuterX', tables.padding.flags1, ImGui.TableFlags_PadOuterX())
    ImGui.SameLine(ctx); demo.HelpMarker('Enable outer-most padding (default if ImGuiTableFlags_BordersOuterV is set)')
    rv,tables.padding.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadOuterX', tables.padding.flags1, ImGui.TableFlags_NoPadOuterX())
    ImGui.SameLine(ctx); demo.HelpMarker('Disable outer-most padding (default if ImGuiTableFlags_BordersOuterV is not set)')
    rv,tables.padding.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadInnerX', tables.padding.flags1, ImGui.TableFlags_NoPadInnerX())
    ImGui.SameLine(ctx); demo.HelpMarker('Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off)')
    rv,tables.padding.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.padding.flags1, ImGui.TableFlags_BordersOuterV())
    rv,tables.padding.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.padding.flags1, ImGui.TableFlags_BordersInnerV())
    rv,tables.padding.show_headers = ImGui.Checkbox(ctx, 'show_headers', tables.padding.show_headers)
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table_padding', 3, tables.padding.flags1) then
      if tables.padding.show_headers then
        ImGui.TableSetupColumn(ctx, 'One')
        ImGui.TableSetupColumn(ctx, 'Two')
        ImGui.TableSetupColumn(ctx, 'Three')
        ImGui.TableHeadersRow(ctx)
      end

      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          if row == 0 then
            ImGui.Text(ctx, ('Avail %.2f'):format(ImGui.GetContentRegionAvail(ctx)))
          else
            local buf = ('Hello %d,%d'):format(column, row)
            ImGui.Button(ctx, buf, -FLT_MIN, 0.0)
          end
          --if (ImGui.TableGetColumnFlags() & ImGuiTableColumnFlags_IsHovered)
          --  ImGui.TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 100, 0, 255))
        end
      end
      ImGui.EndTable(ctx)
    end

    -- Second example: set style.CellPadding to (0.0) or a custom value.
    -- FIXME-TABLE: Vertical border effectively not displayed the same way as horizontal one...
    demo.HelpMarker('Setting style.CellPadding to (0,0) or a custom value.')

    demo.PushStyleCompact()
    rv,tables.padding.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Borders', tables.padding.flags2, ImGui.TableFlags_Borders())
    rv,tables.padding.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersH', tables.padding.flags2, ImGui.TableFlags_BordersH())
    rv,tables.padding.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.padding.flags2, ImGui.TableFlags_BordersV())
    rv,tables.padding.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInner', tables.padding.flags2, ImGui.TableFlags_BordersInner())
    rv,tables.padding.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuter', tables.padding.flags2, ImGui.TableFlags_BordersOuter())
    rv,tables.padding.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.padding.flags2, ImGui.TableFlags_RowBg())
    rv,tables.padding.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.padding.flags2, ImGui.TableFlags_Resizable())
    rv,tables.padding.show_widget_frame_bg = ImGui.Checkbox(ctx, 'show_widget_frame_bg', tables.padding.show_widget_frame_bg)
    rv,tables.padding.cell_padding[1],tables.padding.cell_padding[2] =
      ImGui.SliderDouble2(ctx, 'CellPadding', tables.padding.cell_padding[1],
      tables.padding.cell_padding[2], 0.0, 10.0, '%.0f')
    demo.PopStyleCompact()

    ImGui.PushStyleVar(ctx, ImGui.StyleVar_CellPadding(), table.unpack(tables.padding.cell_padding))
    if ImGui.BeginTable(ctx, 'table_padding_2', 3, tables.padding.flags2) then
      if not tables.padding.show_widget_frame_bg then
        ImGui.PushStyleColor(ctx, ImGui.Col_FrameBg(), 0)
      end
      for cell = 1, 3 * 5 do
        ImGui.TableNextColumn(ctx)
        ImGui.SetNextItemWidth(ctx, -FLT_MIN)
        ImGui.PushID(ctx, cell)
        rv,tables.padding.text_bufs[cell] = ImGui.InputText(ctx, '##cell', tables.padding.text_bufs[cell])
        ImGui.PopID(ctx)
      end
      if not tables.padding.show_widget_frame_bg then
        ImGui.PopStyleColor(ctx)
      end
      ImGui.EndTable(ctx)
    end
    ImGui.PopStyleVar(ctx)

    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Sizing policies') then
    if not tables.sz_policies then
      tables.sz_policies = {
        flags1 = ImGui.TableFlags_BordersV()      |
                 ImGui.TableFlags_BordersOuterH() |
                 ImGui.TableFlags_RowBg()         |
                 ImGui.TableFlags_ContextMenuInBody(),
        sizing_policy_flags = {
          ImGui.TableFlags_SizingFixedFit(),
          ImGui.TableFlags_SizingFixedSame(),
          ImGui.TableFlags_SizingStretchProp(),
          ImGui.TableFlags_SizingStretchSame(),
        },

        flags2 = ImGui.TableFlags_ScrollY() |
                 ImGui.TableFlags_Borders() |
                 ImGui.TableFlags_RowBg()   |
                 ImGui.TableFlags_Resizable(),
        contents_type = 0,
        column_count  = 3,
        text_buf      = '',
      }
    end

    demo.PushStyleCompact()
    rv,tables.sz_policies.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.sz_policies.flags1, ImGui.TableFlags_Resizable())
    rv,tables.sz_policies.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.sz_policies.flags1, ImGui.TableFlags_NoHostExtendX())
    demo.PopStyleCompact()

    for table_n,sizing_flags in ipairs(tables.sz_policies.sizing_policy_flags) do
      ImGui.PushID(ctx, table_n)
      ImGui.SetNextItemWidth(ctx, TEXT_BASE_WIDTH * 30)
      sizing_flags = demo.EditTableSizingFlags(sizing_flags)
      tables.sz_policies.sizing_policy_flags[table_n] = sizing_flags

      -- To make it easier to understand the different sizing policy,
      -- For each policy: we display one table where the columns have equal contents width, and one where the columns have different contents width.
      if ImGui.BeginTable(ctx, 'table1', 3, sizing_flags | tables.sz_policies.flags1) then
        for row = 0, 2 do
          ImGui.TableNextRow(ctx)
          ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'Oh dear')
          ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'Oh dear')
          ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'Oh dear')
        end
        ImGui.EndTable(ctx)
      end
      if ImGui.BeginTable(ctx, 'table2', 3, sizing_flags | tables.sz_policies.flags1) then
        for row = 0, 2 do
          ImGui.TableNextRow(ctx)
          ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'AAAA')
          ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'BBBBBBBB')
          ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'CCCCCCCCCCCC')
        end
        ImGui.EndTable(ctx)
      end
      ImGui.PopID(ctx)
    end

    ImGui.Spacing(ctx)
    ImGui.Text(ctx, 'Advanced')
    ImGui.SameLine(ctx)
    demo.HelpMarker('This section allows you to interact and see the effect of various sizing policies depending on whether Scroll is enabled and the contents of your columns.')

    demo.PushStyleCompact()
    ImGui.PushID(ctx, 'Advanced')
    ImGui.PushItemWidth(ctx, TEXT_BASE_WIDTH * 30)
    tables.sz_policies.flags2 = demo.EditTableSizingFlags(tables.sz_policies.flags2)
    rv,tables.sz_policies.contents_type = ImGui.Combo(ctx, 'Contents', tables.sz_policies.contents_type, 'Show width\0Short Text\0Long Text\0Button\0Fill Button\0InputText\0')
    if tables.sz_policies.contents_type == 4 then -- fill button
      ImGui.SameLine(ctx)
      demo.HelpMarker('Be mindful that using right-alignment (e.g. size.x = -FLT_MIN) creates a feedback loop where contents width can feed into auto-column width can feed into contents width.')
    end
    rv,tables.sz_policies.column_count = ImGui.DragInt(ctx, 'Columns', tables.sz_policies.column_count, 0.1, 1, 64, '%d', ImGui.SliderFlags_AlwaysClamp())
    rv,tables.sz_policies.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.sz_policies.flags2, ImGui.TableFlags_Resizable())
    rv,tables.sz_policies.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_PreciseWidths', tables.sz_policies.flags2, ImGui.TableFlags_PreciseWidths())
    ImGui.SameLine(ctx); demo.HelpMarker('Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.')
    rv,tables.sz_policies.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.sz_policies.flags2, ImGui.TableFlags_ScrollX())
    rv,tables.sz_policies.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.sz_policies.flags2, ImGui.TableFlags_ScrollY())
    rv,tables.sz_policies.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoClip', tables.sz_policies.flags2, ImGui.TableFlags_NoClip())
    ImGui.PopItemWidth(ctx)
    ImGui.PopID(ctx)
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table2', tables.sz_policies.column_count, tables.sz_policies.flags2, 0.0, TEXT_BASE_HEIGHT * 7) then
      for cell = 1, 10 * tables.sz_policies.column_count do
        ImGui.TableNextColumn(ctx)
        local column = ImGui.TableGetColumnIndex(ctx)
        local row = ImGui.TableGetRowIndex(ctx)

        ImGui.PushID(ctx, cell)
        local label = ('Hello %d,%d'):format(column, row)
        local contents_type = tables.sz_policies.contents_type
        if contents_type == 1 then -- short text
          ImGui.Text(ctx, label)
        elseif contents_type == 2 then -- long text
          ImGui.Text(ctx, ('Some %s text %d,%d\nOver two lines..'):format(column == 0 and 'long' or 'longeeer', column, row))
        elseif contents_type == 0 then -- show width
          ImGui.Text(ctx, ('W: %.1f'):format(ImGui.GetContentRegionAvail(ctx)))
        elseif contents_type == 3 then -- button
          ImGui.Button(ctx, label)
        elseif contents_type == 4 then -- fill button
          ImGui.Button(ctx, label, -FLT_MIN, 0.0)
        elseif contents_type == 5 then -- input text
          ImGui.SetNextItemWidth(ctx, -FLT_MIN)
          rv,tables.sz_policies.text_buf = ImGui.InputText(ctx, '##', tables.sz_policies.text_buf)
        end
        ImGui.PopID(ctx)
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Vertical scrolling, with clipping') then
    if not tables.vertical then
      tables.vertical = {
        flags = ImGui.TableFlags_ScrollY()      |
                ImGui.TableFlags_RowBg()        |
                ImGui.TableFlags_BordersOuter() |
                ImGui.TableFlags_BordersV()     |
                ImGui.TableFlags_Resizable()    |
                ImGui.TableFlags_Reorderable()  |
                ImGui.TableFlags_Hideable(),
      }
    end

    demo.HelpMarker('Here we activate ScrollY, which will create a child window container to allow hosting scrollable contents.\n\nWe also demonstrate using ImGuiListClipper to virtualize the submission of many items.')

    demo.PushStyleCompact()
    rv,tables.vertical.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.vertical.flags, ImGui.TableFlags_ScrollY())
    demo.PopStyleCompact()

    -- When using ScrollX or ScrollY we need to specify a size for our table container!
    -- Otherwise by default the table will fit all available space, like a BeginChild() call.
    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 8 }
    if ImGui.BeginTable(ctx, 'table_scrolly', 3, tables.vertical.flags, table.unpack(outer_size)) then
      ImGui.TableSetupScrollFreeze(ctx, 0, 1); -- Make top row always visible
      ImGui.TableSetupColumn(ctx, 'One', ImGui.TableColumnFlags_None())
      ImGui.TableSetupColumn(ctx, 'Two', ImGui.TableColumnFlags_None())
      ImGui.TableSetupColumn(ctx, 'Three', ImGui.TableColumnFlags_None())
      ImGui.TableHeadersRow(ctx)

      -- Demonstrate using clipper for large vertical lists
      local clipper = ImGui.CreateListClipper(ctx)
      ImGui.ListClipper_Begin(clipper, 1000)
      while ImGui.ListClipper_Step(clipper) do
        local display_start, display_end = ImGui.ListClipper_GetDisplayRange(clipper)
        for row = display_start, display_end - 1 do
          ImGui.TableNextRow(ctx)
          for column = 0, 2 do
            ImGui.TableSetColumnIndex(ctx, column)
            ImGui.Text(ctx, ('Hello %d,%d'):format(column, row))
          end
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Horizontal scrolling') then
    if not tables.horizontal then
      tables.horizontal = {
        flags1 = ImGui.TableFlags_ScrollX()      |
                 ImGui.TableFlags_ScrollY()      |
                 ImGui.TableFlags_RowBg()        |
                 ImGui.TableFlags_BordersOuter() |
                 ImGui.TableFlags_BordersV()     |
                 ImGui.TableFlags_Resizable()    |
                 ImGui.TableFlags_Reorderable()  |
                 ImGui.TableFlags_Hideable(),
        freeze_cols = 1,
        freeze_rows = 1,

        flags2 = ImGui.TableFlags_SizingStretchSame() |
                 ImGui.TableFlags_ScrollX()           |
                 ImGui.TableFlags_ScrollY()           |
                 ImGui.TableFlags_BordersOuter()      |
                 ImGui.TableFlags_RowBg()             |
                 ImGui.TableFlags_ContextMenuInBody(),
        inner_width = 1000.0,
      }
    end

    demo.HelpMarker(
      "When ScrollX is enabled, the default sizing policy becomes ImGuiTableFlags_SizingFixedFit, \z
       as automatically stretching columns doesn't make much sense with horizontal scrolling.\n\n\z
       Also note that as of the current version, you will almost always want to enable ScrollY along with ScrollX,\z
       because the container window won't automatically extend vertically to fix contents (this may be improved in future versions).")

    demo.PushStyleCompact()
    rv,tables.horizontal.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.horizontal.flags1, ImGui.TableFlags_Resizable())
    rv,tables.horizontal.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.horizontal.flags1, ImGui.TableFlags_ScrollX())
    rv,tables.horizontal.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.horizontal.flags1, ImGui.TableFlags_ScrollY())
    ImGui.SetNextItemWidth(ctx, ImGui.GetFrameHeight(ctx))
    rv,tables.horizontal.freeze_cols = ImGui.DragInt(ctx, 'freeze_cols', tables.horizontal.freeze_cols, 0.2, 0, 9, nil, ImGui.SliderFlags_NoInput())
    ImGui.SetNextItemWidth(ctx, ImGui.GetFrameHeight(ctx))
    rv,tables.horizontal.freeze_rows = ImGui.DragInt(ctx, 'freeze_rows', tables.horizontal.freeze_rows, 0.2, 0, 9, nil, ImGui.SliderFlags_NoInput())
    demo.PopStyleCompact()

    -- When using ScrollX or ScrollY we need to specify a size for our table container!
    -- Otherwise by default the table will fit all available space, like a BeginChild() call.
    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 8 }
    if ImGui.BeginTable(ctx, 'table_scrollx', 7, tables.horizontal.flags1, table.unpack(outer_size)) then
      ImGui.TableSetupScrollFreeze(ctx, tables.horizontal.freeze_cols, tables.horizontal.freeze_rows)
      ImGui.TableSetupColumn(ctx, 'Line #', ImGui.TableColumnFlags_NoHide()) -- Make the first column not hideable to match our use of TableSetupScrollFreeze()
      ImGui.TableSetupColumn(ctx, 'One')
      ImGui.TableSetupColumn(ctx, 'Two')
      ImGui.TableSetupColumn(ctx, 'Three')
      ImGui.TableSetupColumn(ctx, 'Four')
      ImGui.TableSetupColumn(ctx, 'Five')
      ImGui.TableSetupColumn(ctx, 'Six')
      ImGui.TableHeadersRow(ctx)
      for row = 0, 19 do
        ImGui.TableNextRow(ctx)
        for column = 0, 6 do
          -- Both TableNextColumn() and TableSetColumnIndex() return true when a column is visible or performing width measurement.
          -- Because here we know that:
          -- - A) all our columns are contributing the same to row height
          -- - B) column 0 is always visible,
          -- We only always submit this one column and can skip others.
          -- More advanced per-column clipping behaviors may benefit from polling the status flags via TableGetColumnFlags().
          if ImGui.TableSetColumnIndex(ctx, column) or column == 0 then
            if column == 0 then
              ImGui.Text(ctx, ('Line %d'):format(row))
            else
              ImGui.Text(ctx, ('Hello world %d,%d'):format(column, row))
            end
          end
        end
      end
      ImGui.EndTable(ctx)
    end

    ImGui.Spacing(ctx)
    ImGui.Text(ctx, 'Stretch + ScrollX')
    ImGui.SameLine(ctx)
    demo.HelpMarker(
      "Showcase using Stretch columns + ScrollX together: \z
       this is rather unusual and only makes sense when specifying an 'inner_width' for the table!\n\z
       Without an explicit value, inner_width is == outer_size.x and therefore using Stretch columns + ScrollX together doesn't make sense.")
    demo.PushStyleCompact()
    ImGui.PushID(ctx, 'flags3')
    ImGui.PushItemWidth(ctx, TEXT_BASE_WIDTH * 30)
    rv,tables.horizontal.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.horizontal.flags2, ImGui.TableFlags_ScrollX())
    rv,tables.horizontal.inner_width = ImGui.DragDouble(ctx, 'inner_width', tables.horizontal.inner_width, 1.0, 0.0, FLT_MAX, '%.1f')
    ImGui.PopItemWidth(ctx)
    ImGui.PopID(ctx)
    demo.PopStyleCompact()
    if ImGui.BeginTable(ctx, 'table2', 7, tables.horizontal.flags2, outer_size[1], outer_size[2], tables.horizontal.inner_width) then
      for cell = 1, 20 * 7 do
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, ('Hello world %d,%d'):format(ImGui.TableGetColumnIndex(ctx), ImGui.TableGetRowIndex(ctx)))
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Columns flags') then
    if not tables.col_flags then
      tables.col_flags = {
        columns = {
          { name='One',   flags=ImGui.TableColumnFlags_DefaultSort(), flags_out=0 },
          { name='Two',   flags=ImGui.TableColumnFlags_None(),        flags_out=0 },
          { name='Three', flags=ImGui.TableColumnFlags_DefaultHide(), flags_out=0 },
        },
      }
    end

    -- Create a first table just to show all the options/flags we want to make visible in our example!
    if ImGui.BeginTable(ctx, 'table_columns_flags_checkboxes', #tables.col_flags.columns, ImGui.TableFlags_None()) then
      demo.PushStyleCompact()
      for i,column in ipairs(tables.col_flags.columns) do
        ImGui.TableNextColumn(ctx)
        ImGui.PushID(ctx, i)
        ImGui.AlignTextToFramePadding(ctx) -- FIXME-TABLE: Workaround for wrong text baseline propagation across columns
        ImGui.Text(ctx, ("'%s'"):format(column.name))
        ImGui.Spacing(ctx)
        ImGui.Text(ctx, 'Input flags:')
        column.flags = demo.EditTableColumnsFlags(column.flags)
        ImGui.Spacing(ctx)
        ImGui.Text(ctx, 'Output flags:')
        ImGui.BeginDisabled(ctx)
        demo.ShowTableColumnsStatusFlags(column.flags_out)
        ImGui.EndDisabled(ctx)
        ImGui.PopID(ctx)
      end
      demo.PopStyleCompact()
      ImGui.EndTable(ctx)
    end

    -- Create the real table we care about for the example!
    -- We use a scrolling table to be able to showcase the difference between the _IsEnabled and _IsVisible flags above, otherwise in
    -- a non-scrolling table columns are always visible (unless using ImGuiTableFlags_NoKeepColumnsVisible + resizing the parent window down)
    local flags = ImGui.TableFlags_SizingFixedFit() |
                  ImGui.TableFlags_ScrollX()        |
                  ImGui.TableFlags_ScrollY()        |
                  ImGui.TableFlags_RowBg()          |
                  ImGui.TableFlags_BordersOuter()   |
                  ImGui.TableFlags_BordersV()       |
                  ImGui.TableFlags_Resizable()      |
                  ImGui.TableFlags_Reorderable()    |
                  ImGui.TableFlags_Hideable()       |
                  ImGui.TableFlags_Sortable()
    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 9 }
    if ImGui.BeginTable(ctx, 'table_columns_flags', #tables.col_flags.columns, flags, table.unpack(outer_size)) then
      for i,column in ipairs(tables.col_flags.columns) do
        ImGui.TableSetupColumn(ctx, column.name, column.flags)
      end
      ImGui.TableHeadersRow(ctx)
      for i,column in ipairs(tables.col_flags.columns) do
        column.flags_out = ImGui.TableGetColumnFlags(ctx, i - 1)
      end
      local indent_step = TEXT_BASE_WIDTH / 2
      for row = 0, 7 do
        ImGui.Indent(ctx, indent_step); -- Add some indentation to demonstrate usage of per-column IndentEnable/IndentDisable flags.
        ImGui.TableNextRow(ctx)
        for column = 0, #tables.col_flags.columns - 1 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('%s %s'):format(column == 0 and 'Indented' or 'Hello', ImGui.TableGetColumnName(ctx, column)))
        end
      end
      ImGui.Unindent(ctx, indent_step * 8.0)

      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Columns widths') then
    if not tables.col_widths then
      tables.col_widths = {
        flags1 = ImGui.TableFlags_Borders(), --|
                 -- ImGui.TableFlags_NoBordersInBodyUntilResize(),
        flags2 = ImGui.TableFlags_None(),
      }
    end
    demo.HelpMarker('Using TableSetupColumn() to setup default width.')

    demo.PushStyleCompact()
    rv,tables.col_widths.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.col_widths.flags1, ImGui.TableFlags_Resizable())
    -- rv,tables.col_widths.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBodyUntilResize', tables.col_widths.flags1, ImGui.TableFlags_NoBordersInBodyUntilResize())
    demo.PopStyleCompact()
    if ImGui.BeginTable(ctx, 'table1', 3, tables.col_widths.flags1) then
      -- We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
      ImGui.TableSetupColumn(ctx, 'one', ImGui.TableColumnFlags_WidthFixed(), 100.0) -- Default to 100.0
      ImGui.TableSetupColumn(ctx, 'two', ImGui.TableColumnFlags_WidthFixed(), 200.0) -- Default to 200.0
      ImGui.TableSetupColumn(ctx, 'three', ImGui.TableColumnFlags_WidthFixed());     -- Default to auto
      ImGui.TableHeadersRow(ctx)
      for row = 0, 3 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableSetColumnIndex(ctx, column)
          if row == 0 then
            ImGui.Text(ctx, ('(w: %5.1f)'):format(ImGui.GetContentRegionAvail(ctx)))
          else
            ImGui.Text(ctx, ('Hello %d,%d'):format(column, row))
          end
        end
      end
      ImGui.EndTable(ctx)
    end

    demo.HelpMarker("Using TableSetupColumn() to setup explicit width.\n\nUnless _NoKeepColumnsVisible is set, fixed columns with set width may still be shrunk down if there's not enough space in the host.")

    demo.PushStyleCompact()
    rv,tables.col_widths.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoKeepColumnsVisible', tables.col_widths.flags2, ImGui.TableFlags_NoKeepColumnsVisible())
    rv,tables.col_widths.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.col_widths.flags2, ImGui.TableFlags_BordersInnerV())
    rv,tables.col_widths.flags2 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.col_widths.flags2, ImGui.TableFlags_BordersOuterV())
    demo.PopStyleCompact()
    if ImGui.BeginTable(ctx, 'table2', 4, tables.col_widths.flags2) then
      -- We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
      ImGui.TableSetupColumn(ctx, '', ImGui.TableColumnFlags_WidthFixed(), 100.0)
      ImGui.TableSetupColumn(ctx, '', ImGui.TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 15.0)
      ImGui.TableSetupColumn(ctx, '', ImGui.TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 30.0)
      ImGui.TableSetupColumn(ctx, '', ImGui.TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 15.0)
      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 3 do
          ImGui.TableSetColumnIndex(ctx, column)
          if row == 0 then
            ImGui.Text(ctx, ('(w: %5.1f)'):format(ImGui.GetContentRegionAvail(ctx)))
          else
            ImGui.Text(ctx, ('Hello %d,%d'):format(column, row))
          end
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Nested tables') then
    demo.HelpMarker('This demonstrates embedding a table into another table cell.')

    local flags = ImGui.TableFlags_Borders() | ImGui.TableFlags_Resizable() | ImGui.TableFlags_Reorderable() | ImGui.TableFlags_Hideable()
    if ImGui.BeginTable(ctx, 'table_nested1', 2, flags) then
      ImGui.TableSetupColumn(ctx, 'A0')
      ImGui.TableSetupColumn(ctx, 'A1')
      ImGui.TableHeadersRow(ctx)

      ImGui.TableNextColumn(ctx)
      ImGui.Text(ctx, 'A0 Row 0')

      local rows_height = TEXT_BASE_HEIGHT * 2
      if ImGui.BeginTable(ctx, 'table_nested2', 2, flags) then
        ImGui.TableSetupColumn(ctx, 'B0')
        ImGui.TableSetupColumn(ctx, 'B1')
        ImGui.TableHeadersRow(ctx)

        ImGui.TableNextRow(ctx, ImGui.TableRowFlags_None(), rows_height)
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, 'B0 Row 0')
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, 'B0 Row 1')
        ImGui.TableNextRow(ctx, ImGui.TableRowFlags_None(), rows_height)
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, 'B1 Row 0')
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, 'B1 Row 1')

        ImGui.EndTable(ctx)
      end

      ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'A0 Row 1')
      ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'A1 Row 0')
      ImGui.TableNextColumn(ctx); ImGui.Text(ctx, 'A1 Row 1')
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Row height') then
    demo.HelpMarker("You can pass a 'min_row_height' to TableNextRow().\n\nRows are padded with 'ImGui_StyleVar_CellPadding.y' on top and bottom, so effectively the minimum row height will always be >= 'ImGui_StyleVar_CellPadding.y * 2.0'.\n\nWe cannot honor a _maximum_ row height as that would require a unique clipping rectangle per row.")
    if ImGui.BeginTable(ctx, 'table_row_height', 1, ImGui.TableFlags_BordersOuter() | ImGui.TableFlags_BordersInnerV()) then
      for row = 0, 9 do
        local min_row_height = TEXT_BASE_HEIGHT * 0.30 * row
        ImGui.TableNextRow(ctx, ImGui.TableRowFlags_None(), min_row_height)
        ImGui.TableNextColumn(ctx)
        ImGui.Text(ctx, ('min_row_height = %.2f'):format(min_row_height))
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Outer size') then
    if not tables.outer_sz then
      tables.outer_sz = {
        flags = ImGui.TableFlags_Borders() |
                ImGui.TableFlags_Resizable() |
                ImGui.TableFlags_ContextMenuInBody() |
                ImGui.TableFlags_RowBg() |
                ImGui.TableFlags_SizingFixedFit() |
                ImGui.TableFlags_NoHostExtendX(),
      }
    end

    -- Showcasing use of ImGuiTableFlags_NoHostExtendX and ImGuiTableFlags_NoHostExtendY
    -- Important to that note how the two flags have slightly different behaviors!
    ImGui.Text(ctx, 'Using NoHostExtendX and NoHostExtendY:')
    demo.PushStyleCompact()
    rv,tables.outer_sz.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.outer_sz.flags, ImGui.TableFlags_NoHostExtendX())
    ImGui.SameLine(ctx); demo.HelpMarker('Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.')
    rv,tables.outer_sz.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendY', tables.outer_sz.flags, ImGui.TableFlags_NoHostExtendY())
    ImGui.SameLine(ctx); demo.HelpMarker('Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.')
    demo.PopStyleCompact()

    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 5.5 }
    if ImGui.BeginTable(ctx, 'table1', 3, tables.outer_sz.flags, table.unpack(outer_size)) then
      for row = 0, 9 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.SameLine(ctx)
    ImGui.Text(ctx, 'Hello!')

    ImGui.Spacing(ctx)

    local flags = ImGui.TableFlags_Borders() | ImGui.TableFlags_RowBg()
    ImGui.Text(ctx, 'Using explicit size:')
    if ImGui.BeginTable(ctx, 'table2', 3, flags, TEXT_BASE_WIDTH * 30, 0.0) then
      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.SameLine(ctx)
    if ImGui.BeginTable(ctx, 'table3', 3, flags, TEXT_BASE_WIDTH * 30, 0.0) then
      for row = 0, 2 do
        ImGui.TableNextRow(ctx, 0, TEXT_BASE_HEIGHT * 1.5)
        for column = 0, 2 do
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end

    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Background color') then
    if not tables.bg_col then
      tables.bg_col = {
        flags         = ImGui.TableFlags_RowBg(),
        row_bg_type   = 1,
        row_bg_target = 1,
        cell_bg_type  = 1,
      }
    end

    demo.PushStyleCompact()
    rv,tables.bg_col.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Borders', tables.bg_col.flags, ImGui.TableFlags_Borders())
    rv,tables.bg_col.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.bg_col.flags, ImGui.TableFlags_RowBg())
    ImGui.SameLine(ctx); demo.HelpMarker('ImGuiTableFlags_RowBg automatically sets RowBg0 to alternative colors pulled from the Style.')
    rv,tables.bg_col.row_bg_type = ImGui.Combo(ctx, 'row bg type', tables.bg_col.row_bg_type, "None\0Red\0Gradient\0")
    rv,tables.bg_col.row_bg_target = ImGui.Combo(ctx, 'row bg target', tables.bg_col.row_bg_target, "RowBg0\0RowBg1\0"); ImGui.SameLine(ctx); demo.HelpMarker('Target RowBg0 to override the alternating odd/even colors,\nTarget RowBg1 to blend with them.')
    rv,tables.bg_col.cell_bg_type = ImGui.Combo(ctx, 'cell bg type', tables.bg_col.cell_bg_type, 'None\0Blue\0'); ImGui.SameLine(ctx); demo.HelpMarker('We are colorizing cells to B1->C2 here.')
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table1', 5, tables.bg_col.flags) then
      for row = 0, 5 do
        ImGui.TableNextRow(ctx)

        -- Demonstrate setting a row background color with 'ImGui.TableSetBgColor(ImGuiTableBgTarget_RowBgX, ...)'
        -- We use a transparent color so we can see the one behind in case our target is RowBg1 and RowBg0 was already targeted by the ImGuiTableFlags_RowBg flag.
        if tables.bg_col.row_bg_type ~= 0 then
          local row_bg_color
          if tables.bg_col.row_bg_type == 1 then -- flat
            row_bg_color = 0xb34d4da6
          else -- gradient
            row_bg_color = 0x333333a6
            row_bg_color = row_bg_color + (demo.round((row * 0.1) * 0xFF) << 24)
          end
          ImGui.TableSetBgColor(ctx, ImGui.TableBgTarget_RowBg0() + tables.bg_col.row_bg_target, row_bg_color)
        end

        -- Fill cells
        for column = 0, 4 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('%c%c'):format(string.byte('A') + row, string.byte('0') + column))

          -- Change background of Cells B1->C2
          -- Demonstrate setting a cell background color with 'ImGui.TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
          -- (the CellBg color will be blended over the RowBg and ColumnBg colors)
          -- We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
          if row >= 1 and row <= 2 and column >= 1 and column <= 2 and tables.bg_col.cell_bg_type == 1 then
            ImGui.TableSetBgColor(ctx, ImGui.TableBgTarget_CellBg(), 0x4d4db3a6)
          end
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Tree view') then
    local flags = ImGui.TableFlags_BordersV()      |
                  ImGui.TableFlags_BordersOuterH() |
                  ImGui.TableFlags_Resizable()     |
                  ImGui.TableFlags_RowBg()--         |
                  -- ImGui.TableFlags_NoBordersInBody()

    if ImGui.BeginTable(ctx, '3ways', 3, flags) then
      -- The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
      ImGui.TableSetupColumn(ctx, 'Name', ImGui.TableColumnFlags_NoHide())
      ImGui.TableSetupColumn(ctx, 'Size', ImGui.TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 12.0)
      ImGui.TableSetupColumn(ctx, 'Type', ImGui.TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 18.0)
      ImGui.TableHeadersRow(ctx)

      -- Simple storage to output a dummy file-system.
      local nodes = {
        { name='Root',                          type='Folder',      size=-1,     child_idx= 1,  child_count= 3 }, -- 0
        { name='Music',                         type='Folder',      size=-1,     child_idx= 4,  child_count= 2 }, -- 1
        { name='Textures',                      type='Folder',      size=-1,     child_idx= 6,  child_count= 3 }, -- 2
        { name='desktop.ini',                   type='System file', size= 1024,   child_idx=-1, child_count=-1 }, -- 3
        { name='File1_a.wav',                   type='Audio file',  size= 123000, child_idx=-1, child_count=-1 }, -- 4
        { name='File1_b.wav',                   type='Audio file',  size= 456000, child_idx=-1, child_count=-1 }, -- 5
        { name='Image001.png',                  type='Image file',  size= 203128, child_idx=-1, child_count=-1 }, -- 6
        { name='Copy of Image001.png',          type='Image file',  size= 203256, child_idx=-1, child_count=-1 }, -- 7
        { name='Copy of Image001 (Final2).png', type='Image file',  size= 203512, child_idx=-1, child_count=-1 }, -- 8
      }

      local function DisplayNode(node)
        ImGui.TableNextRow(ctx)
        ImGui.TableNextColumn(ctx)
        local is_folder = node.child_count > 0
        if is_folder then
          local open = ImGui.TreeNode(ctx, node.name, ImGui.TreeNodeFlags_SpanFullWidth())
          ImGui.TableNextColumn(ctx)
          ImGui.TextDisabled(ctx, '--')
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, node.type)
          if open then
            for child_n = 1, node.child_count do
              DisplayNode(nodes[node.child_idx + child_n])
            end
            ImGui.TreePop(ctx)
          end
        else
          ImGui.TreeNode(ctx, node.name, ImGui.TreeNodeFlags_Leaf() | ImGui.TreeNodeFlags_Bullet() | ImGui.TreeNodeFlags_NoTreePushOnOpen() | ImGui.TreeNodeFlags_SpanFullWidth())
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, ('%d'):format(node.size))
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, node.type)
        end
      end

      DisplayNode(nodes[1])

      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Item width') then
    if not tables.item_width then
      tables.item_width = {
        dummy_d = 0.0,
      }
    end

    demo.HelpMarker(
      "Showcase using PushItemWidth() and how it is preserved on a per-column basis.\n\n\z
       Note that on auto-resizing non-resizable fixed columns, querying the content width for e.g. right-alignment doesn't make sense.")
    if ImGui.BeginTable(ctx, 'table_item_width', 3, ImGui.TableFlags_Borders()) then
      ImGui.TableSetupColumn(ctx, 'small')
      ImGui.TableSetupColumn(ctx, 'half')
      ImGui.TableSetupColumn(ctx, 'right-align')
      ImGui.TableHeadersRow(ctx)

      for row = 0, 2 do
        ImGui.TableNextRow(ctx)
        if row == 0 then
          -- Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
          ImGui.TableSetColumnIndex(ctx, 0)
          ImGui.PushItemWidth(ctx, TEXT_BASE_WIDTH * 3.0) -- Small
          ImGui.TableSetColumnIndex(ctx, 1)
          ImGui.PushItemWidth(ctx, 0 - ImGui.GetContentRegionAvail(ctx) * 0.5)
          ImGui.TableSetColumnIndex(ctx, 2)
          ImGui.PushItemWidth(ctx, -FLT_MIN) -- Right-aligned
        end

        -- Draw our contents
        ImGui.PushID(ctx, row)
        ImGui.TableSetColumnIndex(ctx, 0)
        rv,tables.item_width.dummy_d = ImGui.SliderDouble(ctx, 'double0', tables.item_width.dummy_d, 0.0, 1.0)
        ImGui.TableSetColumnIndex(ctx, 1)
        rv,tables.item_width.dummy_d = ImGui.SliderDouble(ctx, 'double1', tables.item_width.dummy_d, 0.0, 1.0)
        ImGui.TableSetColumnIndex(ctx, 2)
        rv,tables.item_width.dummy_d = ImGui.SliderDouble(ctx, '##double2', tables.item_width.dummy_d, 0.0, 1.0) -- No visible label since right-aligned
        ImGui.PopID(ctx)
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  -- Demonstrate using TableHeader() calls instead of TableHeadersRow()
  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Custom headers') then
    if not tables.headers then
      tables.headers = {
        column_selected = { false, false, false },
      }
    end

    local COLUMNS_COUNT = 3
    if ImGui.BeginTable(ctx, 'table_custom_headers', COLUMNS_COUNT, ImGui.TableFlags_Borders() | ImGui.TableFlags_Reorderable() | ImGui.TableFlags_Hideable()) then
      ImGui.TableSetupColumn(ctx, 'Apricot')
      ImGui.TableSetupColumn(ctx, 'Banana')
      ImGui.TableSetupColumn(ctx, 'Cherry')

      -- Instead of calling TableHeadersRow() we'll submit custom headers ourselves
      ImGui.TableNextRow(ctx, ImGui.TableRowFlags_Headers())
      for column = 0, COLUMNS_COUNT - 1 do
        ImGui.TableSetColumnIndex(ctx, column)
        local column_name = ImGui.TableGetColumnName(ctx, column) -- Retrieve name passed to TableSetupColumn()
        ImGui.PushID(ctx, column)
        ImGui.PushStyleVar(ctx, ImGui.StyleVar_FramePadding(), 0, 0)
        rv,tables.headers.column_selected[column + 1] =
          ImGui.Checkbox(ctx, '##checkall', tables.headers.column_selected[column + 1])
        ImGui.PopStyleVar(ctx)
        ImGui.SameLine(ctx, 0.0, (ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())))
        ImGui.TableHeader(ctx, column_name)
        ImGui.PopID(ctx)
      end

      for row = 0, 4 do
        ImGui.TableNextRow(ctx)
        for column = 0, 2 do
          local buf = ('Cell %d,%d'):format(column, row)
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Selectable(ctx, buf, tables.headers.column_selected[column + 1])
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  -- Demonstrate creating custom context menus inside columns, while playing it nice with context menus provided by TableHeadersRow()/TableHeader()
  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Context menus') then
    if not tables.ctx_menus then
      tables.ctx_menus = {
        flags1 = ImGui.TableFlags_Resizable()   |
                 ImGui.TableFlags_Reorderable() |
                 ImGui.TableFlags_Hideable()    |
                 ImGui.TableFlags_Borders()     |
                 ImGui.TableFlags_ContextMenuInBody()
      }
    end
    demo.HelpMarker('By default, right-clicking over a TableHeadersRow()/TableHeader() line will open the default context-menu.\nUsing ImGuiTableFlags_ContextMenuInBody we also allow right-clicking over columns body.')

    demo.PushStyleCompact()
    rv,tables.ctx_menus.flags1 = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ContextMenuInBody', tables.ctx_menus.flags1, ImGui.TableFlags_ContextMenuInBody())
    demo.PopStyleCompact()

    -- Context Menus: first example
    -- [1.1] Right-click on the TableHeadersRow() line to open the default table context menu.
    -- [1.2] Right-click in columns also open the default table context menu (if ImGuiTableFlags_ContextMenuInBody is set)
    local COLUMNS_COUNT = 3
    if ImGui.BeginTable(ctx, 'table_context_menu', COLUMNS_COUNT, tables.ctx_menus.flags1) then
      ImGui.TableSetupColumn(ctx, 'One')
      ImGui.TableSetupColumn(ctx, 'Two')
      ImGui.TableSetupColumn(ctx, 'Three')

      -- [1.1]] Right-click on the TableHeadersRow() line to open the default table context menu.
      ImGui.TableHeadersRow(ctx)

      -- Submit dummy contents
      for row = 0, 3 do
        ImGui.TableNextRow(ctx)
        for column = 0, COLUMNS_COUNT - 1 do
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      ImGui.EndTable(ctx)
    end

    -- Context Menus: second example
    -- [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
    -- [2.2] Right-click on the ".." to open a custom popup
    -- [2.3] Right-click in columns to open another custom popup
    demo.HelpMarker('Demonstrate mixing table context menu (over header), item context button (over button) and custom per-colum context menu (over column body).')
    local flags2 = ImGui.TableFlags_Resizable()      |
                   ImGui.TableFlags_SizingFixedFit() |
                   ImGui.TableFlags_Reorderable()    |
                   ImGui.TableFlags_Hideable()       |
                   ImGui.TableFlags_Borders()
    if ImGui.BeginTable(ctx, 'table_context_menu_2', COLUMNS_COUNT, flags2) then
      ImGui.TableSetupColumn(ctx, 'One')
      ImGui.TableSetupColumn(ctx, 'Two')
      ImGui.TableSetupColumn(ctx, 'Three')

      -- [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
      ImGui.TableHeadersRow(ctx)
      for row = 0, 3 do
        ImGui.TableNextRow(ctx)
        for column = 0, COLUMNS_COUNT - 1 do
          -- Submit dummy contents
          ImGui.TableSetColumnIndex(ctx, column)
          ImGui.Text(ctx, ('Cell %d,%d'):format(column, row))
          ImGui.SameLine(ctx)

          -- [2.2] Right-click on the ".." to open a custom popup
          ImGui.PushID(ctx, row * COLUMNS_COUNT + column)
          ImGui.SmallButton(ctx, "..")
          if ImGui.BeginPopupContextItem(ctx) then
            ImGui.Text(ctx, ('This is the popup for Button("..") in Cell %d,%d'):format(column, row))
            if ImGui.Button(ctx, 'Close') then
              ImGui.CloseCurrentPopup(ctx)
            end
            ImGui.EndPopup(ctx)
          end
          ImGui.PopID(ctx)
        end
      end

      -- [2.3] Right-click anywhere in columns to open another custom popup
      -- (instead of testing for !IsAnyItemHovered() we could also call OpenPopup() with ImGuiPopupFlags_NoOpenOverExistingPopup
      -- to manage popup priority as the popups triggers, here "are we hovering a column" are overlapping)
      local hovered_column = -1
      for column = 0, COLUMNS_COUNT do
        ImGui.PushID(ctx, column)
        if (ImGui.TableGetColumnFlags(ctx, column) & ImGui.TableColumnFlags_IsHovered()) ~= 0 then
          hovered_column = column
        end
        if hovered_column == column and not ImGui.IsAnyItemHovered(ctx) and ImGui.IsMouseReleased(ctx, 1) then
          ImGui.OpenPopup(ctx, 'MyPopup')
        end
        if ImGui.BeginPopup(ctx, 'MyPopup') then
          if column == COLUMNS_COUNT then
            ImGui.Text(ctx, 'This is a custom popup for unused space after the last column.')
          else
            ImGui.Text(ctx, ('This is a custom popup for Column %d'):format(column))
          end
          if ImGui.Button(ctx, 'Close') then
            ImGui.CloseCurrentPopup(ctx)
          end
          ImGui.EndPopup(ctx)
        end
        ImGui.PopID(ctx)
      end

      ImGui.EndTable(ctx)
      ImGui.Text(ctx, ('Hovered column: %d'):format(hovered_column))
    end
    ImGui.TreePop(ctx)
  end

  -- Demonstrate creating multiple tables with the same ID
  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Synced instances') then
    if not tables.synced then
      tables.synced = {
        flags = ImGui.TableFlags_Resizable()      |
                ImGui.TableFlags_Reorderable()    |
                ImGui.TableFlags_Hideable()       |
                ImGui.TableFlags_Borders()        |
                ImGui.TableFlags_SizingFixedFit() |
                ImGui.TableFlags_NoSavedSettings(),
      }
    end
    demo.HelpMarker('Multiple tables with the same identifier will share their settings, width, visibility, order etc.')
    rv,tables.synced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.synced.flags, ImGui.TableFlags_ScrollY())
    rv,tables.synced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_SizingFixedFit', tables.synced.flags, ImGui.TableFlags_SizingFixedFit())
    for n = 0, 2 do
      local buf = ('Synced Table %d'):format(n)
      local open = ImGui.CollapsingHeader(ctx, buf, nil, ImGui.TreeNodeFlags_DefaultOpen())
      if open and ImGui.BeginTable(ctx, 'Table', 3, tables.synced.flags, 0, ImGui.GetTextLineHeightWithSpacing(ctx) * 5) then
        ImGui.TableSetupColumn(ctx, 'One')
        ImGui.TableSetupColumn(ctx, 'Two')
        ImGui.TableSetupColumn(ctx, 'Three')
        ImGui.TableHeadersRow(ctx)
        local cell_count = n == 1 and 27 or 9 -- Make second table have a scrollbar to verify that additional decoration is not affecting column positions.
        for cell = 0, cell_count do
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, ('this cell %d'):format(cell))
        end
        ImGui.EndTable(ctx)
      end
    end
    ImGui.TreePop(ctx)
  end

  -- Demonstrate using Sorting facilities
  -- This is a simplified version of the "Advanced" example, where we mostly focus on the code necessary to handle sorting.
  -- Note that the "Advanced" example also showcase manually triggering a sort (e.g. if item quantities have been modified)
  local template_items_names = {
    'Banana', 'Apple', 'Cherry', 'Watermelon', 'Grapefruit', 'Strawberry', 'Mango',
    'Kiwi', 'Orange', 'Pineapple', 'Blueberry', 'Plum', 'Coconut', 'Pear', 'Apricot'
  }
  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Sorting') then
    if not tables.sorting then
      tables.sorting = {
        flags = ImGui.TableFlags_Resizable()       |
                ImGui.TableFlags_Reorderable()     |
                ImGui.TableFlags_Hideable()        |
                ImGui.TableFlags_Sortable()        |
                ImGui.TableFlags_SortMulti()       |
                ImGui.TableFlags_RowBg()           |
                ImGui.TableFlags_BordersOuter()    |
                ImGui.TableFlags_BordersV()        |
                -- ImGui.TableFlags_NoBordersInBody() |
                ImGui.TableFlags_ScrollY(),
        items = {},
      }

      -- Create item list
      for n = 0, 49 do
        local template_n = n % #template_items_names
        local item = {
          id = n,
          name = template_items_names[template_n + 1],
          quantity = (n * n - n) % 20, -- Assign default quantities
        }
        table.insert(tables.sorting.items, item)
      end
    end

    -- Options
    demo.PushStyleCompact()
    rv,tables.sorting.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_SortMulti', tables.sorting.flags, ImGui.TableFlags_SortMulti())
    ImGui.SameLine(ctx); demo.HelpMarker('When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).')
    rv,tables.sorting.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_SortTristate', tables.sorting.flags, ImGui.TableFlags_SortTristate())
    ImGui.SameLine(ctx); demo.HelpMarker('When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).')
    demo.PopStyleCompact()

    if ImGui.BeginTable(ctx, 'table_sorting', 4, tables.sorting.flags, 0.0, TEXT_BASE_HEIGHT * 15, 0.0) then
      -- Declare columns
      -- We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
      -- This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
      -- Demonstrate using a mixture of flags among available sort-related flags:
      -- - ImGuiTableColumnFlags_DefaultSort
      -- - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending / ImGuiTableColumnFlags_NoSortDescending
      -- - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
      ImGui.TableSetupColumn(ctx, 'ID',       ImGui.TableColumnFlags_DefaultSort()          | ImGui.TableColumnFlags_WidthFixed(),   0.0, MyItemColumnID_ID)
      ImGui.TableSetupColumn(ctx, 'Name',                                                       ImGui.TableColumnFlags_WidthFixed(),   0.0, MyItemColumnID_Name)
      ImGui.TableSetupColumn(ctx, 'Action',   ImGui.TableColumnFlags_NoSort()               | ImGui.TableColumnFlags_WidthFixed(),   0.0, MyItemColumnID_Action)
      ImGui.TableSetupColumn(ctx, 'Quantity', ImGui.TableColumnFlags_PreferSortDescending() | ImGui.TableColumnFlags_WidthStretch(), 0.0, MyItemColumnID_Quantity)
      ImGui.TableSetupScrollFreeze(ctx, 0, 1) -- Make row always visible
      ImGui.TableHeadersRow(ctx)

      -- Sort our data if sort specs have been changed!
      if ImGui.TableNeedSort(ctx) then
        table.sort(tables.sorting.items, demo.CompareTableItems)
      end

      -- Demonstrate using clipper for large vertical lists
      local clipper = ImGui.CreateListClipper(ctx)
      ImGui.ListClipper_Begin(clipper, #tables.sorting.items)
      while ImGui.ListClipper_Step(clipper) do
        local display_start, display_end = ImGui.ListClipper_GetDisplayRange(clipper)
        for row_n = display_start, display_end - 1 do
          -- Display a data item
          local item = tables.sorting.items[row_n + 1]
          ImGui.PushID(ctx, item.id)
          ImGui.TableNextRow(ctx)
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, ('%04d'):format(item.id))
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, item.name)
          ImGui.TableNextColumn(ctx)
          ImGui.SmallButton(ctx, 'None')
          ImGui.TableNextColumn(ctx)
          ImGui.Text(ctx, ('%d'):format(item.quantity))
          ImGui.PopID(ctx)
        end
      end
      ImGui.EndTable(ctx)
    end
    ImGui.TreePop(ctx)
  end

  -- In this example we'll expose most table flags and settings.
  -- For specific flags and settings refer to the corresponding section for more detailed explanation.
  -- This section is mostly useful to experiment with combining certain flags or settings with each others.
  -- ImGui.SetNextItemOpen(ctx, true, ImGui.Cond_Once()) -- [DEBUG]
  DoOpenAction()
  if ImGui.TreeNode(ctx, 'Advanced') then
    if not tables.advanced then
      tables.advanced = {
        items = {},
        flags = ImGui.TableFlags_Resizable()       |
                ImGui.TableFlags_Reorderable()     |
                ImGui.TableFlags_Hideable()        |
                ImGui.TableFlags_Sortable()        |
                ImGui.TableFlags_SortMulti()       |
                ImGui.TableFlags_RowBg()           |
                ImGui.TableFlags_Borders()         |
                -- ImGui.TableFlags_NoBordersInBody() |
                ImGui.TableFlags_ScrollX()         |
                ImGui.TableFlags_ScrollY()         |
                ImGui.TableFlags_SizingFixedFit(),
        contents_type           = 5, -- selectable span row
        freeze_cols             = 1,
        freeze_rows             = 1,
        items_count             = #template_items_names * 2,
        outer_size_value        = { 0.0, TEXT_BASE_HEIGHT * 12 },
        row_min_height          = 0.0, -- Auto
        inner_width_with_scroll = 0.0, -- Auto-extend
        outer_size_enabled      = true,
        show_headers            = true,
        show_wrapped_text       = false,
        items_need_sort         = false,
      }
    end

    -- //static ImGuiTextFilter filter;
    -- ImGui.SetNextItemOpen(ctx, true, ImGui.Cond_Once()) -- FIXME-TABLE: Enabling this results in initial clipped first pass on table which tend to affect column sizing
    if ImGui.TreeNode(ctx, 'Options') then
      -- Make the UI compact because there are so many fields
      demo.PushStyleCompact()
      ImGui.PushItemWidth(ctx, TEXT_BASE_WIDTH * 28.0)

      if ImGui.TreeNode(ctx, 'Features:', ImGui.TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.advanced.flags, ImGui.TableFlags_Resizable())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Reorderable', tables.advanced.flags, ImGui.TableFlags_Reorderable())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Hideable', tables.advanced.flags, ImGui.TableFlags_Hideable())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_Sortable', tables.advanced.flags, ImGui.TableFlags_Sortable())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoSavedSettings', tables.advanced.flags, ImGui.TableFlags_NoSavedSettings())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ContextMenuInBody', tables.advanced.flags, ImGui.TableFlags_ContextMenuInBody())
        ImGui.TreePop(ctx)
      end

      if ImGui.TreeNode(ctx, 'Decorations:', ImGui.TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.advanced.flags, ImGui.TableFlags_RowBg())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.advanced.flags, ImGui.TableFlags_BordersV())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.advanced.flags, ImGui.TableFlags_BordersOuterV())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.advanced.flags, ImGui.TableFlags_BordersInnerV())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersH', tables.advanced.flags, ImGui.TableFlags_BordersH())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterH', tables.advanced.flags, ImGui.TableFlags_BordersOuterH())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerH', tables.advanced.flags, ImGui.TableFlags_BordersInnerH())
        -- rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBody', tables.advanced.flags, ImGui.TableFlags_NoBordersInBody()) ImGui.SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body (borders will always appear in Headers')
        -- rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBodyUntilResize', tables.advanced.flags, ImGui.TableFlags_NoBordersInBodyUntilResize()) ImGui.SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)')
        ImGui.TreePop(ctx)
      end

      if ImGui.TreeNode(ctx, 'Sizing:', ImGui.TreeNodeFlags_DefaultOpen()) then
        tables.advanced.flags = demo.EditTableSizingFlags(tables.advanced.flags)
        ImGui.SameLine(ctx); demo.HelpMarker('In the Advanced demo we override the policy of each column so those table-wide settings have less effect that typical.')
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.advanced.flags, ImGui.TableFlags_NoHostExtendX())
        ImGui.SameLine(ctx); demo.HelpMarker('Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.')
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendY', tables.advanced.flags, ImGui.TableFlags_NoHostExtendY())
        ImGui.SameLine(ctx); demo.HelpMarker('Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.')
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoKeepColumnsVisible', tables.advanced.flags, ImGui.TableFlags_NoKeepColumnsVisible())
        ImGui.SameLine(ctx); demo.HelpMarker('Only available if ScrollX is disabled.')
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_PreciseWidths', tables.advanced.flags, ImGui.TableFlags_PreciseWidths())
        ImGui.SameLine(ctx); demo.HelpMarker('Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.')
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoClip', tables.advanced.flags, ImGui.TableFlags_NoClip())
        ImGui.SameLine(ctx); demo.HelpMarker('Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with ScrollFreeze options.')
        ImGui.TreePop(ctx)
      end

      if ImGui.TreeNode(ctx, 'Padding:', ImGui.TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_PadOuterX',   tables.advanced.flags, ImGui.TableFlags_PadOuterX())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadOuterX', tables.advanced.flags, ImGui.TableFlags_NoPadOuterX())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadInnerX', tables.advanced.flags, ImGui.TableFlags_NoPadInnerX())
        ImGui.TreePop(ctx)
      end

      if ImGui.TreeNode(ctx, 'Scrolling:', ImGui.TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.advanced.flags, ImGui.TableFlags_ScrollX())
        ImGui.SameLine(ctx)
        ImGui.SetNextItemWidth(ctx, ImGui.GetFrameHeight(ctx))
        rv,tables.advanced.freeze_cols = ImGui.DragInt(ctx, 'freeze_cols', tables.advanced.freeze_cols, 0.2, 0, 9, nil, ImGui.SliderFlags_NoInput())
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.advanced.flags, ImGui.TableFlags_ScrollY())
        ImGui.SameLine(ctx)
        ImGui.SetNextItemWidth(ctx, ImGui.GetFrameHeight(ctx))
        rv,tables.advanced.freeze_rows = ImGui.DragInt(ctx, 'freeze_rows', tables.advanced.freeze_rows, 0.2, 0, 9, nil, ImGui.SliderFlags_NoInput())
        ImGui.TreePop(ctx)
      end

      if ImGui.TreeNode(ctx, 'Sorting:', ImGui.TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_SortMulti', tables.advanced.flags, ImGui.TableFlags_SortMulti())
        ImGui.SameLine(ctx); demo.HelpMarker('When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).')
        rv,tables.advanced.flags = ImGui.CheckboxFlags(ctx, 'ImGuiTableFlags_SortTristate', tables.advanced.flags, ImGui.TableFlags_SortTristate())
        ImGui.SameLine(ctx); demo.HelpMarker('When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).')
        ImGui.TreePop(ctx)
      end

      if ImGui.TreeNode(ctx, 'Other:', ImGui.TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.show_headers = ImGui.Checkbox(ctx, 'show_headers', tables.advanced.show_headers)
        rv,tables.advanced.show_wrapped_text = ImGui.Checkbox(ctx, 'show_wrapped_text', tables.advanced.show_wrapped_text)

        rv,tables.advanced.outer_size_value[1],tables.advanced.outer_size_value[2] =
          ImGui.DragDouble2(ctx, '##OuterSize', table.unpack(tables.advanced.outer_size_value))
        ImGui.SameLine(ctx, 0.0, (ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())))
        rv,tables.advanced.outer_size_enabled = ImGui.Checkbox(ctx, 'outer_size', tables.advanced.outer_size_enabled)
        ImGui.SameLine(ctx)
        demo.HelpMarker(
          'If scrolling is disabled (ScrollX and ScrollY not set):\n\z
           - The table is output directly in the parent window.\n\z
           - OuterSize.x < 0.0 will right-align the table.\n\z
           - OuterSize.x = 0.0 will narrow fit the table unless there are any Stretch columns.\n\z
           - OuterSize.y then becomes the minimum size for the table, which will extend vertically if there are more rows (unless NoHostExtendY is set).')

        -- From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
        -- To facilitate toying with this demo we will actually pass 0.0 to the BeginTable() when ScrollX is disabled.
        rv,tables.advanced.inner_width_with_scroll = ImGui.DragDouble(ctx, 'inner_width (when ScrollX active)', tables.advanced.inner_width_with_scroll, 1.0, 0.0, FLT_MAX)

        rv,tables.advanced.row_min_height = ImGui.DragDouble(ctx, 'row_min_height', tables.advanced.row_min_height, 1.0, 0.0, FLT_MAX)
        ImGui.SameLine(ctx); demo.HelpMarker('Specify height of the Selectable item.')

        rv,tables.advanced.items_count = ImGui.DragInt(ctx, 'items_count', tables.advanced.items_count, 0.1, 0, 9999)
        rv,tables.advanced.contents_type = ImGui.Combo(ctx, 'items_type (first column)', tables.advanced.contents_type,
          'Text\0Button\0SmallButton\0FillButton\0Selectable\0Selectable (span row)\0')
        -- //filter.Draw('filter');
        ImGui.TreePop(ctx)
      end

      ImGui.PopItemWidth(ctx)
      demo.PopStyleCompact()
      ImGui.Spacing(ctx)
      ImGui.TreePop(ctx)
    end

    -- Update item list if we changed the number of items
    if #tables.advanced.items ~= tables.advanced.items_count then
      tables.advanced.items = {}
      for n = 0, tables.advanced.items_count - 1 do
        local template_n = n % #template_items_names
        local item = {
          id = n,
          name = template_items_names[template_n + 1],
          quantity = template_n == 3 and 10 or (template_n == 4 and 20 or 0), -- Assign default quantities
        }
        table.insert(tables.advanced.items, item)
      end
    end

    -- const ImDrawList* parent_draw_list = ImGui.GetWindowDrawList();
    -- const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
    -- local table_scroll_cur, table_scroll_max, table_draw_list -- For debug display

    -- Submit table
    local inner_width_to_use = (tables.advanced.flags & ImGui.TableFlags_ScrollX()) ~= 0 and tables.advanced.inner_width_with_scroll or 0.0
    local w, h = 0, 0
    if tables.advanced.outer_size_enabled then
      w, h = table.unpack(tables.advanced.outer_size_value)
    end
    if ImGui.BeginTable(ctx, 'table_advanced', 6, tables.advanced.flags, w, h, inner_width_to_use) then
      -- Declare columns
      -- We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
      -- This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
      ImGui.TableSetupColumn(ctx, 'ID',           ImGui.TableColumnFlags_DefaultSort() | ImGui.TableColumnFlags_WidthFixed() | ImGui.TableColumnFlags_NoHide(), 0.0, MyItemColumnID_ID)
      ImGui.TableSetupColumn(ctx, 'Name',         ImGui.TableColumnFlags_WidthFixed(), 0.0, MyItemColumnID_Name)
      ImGui.TableSetupColumn(ctx, 'Action',       ImGui.TableColumnFlags_NoSort() | ImGui.TableColumnFlags_WidthFixed(), 0.0, MyItemColumnID_Action)
      ImGui.TableSetupColumn(ctx, 'Quantity',     ImGui.TableColumnFlags_PreferSortDescending(), 0.0, MyItemColumnID_Quantity)
      ImGui.TableSetupColumn(ctx, 'Description',  (tables.advanced.flags & ImGui.TableFlags_NoHostExtendX()) ~= 0 and 0 or ImGui.TableColumnFlags_WidthStretch(), 0.0, MyItemColumnID_Description)
      ImGui.TableSetupColumn(ctx, 'Hidden',       ImGui.TableColumnFlags_DefaultHide() | ImGui.TableColumnFlags_NoSort())
      ImGui.TableSetupScrollFreeze(ctx, tables.advanced.freeze_cols, tables.advanced.freeze_rows)

      -- Sort our data if sort specs have been changed!
      local specs_dirty, has_specs = ImGui.TableNeedSort(ctx)
      if has_specs and (specs_dirty or tables.advanced.items_need_sort) then
        table.sort(tables.advanced.items, demo.CompareTableItems)
        tables.advanced.items_need_sort = false
      end

      -- Take note of whether we are currently sorting based on the Quantity field,
      -- we will use this to trigger sorting when we know the data of this column has been modified.
      local sorts_specs_using_quantity = (ImGui.TableGetColumnFlags(ctx, 3) & ImGui.TableColumnFlags_IsSorted()) ~= 0

      -- Show headers
      if tables.advanced.show_headers then
        ImGui.TableHeadersRow(ctx)
      end

      -- Show data
      ImGui.PushButtonRepeat(ctx, true)

      -- Demonstrate using clipper for large vertical lists
      local clipper = ImGui.CreateListClipper(ctx)
      ImGui.ListClipper_Begin(clipper, #tables.advanced.items)
      while ImGui.ListClipper_Step(clipper) do
        local display_start, display_end = ImGui.ListClipper_GetDisplayRange(clipper)
        for row_n = display_start, display_end - 1 do
          local item = tables.advanced.items[row_n + 1]
          -- //if (!filter.PassFilter(item->Name))
          -- //    continue;

          ImGui.PushID(ctx, item.id)
          ImGui.TableNextRow(ctx, ImGui.TableRowFlags_None(), tables.advanced.row_min_height)

          -- For the demo purpose we can select among different type of items submitted in the first column
          ImGui.TableSetColumnIndex(ctx, 0)
          local label = ('%04d'):format(item.id)
          local contents_type = tables.advanced.contents_type
          if contents_type == 0 then -- text
              ImGui.Text(ctx, label)
          elseif contents_type == 1 then -- button
              ImGui.Button(ctx, label)
          elseif contents_type == 2 then -- small button
              ImGui.SmallButton(ctx, label)
          elseif contents_type == 3 then -- fill button
              ImGui.Button(ctx, label, -FLT_MIN, 0.0)
          elseif contents_type == 4 or contents_type == 5 then -- selectable/selectable (span row)
            local selectable_flags = contents_type == 5 and ImGui.SelectableFlags_SpanAllColumns() | ImGui.SelectableFlags_AllowItemOverlap() or ImGui.SelectableFlags_None()
            if ImGui.Selectable(ctx, label, item.is_selected, selectable_flags, 0, tables.advanced.row_min_height) then
              if ImGui.IsKeyDown(ctx, ImGui.Mod_Ctrl()) then
                item.is_selected = not item.is_selected
              else
                for _,it in ipairs(tables.advanced.items) do
                  it.is_selected = it == item
                end
              end
            end
          end

          if ImGui.TableSetColumnIndex(ctx, 1) then
            ImGui.Text(ctx, item.name)
          end

          -- Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
          -- and we are currently sorting on the column showing the Quantity.
          -- To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
          -- You will probably need a more advanced system in your code if you want to automatically sort when a specific entry changes.
          if ImGui.TableSetColumnIndex(ctx, 2) then
            if ImGui.SmallButton(ctx, 'Chop') then item.quantity = item.quantity + 1 end
            if sorts_specs_using_quantity and ImGui.IsItemDeactivated(ctx) then tables.advanced.items_need_sort = true end
            ImGui.SameLine(ctx)
            if ImGui.SmallButton(ctx, 'Eat')  then item.quantity = item.quantity - 1 end
            if sorts_specs_using_quantity and ImGui.IsItemDeactivated(ctx) then tables.advanced.items_need_sort = true end
          end

          if ImGui.TableSetColumnIndex(ctx, 3) then
            ImGui.Text(ctx, ('%d'):format(item.quantity))
          end

          ImGui.TableSetColumnIndex(ctx, 4)
          if tables.advanced.show_wrapped_text then
            ImGui.TextWrapped(ctx, 'Lorem ipsum dolor sit amet')
          else
            ImGui.Text(ctx, 'Lorem ipsum dolor sit amet')
          end

          if ImGui.TableSetColumnIndex(ctx, 5) then
            ImGui.Text(ctx, '1234')
          end

          ImGui.PopID(ctx)
        end
      end
      ImGui.PopButtonRepeat(ctx)

      -- Store some info to display debug details below
      -- table_scroll_cur = { ImGui.GetScrollX(ctx), ImGui.GetScrollY(ctx) }
      -- table_scroll_max = { ImGui.GetScrollMaxX(ctx), ImGui.GetScrollMaxY(ctx) }
      -- table_draw_list  = ImGui.GetWindowDrawList(ctx)
      ImGui.EndTable(ctx)
    end
    -- static bool show_debug_details = false;
    -- ImGui.Checkbox("Debug details", &show_debug_details);
    -- if (show_debug_details && table_draw_list)
    -- {
    --     ImGui.SameLine(0.0, 0.0);
    --     const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
    --     if (table_draw_list == parent_draw_list)
    --         ImGui.Text(": DrawCmd: +%d (in same window)",
    --             table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
    --     else
    --         ImGui.Text(": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
    --             table_draw_list_draw_cmd_count - 1, table_scroll_cur.x, table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
    -- }
    ImGui.TreePop(ctx)
  end

  ImGui.PopID(ctx)

  -- demo.ShowDemoWindowColumns()

  if tables.disable_indent then
    ImGui.PopStyleVar(ctx)
  end
end

-- // Demonstrate old/legacy Columns API!
-- // [2020: Columns are under-featured and not maintained. Prefer using the more flexible and powerful BeginTable() API!]
-- static void ShowDemoWindowColumns()
-- {
--     bool open = ImGui.TreeNode("Legacy Columns API");
--     ImGui.SameLine();
--     HelpMarker("Columns() is an old API! Prefer using the more flexible and powerful BeginTable() API!");
--     if (!open)
--         return;
--
--     // Basic columns
--     if (ImGui.TreeNode("Basic"))
--     {
--         ImGui.Text("Without border:");
--         ImGui.Columns(3, "mycolumns3", false);  // 3-ways, no border
--         ImGui.Separator();
--         for (int n = 0; n < 14; n++)
--         {
--             char label[32];
--             sprintf(label, "Item %d", n);
--             if (ImGui.Selectable(label)) {}
--             //if (ImGui.Button(label, ImVec2(-FLT_MIN,0.0f))) {}
--             ImGui.NextColumn();
--         }
--         ImGui.Columns(1);
--         ImGui.Separator();
--
--         ImGui.Text("With border:");
--         ImGui.Columns(4, "mycolumns"); // 4-ways, with border
--         ImGui.Separator();
--         ImGui.Text("ID"); ImGui.NextColumn();
--         ImGui.Text("Name"); ImGui.NextColumn();
--         ImGui.Text("Path"); ImGui.NextColumn();
--         ImGui.Text("Hovered"); ImGui.NextColumn();
--         ImGui.Separator();
--         const char* names[3] = { "One", "Two", "Three" };
--         const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
--         static int selected = -1;
--         for (int i = 0; i < 3; i++)
--         {
--             char label[32];
--             sprintf(label, "%04d", i);
--             if (ImGui.Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
--                 selected = i;
--             bool hovered = ImGui.IsItemHovered();
--             ImGui.NextColumn();
--             ImGui.Text(names[i]); ImGui.NextColumn();
--             ImGui.Text(paths[i]); ImGui.NextColumn();
--             ImGui.Text("%d", hovered); ImGui.NextColumn();
--         }
--         ImGui.Columns(1);
--         ImGui.Separator();
--         ImGui.TreePop();
--     }
--
--     if (ImGui.TreeNode("Borders"))
--     {
--         // NB: Future columns API should allow automatic horizontal borders.
--         static bool h_borders = true;
--         static bool v_borders = true;
--         static int columns_count = 4;
--         const int lines_count = 3;
--         ImGui.SetNextItemWidth(ImGui.GetFontSize() * 8);
--         ImGui.DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d columns");
--         if (columns_count < 2)
--             columns_count = 2;
--         ImGui.SameLine();
--         ImGui.Checkbox("horizontal", &h_borders);
--         ImGui.SameLine();
--         ImGui.Checkbox("vertical", &v_borders);
--         ImGui.Columns(columns_count, NULL, v_borders);
--         for (int i = 0; i < columns_count * lines_count; i++)
--         {
--             if (h_borders && ImGui.GetColumnIndex() == 0)
--                 ImGui.Separator();
--             ImGui.Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
--             ImGui.Text("Width %.2f", ImGui.GetColumnWidth());
--             ImGui.Text("Avail %.2f", ImGui.GetContentRegionAvail().x);
--             ImGui.Text("Offset %.2f", ImGui.GetColumnOffset());
--             ImGui.Text("Long text that is likely to clip");
--             ImGui.Button("Button", ImVec2(-FLT_MIN, 0.0f));
--             ImGui.NextColumn();
--         }
--         ImGui.Columns(1);
--         if (h_borders)
--             ImGui.Separator();
--         ImGui.TreePop();
--     }
--
--     // Create multiple items in a same cell before switching to next column
--     if (ImGui.TreeNode("Mixed items"))
--     {
--         ImGui.Columns(3, "mixed");
--         ImGui.Separator();
--
--         ImGui.Text("Hello");
--         ImGui.Button("Banana");
--         ImGui.NextColumn();
--
--         ImGui.Text("ImGui");
--         ImGui.Button("Apple");
--         static float foo = 1.0f;
--         ImGui.InputFloat("red", &foo, 0.05f, 0, "%.3f");
--         ImGui.Text("An extra line here.");
--         ImGui.NextColumn();
--
--         ImGui.Text("Sailor");
--         ImGui.Button("Corniflower");
--         static float bar = 1.0f;
--         ImGui.InputFloat("blue", &bar, 0.05f, 0, "%.3f");
--         ImGui.NextColumn();
--
--         if (ImGui.CollapsingHeader("Category A")) { ImGui.Text("Blah blah blah"); } ImGui.NextColumn();
--         if (ImGui.CollapsingHeader("Category B")) { ImGui.Text("Blah blah blah"); } ImGui.NextColumn();
--         if (ImGui.CollapsingHeader("Category C")) { ImGui.Text("Blah blah blah"); } ImGui.NextColumn();
--         ImGui.Columns(1);
--         ImGui.Separator();
--         ImGui.TreePop();
--     }
--
--     // Word wrapping
--     if (ImGui.TreeNode("Word-wrapping"))
--     {
--         ImGui.Columns(2, "word-wrapping");
--         ImGui.Separator();
--         ImGui.TextWrapped("The quick brown fox jumps over the lazy dog.");
--         ImGui.TextWrapped("Hello Left");
--         ImGui.NextColumn();
--         ImGui.TextWrapped("The quick brown fox jumps over the lazy dog.");
--         ImGui.TextWrapped("Hello Right");
--         ImGui.Columns(1);
--         ImGui.Separator();
--         ImGui.TreePop();
--     }
--
--     if (ImGui.TreeNode("Horizontal Scrolling"))
--     {
--         ImGui.SetNextWindowContentSize(ImVec2(1500.0f, 0.0f));
--         ImVec2 child_size = ImVec2(0, ImGui.GetFontSize() * 20.0f);
--         if (ImGui.BeginChild("##ScrollingRegion", child_size, false, ImGuiWindowFlags_HorizontalScrollbar))
--         {
--             ImGui.Columns(10);
--
--             // Also demonstrate using clipper for large vertical lists
--             int ITEMS_COUNT = 2000;
--             ImGuiListClipper clipper;
--             clipper.Begin(ITEMS_COUNT);
--             while (clipper.Step())
--             {
--                 for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
--                     for (int j = 0; j < 10; j++)
--                     {
--                         ImGui.Text("Line %d Column %d...", i, j);
--                         ImGui.NextColumn();
--                     }
--             }
--             ImGui.Columns(1);
--             ImGui.EndChild();
--         }
--         ImGui.TreePop();
--     }
--
--     if (ImGui.TreeNode("Tree"))
--     {
--         ImGui.Columns(2, "tree", true);
--         for (int x = 0; x < 3; x++)
--         {
--             bool open1 = ImGui.TreeNode((void*)(intptr_t)x, "Node%d", x);
--             ImGui.NextColumn();
--             ImGui.Text("Node contents");
--             ImGui.NextColumn();
--             if (open1)
--             {
--                 for (int y = 0; y < 3; y++)
--                 {
--                     bool open2 = ImGui.TreeNode((void*)(intptr_t)y, "Node%d.%d", x, y);
--                     ImGui.NextColumn();
--                     ImGui.Text("Node contents");
--                     if (open2)
--                     {
--                         ImGui.Text("Even more contents");
--                         if (ImGui.TreeNode("Tree in column"))
--                         {
--                             ImGui.Text("The quick brown fox jumps over the lazy dog");
--                             ImGui.TreePop();
--                         }
--                     }
--                     ImGui.NextColumn();
--                     if (open2)
--                         ImGui.TreePop();
--                 }
--                 ImGui.TreePop();
--             }
--         }
--         ImGui.Columns(1);
--         ImGui.TreePop();
--     }
--
--     ImGui.TreePop();
-- }

function demo.ShowDemoWindowInputs()
  local rv

  if ImGui.CollapsingHeader(ctx, 'Inputs & Focus') then
    -- Display inputs
    ImGui.SetNextItemOpen(ctx, true, ImGui.Cond_Once())
    if ImGui.TreeNode(ctx, 'Inputs') then
      demo.HelpMarker(
        "This is a simplified view. See more detailed input state:\n\z
          - in 'Tools->Metrics/Debugger->Inputs'.\n\z
          - in 'Tools->Debug Log->IO'.")
      if ImGui.IsMousePosValid(ctx) then
        ImGui.Text(ctx, ('Mouse pos: (%g, %g)'):format(ImGui.GetMousePos(ctx)))
      else
        ImGui.Text(ctx, 'Mouse pos: <INVALID>')
      end
      ImGui.Text(ctx, ('Mouse delta: (%g, %g)'):format(ImGui.GetMouseDelta(ctx)))

      local buttons = 4
      ImGui.Text(ctx, 'Mouse down:')
      for button = 0, buttons do
        if ImGui.IsMouseDown(ctx, button) then
          local duration = ImGui.GetMouseDownDuration(ctx, button)
          ImGui.SameLine(ctx)
          ImGui.Text(ctx, ('b%d (%.02f secs)'):format(button, duration))
        end
      end

      ImGui.Text(ctx, ('Mouse wheel: %.1f %.1f'):format(ImGui.GetMouseWheel(ctx)))

      ImGui.Text(ctx, 'Keys down:')
      for key, name in demo.EachEnum('Key') do
        if ImGui.IsKeyDown(ctx, key) then
          local duration = ImGui.GetKeyDownDuration(ctx, key)
          ImGui.SameLine(ctx)
          ImGui.Text(ctx, ('"%s" %d (%.02f secs)'):format(name, key, duration))
        end
      end
      ImGui.Text(ctx, ('Keys mods: %s%s%s%s'):format(
        ImGui.IsKeyDown(ctx, ImGui.Mod_Ctrl())  and 'CTRL '   or '',
        ImGui.IsKeyDown(ctx, ImGui.Mod_Shift()) and 'SHIFT '  or '',
        ImGui.IsKeyDown(ctx, ImGui.Mod_Alt())   and 'ALT '    or '',
        ImGui.IsKeyDown(ctx, ImGui.Mod_Super()) and 'SUPER '  or ''))

      ImGui.Text(ctx, 'Chars queue:')
      for next_id = 0, math.huge do
        local rv, c = ImGui.GetInputQueueCharacter(ctx, next_id)
        if not rv then break end
        ImGui.SameLine(ctx)
        ImGui.Text(ctx, ("'%s' (0x%04X)"):format(utf8.char(c), c))
      end

      ImGui.TreePop(ctx)
    end

    -- Display ImGuiIO output flags
    -- ImGui.SetNextItemOpen(ctx, true, ImGui.Cond_Once())
    -- if ImGui.TreeNode(ctx, 'Outputs') then
    --   demo.HelpMarker(
    --    'The value of io.WantCaptureMouse and io.WantCaptureKeyboard are normally set by Dear ImGui \z
    --     to instruct your application of how to route inputs. Typically, when a value is true, it means \z
    --     Dear ImGui wants the corresponding inputs and we expect the underlying application to ignore them.\n\n\z
    --     The most typical case is: when hovering a window, Dear ImGui set io.WantCaptureMouse to true, \z
    --     and underlying application should ignore mouse inputs (in practice there are many and more subtle \z
    --     rules leading to how those flags are set).');
    --   ImGui.Text('io.WantCaptureMouse: %d', io.WantCaptureMouse);
    --   ImGui.Text('io.WantCaptureMouseUnlessPopupClose: %d', io.WantCaptureMouseUnlessPopupClose);
    --   ImGui.Text('io.WantCaptureKeyboard: %d', io.WantCaptureKeyboard);
    --   ImGui.Text('io.WantTextInput: %d', io.WantTextInput);
    --   ImGui.Text('io.WantSetMousePos: %d', io.WantSetMousePos);
    --   ImGui.Text('io.NavActive: %d, io.NavVisible: %d', io.NavActive, io.NavVisible);

      if ImGui.TreeNode(ctx, 'WantCapture override') then
        if not misc.capture_override then
          misc.capture_override = { mouse = -1, keyboard = -1 }
        end

        demo.HelpMarker(
         -- "Hovering the colored canvas will override io.WantCaptureXXX fields.\n\z
         --  Notice how normally (when set to none), the value of io.WantCaptureKeyboard would be false when hovering and true when clicking."
         "SetNextFrameWantCaptureXXX instructs ReaImGui how to route inputs.\n\n\z
          Capturing the keyboard allows receiving input from REAPER's global scope.\n\n\z
          Hovering the colored canvas will call SetNextFrameWantCaptureXXX.")

        local capture_override_desc = { 'None', 'Set to false', 'Set to true' }
        -- ImGui.SetNextItemWidth(ctx, ImGui.GetFontSize(ctx) * 15)
        -- rv,misc.capture_override.mouse = ImGui.SliderInt(ctx, 'SetNextFrameWantCaptureMouse() on hover', misc.capture_override.mouse, -1, 1, capture_override_desc[misc.capture_override.mouse + 2], ImGui.SliderFlags_AlwaysClamp())
        ImGui.SetNextItemWidth(ctx, ImGui.GetFontSize(ctx) * 15)
        rv,misc.capture_override.keyboard = ImGui.SliderInt(ctx, 'SetNextFrameWantCaptureKeyboard() on hover', misc.capture_override.keyboard, -1, 1, capture_override_desc[misc.capture_override.keyboard + 2], ImGui.SliderFlags_AlwaysClamp())

        ImGui.ColorButton(ctx, '##panel', 0xb219b2ff, ImGui.ColorEditFlags_NoTooltip() | ImGui.ColorEditFlags_NoDragDrop(), 128, 96) -- Dummy item
        -- if ImGui.IsItemHovered(ctx) and misc.capture_override.mouse ~= -1 then
        --   ImGui.SetNextFrameWantCaptureMouse(ctx, misc.capture_override.mouse == 1)
        -- end
        if ImGui.IsItemHovered(ctx) and misc.capture_override.keyboard ~= -1 then
          ImGui.SetNextFrameWantCaptureKeyboard(ctx, misc.capture_override.keyboard == 1)
        end

        ImGui.TreePop(ctx)
      end

    --   ImGui.TreePop(ctx)
    -- end

    -- Display mouse cursors
    if ImGui.TreeNode(ctx, 'Mouse Cursors') then
      local current = ImGui.GetMouseCursor(ctx)
      for cursor, name in demo.EachEnum('MouseCursor') do
        if cursor == current then
          ImGui.Text(ctx, ('Current mouse cursor = %d: %s'):format(current, name))
          break
        end
      end
      ImGui.Text(ctx, 'Hover to see mouse cursors:')
      -- ImGui.SameLine(ctx); demo.HelpMarker(
      --   'Your application can render a different mouse cursor based on what ImGui.GetMouseCursor() returns. \z
      --    If software cursor rendering (io.MouseDrawCursor) is set ImGui will draw the right cursor for you, \z
      --    otherwise your backend needs to handle it.')
      for i, name in demo.EachEnum('MouseCursor') do
        local label = ('Mouse cursor %d: %s'):format(i, name)
        ImGui.Bullet(ctx); ImGui.Selectable(ctx, label, false)
        if ImGui.IsItemHovered(ctx) then
          ImGui.SetMouseCursor(ctx, i)
        end
      end
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Tabbing') then
      if not misc.tabbing then
        misc.tabbing = {
          buf = 'hello',
        }
      end

      ImGui.Text(ctx, 'Use TAB/SHIFT+TAB to cycle through keyboard editable fields.')
      rv,misc.tabbing.buf = ImGui.InputText(ctx, '1', misc.tabbing.buf)
      rv,misc.tabbing.buf = ImGui.InputText(ctx, '2', misc.tabbing.buf)
      rv,misc.tabbing.buf = ImGui.InputText(ctx, '3', misc.tabbing.buf)
      ImGui.PushTabStop(ctx, false)
      rv,misc.tabbing.buf = ImGui.InputText(ctx, '4 (tab skip)', misc.tabbing.buf)
      ImGui.SameLine(ctx); demo.HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.")
      ImGui.PopTabStop(ctx)
      rv,misc.tabbing.buf = ImGui.InputText(ctx, '5', misc.tabbing.buf)
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Focus from code') then
      if not misc.focus then
        misc.focus = {
          buf = 'click on a button to set focus',
          d3  = { 0.0, 0.0, 0.0 }
        }
      end

      local focus_1 = ImGui.Button(ctx, 'Focus on 1'); ImGui.SameLine(ctx)
      local focus_2 = ImGui.Button(ctx, 'Focus on 2'); ImGui.SameLine(ctx)
      local focus_3 = ImGui.Button(ctx, 'Focus on 3')
      local has_focus = 0

      if focus_1 then ImGui.SetKeyboardFocusHere(ctx) end
      rv,misc.focus.buf = ImGui.InputText(ctx, '1', misc.focus.buf)
      if ImGui.IsItemActive(ctx) then has_focus = 1 end

      if focus_2 then ImGui.SetKeyboardFocusHere(ctx) end
      rv,misc.focus.buf = ImGui.InputText(ctx, '2', misc.focus.buf)
      if ImGui.IsItemActive(ctx) then has_focus = 2 end

      ImGui.PushTabStop(ctx, false)
      if focus_3 then ImGui.SetKeyboardFocusHere(ctx) end
      rv,misc.focus.buf = ImGui.InputText(ctx, '3 (tab skip)', misc.focus.buf)
      if ImGui.IsItemActive(ctx) then has_focus = 3 end
      ImGui.SameLine(ctx); demo.HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.")
      ImGui.PopTabStop(ctx)

      if has_focus > 0 then
        ImGui.Text(ctx, ('Item with focus: %d'):format(has_focus))
      else
        ImGui.Text(ctx, 'Item with focus: <none>')
      end

      -- Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
      local focus_ahead = -1
      if ImGui.Button(ctx, 'Focus on X') then focus_ahead = 0 end ImGui.SameLine(ctx)
      if ImGui.Button(ctx, 'Focus on Y') then focus_ahead = 1 end ImGui.SameLine(ctx)
      if ImGui.Button(ctx, 'Focus on Z') then focus_ahead = 2 end
      if focus_ahead ~= -1 then ImGui.SetKeyboardFocusHere(ctx, focus_ahead) end
      rv,misc.focus.d3[1],misc.focus.d3[2],misc.focus.d3[3] =
        ImGui.SliderDouble3(ctx, 'Float3', misc.focus.d3[1], misc.focus.d3[2], misc.focus.d3[3], 0.0, 1.0)

      ImGui.TextWrapped(ctx, 'NB: Cursor & selection are preserved when refocusing last used item in code.')
      ImGui.TreePop(ctx)
    end

    if ImGui.TreeNode(ctx, 'Dragging') then
      ImGui.TextWrapped(ctx, 'You can use GetMouseDragDelta(0) to query for the dragged amount on any widget.')
      for button = 0, 2 do
        ImGui.Text(ctx, ('IsMouseDragging(%d):'):format(button))
        ImGui.Text(ctx, ('  w/ default threshold: %s,'):format(ImGui.IsMouseDragging(ctx, button)))
        ImGui.Text(ctx, ('  w/ zero threshold: %s,'):format(ImGui.IsMouseDragging(ctx, button, 0.0)))
        ImGui.Text(ctx, ('  w/ large threshold: %s,'):format(ImGui.IsMouseDragging(ctx, button, 20.0)))
      end

      ImGui.Button(ctx, 'Drag Me')
      if ImGui.IsItemActive(ctx) then
        -- Draw a line between the button and the mouse cursor
        local draw_list = ImGui.GetForegroundDrawList(ctx)
        local mouse_pos = { ImGui.GetMousePos(ctx) }
        local click_pos = { ImGui.GetMouseClickedPos(ctx, 0) }
        local color = ImGui.GetColor(ctx, ImGui.Col_Button())
        ImGui.DrawList_AddLine(draw_list, click_pos[1], click_pos[2], mouse_pos[1], mouse_pos[2], color, 4.0)
      end

      -- Drag operations gets "unlocked" when the mouse has moved past a certain threshold
      -- (the default threshold is stored in io.MouseDragThreshold). You can request a lower or higher
      -- threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta().
      local value_raw = { ImGui.GetMouseDragDelta(ctx, 0, 0, ImGui.MouseButton_Left(), 0.0) }
      local value_with_lock_threshold = { ImGui.GetMouseDragDelta(ctx, 0, 0, ImGui.MouseButton_Left()) }
      local mouse_delta = { ImGui.GetMouseDelta(ctx) }
      ImGui.Text(ctx, 'GetMouseDragDelta(0):')
      ImGui.Text(ctx, ('  w/ default threshold: (%.1f, %.1f)'):format(table.unpack(value_with_lock_threshold)))
      ImGui.Text(ctx, ('  w/ zero threshold: (%.1f, %.1f)'):format(table.unpack(value_raw)))
      ImGui.Text(ctx, ('GetMouseDelta() (%.1f, %.1f)'):format(table.unpack(mouse_delta)))
      ImGui.TreePop(ctx)
    end
  end
end

-------------------------------------------------------------------------------
-- [SECTION] Style Editor / ShowStyleEditor()
-------------------------------------------------------------------------------
-- - ShowFontSelector()
-- - ShowStyleSelector()
-- - ShowStyleEditor()
-------------------------------------------------------------------------------

-- Demo helper function to select among loaded fonts.
-- Here we use the regular BeginCombo()/EndCombo() api which the more flexible one.
-- void ImGui::ShowFontSelector(const char* label)
-- {
--     ImGuiIO& io = ImGui.GetIO();
--     ImFont* font_current = ImGui.GetFont();
--     if (ImGui.BeginCombo(label, font_current->GetDebugName()))
--     {
--         for (int n = 0; n < io.Fonts->Fonts.Size; n++)
--         {
--             ImFont* font = io.Fonts->Fonts[n];
--             ImGui.PushID((void*)font);
--             if (ImGui.Selectable(font->GetDebugName(), font == font_current))
--                 io.FontDefault = font;
--             ImGui.PopID();
--         }
--         ImGui.EndCombo();
--     }
--     ImGui.SameLine();
--     HelpMarker(
--         "- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
--         "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
--         "- Read FAQ and docs/FONTS.md for more details.\n"
--         "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
-- }

-- Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
-- Here we use the simplified Combo() api that packs items into a single literal string.
-- Useful for quick combo boxes where the choices are known locally.
-- bool ImGui::ShowStyleSelector(const char* label)
-- {
--     static int style_idx = -1;
--     if (ImGui.Combo(label, &style_idx, "Dark\0Light\0Classic\0"))
--     {
--         switch (style_idx)
--         {
--         case 0: ImGui.StyleColorsDark(); break;
--         case 1: ImGui.StyleColorsLight(); break;
--         case 2: ImGui.StyleColorsClassic(); break;
--         }
--         return true;
--     }
--     return false;
-- }

function demo.GetStyleData()
  local data = { vars={}, colors={} }
  local vec2 = {
    'ButtonTextAlign', 'SelectableTextAlign', 'CellPadding', 'ItemSpacing',
    'ItemInnerSpacing', 'FramePadding', 'WindowPadding', 'WindowMinSize',
    'WindowTitleAlign', 'SeparatorTextAlign', 'SeparatorTextPadding',
  }

  for i, name in demo.EachEnum('StyleVar') do
    local rv = {ImGui.GetStyleVar(ctx, i)}
    local is_vec2 = false
    for _, vec2_name in ipairs(vec2) do
      if vec2_name == name then
        is_vec2 = true
        break
      end
    end
    data.vars[i] = is_vec2 and rv or rv[1]
  end
  for i in demo.EachEnum('Col') do
    data.colors[i] = ImGui.GetStyleColor(ctx, i)
  end
  return data
end

function demo.CopyStyleData(source, target)
  for i, value in pairs(source.vars) do
    if type(value) == 'table' then
      target.vars[i] = {table.unpack(value)}
    else
      target.vars[i] = value
    end
  end
  for i, value in pairs(source.colors) do
    target.colors[i] = value
  end
end

function demo.PushStyle()
  if app.style_editor then
    app.style_editor.push_count = app.style_editor.push_count + 1
    for i, value in pairs(app.style_editor.style.vars) do
      if type(value) == 'table' then
        ImGui.PushStyleVar(ctx, i, table.unpack(value))
      else
        ImGui.PushStyleVar(ctx, i, value)
      end
    end
    for i, value in pairs(app.style_editor.style.colors) do
      ImGui.PushStyleColor(ctx, i, value)
    end
  end
end

function demo.PopStyle()
  if app.style_editor and app.style_editor.push_count > 0 then
    app.style_editor.push_count = app.style_editor.push_count - 1
    ImGui.PopStyleColor(ctx, #cache['Col'])
    ImGui.PopStyleVar(ctx, #cache['StyleVar'])
  end
end

function demo.ShowStyleEditor()
  local rv

  if not app.style_editor then
    app.style_editor = {
      style  = demo.GetStyleData(),
      ref    = demo.GetStyleData(),
      output_dest = 0,
      output_only_modified = true,
      push_count = 0,
    }
  end

  ImGui.PushItemWidth(ctx, ImGui.GetWindowWidth(ctx) * 0.50)

--     if (ImGui.ShowStyleSelector("Colors##Selector"))
--         ref_saved_style = style;
--     ImGui.ShowFontSelector("Fonts##Selector");

  -- Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0 or 1.0)
  local FrameRounding, GrabRounding = ImGui.StyleVar_FrameRounding(),
                                      ImGui.StyleVar_GrabRounding()
  rv,app.style_editor.style.vars[FrameRounding] = ImGui.SliderDouble(ctx, 'FrameRounding', app.style_editor.style.vars[FrameRounding], 0.0, 12.0, '%.0f')
  if rv then
    app.style_editor.style.vars[GrabRounding] = app.style_editor.style.vars[FrameRounding] -- Make GrabRounding always the same value as FrameRounding
  end

  local borders = { 'WindowBorder', 'FrameBorder', 'PopupBorder' }
  for i, name in ipairs(borders) do
    local var = ImGui[('StyleVar_%sSize'):format(name)]()
    local enable = app.style_editor.style.vars[var] > 0
    if i > 1 then ImGui.SameLine(ctx) end
    rv, enable = ImGui.Checkbox(ctx, name, enable)
    if rv then app.style_editor.style.vars[var] = enable and 1 or 0 end
  end

  -- Save/Revert button
  if ImGui.Button(ctx, 'Save Ref') then
    demo.CopyStyleData(app.style_editor.style, app.style_editor.ref)
  end
  ImGui.SameLine(ctx)
  if ImGui.Button(ctx, 'Revert Ref') then
    demo.CopyStyleData(app.style_editor.ref, app.style_editor.style)
  end
  ImGui.SameLine(ctx)
  demo.HelpMarker(
    'Save/Revert in local non-persistent storage. Default Colors definition are not affected. \z
     Use "Export" below to save them somewhere.')

  local export = function(enumName, funcSuffix, curTable, refTable, isEqual, formatValue)
    local lines, name_maxlen = {}, 0
    for i, name in demo.EachEnum(enumName) do
      if not app.style_editor.output_only_modified or not isEqual(curTable[i], refTable[i]) then
        table.insert(lines, { name, curTable[i] })
        name_maxlen = math.max(name_maxlen, name:len())
      end
    end

    if app.style_editor.output_dest == 0 then
      ImGui.LogToClipboard(ctx)
    else
      ImGui.LogToTTY(ctx)
    end
    for _, line in ipairs(lines) do
      local pad = string.rep('\x20', name_maxlen - line[1]:len())
      ImGui.LogText(ctx, ('ImGui.Push%s(ctx, ImGui.%s_%s(),%s %s)\n')
        :format(funcSuffix, enumName, line[1], pad, formatValue(line[2])))
    end
    if #lines == 1 then
      ImGui.LogText(ctx, ('\nImGui.Pop%s(ctx)\n'):format(funcSuffix))
    elseif #lines > 1 then
      ImGui.LogText(ctx, ('\nImGui.Pop%s(ctx, %d)\n'):format(funcSuffix, #lines))
    end
    ImGui.LogFinish(ctx)
  end

  if ImGui.Button(ctx, 'Export Vars') then
    export('StyleVar', 'StyleVar', app.style_editor.style.vars, app.style_editor.ref.vars,
      function(a, b) if type(a) == 'table' then return a[1] == b[1] and a[2] == b[2] else return a == b end end,
      function(val) if type(val) == 'table' then return ('%g, %g'):format(table.unpack(val)) else return ('%g'):format(val) end end)
  end
  ImGui.SameLine(ctx)
  if ImGui.Button(ctx, 'Export Colors') then
    export('Col', 'StyleColor', app.style_editor.style.colors, app.style_editor.ref.colors,
      function(a, b) return a == b end, function(val) return ('0x%08X'):format(val & 0xffffffff) end)
  end
  ImGui.SameLine(ctx); ImGui.SetNextItemWidth(ctx, 120); rv,app.style_editor.output_dest = ImGui.Combo(ctx, '##output_type', app.style_editor.output_dest, 'To Clipboard\0To TTY\0')
  ImGui.SameLine(ctx); rv,app.style_editor.output_only_modified = ImGui.Checkbox(ctx, 'Only Modified', app.style_editor.output_only_modified)

  ImGui.Separator(ctx)

  if ImGui.BeginTabBar(ctx, '##tabs', ImGui.TabBarFlags_None()) then
    if ImGui.BeginTabItem(ctx, 'Sizes') then
      local slider = function(varname, min, max, format)
        local func = ImGui['StyleVar_' .. varname]
        assert(func, ('%s is not exposed as a StyleVar'):format(varname))
        local var = func()
        if type(app.style_editor.style.vars[var]) == 'table' then
          local rv,val1,val2 = ImGui.SliderDouble2(ctx, varname, app.style_editor.style.vars[var][1], app.style_editor.style.vars[var][2], min, max, format)
          if rv then app.style_editor.style.vars[var] = { val1, val2 } end
        else
          local rv,val = ImGui.SliderDouble(ctx, varname, app.style_editor.style.vars[var], min, max, format)
          if rv then app.style_editor.style.vars[var] = val end
        end
      end

      ImGui.SeparatorText(ctx, 'Main')
      slider('WindowPadding',     0.0, 20.0, '%.0f')
      slider('FramePadding',      0.0, 20.0, '%.0f')
      slider('CellPadding',       0.0, 20.0, '%.0f')
      slider('ItemSpacing',       0.0, 20.0, '%.0f')
      slider('ItemInnerSpacing',  0.0, 20.0, '%.0f')
      -- slider('TouchExtraPadding', 0.0, 10.0, '%.0f')
      slider('IndentSpacing',     0.0, 30.0, '%.0f')
      slider('ScrollbarSize',     1.0, 20.0, '%.0f')
      slider('GrabMinSize',       1.0, 20.0, '%.0f')

      ImGui.SeparatorText(ctx, 'Borders')
      slider('WindowBorderSize', 0.0, 1.0, '%.0f')
      slider('ChildBorderSize',  0.0, 1.0, '%.0f')
      slider('PopupBorderSize',  0.0, 1.0, '%.0f')
      slider('FrameBorderSize',  0.0, 1.0, '%.0f')
      -- slider('TabBorderSize',    0.0, 1.0, '%.0f')

      ImGui.SeparatorText(ctx, 'Rounding')
      slider('WindowRounding',    0.0, 12.0, '%.0f')
      slider('ChildRounding',     0.0, 12.0, '%.0f')
      slider('FrameRounding',     0.0, 12.0, '%.0f')
      slider('PopupRounding',     0.0, 12.0, '%.0f')
      slider('ScrollbarRounding', 0.0, 12.0, '%.0f')
      slider('GrabRounding',      0.0, 12.0, '%.0f')
      slider('TabRounding',       0.0, 12.0, '%.0f')

      ImGui.SeparatorText(ctx, 'Widgets')
      slider('WindowTitleAlign', 0.0, 1.0, '%.2f')
      -- int window_menu_button_position = app.style_editor.style.WindowMenuButtonPosition + 1
      -- if (ctx, ImGui.Combo(ctx, 'WindowMenuButtonPosition', (ctx, int*)&window_menu_button_position, "None\0Left\0Right\0"))
      --     app.style_editor.style.WindowMenuButtonPosition = window_menu_button_position - 1
      -- ImGui.Combo(ctx, 'ColorButtonPosition', (ctx, int*)&app.style_editor.style.ColorButtonPosition, "Left\0Right\0")
      slider('ButtonTextAlign', 0.0, 1.0, '%.2f')
      ImGui.SameLine(ctx); demo.HelpMarker('Alignment applies when a button is larger than its text content.')
      slider('SelectableTextAlign', 0.0, 1.0, '%.2f')
      ImGui.SameLine(ctx); demo.HelpMarker('Alignment applies when a selectable is larger than its text content.')
      slider('SeparatorTextBorderSize', 0.0, 10.0, '%.0f')
      slider('SeparatorTextAlign',      0.0, 1.0, '%.2f')
      slider('SeparatorTextPadding',    0.0, 40.0, '%.0f')
      -- slider('LogSliderDeadzone', 0.0, 12.0, '%.0f')

      -- ImGui.SeparatorText(ctx, 'Misc')
      -- ImGui.Text(ctx, 'Safe Area Padding')
      -- ImGui.SameLine(ctx); demo.HelpMarker('Adjust if you cannot see the edges of your screen (ctx, e.g. on a TV where scaling has not been configured).')
      -- slider('DisplaySafeAreaPadding', 0.0, 30.0, '%.0f')
      ImGui.EndTabItem(ctx)
    end

    if ImGui.BeginTabItem(ctx, 'Colors') then
      if not app.style_editor.colors then
        app.style_editor.colors = {
          filter = ImGui.CreateTextFilter(),
          alpha_flags = ImGui.ColorEditFlags_None(),
        }
        ImGui.Attach(ctx, app.style_editor.colors.filter)
      end

      ImGui.TextFilter_Draw(app.style_editor.colors.filter, ctx, 'Filter colors', ImGui.GetFontSize(ctx) * 16)

      if ImGui.RadioButton(ctx, 'Opaque', app.style_editor.colors.alpha_flags == ImGui.ColorEditFlags_None()) then
        app.style_editor.colors.alpha_flags = ImGui.ColorEditFlags_None()
      end
      ImGui.SameLine(ctx)
      if ImGui.RadioButton(ctx, 'Alpha',  app.style_editor.colors.alpha_flags == ImGui.ColorEditFlags_AlphaPreview()) then
        app.style_editor.colors.alpha_flags = ImGui.ColorEditFlags_AlphaPreview()
      end
      ImGui.SameLine(ctx)
      if ImGui.RadioButton(ctx, 'Both',   app.style_editor.colors.alpha_flags == ImGui.ColorEditFlags_AlphaPreviewHalf()) then
        app.style_editor.colors.alpha_flags = ImGui.ColorEditFlags_AlphaPreviewHalf()
      end
      ImGui.SameLine(ctx)
      demo.HelpMarker(
        'In the color list:\n\z
         Left-click on color square to open color picker,\n\z
         Right-click to open edit options menu.')

      if ImGui.BeginChild(ctx, '##colors', 0, 0, true,
                            ImGui.WindowFlags_AlwaysVerticalScrollbar()   |
                            ImGui.WindowFlags_AlwaysHorizontalScrollbar() |
                         -- ImGui.WindowFlags_NavFlattened()) TODO: BETA/INTERNAL, not exposed yet
                            0) then
        ImGui.PushItemWidth(ctx, -160)
        local inner_spacing = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())
        for i, name in demo.EachEnum('Col') do
          if ImGui.TextFilter_PassFilter(app.style_editor.colors.filter, name) then
            ImGui.PushID(ctx, i)
            rv, app.style_editor.style.colors[i] = ImGui.ColorEdit4(ctx, '##color', app.style_editor.style.colors[i], ImGui.ColorEditFlags_AlphaBar() | app.style_editor.colors.alpha_flags)
            if app.style_editor.style.colors[i] ~= app.style_editor.ref.colors[i] then
              -- Tips: in a real user application, you may want to merge and use an icon font into the main font,
              -- so instead of "Save"/"Revert" you'd use icons!
              -- Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
              ImGui.SameLine(ctx, 0.0, inner_spacing)
              if ImGui.Button(ctx, 'Save') then
                app.style_editor.ref.colors[i] = app.style_editor.style.colors[i]
              end
              ImGui.SameLine(ctx, 0.0, inner_spacing)
              if ImGui.Button(ctx, 'Revert') then
                app.style_editor.style.colors[i] = app.style_editor.ref.colors[i]
              end
            end
            ImGui.SameLine(ctx, 0.0, inner_spacing)
            ImGui.Text(ctx, name)
            ImGui.PopID(ctx)
          end
        end
        ImGui.PopItemWidth(ctx)
        ImGui.EndChild(ctx)
      end

      ImGui.EndTabItem(ctx)
    end

--         if (ImGui.BeginTabItem("Fonts"))
--         {
--             ImGuiIO& io = ImGui.GetIO();
--             ImFontAtlas* atlas = io.Fonts;
--             HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
--             ImGui.ShowFontAtlas(atlas);
--
--             // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
--             // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
--             const float MIN_SCALE = 0.3f;
--             const float MAX_SCALE = 2.0f;
--             HelpMarker(
--                 "Those are old settings provided for convenience.\n"
--                 "However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, "
--                 "rebuild the font atlas, and call app.style_editor.style.ScaleAllSizes() on a reference ImGuiStyle structure.\n"
--                 "Using those settings here will give you poor quality results.");
--             static float window_scale = 1.0f;
--             ImGui.PushItemWidth(ImGui.GetFontSize() * 8);
--             if (ImGui.DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
--                 ImGui.SetWindowFontScale(window_scale);
--             ImGui.DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
--             ImGui.PopItemWidth();
--
--             ImGui.EndTabItem();
--         }
--
    if ImGui.BeginTabItem(ctx, 'Rendering') then
--             ImGui.Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
--             ImGui.SameLine();
--             HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
--
--             ImGui.Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
--             ImGui.SameLine();
--             HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");
--
--             ImGui.Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
      ImGui.PushItemWidth(ctx, ImGui.GetFontSize(ctx) * 8)
--             ImGui.DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
--             if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;
--
--             // When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
--             ImGui.DragFloat("Circle Tessellation Max Error", &style.CircleTessellationMaxError , 0.005f, 0.10f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
--             const bool show_samples = ImGui::IsItemActive();
--             if (show_samples)
--                 ImGui.SetNextWindowPos(ImGui.GetCursorScreenPos());
--             if (show_samples && ImGui::BeginTooltip())
--             {
--                 ImGui.TextUnformatted("(R = radius, N = number of segments)");
--                 ImGui.Spacing();
--                 ImDrawList* draw_list = ImGui.GetWindowDrawList();
--                 const float min_widget_width = ImGui.CalcTextSize("N: MMM\nR: MMM").x;
--                 for (int n = 0; n < 8; n++)
--                 {
--                     const float RAD_MIN = 5.0f;
--                     const float RAD_MAX = 70.0f;
--                     const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);
--
--                     ImGui.BeginGroup();
--
--                     ImGui.Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad));
--
--                     const float canvas_width = IM_MAX(min_widget_width, rad * 2.0f);
--                     const float offset_x     = floorf(canvas_width * 0.5f);
--                     const float offset_y     = floorf(RAD_MAX);
--
--                     const ImVec2 p1 = ImGui.GetCursorScreenPos();
--                     draw_list->AddCircle(ImVec2(p1.x + offset_x, p1.y + offset_y), rad, ImGui.GetColorU32(ImGuiCol_Text));
--                     ImGui.Dummy(ImVec2(canvas_width, RAD_MAX * 2));
--
--                     /*
--                     const ImVec2 p2 = ImGui.GetCursorScreenPos();
--                     draw_list->AddCircleFilled(ImVec2(p2.x + offset_x, p2.y + offset_y), rad, ImGui.GetColorU32(ImGuiCol_Text));
--                     ImGui.Dummy(ImVec2(canvas_width, RAD_MAX * 2));
--                     */
--
--                     ImGui.EndGroup();
--                     ImGui.SameLine();
--                 }
--                 ImGui.EndTooltip();
--             }
--             ImGui.SameLine();
--             HelpMarker("When drawing circle primitives with \"num_segments == 0\" tesselation will be calculated automatically.");

      local Alpha, DisabledAlpha = ImGui.StyleVar_Alpha(), ImGui.StyleVar_DisabledAlpha()
      rv,app.style_editor.style.vars[Alpha] = ImGui.DragDouble(ctx, 'Global Alpha', app.style_editor.style.vars[Alpha], 0.005, 0.20, 1.0, '%.2f') -- Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
      rv,app.style_editor.style.vars[DisabledAlpha] = ImGui.DragDouble(ctx, 'Disabled Alpha', app.style_editor.style.vars[DisabledAlpha], 0.005, 0.0, 1.0, '%.2f'); ImGui.SameLine(ctx); demo.HelpMarker('Additional alpha multiplier for disabled items (multiply over current value of Alpha).')
      ImGui.PopItemWidth(ctx)

      ImGui.EndTabItem(ctx)
    end

    ImGui.EndTabBar(ctx)
  end

  ImGui.PopItemWidth(ctx)
end

-------------------------------------------------------------------------------
-- [SECTION] User Guide / ShowUserGuide()
-------------------------------------------------------------------------------
--
function demo.ShowUserGuide()
  -- ImGuiIO& io = ImGui.GetIO() TODO
  ImGui.BulletText(ctx, 'Double-click on title bar to collapse window.')
  ImGui.BulletText(ctx,
    'Click and drag on lower corner to resize window\n\z
     (double-click to auto fit window to its contents).')
  ImGui.BulletText(ctx, 'CTRL+Click on a slider or drag box to input value as text.')
  ImGui.BulletText(ctx, 'TAB/SHIFT+TAB to cycle through keyboard editable fields.')
  ImGui.BulletText(ctx, 'CTRL+Tab to select a window.')
  -- if (io.FontAllowUserScaling)
  --     ImGui.BulletText(ctx, 'CTRL+Mouse Wheel to zoom window contents.')
  ImGui.BulletText(ctx, 'While inputing text:\n')
  ImGui.Indent(ctx)
  ImGui.BulletText(ctx, 'CTRL+Left/Right to word jump.')
  ImGui.BulletText(ctx, 'CTRL+A or double-click to select all.')
  ImGui.BulletText(ctx, 'CTRL+X/C/V to use clipboard cut/copy/paste.')
  ImGui.BulletText(ctx, 'CTRL+Z,CTRL+Y to undo/redo.')
  ImGui.BulletText(ctx, 'ESCAPE to revert.')
  ImGui.Unindent(ctx)
  ImGui.BulletText(ctx, 'With keyboard navigation enabled:')
  ImGui.Indent(ctx)
  ImGui.BulletText(ctx, 'Arrow keys to navigate.')
  ImGui.BulletText(ctx, 'Space to activate a widget.')
  ImGui.BulletText(ctx, 'Return to input text into a widget.')
  ImGui.BulletText(ctx, 'Escape to deactivate a widget, close popup, exit child window.')
  ImGui.BulletText(ctx, 'Alt to jump to the menu layer of a window.')
  ImGui.Unindent(ctx)
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
-------------------------------------------------------------------------------
-- - ShowExampleAppMainMenuBar()
-- - ShowExampleMenuFile()
-------------------------------------------------------------------------------

-- Demonstrate creating a "main" fullscreen menu bar and populating it.
-- Note the difference between BeginMainMenuBar() and BeginMenuBar():
-- - BeginMenuBar() = menu-bar inside current window (which needs the ImGuiWindowFlags_MenuBar flag!)
-- - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport + call BeginMenuBar() into it.
-- function demo.ShowExampleAppMainMenuBar()
--   if ImGui.BeginMainMenuBar(ctx) then
--     if ImGui.BeginMenu(ctx, 'File') then
--       demo.ShowExampleMenuFile()
--       ImGui.EndMenu(ctx)
--     end
--     if ImGui.BeginMenu(ctx, 'Edit') then
--       if ImGui.MenuItem(ctx, 'Undo', 'CTRL+Z') then end
--       if ImGui.MenuItem(ctx, 'Redo', 'CTRL+Y', false, false) then end -- Disabled item
--       ImGui.Separator(ctx)
--       if ImGui.MenuItem(ctx, 'Cut', 'CTRL+X') then end
--       if ImGui.MenuItem(ctx, 'Copy', 'CTRL+C') then end
--       if ImGui.MenuItem(ctx, 'Paste', 'CTRL+V') then end
--       ImGui.EndMenu(ctx)
--     end
--     ImGui.EndMainMenuBar(ctx)
--   end
-- end

-- Note that shortcuts are currently provided for display only
-- (future version will add explicit flags to BeginMenu() to request processing shortcuts)
function demo.ShowExampleMenuFile()
  local rv

  ImGui.MenuItem(ctx, '(demo menu)', nil, false, false)
  if ImGui.MenuItem(ctx, 'New') then end
  if ImGui.MenuItem(ctx, 'Open', 'Ctrl+O') then end
  if ImGui.BeginMenu(ctx, 'Open Recent') then
    ImGui.MenuItem(ctx, 'fish_hat.c')
    ImGui.MenuItem(ctx, 'fish_hat.inl')
    ImGui.MenuItem(ctx, 'fish_hat.h')
    if ImGui.BeginMenu(ctx,'More..') then
      ImGui.MenuItem(ctx, 'Hello')
      ImGui.MenuItem(ctx, 'Sailor')
      if ImGui.BeginMenu(ctx, 'Recurse..') then
        demo.ShowExampleMenuFile()
        ImGui.EndMenu(ctx)
      end
      ImGui.EndMenu(ctx)
      end
    ImGui.EndMenu(ctx)
  end
  if ImGui.MenuItem(ctx, 'Save', 'Ctrl+S') then end
  if ImGui.MenuItem(ctx, 'Save As..') then end

  ImGui.Separator(ctx)
  if ImGui.BeginMenu(ctx, 'Options') then
    rv,demo.menu.enabled = ImGui.MenuItem(ctx, 'Enabled', '', demo.menu.enabled)
    if ImGui.BeginChild(ctx, 'child', 0, 60, true) then
      for i = 0, 9 do
        ImGui.Text(ctx, ('Scrolling Text %d'):format(i))
      end
      ImGui.EndChild(ctx)
    end
    rv,demo.menu.f = ImGui.SliderDouble(ctx, 'Value', demo.menu.f, 0.0, 1.0)
    rv,demo.menu.f = ImGui.InputDouble(ctx, 'Input', demo.menu.f, 0.1)
    rv,demo.menu.n = ImGui.Combo(ctx, 'Combo', demo.menu.n, 'Yes\0No\0Maybe\0')
    ImGui.EndMenu(ctx)
  end

  if ImGui.BeginMenu(ctx, 'Colors') then
    local sz = ImGui.GetTextLineHeight(ctx)
    local draw_list = ImGui.GetWindowDrawList(ctx)
    for i, name in demo.EachEnum('Col') do
      local x, y = ImGui.GetCursorScreenPos(ctx)
      ImGui.DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, ImGui.GetColor(ctx, i))
      ImGui.Dummy(ctx, sz, sz)
      ImGui.SameLine(ctx)
      ImGui.MenuItem(ctx, name)
    end
    ImGui.EndMenu(ctx)
  end

  -- Here we demonstrate appending again to the "Options" menu (which we already created above)
  -- Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
  -- In a real code-base using it would make senses to use this feature from very different code locations.
  if ImGui.BeginMenu(ctx, 'Options') then -- <-- Append!
    rv,demo.menu.b = ImGui.Checkbox(ctx, 'SomeOption', demo.menu.b)
    ImGui.EndMenu(ctx)
  end

  if ImGui.BeginMenu(ctx, 'Disabled', false) then -- Disabled
    error('never called')
  end
  if ImGui.MenuItem(ctx, 'Checked', nil, true) then end
  ImGui.Separator(ctx)
  if ImGui.MenuItem(ctx, 'Quit', 'Alt+F4') then end
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Debug Console / ShowExampleAppConsole()
-------------------------------------------------------------------------------

-- Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
-- For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
local ExampleAppConsole = {}
function ExampleAppConsole:new(ctx)
  local instance = {
    ctx          = ctx,
    inputbuf     = '',
    commands     = {},
    history      = {},
    history_pos  = 0, -- 0: new line, 1..#history: browsing history
    filter       = ImGui.CreateTextFilter(),
    auto_scroll      = true,
    scroll_to_bottom = false,
    callback     = ImGui.CreateFunctionFromEEL([[
    function toupper(c)
    (
      c >= 'a' && c <= 'z' ? c - 32 : c;
    );

    EventFlag == InputTextFlags_CallbackCompletion ? (
      // Example of TEXT COMPLETION

      // Locate beginning of current word
      word_end   = CursorPos;
      word_start = word_end;
      while(
        c = str_getchar(#Buf, word_start - 1);
        (c == ' ' || c == '\t' || c == ',' || c == ';') ? 0 : (
          word_start > 0 ? (word_start -= 1; 1) : 0;
        );
      );
      word = #;
      strcpy_substr(word, #Buf, word_start, word_end - word_start);

      // Build a list of candidates
      Candidates = CommandsCount + 1; // Place the array after commands
      CandidatesCount = 0;
      i = 0;
      loop(CommandsCount, (
        strnicmp(Commands + i, word, strlen(word)) == 0 ? (
          strcpy(Candidates + CandidatesCount, Commands + i);
          CandidatesCount += 1;
        );
        i += 1;
      ));

      CandidatesCount == 0 ? (
        // No match (Lua will log a message in CallbackPost())
        0;
      ) : CandidatesCount == 1 ? (
        // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
        InputTextCallback_DeleteChars(word_start, word_end - word_start);
        InputTextCallback_InsertChars(CursorPos, Candidates);
        InputTextCallback_InsertChars(CursorPos, " ");
      ) : (
        // Multiple matches. Complete as much as we can...
        // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
        match_len = strlen(word);
        while(
          c = 0;
          all_candidates_matches = 1;
          i = 0;
          while(i < CandidatesCount && all_candidates_matches) (
            i == 0 ? (
              c = toupper(str_getchar(Candidates, match_len));
            ) : (c == 0 || c != toupper(str_getchar(Candidates + i, match_len))) ?
              all_candidates_matches = 0;
            i += 1;
          );
          all_candidates_matches ? match_len += 1 : 0;
        );

        match_len > 0 ? (
          candidate = #;
          strncpy(candidate, Candidates, match_len);
          InputTextCallback_DeleteChars(word_start, word_end - word_start);
          InputTextCallback_InsertChars(CursorPos, candidate);
        );

        // Lua will print the list of possible matches in CallbackPost()
      );
    );

    EventFlag == InputTextFlags_CallbackHistory ? (
      // Example of HISTORY
      prev_history_pos = HistoryPos;
      history_line = #;
      EventKey == Key_UpArrow ? (
        HistoryPos == 0
          ? HistoryPos = HistorySize
          : HistoryPos > 1 ? HistoryPos -= 1;
        strcpy(history_line, #HistoryPrev);
      );
      EventKey == Key_DownArrow ? (
        HistoryPos != 0 ? (
          HistoryPos += 1;
          HistoryPos > HistorySize ? HistoryPos = 0;
        );
        strcpy(history_line, #HistoryNext);
      );

      // A better implementation would preserve the data on the current input line along with cursor position.
      prev_history_pos != HistoryPos ? (
        InputTextCallback_DeleteChars(0, strlen(Buf));
        InputTextCallback_InsertChars(0, history_line);
      );
    );
    ]]),
  }
  ImGui.Attach(ctx, instance.callback)
  ImGui.Attach(ctx, instance.filter)
  self.__index = self

  local use_flags = {
    'InputTextFlags_CallbackCompletion',
    'InputTextFlags_CallbackHistory',
    'Key_UpArrow', 'Key_DownArrow',
  }
  for i, flag in ipairs(use_flags) do
    ImGui.Function_SetValue(instance.callback, flag, ImGui[flag]())
  end
  ImGui.Function_SetValue(instance.callback, 'CommandsCount', 0)

  setmetatable(instance, self)

  instance:ClearLog()
  instance:AddLog('Welcome to Dear ImGui!')

  -- "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
  instance:AddCommand('HELP')
  instance:AddCommand('HISTORY')
  instance:AddCommand('CLEAR')
  instance:AddCommand('CLASSIFY')

  return instance
end

function ExampleAppConsole:ClearLog()
  self.items = {}
end

function ExampleAppConsole:AddLog(fmt, ...)
  self.items[#self.items + 1] = fmt:format(...)
end

function ExampleAppConsole:Draw(title)
  ImGui.SetNextWindowSize(self.ctx, 520, 600, ImGui.Cond_FirstUseEver())
  local rv, open = ImGui.Begin(self.ctx, title, true)
  if not rv then return open end

  -- As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
  -- So e.g. IsItemHovered() will return true when hovering the title bar.
  -- Here we create a context menu only available from the title bar.
  if ImGui.BeginPopupContextItem(self.ctx) then
    if ImGui.MenuItem(self.ctx, 'Close Console') then
      open = false
    end
    ImGui.EndPopup(self.ctx)
  end

  ImGui.TextWrapped(self.ctx,
    'This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate \z
     implementation may want to store entries along with extra data such as timestamp, emitter, etc.')
  ImGui.TextWrapped(self.ctx, "Enter 'HELP' for help.")

  -- TODO: display items starting from the bottom

  if ImGui.SmallButton(self.ctx, 'Add Debug Text') then
    self:AddLog('%d some text', #self.items)
    self:AddLog("some more text")
    self:AddLog("display very important message here!")
  end
  ImGui.SameLine(self.ctx)
  if ImGui.SmallButton(self.ctx, 'Add Debug Error') then self:AddLog("[error] something went wrong") end
  ImGui.SameLine(self.ctx)
  if ImGui.SmallButton(self.ctx, 'Clear') then self:ClearLog() end
  ImGui.SameLine(self.ctx)
  local copy_to_clipboard = ImGui.SmallButton(self.ctx, 'Copy')
  --static float t = 0.0f; if (ImGui.GetTime() - t > 0.02f) { t = ImGui.GetTime(); AddLog("Spam %f", t); }

  ImGui.Separator(self.ctx)

  -- Options menu
  if ImGui.BeginPopup(self.ctx, 'Options') then
    rv, self.auto_scroll = ImGui.Checkbox(self.ctx, 'Auto-scroll', self.auto_scroll)
    ImGui.EndPopup(self.ctx)
  end

  -- Options, Filter
  if ImGui.Button(self.ctx, 'Options') then
    ImGui.OpenPopup(self.ctx, 'Options')
  end
  ImGui.SameLine(self.ctx)
  ImGui.TextFilter_Draw(self.filter, ctx, 'Filter ("incl,-excl") ("error")', 180)
  ImGui.Separator(self.ctx)

  -- Reserve enough left-over height for 1 separator + 1 input text
  local footer_height_to_reserve = select(2, ImGui.GetStyleVar(self.ctx, ImGui.StyleVar_ItemSpacing())) + ImGui.GetFrameHeightWithSpacing(self.ctx)
  if ImGui.BeginChild(self.ctx, 'ScrollingRegion', 0, -footer_height_to_reserve, false, ImGui.WindowFlags_HorizontalScrollbar()) then
    if ImGui.BeginPopupContextWindow(self.ctx) then
      if ImGui.Selectable(self.ctx, 'Clear') then self:ClearLog() end
      ImGui.EndPopup(self.ctx)
    end

    -- Display every line as a separate entry so we can change their color or add custom widgets.
    -- If you only want raw text you can use ImGui.TextUnformatted(log.begin(), log.end());
    -- NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
    -- to only process visible items. The clipper will automatically measure the height of your first item and then
    -- "seek" to display only items in the visible area.
    -- To use the clipper we can replace your standard loop:
    --      for (int i = 0; i < Items.Size; i++)
    --   With:
    --      ImGuiListClipper clipper;
    --      clipper.Begin(Items.Size);
    --      while (clipper.Step())
    --         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    -- - That your items are evenly spaced (same height)
    -- - That you have cheap random access to your elements (you can access them given their index,
    --   without processing all the ones before)
    -- You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
    -- We would need random-access on the post-filtered list.
    -- A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
    -- or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
    -- and appending newly elements as they are inserted. This is left as a task to the user until we can manage
    -- to improve this example code!
    -- If your items are of variable height:
    -- - Split them into same height items would be simpler and facilitate random-seeking into your list.
    -- - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
    ImGui.PushStyleVar(self.ctx, ImGui.StyleVar_ItemSpacing(), 4, 1) -- Tighten spacing
    if copy_to_clipboard then
      ImGui.LogToClipboard(self.ctx)
    end
    for i, item in ipairs(self.items) do
      if ImGui.TextFilter_PassFilter(self.filter, item) then
        -- Normally you would store more information in your item than just a string.
        -- (e.g. make Items[] an array of structure, store color/type etc.)
        local color
        if item:find("[error]", 1, true) then color = 0xFF6666FF
        elseif item:sub(1, 2) == '# '   then color = 0xFFCC99FF end
        if color then
          ImGui.PushStyleColor(self.ctx, ImGui.Col_Text(), color)
        end
        ImGui.Text(self.ctx, item)
        if color then
          ImGui.PopStyleColor(self.ctx)
        end
      end
    end
    if copy_to_clipboard then
      ImGui.LogFinish(self.ctx)
    end

    -- Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
    -- Using a scrollbar or mouse-wheel will take away from the bottom edge.
    if self.scroll_to_bottom or (self.auto_scroll and ImGui.GetScrollY(self.ctx) >= ImGui.GetScrollMaxY(self.ctx)) then
      ImGui.SetScrollHereY(self.ctx, 1.0)
    end
    self.scroll_to_bottom = false

    ImGui.PopStyleVar(self.ctx)
    ImGui.EndChild(self.ctx)
  end
  ImGui.Separator(self.ctx)

  -- Command-line
  local reclaim_focus = false
  local input_text_flags = ImGui.InputTextFlags_EnterReturnsTrue()   |
                           ImGui.InputTextFlags_EscapeClearsAll()    |
                           ImGui.InputTextFlags_CallbackCompletion() |
                           ImGui.InputTextFlags_CallbackHistory()
  local rv
  self:CallbackPre()
  rv, self.input_buf = ImGui.InputText(self.ctx, 'Input', self.input_buf, input_text_flags, self.callback)
  self:CallbackPost()
  if rv then
    local s = self.input_buf:match('^ *(.-) *$')
    if #s > 0 then
      self:ExecCommand(s)
      self.input_buf = ''
    end
    reclaim_focus = true
  end
  -- self:AddLog('cursor: %d, selection: %d-%d',
  --   ImGui.Function_GetValue(self.callback, 'CursorPos'),
  --   ImGui.Function_GetValue(self.callback, 'SelectionStart'),
  --   ImGui.Function_GetValue(self.callback, 'SelectionEnd'))

  -- Auto-focus on window apparition
  ImGui.SetItemDefaultFocus(self.ctx)
  if reclaim_focus then
    ImGui.SetKeyboardFocusHere(self.ctx, -1) -- Auto focus previous widget
  end

  ImGui.End(self.ctx)
  return open
end

function ExampleAppConsole:stricmp(a, b)
  return a:upper() == b:upper()
end

function ExampleAppConsole:ExecCommand(command_line)
  self:AddLog('# %s\n', command_line)

  -- Insert into history. First find match and delete it so it can be pushed to the back.
  -- This isn't trying to be smart or optimal.
  self.history_pos = 0
  for i = #self.history, 1, -1 do
    if self:stricmp(self.history[i], command_line) then
      table.remove(self.history, i)
      break
    end
  end
  self.history[#self.history + 1] = command_line

  -- Process command
  if self:stricmp(command_line, 'CLEAR') then
    self:ClearLog()
  elseif self:stricmp(command_line, 'HELP') then
    self:AddLog('Commands:')
    local CommandsCount = ImGui.Function_GetValue(self.callback, 'CommandsCount')
    for i = 0, CommandsCount - 1 do
      ImGui.Function_SetValue(self.callback, 'command', i)
      self:AddLog('- %s\n', ImGui.Function_GetValue_String(self.callback, 'command'))
    end
  elseif self:stricmp(command_line, 'HISTORY') then
    local first = math.max(1, #self.history - 10)
    for i = first, #self.history do
      self:AddLog("%3d: %s\n", i, self.history[i])
    end
  else
    self:AddLog("Unknown command: '%s'\n", command_line)
  end

  -- On command input, we scroll to bottom even if AutoScroll==false
  self.scroll_to_bottom = true
end

function ExampleAppConsole:AddCommand(name)
  ImGui.Function_SetValue_String(self.callback, 'CommandsCount', name)
  local CommandsCount = ImGui.Function_GetValue(self.callback, 'CommandsCount')
  ImGui.Function_SetValue(self.callback, 'CommandsCount', CommandsCount + 1)
end

function ExampleAppConsole:CallbackPre()
  -- Prepare callback data
  ImGui.Function_SetValue(self.callback, 'HistoryPos',  self.history_pos)
  ImGui.Function_SetValue(self.callback, 'HistorySize', #self.history)
  ImGui.Function_SetValue_String(self.callback, '#HistoryPrev',
    self.history[self.history_pos == 0 and #self.history or self.history_pos - 1])
  ImGui.Function_SetValue_String(self.callback, '#HistoryNext',
    self.history[self.history_pos + 1])
end

function ExampleAppConsole:CallbackPost()
  -- Callback post-processing
  self.history_pos = ImGui.Function_GetValue(self.callback, 'HistoryPos')

  if ImGui.Function_GetValue(self.callback, 'EventFlag') == ImGui.InputTextFlags_CallbackCompletion() then
    local CandidatesCount = ImGui.Function_GetValue(self.callback, 'CandidatesCount')
    if CandidatesCount == 0 then
      self:AddLog('No match for "%s"!\n', ImGui.Function_GetValue_String(self.callback, 'word'))
    elseif CandidatesCount > 1 then
      -- List matches
      local Candidates = ImGui.Function_GetValue(self.callback, 'Candidates')
      self:AddLog('Possible matches:\n')
      for i = 0, CandidatesCount - 1 do
        ImGui.Function_SetValue(self.callback, 'candidate', Candidates + i)
        self:AddLog('- %s\n', ImGui.Function_GetValue_String(self.callback, 'candidate'))
      end
    end
    ImGui.Function_SetValue(self.callback, 'EventFlag', 0)
  end
end

function demo.ShowExampleAppConsole()
  if not app.console then
    app.console = ExampleAppConsole:new(ctx)
  end

  return app.console:Draw('Example: Console')
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Debug Log / ShowExampleAppLog()
-------------------------------------------------------------------------------

-- Usage:
--   local my_log = ExampleAppLog:new(ctx)
--   my_log:add_log('Hello %d world\n', 123)
--   my_log:draw('title')

local ExampleAppLog = {}
function ExampleAppLog:new(ctx)
  local instance = {
    ctx          = ctx,
    lines        = {},
    filter       = ImGui.CreateTextFilter(),
    auto_scroll  = true, -- Keep scrolling if already at the bottom.
  }
  ImGui.Attach(ctx, instance.filter)
  self.__index = self
  return setmetatable(instance, self)
end

function ExampleAppLog.Clear(self)
  self.lines = {}
end

function ExampleAppLog.AddLog(self, fmt, ...)
  local text = fmt:format(...)
  for line in text:gmatch("[^\r\n]+") do
    table.insert(self.lines, line)
  end
end

function ExampleAppLog.Draw(self, title, p_open)
  local rv,p_open = ImGui.Begin(self.ctx, title, p_open)
  if not rv then return p_open end

  -- Options menu
  if ImGui.BeginPopup(self.ctx, 'Options') then
    rv,self.auto_scroll = ImGui.Checkbox(self.ctx, 'Auto-scroll', self.auto_scroll)
    ImGui.EndPopup(self.ctx)
  end

  -- Main window
  if ImGui.Button(self.ctx, 'Options') then
    ImGui.OpenPopup(self.ctx, 'Options')
  end
  ImGui.SameLine(self.ctx)
  local clear = ImGui.Button(self.ctx, 'Clear')
  ImGui.SameLine(self.ctx)
  local copy = ImGui.Button(self.ctx, 'Copy')
  ImGui.SameLine(self.ctx)
  ImGui.TextFilter_Draw(self.filter, ctx, 'Filter', -100.0)

  ImGui.Separator(self.ctx)
  if ImGui.BeginChild(self.ctx, 'scrolling', 0, 0, false, ImGui.WindowFlags_HorizontalScrollbar()) then
    if clear then
      self:Clear()
    end
    if copy then
      ImGui.LogToClipboard(self.ctx)
    end

    ImGui.PushStyleVar(self.ctx, ImGui.StyleVar_ItemSpacing(), 0, 0)
    if ImGui.TextFilter_IsActive(self.filter) then
      -- In this example we don't use the clipper when Filter is enabled.
      -- This is because we don't have a random access on the result on our filter.
      -- A real application processing logs with ten of thousands of entries may want to store the result of
      -- search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
      for line_no, line in ipairs(self.lines) do
        if ImGui.TextFilter_PassFilter(self.filter, line) then
          ImGui.Text(self.ctx, line)
        end
      end
    else
      -- The simplest and easy way to display the entire buffer:
      --   ImGui.Text(text)
      -- And it'll just work. Text() has specialization for large blob of text and will fast-forward
      -- to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
      -- within the visible area.
      -- If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
      -- on your side is recommended. Using ImGuiListClipper requires
      -- - A) random access into your data
      -- - B) items all being the  same height,
      -- both of which we can handle since we an array pointing to the beginning of each line of text.
      -- When using the filter (in the block of code above) we don't have random access into the data to display
      -- anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
      -- it possible (and would be recommended if you want to search through tens of thousands of entries).
      local clipper = ImGui.CreateListClipper(self.ctx)
      ImGui.ListClipper_Begin(clipper, #self.lines)
      while ImGui.ListClipper_Step(clipper) do
        local display_start, display_end = ImGui.ListClipper_GetDisplayRange(clipper)
        for line_no = display_start, display_end - 1 do
          ImGui.Text(self.ctx, self.lines[line_no + 1])
        end
      end
      ImGui.ListClipper_End(clipper)
    end
    ImGui.PopStyleVar(self.ctx)

    -- Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
    -- Using a scrollbar or mouse-wheel will take away from the bottom edge.
    if self.auto_scroll and ImGui.GetScrollY(self.ctx) >= ImGui.GetScrollMaxY(self.ctx) then
      ImGui.SetScrollHereY(self.ctx, 1.0)
    end

    ImGui.EndChild(self.ctx)
  end

  ImGui.End(self.ctx)

  return p_open
end

-- Demonstrate creating a simple log window with basic filtering.
function demo.ShowExampleAppLog()
  if not app.log then
    app.log = ExampleAppLog:new(ctx)
    app.log.counter = 0
  end

  -- For the demo: add a debug button _BEFORE_ the normal log window contents
  -- We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
  -- Most of the contents of the window will be added by the log.Draw() call.
  ImGui.SetNextWindowSize(ctx, 500, 400, ImGui.Cond_FirstUseEver())
  local rv,open = ImGui.Begin(ctx, 'Example: Log', true)
  if not rv then return open end

  if ImGui.SmallButton(ctx, '[Debug] Add 5 entries') then
    local categories = { 'info', 'warn', 'error' }
    local words = { 'Bumfuzzled', 'Cattywampus', 'Snickersnee',
                    'Abibliophobia', 'Absquatulate', 'Nincompoop',
                    'Pauciloquent' }
    for n = 0, 5 - 1 do
      local category = categories[(app.log.counter % #categories) + 1]
      local word = words[(app.log.counter % #words) + 1]
      app.log:AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
          ImGui.GetFrameCount(ctx), category, ImGui.GetTime(ctx), word)
      app.log.counter = app.log.counter + 1
    end
  end
  ImGui.End(ctx)

  -- Actually call in the regular Log helper (which will Begin() into the same window as we just did)
  app.log:Draw('Example: Log')

  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
-------------------------------------------------------------------------------

-- Demonstrate create a window with multiple child windows.
function demo.ShowExampleAppLayout()
  if not app.layout then
    app.layout = {
      selected = 0,
    }
  end

  ImGui.SetNextWindowSize(ctx, 500, 440, ImGui.Cond_FirstUseEver())
  local rv,open = ImGui.Begin(ctx, 'Example: Simple layout', true, ImGui.WindowFlags_MenuBar())
  if not rv then return open end

  if ImGui.BeginMenuBar(ctx) then
    if ImGui.BeginMenu(ctx, 'File') then
      if ImGui.MenuItem(ctx, 'Close', 'Ctrl+W') then open = false end
      ImGui.EndMenu(ctx)
    end
    ImGui.EndMenuBar(ctx)
  end

  -- Left
  if ImGui.BeginChild(ctx, 'left pane', 150, 0, true) then
    for i = 0, 100 - 1 do
      if ImGui.Selectable(ctx, ('MyObject %d'):format(i), app.layout.selected == i) then
        app.layout.selected = i
      end
    end
    ImGui.EndChild(ctx)
  end
  ImGui.SameLine(ctx)

  -- Right
  ImGui.BeginGroup(ctx)
  if ImGui.BeginChild(ctx, 'item view', 0, -ImGui.GetFrameHeightWithSpacing(ctx)) then -- Leave room for 1 line below us
    ImGui.Text(ctx, ('MyObject: %d'):format(app.layout.selected))
    ImGui.Separator(ctx)
    if ImGui.BeginTabBar(ctx, '##Tabs', ImGui.TabBarFlags_None()) then
      if ImGui.BeginTabItem(ctx, 'Description') then
        ImGui.TextWrapped(ctx, 'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ')
        ImGui.EndTabItem(ctx)
      end
      if ImGui.BeginTabItem(ctx, 'Details') then
        ImGui.Text(ctx, 'ID: 0123456789')
        ImGui.EndTabItem(ctx)
      end
      ImGui.EndTabBar(ctx)
    end
    ImGui.EndChild(ctx)
  end
  if ImGui.Button(ctx, 'Revert') then end
  ImGui.SameLine(ctx)
  if ImGui.Button(ctx, 'Save') then end
  ImGui.EndGroup(ctx)

  ImGui.End(ctx)
  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
-------------------------------------------------------------------------------

function demo.ShowPlaceholderObject(prefix, uid)
  local rv

  -- Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
  ImGui.PushID(ctx, uid)

  -- Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add vertical spacing to make the tree lines equal high.
  ImGui.TableNextRow(ctx)
  ImGui.TableSetColumnIndex(ctx, 0)
  ImGui.AlignTextToFramePadding(ctx)
  local node_open = ImGui.TreeNodeEx(ctx, 'Object', ('%s_%u'):format(prefix, uid))
  ImGui.TableSetColumnIndex(ctx, 1)
  ImGui.Text(ctx, 'my sailor is rich')

  if node_open then
    for i = 0, #app.property_editor.placeholder_members - 1 do
      ImGui.PushID(ctx, i) -- Use field index as identifier.
      if i < 2 then
        demo.ShowPlaceholderObject('Child', 424242)
      else
        -- Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
        ImGui.TableNextRow(ctx)
        ImGui.TableSetColumnIndex(ctx, 0)
        ImGui.AlignTextToFramePadding(ctx)
        local flags = ImGui.TreeNodeFlags_Leaf()             |
                      ImGui.TreeNodeFlags_NoTreePushOnOpen() |
                      ImGui.TreeNodeFlags_Bullet()
        ImGui.TreeNodeEx(ctx, 'Field', ('Field_%d'):format(i), flags)

        ImGui.TableSetColumnIndex(ctx, 1)
        ImGui.SetNextItemWidth(ctx, -FLT_MIN)
        if i >= 5 then
          rv,app.property_editor.placeholder_members[i] =
            ImGui.InputDouble(ctx, '##value', app.property_editor.placeholder_members[i], 1.0)
        else
          rv,app.property_editor.placeholder_members[i] =
            ImGui.DragDouble(ctx, '##value', app.property_editor.placeholder_members[i], 0.01)
        end
      end
      ImGui.PopID(ctx)
    end
    ImGui.TreePop(ctx)
  end
  ImGui.PopID(ctx)
end

-- Demonstrate create a simple property editor.
function demo.ShowExampleAppPropertyEditor()
  if not app.property_editor then
    app.property_editor = {
      placeholder_members = { 0.0, 0.0, 1.0, 3.1416, 100.0, 999.0, 0.0, 0.0 },
    }
  end

  ImGui.SetNextWindowSize(ctx, 430, 450, ImGui.Cond_FirstUseEver())
  local rv,open = ImGui.Begin(ctx, 'Example: Property editor', true)
  if not rv then return open end

  demo.HelpMarker(
    'This example shows how you may implement a property editor using two columns.\n\z
     All objects/fields data are dummies here.\n\z
     Remember that in many simple cases, you can use ImGui.SameLine(xxx) to position\n\z
     your cursor horizontally instead of using the Columns() API.')

  ImGui.PushStyleVar(ctx, ImGui.StyleVar_FramePadding(), 2, 2)
  if ImGui.BeginTable(ctx, 'split', 2, ImGui.TableFlags_BordersOuter() | ImGui.TableFlags_Resizable()) then
    -- Iterate placeholder objects (all the same data)
    for obj_i = 0, 4 - 1 do
      demo.ShowPlaceholderObject('Object', obj_i)
      -- ImGui.Separator(ctx)
    end
    ImGui.EndTable(ctx)
  end
  ImGui.PopStyleVar(ctx)
  ImGui.End(ctx)
  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Long Text / ShowExampleAppLongText()
-------------------------------------------------------------------------------

-- Demonstrate/test rendering huge amount of text, and the incidence of clipping.
function demo.ShowExampleAppLongText()
  if not app.long_text then
    app.long_text = {
      test_type = 0,
      log       = '',
      lines     = 0,
    }
  end

  ImGui.SetNextWindowSize(ctx, 520, 600, ImGui.Cond_FirstUseEver())
  local rv, open = ImGui.Begin(ctx, 'Example: Long text display', true)
  if not rv then return open end

  ImGui.Text(ctx, 'Printing unusually long amount of text.')
  rv,app.long_text.test_type = ImGui.Combo(ctx, 'Test type', app.long_text.test_type,
    'Single call to Text()\0\z
     Multiple calls to Text(), clipped\0\z
     Multiple calls to Text(), not clipped (slow)\0')
  ImGui.Text(ctx, ('Buffer contents: %d lines, %d bytes'):format(app.long_text.lines, app.long_text.log:len()))
  if ImGui.Button(ctx, 'Clear') then app.long_text.log = ''; app.long_text.lines = 0 end
  ImGui.SameLine(ctx)
  if ImGui.Button(ctx, 'Add 1000 lines') then
    local newLines = ''
    for i = 0, 1000 - 1 do
      newLines = newLines .. ('%i The quick brown fox jumps over the lazy dog\n'):format(app.long_text.lines + i)
    end
    app.long_text.log = app.long_text.log .. newLines
    app.long_text.lines = app.long_text.lines + 1000
  end

  if ImGui.BeginChild(ctx, 'Log') then
    if app.long_text.test_type == 0 then
      -- Single call to TextUnformatted() with a big buffer
      ImGui.Text(ctx, app.long_text.log)
    elseif app.long_text.test_type == 1 then
      -- Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGui_ListClipper helper.
      ImGui.PushStyleVar(ctx, ImGui.StyleVar_ItemSpacing(), 0, 0)
      local clipper = ImGui.CreateListClipper(ctx)
      ImGui.ListClipper_Begin(clipper, app.long_text.lines)
      while ImGui.ListClipper_Step(clipper) do
        local display_start, display_end = ImGui.ListClipper_GetDisplayRange(clipper)
        for i = display_start, display_end - 1 do
          ImGui.Text(ctx, ('%i The quick brown fox jumps over the lazy dog'):format(i))
        end
      end
      ImGui.PopStyleVar(ctx)
    elseif app.long_text.test_type == 2 then
      -- Multiple calls to Text(), not clipped (slow)
      ImGui.PushStyleVar(ctx, ImGui.StyleVar_ItemSpacing(), 0, 0)
      for i = 0, app.long_text.lines do
        ImGui.Text(ctx, ('%i The quick brown fox jumps over the lazy dog'):format(i))
      end
      ImGui.PopStyleVar(ctx)
    end
    ImGui.EndChild(ctx)
  end

  ImGui.End(ctx)

  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
-------------------------------------------------------------------------------

-- Demonstrate creating a window which gets auto-resized according to its content.
function demo.ShowExampleAppAutoResize()
  if not app.auto_resize then
    app.auto_resize = {
      lines = 10,
    }
  end

  local rv,open = ImGui.Begin(ctx, 'Example: Auto-resizing window', true, ImGui.WindowFlags_AlwaysAutoResize())
  if not rv then return open end

  ImGui.Text(ctx,
    "Window will resize every-frame to the size of its content.\n\z
     Note that you probably don't want to query the window size to\n\z
     output your content because that would create a feedback loop.")
  rv,app.auto_resize.lines = ImGui.SliderInt(ctx, 'Number of lines', app.auto_resize.lines, 1, 20)
  for i = 1, app.auto_resize.lines do
    ImGui.Text(ctx, ('%sThis is line %d'):format(('\x20'):rep(i * 4), i)) -- Pad with space to extend size horizontally
  end
  ImGui.End(ctx)
  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
-------------------------------------------------------------------------------

-- Demonstrate creating a window with custom resize constraints.
-- Note that size constraints currently don't work on a docked window.
function demo.ShowExampleAppConstrainedResize()
  if not app.constrained_resize then
    app.constrained_resize = {
      auto_resize    = false,
      window_padding = true,
      type           = 5,
      display_lines  = 10,
    }
    -- Helper functions to demonstrate programmatic constraints
    -- FIXME: This doesn't take account of decoration size (e.g. title bar), library should make this easier.
    app.constrained_resize.aspect_ratio = ImGui.CreateFunctionFromEEL([[
      DesiredSize.x = max(DesiredSize.x, DesiredSize.y);
      DesiredSize.y = floor(DesiredSize.x / aspect_ratio);
    ]])
    app.constrained_resize.square       = ImGui.CreateFunctionFromEEL([[
      DesiredSize.x = DesiredSize.y = max(DesiredSize.x, DesiredSize.y);
    ]])
    app.constrained_resize.step         = ImGui.CreateFunctionFromEEL([[
      DesiredSize.x = floor(DesiredSize.x / fixed_step + 0.5) * fixed_step;
      DesiredSize.y = floor(DesiredSize.y / fixed_step + 0.5) * fixed_step;
    ]])
    ImGui.Attach(ctx, app.constrained_resize.aspect_ratio)
    ImGui.Attach(ctx, app.constrained_resize.square)
    ImGui.Attach(ctx, app.constrained_resize.step)
  end

  -- Submit constraint
  ImGui.Function_SetValue(app.constrained_resize.aspect_ratio, 'aspect_ratio', 16 / 9)
  ImGui.Function_SetValue(app.constrained_resize.step,         'fixed_step',   100)
  if app.constrained_resize.type == 0 then ImGui.SetNextWindowSizeConstraints(ctx, 100, 100,     500, 500)     end -- Between 100x100 and 500x500
  if app.constrained_resize.type == 1 then ImGui.SetNextWindowSizeConstraints(ctx, 100, 100, FLT_MAX, FLT_MAX) end -- Width > 100, Height > 100
  if app.constrained_resize.type == 2 then ImGui.SetNextWindowSizeConstraints(ctx,  -1,   0,      -1, FLT_MAX) end -- Vertical only
  if app.constrained_resize.type == 3 then ImGui.SetNextWindowSizeConstraints(ctx,   0,  -1, FLT_MAX, -1)      end -- Horizontal only
  if app.constrained_resize.type == 4 then ImGui.SetNextWindowSizeConstraints(ctx, 400,  -1,     500, -1)      end -- Width Between and 400 and 500
  if app.constrained_resize.type == 5 then ImGui.SetNextWindowSizeConstraints(ctx,   0,   0, FLT_MAX, FLT_MAX, app.constrained_resize.aspect_ratio) end -- Aspect ratio
  if app.constrained_resize.type == 6 then ImGui.SetNextWindowSizeConstraints(ctx,   0,   0, FLT_MAX, FLT_MAX, app.constrained_resize.square)       end -- Always Square
  if app.constrained_resize.type == 7 then ImGui.SetNextWindowSizeConstraints(ctx,   0,   0, FLT_MAX, FLT_MAX, app.constrained_resize.step)         end -- Fixed Step

  -- Submit window
  if not app.constrained_resize.window_padding then
    ImGui.PushStyleVar(ctx, ImGui.StyleVar_WindowPadding(), 0, 0)
  end
  local window_flags = app.constrained_resize.auto_resize and ImGui.WindowFlags_AlwaysAutoResize() or 0
  local visible,open = ImGui.Begin(ctx, 'Example: Constrained Resize', true, window_flags)
  if not app.constrained_resize.window_padding then
    ImGui.PopStyleVar(ctx)
  end
  if not visible then return open end

  if ImGui.IsKeyDown(ctx, ImGui.Mod_Shift()) then
    -- Display a dummy viewport (in your real app you would likely use ImageButton() to display a texture.
    local avail_size_w, avail_size_h = ImGui.GetContentRegionAvail(ctx)
    local pos_x, pos_y = ImGui.GetCursorScreenPos(ctx)
    ImGui.ColorButton(ctx, 'viewport', 0x7f337fff, ImGui.ColorEditFlags_NoTooltip() | ImGui.ColorEditFlags_NoDragDrop(), avail_size_w, avail_size_h)
    ImGui.SetCursorScreenPos(ctx, pos_x + 10, pos_y + 10)
    ImGui.Text(ctx, ('%.2f x %.2f'):format(avail_size_w, avail_size_h))
  else
    ImGui.Text(ctx, '(Hold SHIFT to display a dummy viewport)')
    if ImGui.IsWindowDocked(ctx) then
      ImGui.Text(ctx, "Warning: Sizing Constraints won't work if the window is docked!")
    end
    if ImGui.Button(ctx, 'Set 200x200') then ImGui.SetWindowSize(ctx, 200, 200) end ImGui.SameLine(ctx)
    if ImGui.Button(ctx, 'Set 500x500') then ImGui.SetWindowSize(ctx, 500, 500) end ImGui.SameLine(ctx)
    if ImGui.Button(ctx, 'Set 800x200') then ImGui.SetWindowSize(ctx, 800, 200) end
    ImGui.SetNextItemWidth(ctx, ImGui.GetFontSize(ctx) * 20)
    rv,app.constrained_resize.type = ImGui.Combo(ctx, 'Constraint', app.constrained_resize.type,
      'Between 100x100 and 500x500\0\z
      At least 100x100\0\z
      Resize vertical only\0\z
      Resize horizontal only\0\z
      Width Between 400 and 500\0\z
      Custom: Aspect Ratio 16:9\0\z
      Custom: Always Square\0\z
      Custom: Fixed Steps (100)\0')
    ImGui.SetNextItemWidth(ctx, ImGui.GetFontSize(ctx) * 20)
    rv,app.constrained_resize.display_lines = ImGui.DragInt(ctx, 'Lines', app.constrained_resize.display_lines, 0.2, 1, 100)
    rv,app.constrained_resize.auto_resize = ImGui.Checkbox(ctx, 'Auto-resize', app.constrained_resize.auto_resize)
    rv,app.constrained_resize.window_padding = ImGui.Checkbox(ctx, 'Window padding', app.constrained_resize.window_padding)
    for i = 1, app.constrained_resize.display_lines do
      ImGui.Text(ctx, ('%sHello, sailor! Making this line long enough for the example.'):format((' '):rep(i * 4)))
    end
  end
  ImGui.End(ctx)

  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
-------------------------------------------------------------------------------

-- Demonstrate creating a simple static window with no decoration
-- + a context-menu to choose which corner of the screen to use.
function demo.ShowExampleAppSimpleOverlay()
  if not app.simple_overlay then
    app.simple_overlay = {
      location = 0,
    }
  end

  local window_flags = ImGui.WindowFlags_NoDecoration()       |
                       ImGui.WindowFlags_NoDocking()          |
                       ImGui.WindowFlags_AlwaysAutoResize()   |
                       ImGui.WindowFlags_NoSavedSettings()    |
                       ImGui.WindowFlags_NoFocusOnAppearing() |
                       ImGui.WindowFlags_NoNav()

  if app.simple_overlay.location >= 0 then
    local PAD = 10.0
    local viewport = ImGui.GetMainViewport(ctx)
    local work_pos_x, work_pos_y = ImGui.Viewport_GetWorkPos(viewport) -- Use work area to avoid menu-bar/task-bar, if any!
    local work_size_w, work_size_h = ImGui.Viewport_GetWorkSize(viewport)
    local window_pos_x, window_pos_y, window_pos_pivot_x, window_pos_pivot_y
    window_pos_x = app.simple_overlay.location & 1 ~= 0 and work_pos_x + work_size_w - PAD or work_pos_x + PAD
    window_pos_y = app.simple_overlay.location & 2 ~= 0 and work_pos_y + work_size_h - PAD or work_pos_y + PAD
    window_pos_pivot_x = app.simple_overlay.location & 1 ~= 0 and 1.0 or 0.0
    window_pos_pivot_y = app.simple_overlay.location & 2 ~= 0 and 1.0 or 0.0
    ImGui.SetNextWindowPos(ctx, window_pos_x, window_pos_y, ImGui.Cond_Always(), window_pos_pivot_x, window_pos_pivot_y)
    -- ImGui::SetNextWindowViewport(viewport->ID) TODO?
    window_flags = window_flags | ImGui.WindowFlags_NoMove()
  elseif app.simple_overlay.location == -2 then
    -- Center window
    local center_x, center_y = ImGui.Viewport_GetCenter(ImGui.GetMainViewport(ctx))
    ImGui.SetNextWindowPos(ctx, center_x, center_y, ImGui.Cond_Always(), 0.5, 0.5)
    window_flags = window_flags | ImGui.WindowFlags_NoMove()
  end

  ImGui.SetNextWindowBgAlpha(ctx, 0.35) -- Transparent background

  local rv,open = ImGui.Begin(ctx, 'Example: Simple overlay', true, window_flags)
  if not rv then return open end

  ImGui.Text(ctx, 'Simple overlay\n(right-click to change position)')
  ImGui.Separator(ctx)
  if ImGui.IsMousePosValid(ctx) then
    ImGui.Text(ctx, ('Mouse Position: (%.1f,%.1f)'):format(ImGui.GetMousePos(ctx)))
  else
    ImGui.Text(ctx, 'Mouse Position: <invalid>')
  end
  if ImGui.BeginPopupContextWindow(ctx) then
    if ImGui.MenuItem(ctx, 'Custom',       nil, app.simple_overlay.location == -1) then app.simple_overlay.location = -1 end
    if ImGui.MenuItem(ctx, 'Center',       nil, app.simple_overlay.location == -2) then app.simple_overlay.location = -2 end
    if ImGui.MenuItem(ctx, 'Top-left',     nil, app.simple_overlay.location ==  0) then app.simple_overlay.location =  0 end
    if ImGui.MenuItem(ctx, 'Top-right',    nil, app.simple_overlay.location ==  1) then app.simple_overlay.location =  1 end
    if ImGui.MenuItem(ctx, 'Bottom-left',  nil, app.simple_overlay.location ==  2) then app.simple_overlay.location =  2 end
    if ImGui.MenuItem(ctx, 'Bottom-right', nil, app.simple_overlay.location ==  3) then app.simple_overlay.location =  3 end
    if ImGui.MenuItem(ctx, 'Close') then open = false end
    ImGui.EndPopup(ctx)
  end
  ImGui.End(ctx)

  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
-------------------------------------------------------------------------------

-- Demonstrate creating a window covering the entire screen/viewport
function demo.ShowExampleAppFullscreen()
  if not app.fullscreen then
    app.fullscreen = {
      use_work_area = true,
      flags = ImGui.WindowFlags_NoDecoration() |
              ImGui.WindowFlags_NoMove()       |
              ImGui.WindowFlags_NoSavedSettings(),
    }
  end

  -- We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
  -- Based on your use case you may want one or the other.
  local viewport = ImGui.GetMainViewport(ctx)
  local getViewportPos  = app.fullscreen.use_work_area and ImGui.Viewport_GetWorkPos or ImGui.Viewport_GetPos
  local getViewportSize = app.fullscreen.use_work_area and ImGui.Viewport_GetWorkSize or ImGui.Viewport_GetSize
  ImGui.SetNextWindowPos(ctx, getViewportPos(viewport))
  ImGui.SetNextWindowSize(ctx, getViewportSize(viewport))

  local rv,open = ImGui.Begin(ctx, 'Example: Fullscreen window', true, app.fullscreen.flags)
  if not rv then return open end

  rv,app.fullscreen.use_work_area = ImGui.Checkbox(ctx, 'Use work area instead of main area', app.fullscreen.use_work_area)
  ImGui.SameLine(ctx)
  demo.HelpMarker('Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the difference.')

  rv,app.fullscreen.flags = ImGui.CheckboxFlags(ctx, 'ImGuiWindowFlags_NoBackground', app.fullscreen.flags, ImGui.WindowFlags_NoBackground())
  rv,app.fullscreen.flags = ImGui.CheckboxFlags(ctx, 'ImGuiWindowFlags_NoDecoration', app.fullscreen.flags, ImGui.WindowFlags_NoDecoration())
  ImGui.Indent(ctx)
  rv,app.fullscreen.flags = ImGui.CheckboxFlags(ctx, 'ImGuiWindowFlags_NoTitleBar', app.fullscreen.flags, ImGui.WindowFlags_NoTitleBar())
  rv,app.fullscreen.flags = ImGui.CheckboxFlags(ctx, 'ImGuiWindowFlags_NoCollapse', app.fullscreen.flags, ImGui.WindowFlags_NoCollapse())
  rv,app.fullscreen.flags = ImGui.CheckboxFlags(ctx, 'ImGuiWindowFlags_NoScrollbar', app.fullscreen.flags, ImGui.WindowFlags_NoScrollbar())
  ImGui.Unindent(ctx)

  if ImGui.Button(ctx, 'Close this window') then
    open = false
  end

  ImGui.End(ctx)

  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Manipulating window titles / ShowExampleAppWindowTitles()
-------------------------------------------------------------------------------

-- Demonstrate the use of "##" and "###" in identifiers to manipulate ID generation.
-- This applies to all regular items as well.
-- Read FAQ section "How can I have multiple widgets with the same label?" for details.
function demo.ShowExampleAppWindowTitles()
  local viewport = ImGui.GetMainViewport(ctx)
  local base_pos = {ImGui.Viewport_GetPos(viewport)}

  -- By default, Windows are uniquely identified by their title.
  -- You can use the "##" and "###" markers to manipulate the display/ID.

  -- Using "##" to display same title but have unique identifier.
  ImGui.SetNextWindowPos(ctx, base_pos[1] + 100, base_pos[2] + 100, ImGui.Cond_FirstUseEver())
  if ImGui.Begin(ctx, 'Same title as another window##1') then
    ImGui.Text(ctx, 'This is window 1.\nMy title is the same as window 2, but my identifier is unique.')
    ImGui.End(ctx)
  end

  ImGui.SetNextWindowPos(ctx, base_pos[1] + 100, base_pos[2] + 200, ImGui.Cond_FirstUseEver())
  if ImGui.Begin(ctx, 'Same title as another window##2') then
    ImGui.Text(ctx, 'This is window 2.\nMy title is the same as window 1, but my identifier is unique.')
    ImGui.End(ctx)
  end

  -- Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
  ImGui.SetNextWindowPos(ctx, base_pos[1] + 100, base_pos[2] + 300, ImGui.Cond_FirstUseEver())
  spinners = { '|', '/', '-', '\\' }
  local spinner = math.floor(ImGui.GetTime(ctx) / 0.25) & 3
  if ImGui.Begin(ctx, ('Animated title %s %d###AnimatedTitle'):format(spinners[spinner+1], ImGui.GetFrameCount(ctx))) then
    ImGui.Text(ctx, 'This window has a changing title.')
    ImGui.End(ctx)
  end
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
-------------------------------------------------------------------------------

-- Demonstrate using the low-level ImDrawList to draw custom shapes.
function demo.ShowExampleAppCustomRendering()
  if not app.rendering then
    app.rendering = {
      sz                         = 36.0,
      thickness                  = 3.0,
      ngon_sides                 = 6,
      circle_segments_override   = false,
      circle_segments_override_v = 12,
      curve_segments_override    = false,
      curve_segments_override_v  = 8,
      col                        = 0xffff66ff,

      points                     = {},
      scrolling                  = {0.0, 0.0},
      opt_enable_grid            = true,
      opt_enable_context_menu    = true,
      adding_line                = false,

      draw_bg                    = true,
      draw_fg                    = true,
    }
  end

  local rv,open = ImGui.Begin(ctx, 'Example: Custom rendering', true)
  if not rv then return open end

  if ImGui.BeginTabBar(ctx, '##TabBar') then
    if ImGui.BeginTabItem(ctx, 'Primitives') then
      ImGui.PushItemWidth(ctx, -ImGui.GetFontSize(ctx) * 15)
      local draw_list = ImGui.GetWindowDrawList(ctx)

      -- Draw gradients
      -- (note that those are currently exacerbating our sRGB/Linear issues)
      -- Calling ImGui.GetColor[Ex]() multiplies the given colors by the current Style Alpha
      ImGui.Text(ctx, 'Gradients')
      local gradient_size = {ImGui.CalcItemWidth(ctx), ImGui.GetFrameHeight(ctx)}

      local p0 = {ImGui.GetCursorScreenPos(ctx)}
      local p1 = {p0[1] + gradient_size[1], p0[2] + gradient_size[2]}
      local col_a = ImGui.GetColorEx(ctx, 0x000000FF)
      local col_b = ImGui.GetColorEx(ctx, 0xFFFFFFFF)
      ImGui.DrawList_AddRectFilledMultiColor(draw_list, p0[1], p0[2], p1[1], p1[2], col_a, col_b, col_b, col_a)
      ImGui.InvisibleButton(ctx, '##gradient1', gradient_size[1], gradient_size[2])

      local p0 = {ImGui.GetCursorScreenPos(ctx)}
      local p1 = {p0[1] + gradient_size[1], p0[2] + gradient_size[2]}
      local col_a = ImGui.GetColorEx(ctx, 0x00FF00FF)
      local col_b = ImGui.GetColorEx(ctx, 0xFF0000FF)
      ImGui.DrawList_AddRectFilledMultiColor(draw_list, p0[1], p0[2], p1[1], p1[2], col_a, col_b, col_b, col_a)
      ImGui.InvisibleButton(ctx, '##gradient2', gradient_size[1], gradient_size[2])

      -- Draw a bunch of primitives
      local item_inner_spacing_x = ImGui.GetStyleVar(ctx, ImGui.StyleVar_ItemInnerSpacing())
      ImGui.Text(ctx, 'All primitives')
      rv,app.rendering.sz = ImGui.DragDouble(ctx, 'Size', app.rendering.sz, 0.2, 2.0, 100.0, '%.0f')
      rv,app.rendering.thickness = ImGui.DragDouble(ctx, 'Thickness', app.rendering.thickness, 0.05, 1.0, 8.0, '%.02f')
      rv,app.rendering.ngon_sides = ImGui.SliderInt(ctx, 'N-gon sides', app.rendering.ngon_sides, 3, 12)
      rv,app.rendering.circle_segments_override = ImGui.Checkbox(ctx, '##circlesegmentoverride', app.rendering.circle_segments_override)
      ImGui.SameLine(ctx, 0.0, item_inner_spacing_x)
      rv,app.rendering.circle_segments_override_v = ImGui.SliderInt(ctx, 'Circle segments override', app.rendering.circle_segments_override_v, 3, 40)
      if rv then app.rendering.circle_segments_override = true end
      rv,app.rendering.curve_segments_override = ImGui.Checkbox(ctx, '##curvessegmentoverride', app.rendering.curve_segments_override)
      ImGui.SameLine(ctx, 0.0, item_inner_spacing_x)
      rv,app.rendering.curve_segments_override_v = ImGui.SliderInt(ctx, 'Curves segments override', app.rendering.curve_segments_override_v, 3, 40)
      if rv then app.rendering.curve_segments_override = true end
      rv,app.rendering.col = ImGui.ColorEdit4(ctx, 'Color', app.rendering.col)

      local p = {ImGui.GetCursorScreenPos(ctx)}
      local spacing = 10.0
      local corners_tl_br = ImGui.DrawFlags_RoundCornersTopLeft() | ImGui.DrawFlags_RoundCornersBottomRight()
      local col = app.rendering.col
      local sz = app.rendering.sz
      local rounding = sz / 5.0
      local circle_segments = app.rendering.circle_segments_override and app.rendering.circle_segments_override_v or 0
      local curve_segments  = app.rendering.curve_segments_override  and app.rendering.curve_segments_override_v  or 0
      local x = p[1] + 4.0
      local y = p[2] + 4.0
      for n = 1, 2 do
        -- First line uses a thickness of 1.0, second line uses the configurable thickness
        local th = n == 1 and 1.0 or app.rendering.thickness
        ImGui.DrawList_AddNgon(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, app.rendering.ngon_sides, th); x = x + sz + spacing  -- N-gon
        ImGui.DrawList_AddCircle(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, circle_segments, th);        x = x + sz + spacing  -- Circle
        ImGui.DrawList_AddRect(draw_list, x, y, x + sz, y + sz, col, 0.0, ImGui.DrawFlags_None(), th);      x = x + sz + spacing    -- Square
        ImGui.DrawList_AddRect(draw_list, x, y, x + sz, y + sz, col, rounding, ImGui.DrawFlags_None(), th); x = x + sz + spacing    -- Square with all rounded corners
        ImGui.DrawList_AddRect(draw_list, x, y, x + sz, y + sz, col, rounding, corners_tl_br, th);            x = x + sz + spacing  -- Square with two rounded corners
        ImGui.DrawList_AddTriangle(draw_list, x+sz*0.5, y, x+sz, y+sz-0.5, x, y+sz-0.5, col, th);             x = x + sz + spacing  -- Triangle
        -- ImGui.DrawList_AddTriangle(draw_list, x+sz*0.2, y, x, y+sz-0.5, x+sz*0.4, y+sz-0.5, col, th);      x = x + sz*0.4 + spacing -- Thin triangle
        ImGui.DrawList_AddLine(draw_list, x, y, x + sz, y, col, th);                                          x = x + sz + spacing  -- Horizontal line (note: drawing a filled rectangle will be faster!)
        ImGui.DrawList_AddLine(draw_list, x, y, x, y + sz, col, th);                                          x = x +      spacing  -- Vertical line (note: drawing a filled rectangle will be faster!)
        ImGui.DrawList_AddLine(draw_list, x, y, x + sz, y + sz, col, th);                                     x = x + sz + spacing  -- Diagonal line

        -- Quadratic Bezier Curve (3 control points)
        local cp3 = {{x, y + sz * 0.6}, {x + sz * 0.5, y - sz * 0.4}, {x + sz, y + sz}}
        ImGui.DrawList_AddBezierQuadratic(draw_list,
          cp3[1][1], cp3[1][2], cp3[2][1], cp3[2][2], cp3[3][1], cp3[3][2],
          col, th, curve_segments)
        x = x + sz + spacing

        -- Cubic Bezier Curve (4 control points)
        local cp4 = {{x, y}, {x + sz * 1.3, y + sz * 0.3}, {x + sz - sz * 1.3, y + sz - sz * 0.3}, {x + sz, y + sz}}
        ImGui.DrawList_AddBezierCubic(draw_list,
          cp4[1][1], cp4[1][2], cp4[2][1], cp4[2][2], cp4[3][1], cp4[3][2], cp4[4][1], cp4[4][2],
          col, th, curve_segments)

        x = p[1] + 4
        y = y + sz + spacing
      end
      ImGui.DrawList_AddNgonFilled(draw_list, x + sz * 0.5, y + sz * 0.5, sz*0.5, col, app.rendering.ngon_sides); x = x + sz + spacing  -- N-gon
      ImGui.DrawList_AddCircleFilled(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, circle_segments);            x = x + sz + spacing  -- Circle
      ImGui.DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, col);                                         x = x + sz + spacing  -- Square
      ImGui.DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, col, 10.0);                                   x = x + sz + spacing  -- Square with all rounded corners
      ImGui.DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, col, 10.0, corners_tl_br);                    x = x + sz + spacing  -- Square with two rounded corners
      ImGui.DrawList_AddTriangleFilled(draw_list, x+sz*0.5, y, x+sz, y+sz-0.5, x, y+sz-0.5, col);                 x = x + sz + spacing  -- Triangle
      -- ImGui.DrawList_AddTriangleFilled(draw_list, x+sz*0.2, y, x, y+sz-0.5, x+sz*0.4, y+sz-0.5, col);          x = x + sz*0.4 + spacing -- Thin triangle
      ImGui.DrawList_AddRectFilled(draw_list, x, y, x + sz, y + app.rendering.thickness, col);                    x = x + sz + spacing  -- Horizontal line (faster than AddLine, but only handle integer thickness)
      ImGui.DrawList_AddRectFilled(draw_list, x, y, x + app.rendering.thickness, y + sz, col);                    x = x + spacing * 2.0 -- Vertical line (faster than AddLine, but only handle integer thickness)
      ImGui.DrawList_AddRectFilled(draw_list, x, y, x + 1, y + 1, col);                                           x = x + sz            -- Pixel (faster than AddLine)
      ImGui.DrawList_AddRectFilledMultiColor(draw_list, x, y, x + sz, y + sz, 0x000000ff, 0xff0000ff, 0xffff00ff, 0x00ff00ff)

      ImGui.Dummy(ctx, (sz + spacing) * 10.2, (sz + spacing) * 3.0)
      ImGui.PopItemWidth(ctx)
      ImGui.EndTabItem(ctx)
    end

    if ImGui.BeginTabItem(ctx, 'Canvas') then
      rv,app.rendering.opt_enable_grid =
        ImGui.Checkbox(ctx, 'Enable grid', app.rendering.opt_enable_grid)
      rv,app.rendering.opt_enable_context_menu =
        ImGui.Checkbox(ctx, 'Enable context menu', app.rendering.opt_enable_context_menu)
      ImGui.Text(ctx, 'Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.')

      -- Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
      -- Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
      -- To use a child window instead we could use, e.g:
      --   ImGui.PushStyleVar(ctx, ImGui.StyleVar_WindowPadding(), 0, 0) -- Disable padding
      --   ImGui.PushStyleColor(ctx, ImGui.Col_ChildBg(), 0x323232ff)    -- Set a background color
      --   if ImGui.BeginChild(ctx, 'canvas', 0.0, 0.0, true, ImGui.WindowFlags_NoMove()) then
      --     ImGui.PopStyleColor(ctx)
      --     ImGui.PopStyleVar(ctx)
      --     [...]
      --     ImGui.EndChild(ctx)
      --   end

      -- Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
      local canvas_p0 = {ImGui.GetCursorScreenPos(ctx)}      -- ImDrawList API uses screen coordinates!
      local canvas_sz = {ImGui.GetContentRegionAvail(ctx)}   -- Resize canvas to what's available
      if canvas_sz[1] < 50.0 then canvas_sz[1] = 50.0 end
      if canvas_sz[2] < 50.0 then canvas_sz[2] = 50.0 end
      local canvas_p1 = {canvas_p0[1] + canvas_sz[1], canvas_p0[2] + canvas_sz[2]}

      -- Draw border and background color
      local mouse_pos = {ImGui.GetMousePos(ctx)}
      local draw_list = ImGui.GetWindowDrawList(ctx)
      ImGui.DrawList_AddRectFilled(draw_list, canvas_p0[1], canvas_p0[2], canvas_p1[1], canvas_p1[2], 0x323232ff)
      ImGui.DrawList_AddRect(draw_list, canvas_p0[1], canvas_p0[2], canvas_p1[1], canvas_p1[2], 0xffffffff)

      -- This will catch our interactions
      ImGui.InvisibleButton(ctx, 'canvas', canvas_sz[1], canvas_sz[2], ImGui.ButtonFlags_MouseButtonLeft() | ImGui.ButtonFlags_MouseButtonRight())
      local is_hovered = ImGui.IsItemHovered(ctx) -- Hovered
      local is_active = ImGui.IsItemActive(ctx)   -- Held
      local origin = {canvas_p0[1] + app.rendering.scrolling[1], canvas_p0[2] + app.rendering.scrolling[2]} -- Lock scrolled origin
      local mouse_pos_in_canvas = {mouse_pos[1] - origin[1], mouse_pos[2] - origin[2]}

      -- Add first and second point
      if is_hovered and not app.rendering.adding_line and ImGui.IsMouseClicked(ctx, ImGui.MouseButton_Left()) then
        table.insert(app.rendering.points, mouse_pos_in_canvas)
        table.insert(app.rendering.points, mouse_pos_in_canvas)
        app.rendering.adding_line = true
      end
      if app.rendering.adding_line then
        app.rendering.points[#app.rendering.points] = mouse_pos_in_canvas
        if not ImGui.IsMouseDown(ctx, ImGui.MouseButton_Left()) then
          app.rendering.adding_line = false
        end
      end

      -- Pan (we use a zero mouse threshold when there's no context menu)
      -- You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
      local mouse_threshold_for_pan = app.rendering.opt_enable_context_menu and -1.0 or 0.0
      if is_active and ImGui.IsMouseDragging(ctx, ImGui.MouseButton_Right(), mouse_threshold_for_pan) then
        local mouse_delta = {ImGui.GetMouseDelta(ctx)}
        app.rendering.scrolling[1] = app.rendering.scrolling[1] + mouse_delta[1]
        app.rendering.scrolling[2] = app.rendering.scrolling[2] + mouse_delta[2]
      end

      local removeLastLine = function()
        table.remove(app.rendering.points)
        table.remove(app.rendering.points)
      end

      -- Context menu (under default mouse threshold)
      local drag_delta = {ImGui.GetMouseDragDelta(ctx, 0, 0, ImGui.MouseButton_Right())}
      if app.rendering.opt_enable_context_menu and drag_delta[1] == 0.0 and drag_delta[2] == 0.0 then
        ImGui.OpenPopupOnItemClick(ctx, 'context', ImGui.PopupFlags_MouseButtonRight())
      end
      if ImGui.BeginPopup(ctx, 'context') then
        if app.rendering.adding_line then
          removeLastLine()
          app.rendering.adding_line = false
        end
        if ImGui.MenuItem(ctx, 'Remove one', nil, false, #app.rendering.points > 0) then removeLastLine() end
        if ImGui.MenuItem(ctx, 'Remove all', nil, false, #app.rendering.points > 0) then app.rendering.points = {} end
        ImGui.EndPopup(ctx)
      end

      -- Draw grid + all lines in the canvas
      ImGui.DrawList_PushClipRect(draw_list, canvas_p0[1], canvas_p0[2], canvas_p1[1], canvas_p1[2], true)
      if app.rendering.opt_enable_grid then
        local GRID_STEP = 64.0
        for x = math.fmod(app.rendering.scrolling[1], GRID_STEP), canvas_sz[1], GRID_STEP do
          ImGui.DrawList_AddLine(draw_list, canvas_p0[1] + x, canvas_p0[2], canvas_p0[1] + x, canvas_p1[2], 0xc8c8c828)
        end
        for y = math.fmod(app.rendering.scrolling[2], GRID_STEP), canvas_sz[2], GRID_STEP do
          ImGui.DrawList_AddLine(draw_list, canvas_p0[1], canvas_p0[2] + y, canvas_p1[1], canvas_p0[2] + y, 0xc8c8c828)
        end
      end
      for n = 1, #app.rendering.points, 2 do
        ImGui.DrawList_AddLine(draw_list,
          origin[1] + app.rendering.points[n  ][1], origin[2] + app.rendering.points[n  ][2],
          origin[1] + app.rendering.points[n+1][1], origin[2] + app.rendering.points[n+1][2],
          0xffff00ff, 2.0)
      end
      ImGui.DrawList_PopClipRect(draw_list)

      ImGui.EndTabItem(ctx)
    end

    if ImGui.BeginTabItem(ctx, 'BG/FG draw lists') then
      rv,app.rendering.draw_bg = ImGui.Checkbox(ctx, 'Draw in Background draw list', app.rendering.draw_bg)
      ImGui.SameLine(ctx); demo.HelpMarker('The Background draw list will be rendered below every Dear ImGui windows.')
      rv,app.rendering.draw_fg = ImGui.Checkbox(ctx, 'Draw in Foreground draw list', app.rendering.draw_fg)
      ImGui.SameLine(ctx); demo.HelpMarker('The Foreground draw list will be rendered over every Dear ImGui windows.')
      local window_pos = {ImGui.GetWindowPos(ctx)}
      local window_size = {ImGui.GetWindowSize(ctx)}
      local window_center = {window_pos[1] + window_size[1] * 0.5, window_pos[2] + window_size[2] * 0.5}
      if app.rendering.draw_bg then
        ImGui.DrawList_AddCircle(ImGui.GetBackgroundDrawList(ctx),
          window_center[1], window_center[2], window_size[1] * 0.6,
          0xFF0000c8, nil, 10 + 4)
      end
      if app.rendering.draw_fg then
        ImGui.DrawList_AddCircle(ImGui.GetForegroundDrawList(ctx),
          window_center[1], window_center[2], window_size[2] * 0.6,
          0x00FF00c8, nil, 10)
      end
      ImGui.EndTabItem(ctx)
    end

    ImGui.EndTabBar(ctx)
  end

  ImGui.End(ctx)
  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
-------------------------------------------------------------------------------

-- Demonstrate using DockSpace() to create an explicit docking node within an existing window.
-- Note that you dock windows into each others _without_ a dockspace, by just clicking on
-- a window title bar or tab and moving it.
-- DockSpace() and DockSpaceOverViewport() are only useful to construct a central docking
-- location for your application.
-- void ShowExampleAppDockSpace(bool* p_open)
-- {
--     // In 99% case you should be able to just call DockSpaceOverViewport() and ignore all the code below!
--     // In this specific demo, we are not using DockSpaceOverViewport() because:
--     // - we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
--     // - we allow the host window to have padding (when opt_padding == true)
--     // - we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport() in your code!)
--     // TL;DR; this demo is more complicated than what you would normally use.
--     // If we removed all the options we are showcasing, this demo would become:
--     //     void ShowExampleAppDockSpace()
--     //     {
--     //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
--     //     }
--
--     static bool opt_fullscreen = true;
--     static bool opt_padding = false;
--     static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
--
--     // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
--     // because it would be confusing to have two docking targets within each others.
--     ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
--     if (opt_fullscreen)
--     {
--         const ImGuiViewport* viewport = ImGui::GetMainViewport();
--         ImGui::SetNextWindowPos(viewport->WorkPos);
--         ImGui::SetNextWindowSize(viewport->WorkSize);
--         ImGui::SetNextWindowViewport(viewport->ID);
--         ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
--         ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
--         window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
--         window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
--     }
--     else
--     {
--         dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
--     }
--
--     // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
--     // and handle the pass-thru hole, so we ask Begin() to not render a background.
--     if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
--         window_flags |= ImGuiWindowFlags_NoBackground;
--
--     // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
--     // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
--     // all active windows docked into it will lose their parent and become undocked.
--     // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
--     // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
--     if (!opt_padding)
--         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
--     ImGui::Begin("DockSpace Demo", p_open, window_flags);
--     if (!opt_padding)
--         ImGui::PopStyleVar();
--
--     if (opt_fullscreen)
--         ImGui::PopStyleVar(2);
--
--     // DockSpace
--     ImGuiIO& io = ImGui::GetIO();
--     if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
--     {
--         ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
--         ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
--     }
--     else
--     {
--         ShowDockingDisabledMessage();
--     }
--
--     if (ImGui::BeginMenuBar())
--     {
--         if (ImGui::BeginMenu("Options"))
--         {
--             // Disabling fullscreen would allow the window to be moved to the front of other windows,
--             // which we can't undo at the moment without finer window depth/z control.
--             ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
--             ImGui::MenuItem("Padding", NULL, &opt_padding);
--             ImGui::Separator();
--
--             if (ImGui::MenuItem("Flag: NoSplit",                "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
--             if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
--             if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
--             if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
--             if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
--             ImGui::Separator();
--
--             if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
--                 *p_open = false;
--             ImGui::EndMenu();
--         }
--         HelpMarker(
--             "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
--             "- Drag from window title bar or their tab to dock/undock." "\n"
--             "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
--             "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
--             "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)" "\n"
--             "This demo app has nothing to do with enabling docking!" "\n\n"-
--             "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window. This is useful so you can decorate your main application window (e.g. with a menu bar)." "\n\n"
--             "ImGui::DockSpace() comes with one hard constraint: it needs to be submitted _before_ any window which may be docked into it. Therefore, if you use a dock spot as the central point of your application, you'll probably want it to be part of the very first window you are submitting to imgui every frame." "\n\n"
--             "(NB: because of this constraint, the implicit \"Debug\" window can not be docked into an explicit DockSpace() node, because that window is submitted as part of the NewFrame() call. An easy workaround is that you can create your own implicit \"Debug##2\" window after calling DockSpace() and leave it in the window stack for anyone to use.)"
--         );
--
--         ImGui::EndMenuBar();
--     }
--
--     ImGui::End();
-- }

-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
-- //-----------------------------------------------------------------------------
--
-- // Simplified structure to mimic a Document model
-- struct MyDocument
-- {
--     const char* Name;       // Document title
--     bool        Open;       // Set when open (we keep an array of all available documents to simplify demo code!)
--     bool        OpenPrev;   // Copy of Open from last update.
--     bool        Dirty;      // Set when the document has been modified
--     bool        WantClose;  // Set when the document
--     ImVec4      Color;      // An arbitrary variable associated to the document
--
--     MyDocument(const char* name, bool open = true, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
--     {
--         Name = name;
--         Open = OpenPrev = open;
--         Dirty = false;
--         WantClose = false;
--         Color = color;
--     }
--     void DoOpen()       { Open = true; }
--     void DoQueueClose() { WantClose = true; }
--     void DoForceClose() { Open = false; Dirty = false; }
--     void DoSave()       { Dirty = false; }
--
--     // Display placeholder contents for the Document
--     static void DisplayContents(MyDocument* doc)
--     {
--         ImGui.PushID(doc);
--         ImGui.Text("Document \"%s\"", doc->Name);
--         ImGui.PushStyleColor(ImGuiCol_Text, doc->Color);
--         ImGui.TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
--         ImGui.PopStyleColor();
--         if (ImGui.Button("Modify", ImVec2(100, 0)))
--             doc->Dirty = true;
--         ImGui.SameLine();
--         if (ImGui.Button("Save", ImVec2(100, 0)))
--             doc->DoSave();
--         ImGui.ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
--         ImGui.PopID();
--     }
--
--     // Display context menu for the Document
--     static void DisplayContextMenu(MyDocument* doc)
--     {
--         if (!ImGui.BeginPopupContextItem())
--             return;
--
--         char buf[256];
--         sprintf(buf, "Save %s", doc->Name);
--         if (ImGui.MenuItem(buf, "CTRL+S", false, doc->Open))
--             doc->DoSave();
--         if (ImGui.MenuItem("Close", "CTRL+W", false, doc->Open))
--             doc->DoQueueClose();
--         ImGui.EndPopup();
--     }
-- };
--
-- struct ExampleAppDocuments
-- {
--     ImVector<MyDocument> Documents;
--
--     ExampleAppDocuments()
--     {
--         Documents.push_back(MyDocument("Lettuce",             true,  ImVec4(0.4f, 0.8f, 0.4f, 1.0f)));
--         Documents.push_back(MyDocument("Eggplant",            true,  ImVec4(0.8f, 0.5f, 1.0f, 1.0f)));
--         Documents.push_back(MyDocument("Carrot",              true,  ImVec4(1.0f, 0.8f, 0.5f, 1.0f)));
--         Documents.push_back(MyDocument("Tomato",              false, ImVec4(1.0f, 0.3f, 0.4f, 1.0f)));
--         Documents.push_back(MyDocument("A Rather Long Title", false));
--         Documents.push_back(MyDocument("Some Document",       false));
--     }
-- };
--
-- // [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
-- // If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo,
-- // as opposed to clicking on the regular tab closing button) and stops being submitted, it will take a frame for
-- // the tab bar to notice its absence. During this frame there will be a gap in the tab bar, and if the tab that has
-- // disappeared was the selected one, the tab bar will report no selected tab during the frame. This will effectively
-- // give the impression of a flicker for one frame.
-- // We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
-- // Note that this completely optional, and only affect tab bars with the ImGuiTabBarFlags_Reorderable flag.
-- static void NotifyOfDocumentsClosedElsewhere(ExampleAppDocuments& app)
-- {
--     for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--     {
--         MyDocument* doc = &app.Documents[doc_n];
--         if (!doc->Open && doc->OpenPrev)
--             ImGui.SetTabItemClosed(doc->Name);
--         doc->OpenPrev = doc->Open;
--     }
-- }
--
-- void ShowExampleAppDocuments(bool* p_open)
-- {
--     static ExampleAppDocuments app;
--
--     // Options
--     enum Target
--     {
--         Target_None,
--         Target_Tab,                 // Create documents as local tab into a local tab bar
--         Target_DockSpaceAndWindow   // Create documents as regular windows, and create an embedded dockspace
--     };
--     static Target opt_target = Target_Tab;
--     static bool opt_reorderable = true;
--     static ImGuiTabBarFlags opt_fitting_flags = ImGuiTabBarFlags_FittingPolicyDefault_;
--
--     // When (opt_target == Target_DockSpaceAndWindow) there is the possibily that one of our child Document window (e.g. "Eggplant")
--     // that we emit gets docked into the same spot as the parent window ("Example: Documents").
--     // This would create a problematic feedback loop because selecting the "Eggplant" tab would make the "Example: Documents" tab
--     // not visible, which in turn would stop submitting the "Eggplant" window.
--     // We avoid this problem by submitting our documents window even if our parent window is not currently visible.
--     // Another solution may be to make the "Example: Documents" window use the ImGuiWindowFlags_NoDocking.
--
--     bool window_contents_visible = ImGui.Begin("Example: Documents", p_open, ImGuiWindowFlags_MenuBar);
--     if (!window_contents_visible && opt_target != Target_DockSpaceAndWindow)
--     {
--         ImGui.End();
--         return;
--     }
--
--     // Menu
--     if (ImGui.BeginMenuBar())
--     {
--         if (ImGui.BeginMenu("File"))
--         {
--             int open_count = 0;
--             for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--                 open_count += app.Documents[doc_n].Open ? 1 : 0;
--
--             if (ImGui.BeginMenu("Open", open_count < app.Documents.Size))
--             {
--                 for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--                 {
--                     MyDocument* doc = &app.Documents[doc_n];
--                     if (!doc->Open)
--                         if (ImGui.MenuItem(doc->Name))
--                             doc->DoOpen();
--                 }
--                 ImGui.EndMenu();
--             }
--             if (ImGui.MenuItem("Close All Documents", NULL, false, open_count > 0))
--                 for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--                     app.Documents[doc_n].DoQueueClose();
--             if (ImGui.MenuItem("Exit", "Ctrl+F4") && p_open)
--                 *p_open = false;
--             ImGui.EndMenu();
--         }
--         ImGui.EndMenuBar();
--     }
--
--     // [Debug] List documents with one checkbox for each
--     for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--     {
--         MyDocument* doc = &app.Documents[doc_n];
--         if (doc_n > 0)
--             ImGui.SameLine();
--         ImGui.PushID(doc);
--         if (ImGui.Checkbox(doc->Name, &doc->Open))
--             if (!doc->Open)
--                 doc->DoForceClose();
--         ImGui.PopID();
--     }
--     ImGui.PushItemWidth(ImGui.GetFontSize() * 12);
--     ImGui.Combo("Output", (int*)&opt_target, "None\0TabBar+Tabs\0DockSpace+Window\0");
--     ImGui.PopItemWidth();
--     bool redock_all = false;
--     if (opt_target == Target_Tab)                { ImGui.SameLine(); ImGui.Checkbox("Reorderable Tabs", &opt_reorderable); }
--     if (opt_target == Target_DockSpaceAndWindow) { ImGui.SameLine(); redock_all = ImGui.Button("Redock all"); }
--
--     ImGui.Separator();
--
--     // About the ImGuiWindowFlags_UnsavedDocument / ImGuiTabItemFlags_UnsavedDocument flags.
--     // They have multiple effects:
--     // - Display a dot next to the title.
--     // - Tab is selected when clicking the X close button.
--     // - Closure is not assumed (will wait for user to stop submitting the tab).
--     //   Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
--     //   We need to assume closure by default otherwise waiting for "lack of submission" on the next frame would leave an empty
--     //   hole for one-frame, both in the tab-bar and in tab-contents when closing a tab/window.
--     //   The rarely used SetTabItemClosed() function is a way to notify of programmatic closure to avoid the one-frame hole.
--
--     // Tabs
--     if (opt_target == Target_Tab)
--     {
--         ImGuiTabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? ImGuiTabBarFlags_Reorderable : 0);
--         if (ImGui.BeginTabBar("##tabs", tab_bar_flags))
--         {
--             if (opt_reorderable)
--                 NotifyOfDocumentsClosedElsewhere(app);
--
--             // [DEBUG] Stress tests
--             //if ((ImGui.GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
--             //if (ImGui.GetIO().KeyCtrl) ImGui.SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..
--
--             // Submit Tabs
--             for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--             {
--                 MyDocument* doc = &app.Documents[doc_n];
--                 if (!doc->Open)
--                     continue;
--
--                 ImGuiTabItemFlags tab_flags = (doc->Dirty ? ImGuiTabItemFlags_UnsavedDocument : 0);
--                 bool visible = ImGui.BeginTabItem(doc->Name, &doc->Open, tab_flags);
--
--                 // Cancel attempt to close when unsaved add to save queue so we can display a popup.
--                 if (!doc->Open && doc->Dirty)
--                 {
--                     doc->Open = true;
--                     doc->DoQueueClose();
--                 }
--
--                 MyDocument::DisplayContextMenu(doc);
--                 if (visible)
--                 {
--                     MyDocument::DisplayContents(doc);
--                     ImGui.EndTabItem();
--                 }
--             }
--
--             ImGui.EndTabBar();
--         }
--     }
--     else if (opt_target == Target_DockSpaceAndWindow)
--     {
--         if (ImGui.GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
--         {
--             NotifyOfDocumentsClosedElsewhere(app);
--
--             // Create a DockSpace node where any window can be docked
--             ImGuiID dockspace_id = ImGui.GetID("MyDockSpace");
--             ImGui.DockSpace(dockspace_id);
--
--             // Create Windows
--             for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--             {
--                 MyDocument* doc = &app.Documents[doc_n];
--                 if (!doc->Open)
--                     continue;
--
--                 ImGui.SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
--                 ImGuiWindowFlags window_flags = (doc->Dirty ? ImGuiWindowFlags_UnsavedDocument : 0);
--                 bool visible = ImGui.Begin(doc->Name, &doc->Open, window_flags);
--
--                 // Cancel attempt to close when unsaved add to save queue so we can display a popup.
--                 if (!doc->Open && doc->Dirty)
--                 {
--                     doc->Open = true;
--                     doc->DoQueueClose();
--                 }
--
--                 MyDocument::DisplayContextMenu(doc);
--                 if (visible)
--                     MyDocument::DisplayContents(doc);
--
--                 ImGui.End();
--             }
--         }
--         else
--         {
--             ShowDockingDisabledMessage();
--         }
--     }
--
--     // Early out other contents
--     if (!window_contents_visible)
--     {
--         ImGui.End();
--         return;
--     }
--
--     // Update closing queue
--     static ImVector<MyDocument*> close_queue;
--     if (close_queue.empty())
--     {
--         // Close queue is locked once we started a popup
--         for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--         {
--             MyDocument* doc = &app.Documents[doc_n];
--             if (doc->WantClose)
--             {
--                 doc->WantClose = false;
--                 close_queue.push_back(doc);
--             }
--         }
--     }
--
--     // Display closing confirmation UI
--     if (!close_queue.empty())
--     {
--         int close_queue_unsaved_documents = 0;
--         for (int n = 0; n < close_queue.Size; n++)
--             if (close_queue[n]->Dirty)
--                 close_queue_unsaved_documents++;
--
--         if (close_queue_unsaved_documents == 0)
--         {
--             // Close documents when all are unsaved
--             for (int n = 0; n < close_queue.Size; n++)
--                 close_queue[n]->DoForceClose();
--             close_queue.clear();
--         }
--         else
--         {
--             if (!ImGui.IsPopupOpen("Save?"))
--                 ImGui.OpenPopup("Save?");
--             if (ImGui.BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
--             {
--                 ImGui.Text("Save change to the following items?");
--                 float item_height = ImGui.GetTextLineHeightWithSpacing();
--                 if (ImGui.BeginChildFrame(ImGui.GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height)))
--                 {
--                     for (int n = 0; n < close_queue.Size; n++)
--                         if (close_queue[n]->Dirty)
--                             ImGui.Text("%s", close_queue[n]->Name);
--                     ImGui.EndChildFrame();
--                 }
--
--                 ImVec2 button_size(ImGui.GetFontSize() * 7.0f, 0.0f);
--                 if (ImGui.Button("Yes", button_size))
--                 {
--                     for (int n = 0; n < close_queue.Size; n++)
--                     {
--                         if (close_queue[n]->Dirty)
--                             close_queue[n]->DoSave();
--                         close_queue[n]->DoForceClose();
--                     }
--                     close_queue.clear();
--                     ImGui.CloseCurrentPopup();
--                 }
--                 ImGui.SameLine();
--                 if (ImGui.Button("No", button_size))
--                 {
--                     for (int n = 0; n < close_queue.Size; n++)
--                         close_queue[n]->DoForceClose();
--                     close_queue.clear();
--                     ImGui.CloseCurrentPopup();
--                 }
--                 ImGui.SameLine();
--                 if (ImGui.Button("Cancel", button_size))
--                 {
--                     close_queue.clear();
--                     ImGui.CloseCurrentPopup();
--                 }
--                 ImGui.EndPopup();
--             }
--         }
--     }
--
--     ImGui.End();
-- }

local public, public_functions = {}, {
  'ShowDemoWindow', 'ShowStyleEditor', 'PushStyle', 'PopStyle',
}
for _, fn in ipairs(public_functions) do
  public[fn] = function(user_ctx, ...)
    ctx = user_ctx
    demo[fn](...)
  end
end
return public
