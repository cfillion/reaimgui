-- Lua/ReaImGui port of Dear ImGui's C++ demo code (v1.82)

--[[
Index of this file:

// [SECTION] Forward Declarations, Helpers
// [SECTION] Demo Window / ShowDemoWindow()
// - sub section: ShowDemoWindowWidgets()
// - sub section: ShowDemoWindowLayout()
// - sub section: ShowDemoWindowPopups()
// - sub section: ShowDemoWindowTables()
// - sub section: ShowDemoWindowMisc()
// [SECTION] About Window / ShowAboutWindow()
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
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()

--]]

local r = reaper
local FLT_MIN, FLT_MAX = 1.17549e-38, 3.40282e+38
local IMGUI_VERSION, REAIMGUI_VERSION = r.ImGui_GetVersion()

-- Global data storage
demo = {
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
  no_bring_to_front = false,
}

config  = {}
widgets = {}
layout  = {}
popups  = {}
tables  = {}
misc    = {}
app     = {}

-- Hajime!

local ctx = r.ImGui_CreateContext('ImGui Demo', 590, 720)

function demo.loop()
  if r.ImGui_IsCloseRequested(ctx) then
    r.ImGui_DestroyContext(ctx)
    return
  end

  if demo.open then
    demo.open = demo.ShowDemoWindow()
  else
    r.ImGui_Text(ctx, 'Bye!')
  end

  r.defer(demo.loop)
end

-------------------------------------------------------------------------------
-- [SECTION] Forward Declarations, Helpers
-------------------------------------------------------------------------------

-- Helper to display a little (?) mark which shows a tooltip when hovered.
-- In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
function demo.HelpMarker(desc)
  r.ImGui_TextDisabled(ctx, '(?)')
  if r.ImGui_IsItemHovered(ctx) then
    r.ImGui_BeginTooltip(ctx)
    r.ImGui_PushTextWrapPos(ctx, r.ImGui_GetFontSize(ctx) * 35.0)
    r.ImGui_Text(ctx, desc)
    r.ImGui_PopTextWrapPos(ctx)
    r.ImGui_EndTooltip(ctx)
  end
end

function demo.RgbaToArgb(rgba)
  return (rgba >> 8 & 0x00FFFFFF) | (rgba << 24 & 0xFF000000)
end

function demo.ArgbToRgba(argb)
  return (argb << 8) | (argb >> 24 & 0xFF)
end

function demo.round(n)
  return math.floor(n + .5)
end

function demo.clamp(v, mn, mx)
  if v < mn then return mn end
  if v > mx then return mx end
  return v
end

-- Helper to display basic user controls.
function demo.ShowUserGuide()
  -- ImGuiIO& io = r.ImGui_GetIO() TODO
  r.ImGui_BulletText(ctx, 'Double-click on title bar to collapse window.')
  r.ImGui_BulletText(ctx, 
  'Click and drag on lower corner to resize window\n' ..
  '(double-click to auto fit window to its contents).')
  r.ImGui_BulletText(ctx, 'CTRL+Click on a slider or drag box to input value as text.')
  r.ImGui_BulletText(ctx, 'TAB/SHIFT+TAB to cycle through keyboard editable fields.')
  -- if (io.FontAllowUserScaling)
  --     r.ImGui_BulletText(ctx, 'CTRL+Mouse Wheel to zoom window contents.')
  r.ImGui_BulletText(ctx, 'While inputing text:\n')
  r.ImGui_Indent(ctx)
  r.ImGui_BulletText(ctx, 'CTRL+Left/Right to word jump.')
  r.ImGui_BulletText(ctx, 'CTRL+A or double-click to select all.')
  r.ImGui_BulletText(ctx, 'CTRL+X/C/V to use clipboard cut/copy/paste.')
  r.ImGui_BulletText(ctx, 'CTRL+Z,CTRL+Y to undo/redo.')
  r.ImGui_BulletText(ctx, 'ESCAPE to revert.')
  r.ImGui_BulletText(ctx, 'You can apply arithmetic operators +,*,/ on numerical values.\nUse +- to subtract.')
  r.ImGui_Unindent(ctx)
  r.ImGui_BulletText(ctx, 'With keyboard navigation enabled:')
  r.ImGui_Indent(ctx)
  r.ImGui_BulletText(ctx, 'Arrow keys to navigate.')
  r.ImGui_BulletText(ctx, 'Space to activate a widget.')
  r.ImGui_BulletText(ctx, 'Return to input text into a widget.');
  r.ImGui_BulletText(ctx, 'Escape to deactivate a widget, close popup, exit child window.')
  r.ImGui_BulletText(ctx, 'Alt to jump to the menu layer of a window.')
  r.ImGui_BulletText(ctx, 'CTRL+Tab to select a window.')
  r.ImGui_Unindent(ctx)
end

-------------------------------------------------------------------------------
-- [SECTION] Demo Window / ShowDemoWindow()
-------------------------------------------------------------------------------
-- - ShowDemoWindowWidgets()
-- - ShowDemoWindowLayout()
-- - ShowDemoWindowPopups()
-- - ShowDemoWindowTables()
-- - ShowDemoWindowColumns()
-- - ShowDemoWindowMisc()
-------------------------------------------------------------------------------

show_app = {
  -- Examples Apps (accessible from the "Examples" menu)
  main_menu_bar      = false,
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

  -- Dear ImGui Apps (accessible from the "Tools" menu)
  metrics      = false,
  -- style_editor = false
  about        = false,
}

-- Demonstrate most Dear ImGui features (this is big function!)
-- You may execute this function to experiment with the UI and understand what it does.
-- You may then search for keywords in the code when you are interested by a specific feature.
function demo.ShowDemoWindow()
  local rv, open = nil, true

  if show_app.main_menu_bar      then                               demo.ShowExampleAppMainMenuBar()       end
  if show_app.documents          then show_app.documents          = demo.ShowExampleAppDocuments()         end
  if show_app.console            then show_app.console            = demo.ShowExampleAppConsole()           end
  if show_app.log                then show_app.log                = demo.ShowExampleAppLog()               end
  if show_app.layout             then show_app.layout             = demo.ShowExampleAppLayout()            end
  if show_app.property_editor    then show_app.property_editor    = demo.ShowExampleAppPropertyEditor()    end
  if show_app.long_text          then show_app.long_text          = demo.ShowExampleAppLongText()          end
  if show_app.auto_resize        then show_app.auto_resize        = demo.ShowExampleAppAutoResize()        end
  if show_app.constrained_resize then show_app.constrained_resize = demo.ShowExampleAppConstrainedResize() end
  if show_app.simple_overlay     then show_app.simple_overlay     = demo.ShowExampleAppSimpleOverlay()     end
  if show_app.fullscreen         then show_app.fullscreen         = demo.ShowExampleAppFullscreen()        end
  if show_app.window_titles      then demo.ShowExampleAppWindowTitles()                                    end
  if show_app.custom_rendering   then show_app.custom_rendering   = demo.ShowExampleAppCustomRendering()   end

  if show_app.metrics then show_app.metrics = r.ImGui_ShowMetricsWindow(ctx, show_app.metrics) end
  if show_app.about   then show_app.about   = demo.ShowAboutWindow(show_app.about)     end
  -- if (show_app_style_editor)
  -- {
  --     r.ImGui_Begin("Dear ImGui Style Editor", &show_app_style_editor);
  --     r.ImGui_ShowStyleEditor();
  --     r.ImGui_End();
  -- }

  -- Demonstrate the various window flags. Typically you would just use the default!
  local window_flags = r.ImGui_WindowFlags_None()
  if demo.no_titlebar       then window_flags = window_flags | r.ImGui_WindowFlags_NoTitleBar()            end
  if demo.no_scrollbar      then window_flags = window_flags | r.ImGui_WindowFlags_NoScrollbar()           end
  if not demo.no_menu       then window_flags = window_flags | r.ImGui_WindowFlags_MenuBar()               end
  if demo.no_move           then window_flags = window_flags | r.ImGui_WindowFlags_NoMove()                end
  if demo.no_resize         then window_flags = window_flags | r.ImGui_WindowFlags_NoResize()              end
  if demo.no_collapse       then window_flags = window_flags | r.ImGui_WindowFlags_NoCollapse()            end
  if demo.no_nav            then window_flags = window_flags | r.ImGui_WindowFlags_NoNav()                 end
  if demo.no_background     then window_flags = window_flags | r.ImGui_WindowFlags_NoBackground()          end
  if demo.no_bring_to_front then window_flags = window_flags | r.ImGui_WindowFlags_NoBringToFrontOnFocus() end
  if demo.no_close          then open = nil end -- Don't pass our bool* to Begin

  if r.ImGui_BeginPopupContextVoid(ctx, 'dock') then
    local dock = r.ImGui_GetDock(ctx)
    if r.ImGui_MenuItem(ctx, 'Duck in docker', nil, dock & 1) then
      r.ImGui_SetDock(ctx, dock ~ 1)
    end
    r.ImGui_EndPopup(ctx)
  end

  -- We specify a default position/size in case there's no data in the .ini file.
  -- We only do it to make the demo applications a little more welcoming, but typically this isn't required.
  local main_viewport = r.ImGui_GetMainViewport(ctx)
  local work_pos = {r.ImGui_Viewport_GetWorkPos(main_viewport)}
  r.ImGui_SetNextWindowPos(ctx, work_pos[1] + 20, work_pos[2] + 20, r.ImGui_Cond_FirstUseEver())
  r.ImGui_SetNextWindowSize(ctx, 550, 680, r.ImGui_Cond_FirstUseEver())

  -- Main body of the Demo window starts here.
  rv,open = r.ImGui_Begin(ctx, 'Dear ImGui Demo', open, window_flags)
  if demo.no_close then open = true end
  if not rv then
    -- Early out if the window is collapsed, as an optimization.
    r.ImGui_End(ctx)
    return open
  end

  -- Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.

  -- e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)
  --r.ImGui_PushItemWidth(-r.ImGui_GetWindowWidth() * 0.35f);

  -- e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
  r.ImGui_PushItemWidth(ctx, r.ImGui_GetFontSize(ctx) * -12)

  -- Menu Bar
  if r.ImGui_BeginMenuBar(ctx) then
    if r.ImGui_BeginMenu(ctx, 'Menu') then
      demo.ShowExampleMenuFile()
      r.ImGui_EndMenu(ctx)
    end
    if r.ImGui_BeginMenu(ctx, 'Examples') then
      rv,show_app.main_menu_bar =
        r.ImGui_MenuItem(ctx, 'Main menu bar', nil, show_app.main_menu_bar)
      rv,show_app.console =
        r.ImGui_MenuItem(ctx, 'Console', nil, show_app.console, false)
      rv,show_app.log =
        r.ImGui_MenuItem(ctx, 'Log', nil, show_app.log)
      rv,show_app.layout =
        r.ImGui_MenuItem(ctx, 'Simple layout', nil, show_app.layout)
      rv,show_app.property_editor =
        r.ImGui_MenuItem(ctx, 'Property editor', nil, show_app.property_editor)
      rv,show_app.long_text =
        r.ImGui_MenuItem(ctx, 'Long text display', nil, show_app.long_text)
      rv,show_app.auto_resize =
        r.ImGui_MenuItem(ctx, 'Auto-resizing window', nil, show_app.auto_resize)
      rv,show_app.constrained_resize =
        r.ImGui_MenuItem(ctx, 'Constrained-resizing window', nil, show_app.constrained_resize)
      rv,show_app.simple_overlay =
        r.ImGui_MenuItem(ctx, 'Simple overlay', nil, show_app.simple_overlay)
      rv,show_app.fullscreen =
        r.ImGui_MenuItem(ctx, 'Fullscreen window', nil, show_app.fullscreen)
      rv,show_app.window_titles =
        r.ImGui_MenuItem(ctx, 'Manipulating window titles', nil, show_app.window_titles)
      rv,show_app.custom_rendering =
        r.ImGui_MenuItem(ctx, 'Custom rendering', nil, show_app.custom_rendering)
      rv,show_app.documents =
        r.ImGui_MenuItem(ctx, 'Documents', nil, show_app.documents, false)
      r.ImGui_EndMenu(ctx)
    end
    if r.ImGui_BeginMenu(ctx, 'Tools') then
      rv,show_app.metrics =
        r.ImGui_MenuItem(ctx, 'Metrics/Debugger', nil, show_app.metrics)
      rv,show_app.style_editor =
        r.ImGui_MenuItem(ctx, 'Style Editor', nil, show_app.style_editor, false)
      rv,show_app.about =
        r.ImGui_MenuItem(ctx, 'About Dear ImGui', nil, show_app.about)
      r.ImGui_EndMenu(ctx)
    end
    r.ImGui_EndMenuBar(ctx)
  end

  r.ImGui_Text(ctx, ('dear imgui says hello. (%s / %s)'):format(IMGUI_VERSION, REAIMGUI_VERSION))
  r.ImGui_Spacing(ctx)

  if r.ImGui_CollapsingHeader(ctx, 'Help') then
    r.ImGui_Text(ctx, 'ABOUT THIS DEMO:')
    r.ImGui_BulletText(ctx, 'Sections below are demonstrating many aspects of the library.')
    r.ImGui_BulletText(ctx, 'The "Examples" menu above leads to more demo contents.')
    r.ImGui_BulletText(ctx, 'The "Tools" menu above gives access to: About Box, Style Editor,\n' ..
                            'and Metrics/Debugger (general purpose Dear ImGui debugging tool).')
    r.ImGui_Separator(ctx)

    -- r.ImGui_Text(ctx, 'PROGRAMMER GUIDE:')
    -- r.ImGui_BulletText(ctx, 'See the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!')
    -- r.ImGui_BulletText(ctx, 'See comments in imgui.cpp.')
    -- r.ImGui_BulletText(ctx, 'See example applications in the examples/ folder.')
    -- r.ImGui_BulletText(ctx, 'Read the FAQ at http://www.dearimgui.org/faq/')
    -- r.ImGui_BulletText(ctx, "Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.")
    -- r.ImGui_BulletText(ctx, "Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.")
    -- r.ImGui_Separator(ctx)

    r.ImGui_Text(ctx, 'USER GUIDE:')
    demo.ShowUserGuide()
  end

  if r.ImGui_CollapsingHeader(ctx, 'Configuration') then
    if r.ImGui_TreeNode(ctx, 'Configuration##2') then
      if not config.flags then
        config.flags = r.ImGui_ConfigFlags_None()
      end

      rv,config.flags = r.ImGui_CheckboxFlags(ctx, 'ConfigFlags_NavEnableKeyboard', config.flags, r.ImGui_ConfigFlags_NavEnableKeyboard())
      r.ImGui_SameLine(ctx); demo.HelpMarker('Enable keyboard controls.')
      -- r.ImGui_CheckboxFlags("io.ConfigFlags: NavEnableGamepad",     &io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad)
      -- r.ImGui_SameLine(ctx); demo.HelpMarker("Enable gamepad controls. Require backend to set io.BackendFlags |= ImGuiBackendFlags_HasGamepad.\n\nRead instructions in imgui.cpp for details.")
      rv,config.flags = r.ImGui_CheckboxFlags(ctx, 'ConfigFlags_NavEnableSetMousePos', config.flags, r.ImGui_ConfigFlags_NavEnableSetMousePos())
      r.ImGui_SameLine(ctx); demo.HelpMarker('Instruct navigation to move the mouse cursor.')
      rv,config.flags = r.ImGui_CheckboxFlags(ctx, 'ConfigFlags_NoMouse', config.flags, r.ImGui_ConfigFlags_NoMouse())
      if (config.flags & r.ImGui_ConfigFlags_NoMouse()) ~= 0 then
        -- The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to fix it:
        if r.ImGui_GetTime(ctx) % 0.40 < 0.20 then
          r.ImGui_SameLine(ctx)
          r.ImGui_Text(ctx, '<<PRESS SPACE TO DISABLE>>')
        end
        if r.ImGui_IsKeyPressed(ctx, 0x20) then
          config.flags = config.flags & ~r.ImGui_ConfigFlags_NoMouse()
        end
      end
      rv,config.flags = r.ImGui_CheckboxFlags(ctx, 'ConfigFlags_NoMouseCursorChange', config.flags, r.ImGui_ConfigFlags_NoMouseCursorChange())
      r.ImGui_SameLine(ctx); demo.HelpMarker('Instruct backend to not alter mouse cursor shape and visibility.')
      -- r.ImGui_Checkbox(ctx, 'io.ConfigInputTextCursorBlink', &io.ConfigInputTextCursorBlink)
      -- r.ImGui_SameLine(ctx); demo.HelpMarker("Enable blinking cursor (optional as some users consider it to be distracting)")
      -- r.ImGui_Checkbox(ctx, 'io.ConfigDragClickToInputText', &io.ConfigDragClickToInputText)
      -- r.ImGui_SameLine(ctx); demo.HelpMarker("Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving).")
      -- r.ImGui_Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges)
      -- r.ImGui_SameLine(ctx); demo.HelpMarker('Enable resizing of windows from their edges and from the lower-left corner.\nThis requires (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback.')
      -- r.ImGui_Checkbox(ctx, 'io.ConfigWindowsMoveFromTitleBarOnly', &io.ConfigWindowsMoveFromTitleBarOnly)
      -- r.ImGui_Checkbox(ctx, 'io.MouseDrawCursor', &io.MouseDrawCursor)
      -- r.ImGui_SameLine(ctx); HelpMarker('Instruct Dear ImGui to render a mouse cursor itself. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).')
      -- r.ImGui_Text(ctx, "Also see Style->Rendering for rendering options.")
      r.ImGui_SetConfigFlags(ctx, config.flags)
      r.ImGui_TreePop(ctx)
      r.ImGui_Separator(ctx)
    end

--         if (r.ImGui_TreeNode("Backend Flags"))
--         {
--             HelpMarker(
--                 "Those flags are set by the backends (imgui_impl_xxx files) to specify their capabilities.\n"
--                 "Here we expose then as read-only fields to avoid breaking interactions with your backend.");
--
--             // Make a local copy to avoid modifying actual backend flags.
--             ImGuiBackendFlags backend_flags = io.BackendFlags;
--             r.ImGui_CheckboxFlags("io.BackendFlags: HasGamepad",           &backend_flags, ImGuiBackendFlags_HasGamepad);
--             r.ImGui_CheckboxFlags("io.BackendFlags: HasMouseCursors",      &backend_flags, ImGuiBackendFlags_HasMouseCursors);
--             r.ImGui_CheckboxFlags("io.BackendFlags: HasSetMousePos",       &backend_flags, ImGuiBackendFlags_HasSetMousePos);
--             r.ImGui_CheckboxFlags("io.BackendFlags: RendererHasVtxOffset", &backend_flags, ImGuiBackendFlags_RendererHasVtxOffset);
--             r.ImGui_TreePop();
--             r.ImGui_Separator();
--         }
--
--         if (r.ImGui_TreeNode("Style"))
--         {
--             HelpMarker("The same contents can be accessed in 'Tools->Style Editor' or by calling the ShowStyleEditor() function.");
--             r.ImGui_ShowStyleEditor();
--             r.ImGui_TreePop();
--             r.ImGui_Separator();
--         }

    if r.ImGui_TreeNode(ctx, 'Capture/Logging') then
      if not config.logging then
        config.logging = {
          auto_open_depth = 2,
        }
      end

      demo.HelpMarker(
        'The logging API redirects all text output so you can easily capture the content of \z
         a window or a block. Tree nodes can be automatically expanded.\n\z
         Try opening any of the contents below in this window and then click one of the "Log To" button.')
      r.ImGui_PushID(ctx, 'LogButtons')
      local log_to_tty = r.ImGui_Button(ctx, 'Log To TTY'); r.ImGui_SameLine(ctx)
      local log_to_file = r.ImGui_Button(ctx, 'Log To File'); r.ImGui_SameLine(ctx)
      local log_to_clipboard = r.ImGui_Button(ctx, 'Log To Clipboard'); r.ImGui_SameLine(ctx)
      r.ImGui_PushAllowKeyboardFocus(ctx, false)
      r.ImGui_SetNextItemWidth(ctx, 80.0)
      rv,config.logging.auto_open_depth =
        r.ImGui_SliderInt(ctx, 'Open Depth', config.logging.auto_open_depth, 0, 9)
      r.ImGui_PopAllowKeyboardFocus(ctx)
      r.ImGui_PopID(ctx)

      -- Start logging at the end of the function so that the buttons don't appear in the log
      local depth = config.logging.auto_open_depth
      if log_to_tty       then r.ImGui_LogToTTY(ctx, depth)       end
      if log_to_file      then r.ImGui_LogToFile(ctx, depth)      end
      if log_to_clipboard then r.ImGui_LogToClipboard(ctx, depth) end

      demo.HelpMarker('You can also call r.ImGui_LogText() to output directly to the log without a visual output.')
      if r.ImGui_Button(ctx, 'Copy "Hello, world!" to clipboard') then
        r.ImGui_LogToClipboard(ctx, depth)
        r.ImGui_LogText(ctx, 'Hello, world!')
        r.ImGui_LogFinish(ctx)
      end
      r.ImGui_TreePop(ctx)
    end
  end

  if r.ImGui_CollapsingHeader(ctx, 'Window options') then
    if r.ImGui_BeginTable(ctx, 'split', 3) then
      r.ImGui_TableNextColumn(ctx); rv,demo.no_titlebar       = r.ImGui_Checkbox(ctx, 'No titlebar', demo.no_titlebar)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_scrollbar      = r.ImGui_Checkbox(ctx, 'No scrollbar', demo.no_scrollbar)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_menu           = r.ImGui_Checkbox(ctx, 'No menu', demo.no_menu)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_move           = r.ImGui_Checkbox(ctx, 'No move', demo.no_move)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_resize         = r.ImGui_Checkbox(ctx, 'No resize', demo.no_resize)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_collapse       = r.ImGui_Checkbox(ctx, 'No collapse', demo.no_collapse)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_close          = r.ImGui_Checkbox(ctx, 'No close', demo.no_close)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_nav            = r.ImGui_Checkbox(ctx, 'No nav', demo.no_nav)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_background     = r.ImGui_Checkbox(ctx, 'No background', demo.no_background)
      r.ImGui_TableNextColumn(ctx); rv,demo.no_bring_to_front = r.ImGui_Checkbox(ctx, 'No bring to front', demo.no_bring_to_front)
      r.ImGui_EndTable(ctx)
    end
  end

  -- All demo contents
  demo.ShowDemoWindowWidgets()
  demo.ShowDemoWindowLayout()
  demo.ShowDemoWindowPopups()
  demo.ShowDemoWindowTables()
  demo.ShowDemoWindowMisc()

  -- End of ShowDemoWindow()
  r.ImGui_PopItemWidth(ctx)
  r.ImGui_End(ctx)

  return open
end

function demo.ShowDemoWindowWidgets()
  if not r.ImGui_CollapsingHeader(ctx, 'Widgets') then
    return
  end

  local rv

  if r.ImGui_TreeNode(ctx, 'Basic') then
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

    if r.ImGui_Button(ctx, 'Button') then
      widgets.basic.clicked = widgets.basic.clicked + 1
    end
    if widgets.basic.clicked & 1 ~= 0 then
      r.ImGui_SameLine(ctx)
      r.ImGui_Text(ctx, 'Thanks for clicking me!')
    end

    rv,widgets.basic.check = r.ImGui_Checkbox(ctx, 'checkbox', widgets.basic.check)

    rv,widgets.basic.radio = r.ImGui_RadioButtonEx(ctx, 'radio a', widgets.basic.radio, 0); r.ImGui_SameLine(ctx)
    rv,widgets.basic.radio = r.ImGui_RadioButtonEx(ctx, 'radio b', widgets.basic.radio, 1); r.ImGui_SameLine(ctx)
    rv,widgets.basic.radio = r.ImGui_RadioButtonEx(ctx, 'radio c', widgets.basic.radio, 2)

    -- Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
    for i = 0, 6 do
     if i > 0 then
      r.ImGui_SameLine(ctx)
     end
     r.ImGui_PushID(ctx, i)
     local buttonColor  = reaper.ImGui_ColorConvertHSVtoRGB(i / 7.0, 0.6, 0.6, 1.0)
     local hoveredColor = reaper.ImGui_ColorConvertHSVtoRGB(i / 7.0, 0.7, 0.7, 1.0)
     local activeColor  = reaper.ImGui_ColorConvertHSVtoRGB(i / 7.0, 0.8, 0.8, 1.0)
     r.ImGui_PushStyleColor(ctx, r.ImGui_Col_Button(),        buttonColor)
     r.ImGui_PushStyleColor(ctx, r.ImGui_Col_ButtonHovered(), hoveredColor)
     r.ImGui_PushStyleColor(ctx, r.ImGui_Col_ButtonActive(),  activeColor)
     r.ImGui_Button(ctx, 'Click')
     r.ImGui_PopStyleColor(ctx, 3)
     r.ImGui_PopID(ctx)
    end

    -- Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
    -- (otherwise a Text+SameLine+Button sequence will have the text a little too high by default!)
    -- See 'Demo->Layout->Text Baseline Alignment' for details.
    r.ImGui_AlignTextToFramePadding(ctx)
    r.ImGui_Text(ctx, 'Hold to repeat:')
    r.ImGui_SameLine(ctx)

    -- Arrow buttons with Repeater
    local spacing = r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemInnerSpacing())
    r.ImGui_PushButtonRepeat(ctx, true)
    if r.ImGui_ArrowButton(ctx, '##left', r.ImGui_Dir_Left()) then
      widgets.basic.counter = widgets.basic.counter - 1
    end
    r.ImGui_SameLine(ctx, 0.0, spacing)
    if r.ImGui_ArrowButton(ctx, '##right', r.ImGui_Dir_Right()) then
      widgets.basic.counter = widgets.basic.counter + 1
    end
    r.ImGui_PopButtonRepeat(ctx)
    r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, ("%d"):format(widgets.basic.counter))

    r.ImGui_Text(ctx, 'Hover over me')
    if r.ImGui_IsItemHovered(ctx) then
      r.ImGui_SetTooltip(ctx, 'I am a tooltip')
    end

    r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, '- or me')
    if r.ImGui_IsItemHovered(ctx) then
      r.ImGui_BeginTooltip(ctx)
      r.ImGui_Text(ctx, 'I am a fancy tooltip')
      r.ImGui_PlotLines(ctx, 'Curve', widgets.basic.tooltip, 0)
      r.ImGui_EndTooltip(ctx)
    end

    r.ImGui_Separator(ctx)

    r.ImGui_LabelText(ctx, 'label', 'Value');

    -- Using the _simplified_ one-liner Combo() api here
    -- See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
    local items = "AAAA\31BBBB\31CCCC\31DDDD\31EEEE\31FFFF\31GGGG\31HHHH\31IIIIIII\31JJJJ\31KKKKKKK\31"
    rv,widgets.basic.curitem = r.ImGui_Combo(ctx, 'combo', widgets.basic.curitem, items)
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'Using the simplified one-liner Combo API here.\n' ..
      'Refer to the "Combo" section below for an explanation of how to use the more flexible and general BeginCombo/EndCombo API.')

    rv,widgets.basic.str0 = r.ImGui_InputText(ctx, 'input text', widgets.basic.str0);
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'USER:\n' ..
      'Hold SHIFT or use mouse to select text.\n' ..
      'CTRL+Left/Right to word jump.\n' ..
      'CTRL+A or double-click to select all.\n' ..
      'CTRL+X,CTRL+C,CTRL+V clipboard.\n' ..
      'CTRL+Z,CTRL+Y undo/redo.\n' ..
      'ESCAPE to revert.\n\n')

    rv,widgets.basic.str1 = r.ImGui_InputTextWithHint(ctx, 'input text (w/ hint)', 'enter text here', widgets.basic.str1);

    rv,widgets.basic.i0 = r.ImGui_InputInt(ctx, 'input int', widgets.basic.i0)
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'You can apply arithmetic operators +,*,/ on numerical values.\n' ..
      '  e.g. [ 100 ], input \'*2\', result becomes [ 200 ]\n' ..
      'Use +- to subtract.')

    rv,widgets.basic.d0 = r.ImGui_InputDouble(ctx, 'input double', widgets.basic.d0, 0.01, 1.0, '%.8f')
    rv,widgets.basic.d1 = r.ImGui_InputDouble(ctx, 'input scientific', widgets.basic.d1, 0.0, 0.0, '%e')
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'You can input value using the scientific notation,\n' ..
      '  e.g. "1e+8" becomes "100000000".')

    r.ImGui_InputDoubleN(ctx, 'input reaper.array', widgets.basic.vec4a)

    rv,widgets.basic.i1 = r.ImGui_DragInt(ctx, 'drag int', widgets.basic.i1, 1)
    r.ImGui_SameLine(ctx); demo.HelpMarker(
        'Click and drag to edit value.\n' ..
        'Hold SHIFT/ALT for faster/slower edit.\n' ..
        'Double-click or CTRL+click to input value.')

    rv,widgets.basic.i2 = r.ImGui_DragInt(ctx, 'drag int 0..100', widgets.basic.i2, 1, 0, 100, '%d%%', r.ImGui_SliderFlags_AlwaysClamp())

    rv,widgets.basic.d2 = r.ImGui_DragDouble(ctx, 'drag double', widgets.basic.d2, 0.005)
    rv,widgets.basic.d3 = r.ImGui_DragDouble(ctx, 'drag small double', widgets.basic.d3, 0.0001, 0.0, 0.0, '%.06f ns')

    rv,widgets.basic.i3 = r.ImGui_SliderInt(ctx, "slider int", widgets.basic.i3, -1, 3)
    r.ImGui_SameLine(ctx); demo.HelpMarker('CTRL+click to input value.')

    rv,widgets.basic.d4 = r.ImGui_SliderDouble(ctx, 'slider double', widgets.basic.d4, 0.0, 1.0, 'ratio = %.3f')
    rv,widgets.basic.d5 = r.ImGui_SliderDouble(ctx, 'slider double (log)', widgets.basic.d5, -10.0, 10.0, '%.4f', r.ImGui_SliderFlags_Logarithmic())

    rv,widgets.basic.angle = r.ImGui_SliderAngle(ctx, 'slider angle', widgets.basic.angle)

    -- Using the format string to display a name instead of an integer.
    -- Here we completely omit '%d' from the format string, so it'll only display a name.
    -- This technique can also be used with DragInt().
    local elements = { 'Fire', 'Earth', 'Air', 'Water' }
    local current_elem = elements[widgets.basic.elem] or 'Unknown'
    rv,widgets.basic.elem = r.ImGui_SliderInt(ctx, 'slider enum', widgets.basic.elem, 1, #elements, current_elem)
    r.ImGui_SameLine(ctx)
    demo.HelpMarker(
      'Using the format string parameter to display a name instead \z
       of the underlying integer.'
    )

    foo = widgets.basic.col1
    rv,widgets.basic.col1 = r.ImGui_ColorEdit3(ctx, 'color 1', widgets.basic.col1)
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'Click on the color square to open a color picker.\n\z
       Click and hold to use drag and drop.\n\z
       Right-click on the color square to show options.\n\z
       CTRL+click on individual component to input value.'
    )

    rv, widgets.basic.col2 = r.ImGui_ColorEdit4(ctx, 'color 2', widgets.basic.col2)

    -- Using the _simplified_ one-liner ListBox() api here
    -- See "List boxes" section for examples of how to use the more flexible BeginListBox()/EndListBox() api.
    local items = 'Apple\31Banana\31Cherry\31Kiwi\31Mango\31Orange\31Pineapple\31Strawberry\31Watermelon\31'
    rv,widgets.basic.listcur = r.ImGui_ListBox(ctx, 'listbox\n(single select)', widgets.basic.listcur, items, 4)
    r.ImGui_SameLine(ctx)
    demo.HelpMarker(
      'Using the simplified one-liner ListBox API here.\n\z
       Refer to the "List boxes" section below for an explanation of how to use\z
       the more flexible and general BeginListBox/EndListBox API.'
    )

    r.ImGui_TreePop(ctx)
  end

--     // Testing ImGuiOnceUponAFrame helper.
--     //static ImGuiOnceUponAFrame once;
--     //for (int i = 0; i < 5; i++)
--     //    if (once)
--     //        r.ImGui_Text("This will be displayed only once.");

  if r.ImGui_TreeNode(ctx, 'Trees') then
    if not widgets.trees then
      widgets.trees = {
        base_flags = r.ImGui_TreeNodeFlags_OpenOnArrow() |
                    r.ImGui_TreeNodeFlags_OpenOnDoubleClick() |
                    r.ImGui_TreeNodeFlags_SpanAvailWidth(),
        align_label_with_current_x_position = false,
        test_drag_and_drop = false,
        selection_mask = 1 << 2,
      }
    end

    if r.ImGui_TreeNode(ctx, 'Basic trees') then
      for i = 0, 4 do
        -- Use SetNextItemOpen() so set the default state of a node to be open. We could
        -- also use TreeNodeEx() with the ImGui_TreeNodeFlags_DefaultOpen flag to achieve the same thing!
        if i == 0 then
          r.ImGui_SetNextItemOpen(ctx, true, r.ImGui_Cond_Once())
        end

        if r.ImGui_TreeNodeEx(ctx, i, ('Child %d'):format(i)) then
          r.ImGui_Text(ctx, 'blah blah')
          r.ImGui_SameLine(ctx)
          if r.ImGui_SmallButton(ctx, 'button') then end
          r.ImGui_TreePop(ctx)
        end
      end
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Advanced, with Selectable nodes') then
      demo.HelpMarker(
        'This is a more typical looking tree with selectable nodes.\n' ..
        'Click to select, CTRL+Click to toggle, click on arrows or double-click to open.')
      rv,widgets.trees.base_flags = r.ImGui_CheckboxFlags(ctx, 'ImGui_TreeNodeFlags_OpenOnArrow',       widgets.trees.base_flags, r.ImGui_TreeNodeFlags_OpenOnArrow())
      rv,widgets.trees.base_flags = r.ImGui_CheckboxFlags(ctx, 'ImGui_TreeNodeFlags_OpenOnDoubleClick', widgets.trees.base_flags, r.ImGui_TreeNodeFlags_OpenOnDoubleClick())
      rv,widgets.trees.base_flags = r.ImGui_CheckboxFlags(ctx, 'ImGui_TreeNodeFlags_SpanAvailWidth',    widgets.trees.base_flags, r.ImGui_TreeNodeFlags_SpanAvailWidth()); r.ImGui_SameLine(ctx); demo.HelpMarker('Extend hit area to all available width instead of allowing more items to be laid out after the node.')
      rv,widgets.trees.base_flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTreeNodeFlags_SpanFullWidth', widgets.trees.base_flags, r.ImGui_TreeNodeFlags_SpanFullWidth());
      rv,widgets.trees.align_label_with_current_x_position = r.ImGui_Checkbox(ctx, "Align label with current X position", widgets.trees.align_label_with_current_x_position);
      rv,widgets.trees.test_drag_and_drop = r.ImGui_Checkbox(ctx, "Test tree node as drag source", widgets.trees.test_drag_and_drop);
      r.ImGui_Text(ctx, 'Hello!')
      if widgets.trees.align_label_with_current_x_position then
        r.ImGui_Unindent(ctx, r.ImGui_GetTreeNodeToLabelSpacing(ctx))
      end

      -- 'selection_mask' is dumb representation of what may be user-side selection state.
      --  You may retain selection state inside or outside your objects in whatever format you see fit.
      -- 'node_clicked' is temporary storage of what node we have clicked to process selection at the end
      -- of the loop. May be a pointer to your own node type, etc.
      local node_clicked = -1

      for i = 0, 5 do
        -- Disable the default "open on single-click behavior" + set Selected flag according to our selection.
        local node_flags = widgets.trees.base_flags
        local is_selected = (widgets.trees.selection_mask & (1 << i)) ~= 0
        if is_selected then
          node_flags = node_flags | r.ImGui_TreeNodeFlags_Selected()
        end
        if i < 3 then
          -- Items 0..2 are Tree Node
          local node_open = r.ImGui_TreeNodeEx(ctx, i, ('Selectable Node %d'):format(i), node_flags)
          if r.ImGui_IsItemClicked(ctx) then
            node_clicked = i
          end
          if widgets.trees.test_drag_and_drop and r.ImGui_BeginDragDropSource(ctx) then
            r.ImGui_SetDragDropPayload(ctx, '_TREENODE', nil, 0)
            r.ImGui_Text(ctx, 'This is a drag and drop source')
            r.ImGui_EndDragDropSource(ctx)
          end
          if node_open then
            r.ImGui_BulletText(ctx, 'Blah blah\nBlah Blah')
            r.ImGui_TreePop(ctx)
          end
        else
          -- Items 3..5 are Tree Leaves
          -- The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we can
          -- use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
          node_flags = node_flags | r.ImGui_TreeNodeFlags_Leaf() | r.ImGui_TreeNodeFlags_NoTreePushOnOpen() -- | r.ImGui_TreeNodeFlags_Bullet()
          r.ImGui_TreeNodeEx(ctx, i, ('Selectable Leaf %d'):format(i), node_flags)
          if r.ImGui_IsItemClicked(ctx) then
            node_clicked = i
          end
          if widgets.trees.test_drag_and_drop and r.ImGui_BeginDragDropSource(ctx) then
            r.ImGui_SetDragDropPayload(ctx, '_TREENODE', nil, 0)
            r.ImGui_Text(ctx, 'This is a drag and drop source')
            r.ImGui_EndDragDropSource(ctx)
          end
        end
      end

      if node_clicked ~= -1 then
        -- Update selection state
        -- (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
        if (r.ImGui_GetKeyMods(ctx) & r.ImGui_KeyModFlags_Ctrl()) ~= 0 then -- CTRL+click to toggle
          widgets.trees.selection_mask = widgets.trees.selection_mask ~ (1 << node_clicked)
        elseif widgets.trees.selection_mask & (1 << node_clicked) == 0 then -- Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
          widgets.trees.selection_mask = (1 << node_clicked)              -- Click to single-select
        end
      end

      if widgets.trees.align_label_with_current_x_position then
        r.ImGui_Indent(ctx, r.ImGui_GetTreeNodeToLabelSpacing(ctx))
      end

      r.ImGui_TreePop(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Collapsing Headers') then
    if not widgets.cheads then
      widgets.cheads = {
        closable_group = true,
      }
    end

    rv,widgets.cheads.closable_group = r.ImGui_Checkbox(ctx, 'Show 2nd header', widgets.cheads.closable_group)

    if r.ImGui_CollapsingHeader(ctx, 'Header', nil, r.ImGui_TreeNodeFlags_None()) then
      r.ImGui_Text(ctx, ('IsItemHovered: %s'):format(r.ImGui_IsItemHovered(ctx)))
      for i = 0, 4 do
        r.ImGui_Text(ctx, ('Some content %s'):format(i))
      end
    end

    rv,widgets.cheads.closable_group = r.ImGui_CollapsingHeader(ctx, 'Header with a close button', widgets.cheads.closable_group)
    if rv then
      r.ImGui_Text(ctx, ('IsItemHovered: %s'):format(r.ImGui_IsItemHovered(ctx)))
      for i = 0, 4 do
        r.ImGui_Text(ctx, ('More content %d'):format(i))
      end
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Bullets') then
    r.ImGui_BulletText(ctx, 'Bullet point 1')
    r.ImGui_BulletText(ctx, 'Bullet point 2\nOn multiple lines')
    if r.ImGui_TreeNode(ctx, 'Tree node') then
      r.ImGui_BulletText(ctx, 'Another bullet point')
      r.ImGui_TreePop(ctx)
    end
    r.ImGui_Bullet(ctx); r.ImGui_Text(ctx, 'Bullet point 3 (two calls)')
    r.ImGui_Bullet(ctx); r.ImGui_SmallButton(ctx, 'Button')
    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Text') then
    if not widgets.text then
      widgets.text = {
        wrap_width = 200.0,
        utf8 = '日本語',
      }
    end

    if r.ImGui_TreeNode(ctx, 'Colorful Text') then
      -- Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
      r.ImGui_TextColored(ctx, 0xFF00FFFF, 'Pink')
      r.ImGui_TextColored(ctx, 0xFFFF00FF, 'Yellow')
      r.ImGui_TextDisabled(ctx, 'Disabled');
      r.ImGui_SameLine(ctx); demo.HelpMarker('The TextDisabled color is stored in ImGuiStyle.')
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Word Wrapping') then
      -- Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
      r.ImGui_TextWrapped(ctx,
        'This text should automatically wrap on the edge of the window. The current implementation ' ..
        'for text wrapping follows simple rules suitable for English and possibly other languages.')
      r.ImGui_Spacing(ctx)

      rv,widgets.text.wrap_width = r.ImGui_SliderDouble(ctx, 'Wrap width', widgets.text.wrap_width, -20, 600, '%.0f')

      local draw_list = r.ImGui_GetWindowDrawList(ctx)
      for n = 0, 1 do
        r.ImGui_Text(ctx, ('Test paragraph %d:'):format(n))

        local screen_x, screen_y = r.ImGui_GetCursorScreenPos(ctx)
        local marker_min_x, marker_min_y = screen_x + widgets.text.wrap_width, screen_y
        local marker_max_x, marker_max_y = screen_x + widgets.text.wrap_width + 10, screen_y + r.ImGui_GetTextLineHeight(ctx)

        local window_x, window_y = r.ImGui_GetCursorPos(ctx)
        r.ImGui_PushTextWrapPos(ctx, window_x + widgets.text.wrap_width)

        if n == 0 then
          r.ImGui_Text(ctx, ('The lazy dog is a good dog. This paragraph should fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.'):format(widgets.text.wrap_width))
        else
          r.ImGui_Text(ctx, 'aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh')
        end

        -- Draw actual text bounding box, following by marker of our expected limit (should not overlap!)
        local text_min_x, text_min_y = r.ImGui_GetItemRectMin(ctx)
        local text_max_x, text_max_y = r.ImGui_GetItemRectMax(ctx)
        r.ImGui_DrawList_AddRect(draw_list, text_min_x, text_min_y, text_max_x, text_max_y, 0xFFFF00FF)
        r.ImGui_DrawList_AddRectFilled(draw_list, marker_min_x, marker_min_y, marker_max_x, marker_max_y, 0xFF00FFFF)

        r.ImGui_PopTextWrapPos(ctx)
      end

      r.ImGui_TreePop(ctx)
    end

    -- Not supported by the default built-in font TODO
    -- if r.ImGui_TreeNode(ctx, 'UTF-8 Text') then
    --   -- UTF-8 test with Japanese characters
    --   -- (Needs a suitable font? Try "Google Noto" or "Arial Unicode". See docs/FONTS.md for details.)
    --   -- so you can safely copy & paste garbled characters into another application.
    --   r.ImGui_TextWrapped(ctx,
    --     'CJK text will only appears if the font was loaded with the appropriate CJK character ranges. ' ..
    --     'Call io.Fonts->AddFontFromFileTTF() manually to load extra character ranges. ' ..
    --     'Read docs/FONTS.md for details.')
    --   r.ImGui_Text(ctx, 'Hiragana: かきくけこ (kakikukeko)')
    --   r.ImGui_Text(ctx, 'Kanjis: 日本語 (nihongo)')
    --   rv,widgets.text.utf8 = r.ImGui_InputText(ctx, 'UTF-8 input', widgets.text.utf8)
    --
    --   r.ImGui_TreePop(ctx)
    -- end

    r.ImGui_TreePop(ctx)
  end

--     if (r.ImGui_TreeNode("Images"))
--     {
--         ImGuiIO& io = r.ImGui_GetIO();
--         r.ImGui_TextWrapped(
--             "Below we are displaying the font texture (which is the only texture we have access to in this demo). "
--             "Use the 'ImTextureID' type as storage to pass pointers or identifier to your own texture data. "
--             "Hover the texture for a zoomed view!");
--
--         // Below we are displaying the font texture because it is the only texture we have access to inside the demo!
--         // Remember that ImTextureID is just storage for whatever you want it to be. It is essentially a value that
--         // will be passed to the rendering backend via the ImDrawCmd structure.
--         // If you use one of the default imgui_impl_XXXX.cpp rendering backend, they all have comments at the top
--         // of their respective source file to specify what they expect to be stored in ImTextureID, for example:
--         // - The imgui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer
--         // - The imgui_impl_opengl3.cpp renderer expect a GLuint OpenGL texture identifier, etc.
--         // More:
--         // - If you decided that ImTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers
--         //   to r.ImGui_Image(), and gather width/height through your own functions, etc.
--         // - You can use ShowMetricsWindow() to inspect the draw data that are being passed to your renderer,
--         //   it will help you debug issues if you are confused about it.
--         // - Consider using the lower-level ImDrawList::AddImage() API, via r.ImGui_GetWindowDrawList()->AddImage().
--         // - Read https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
--         // - Read https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
--         ImTextureID my_tex_id = io.Fonts->TexID;
--         float my_tex_w = (float)io.Fonts->TexWidth;
--         float my_tex_h = (float)io.Fonts->TexHeight;
--         {
--             r.ImGui_Text("%.0fx%.0f", my_tex_w, my_tex_h);
--             ImVec2 pos = r.ImGui_GetCursorScreenPos();
--             ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
--             ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
--             ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
--             ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
--             r.ImGui_Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
--             if (r.ImGui_IsItemHovered())
--             {
--                 r.ImGui_BeginTooltip();
--                 float region_sz = 32.0f;
--                 float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
--                 float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
--                 float zoom = 4.0f;
--                 if (region_x < 0.0f) { region_x = 0.0f; }
--                 else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
--                 if (region_y < 0.0f) { region_y = 0.0f; }
--                 else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
--                 r.ImGui_Text("Min: (%.2f, %.2f)", region_x, region_y);
--                 r.ImGui_Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
--                 ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
--                 ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
--                 r.ImGui_Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
--                 r.ImGui_EndTooltip();
--             }
--         }
--         r.ImGui_TextWrapped("And now some textured buttons..");
--         static int pressed_count = 0;
--         for (int i = 0; i < 8; i++)
--         {
--             r.ImGui_PushID(i);
--             int frame_padding = -1 + i;                             // -1 == uses default padding (style.FramePadding)
--             ImVec2 size = ImVec2(32.0f, 32.0f);                     // Size of the image we want to make visible
--             ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
--             ImVec2 uv1 = ImVec2(32.0f / my_tex_w, 32.0f / my_tex_h);// UV coordinates for (32,32) in our texture
--             ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);         // Black background
--             ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       // No tint
--             if (r.ImGui_ImageButton(my_tex_id, size, uv0, uv1, frame_padding, bg_col, tint_col))
--                 pressed_count += 1;
--             r.ImGui_PopID();
--             r.ImGui_SameLine();
--         }
--         r.ImGui_NewLine();
--         r.ImGui_Text("Pressed %d times.", pressed_count);
--         r.ImGui_TreePop();
--     }

  if r.ImGui_TreeNode(ctx, 'Combo') then
    if not widgets.combos then
      widgets.combos = {
        flags = r.ImGui_ComboFlags_None(),
        current_item1 = 1,
        current_item2 = 0,
        current_item3 = -1,
      }
    end

    rv,widgets.combos.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiComboFlags_PopupAlignLeft', widgets.combos.flags, r.ImGui_ComboFlags_PopupAlignLeft())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Only makes a difference if the popup is larger than the combo')

    rv,widgets.combos.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiComboFlags_NoArrowButton', widgets.combos.flags, r.ImGui_ComboFlags_NoArrowButton())
    if rv then
      widgets.combos.flags = widgets.combos.flags & ~r.ImGui_ComboFlags_NoPreview() -- Clear the other flag, as we cannot combine both
    end

    rv,widgets.combos.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiComboFlags_NoPreview', widgets.combos.flags, r.ImGui_ComboFlags_NoPreview())
    if rv then
      widgets.combos.flags = widgets.combos.flags & ~r.ImGui_ComboFlags_NoArrowButton() -- Clear the other flag, as we cannot combine both
    end

    -- Using the generic BeginCombo() API, you have full control over how to display the combo contents.
    -- (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
    -- stored in the object itself, etc.)
    local combo_items = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" }
    local combo_label = combo_items[widgets.combos.current_item1] -- Label to preview before opening the combo (technically it could be anything)
    if r.ImGui_BeginCombo(ctx, 'combo 1', combo_label, widgets.combos.flags) then
      for i,v in ipairs(combo_items) do
        local is_selected = widgets.combos.current_item1 == i
        if r.ImGui_Selectable(ctx, combo_items[i], is_selected) then
          widgets.combos.current_item1 = i
        end

        -- Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if is_selected then
          r.ImGui_SetItemDefaultFocus(ctx)
        end
      end
      r.ImGui_EndCombo(ctx)
    end

    -- Simplified one-liner Combo() API, using values packed in a single constant string
    combo_items = 'aaaa\31bbbb\31cccc\31dddd\31eeee\31'
    rv,widgets.combos.current_item2 = r.ImGui_Combo(ctx, 'combo 2 (one-liner)', widgets.combos.current_item2, combo_items)

    -- Simplified one-liner Combo() using an array of const char*
    -- If the selection isn't within 0..count, Combo won't display a preview
    rv,widgets.combos.current_item3 = r.ImGui_Combo(ctx, 'combo 3 (out of range)', widgets.combos.current_item3, combo_items)

--         // Simplified one-liner Combo() using an accessor function
--         struct Funcs { static bool ItemGetter(void* data, int n, const char** out_str) { *out_str = ((const char**)data)[n]; return true; } };
--         static int item_current_4 = 0;
--         r.ImGui_Combo("combo 4 (function)", &item_current_4, &Funcs::ItemGetter, items, IM_ARRAYSIZE(items));

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'List boxes') then
    if not widgets.lists then
      widgets.lists = { current_idx = 1 }
    end

    -- Using the generic BeginListBox() API, you have full control over how to display the combo contents.
    -- (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
    -- stored in the object itself, etc.)
    local items = {
      'AAAA', 'BBBB', 'CCCC', 'DDDD', 'EEEE', 'FFFF', 'GGGG',
      'HHHH', 'IIII', 'JJJJ', 'KKKK', 'LLLLLLL', 'MMMM', 'OOOOOOO'
    }
    if r.ImGui_BeginListBox(ctx, 'listbox 1') then
      for n,v in ipairs(items) do
        local is_selected = widgets.lists.current_idx == n
        if r.ImGui_Selectable(ctx, v, is_selected) then
          widgets.lists.current_idx = n
        end

        -- Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if is_selected then
          r.ImGui_SetItemDefaultFocus(ctx)
        end
      end
      r.ImGui_EndListBox(ctx)
    end

    -- Custom size: use all width, 5 items tall
    r.ImGui_Text(ctx, 'Full-width:')
    if r.ImGui_BeginListBox(ctx, '##listbox 2', -FLT_MIN, 5 * r.ImGui_GetTextLineHeightWithSpacing(ctx)) then
      for n,v in ipairs(items) do
        local is_selected = widgets.lists.current_idx == n
        if r.ImGui_Selectable(ctx, v, is_selected) then
          widgets.lists.current_idx = n;
        end

        -- Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if is_selected then
          r.ImGui_SetItemDefaultFocus(ctx)
        end
      end
      r.ImGui_EndListBox(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Selectables') then
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
    if r.ImGui_TreeNode(ctx, 'Basic') then
      rv,widgets.selectables.basic[1] = r.ImGui_Selectable(ctx, '1. I am selectable', widgets.selectables.basic[1])
      rv,widgets.selectables.basic[2] = r.ImGui_Selectable(ctx, '2. I am selectable', widgets.selectables.basic[2])
      r.ImGui_Text(ctx, '3. I am not selectable')
      rv,widgets.selectables.basic[4] = r.ImGui_Selectable(ctx, '4. I am selectable', widgets.selectables.basic[4])
      if r.ImGui_Selectable(ctx, '5. I am double clickable', widgets.selectables.basic[5], r.ImGui_SelectableFlags_AllowDoubleClick()) then
        if r.ImGui_IsMouseDoubleClicked(ctx, 0) then
          widgets.selectables.basic[5] = not widgets.selectables.basic[5]
        end
      end
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Selection State: Single Selection') then
      for i = 0, 4 do
        if r.ImGui_Selectable(ctx, ('Object %d'):format(i), widgets.selectables.single == i) then
          widgets.selectables.single = i
        end
      end
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Selection State: Multiple Selection') then
      demo.HelpMarker('Hold CTRL and click to select multiple items.')
      for i,sel in ipairs(widgets.selectables.multiple) do
        if r.ImGui_Selectable(ctx, ('Object %d'):format(i-1), sel) then
          if (r.ImGui_GetKeyMods(ctx) & r.ImGui_KeyModFlags_Ctrl()) == 0 then -- Clear selection when CTRL is not held
            for j = 1, #widgets.selectables.multiple do
              widgets.selectables.multiple[j] = false
            end
          end
          widgets.selectables.multiple[i] = not sel
        end
      end
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Rendering more text into the same line') then
      rv,widgets.selectables.sameline[1] = r.ImGui_Selectable(ctx, 'main.c',    widgets.selectables.sameline[1]); r.ImGui_SameLine(ctx, 300); r.ImGui_Text(ctx, ' 2,345 bytes')
      rv,widgets.selectables.sameline[2] = r.ImGui_Selectable(ctx, 'Hello.cpp', widgets.selectables.sameline[2]); r.ImGui_SameLine(ctx, 300); r.ImGui_Text(ctx, '12,345 bytes')
      rv,widgets.selectables.sameline[3] = r.ImGui_Selectable(ctx, 'Hello.h',   widgets.selectables.sameline[3]); r.ImGui_SameLine(ctx, 300); r.ImGui_Text(ctx, ' 2,345 bytes')
      r.ImGui_TreePop(ctx)
    end
    if r.ImGui_TreeNode(ctx, 'In columns') then
      if r.ImGui_BeginTable(ctx, 'split1', 3, r.ImGui_TableFlags_Resizable()) then-- | r.ImGui_TableFlags_NoSavedSettings())
        for i,sel in ipairs(widgets.selectables.columns) do
          r.ImGui_TableNextColumn(ctx)
          rv,widgets.selectables.columns[i] = r.ImGui_Selectable(ctx, ('Item %d'):format(i-1), sel);
        end
        r.ImGui_EndTable(ctx)
      end
      r.ImGui_Separator(ctx)
      if r.ImGui_BeginTable(ctx, 'split2', 3, r.ImGui_TableFlags_Resizable()) then-- | r.ImGui_TableFlags_NoSavedSettings())
        for i,sel in ipairs(widgets.selectables.columns) do
          r.ImGui_TableNextRow(ctx)
          r.ImGui_TableNextColumn(ctx)
          rv,widgets.selectables.columns[i] = r.ImGui_Selectable(ctx, ('Item %d'):format(i-1), sel, r.ImGui_SelectableFlags_SpanAllColumns());
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, 'Some other contents');
          r.ImGui_TableNextColumn(ctx);
          r.ImGui_Text(ctx, '123456');
        end
        r.ImGui_EndTable(ctx)
      end
      r.ImGui_TreePop(ctx)
    end

    -- Add in a bit of silly fun...
    if r.ImGui_TreeNode(ctx, 'Grid') then
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
        local time = r.ImGui_GetTime(ctx)
        r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_SelectableTextAlign(),
          0.5 + 0.5 * math.cos(time * 2.0), 0.5 + 0.5 * math.sin(time * 3.0))
      end

      for ri,row in ipairs(widgets.selectables.grid) do
        for ci,col in ipairs(row) do
          if ci > 1 then
            r.ImGui_SameLine(ctx)
          end
          r.ImGui_PushID(ctx, ri * #widgets.selectables.grid + ci);
          if r.ImGui_Selectable(ctx, 'Sailor', col, 0, 50, 50) then
            -- Toggle clicked cell + toggle neighbors
            row[ci] = not row[ci]
            if ci > 1 then row[ci - 1] = not row[ci - 1]; end
            if ci < 4 then row[ci + 1] = not row[ci + 1]; end
            if ri > 1 then widgets.selectables.grid[ri - 1][ci] = not widgets.selectables.grid[ri - 1][ci]; end
            if ri < 4 then widgets.selectables.grid[ri + 1][ci] = not widgets.selectables.grid[ri + 1][ci]; end
          end
          r.ImGui_PopID(ctx);
        end
      end

      if winning_state then
        r.ImGui_PopStyleVar(ctx)
      end
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Alignment') then
      demo.HelpMarker(
        'By default, Selectables uses style.SelectableTextAlign but it can be overridden on a per-item ' ..
        "basis using PushStyleVar(). You'll probably want to always keep your default situation to " ..
        'left-align otherwise it becomes difficult to layout multiple items on a same line')

      for y = 1, 3 do
        for x = 1, 3 do
          local align_x, align_y = (x-1) / 2.0, (y-1) / 2.0
          local name = ('(%.1f,%.1f)'):format(align_x, align_y)
          if x > 1 then r.ImGui_SameLine(ctx); end
          r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_SelectableTextAlign(), align_x, align_y)
          local row = widgets.selectables.align[y]
          rv,row[x] = r.ImGui_Selectable(ctx, name, row[x], r.ImGui_SelectableFlags_None(), 80, 80)
          r.ImGui_PopStyleVar(ctx)
        end
      end

      r.ImGui_TreePop(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Text Input') then
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
        flags = r.ImGui_InputTextFlags_AllowTabInput(),
        buf = { '', '', '', '', '' },
        password = 'hunter2',
      }
    end

    if r.ImGui_TreeNode(ctx, 'Multi-line Text Input') then
      rv,widgets.input.multiline.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiInputTextFlags_ReadOnly', widgets.input.multiline.flags, r.ImGui_InputTextFlags_ReadOnly());
      rv,widgets.input.multiline.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiInputTextFlags_AllowTabInput', widgets.input.multiline.flags, r.ImGui_InputTextFlags_AllowTabInput());
      rv,widgets.input.multiline.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiInputTextFlags_CtrlEnterForNewLine', widgets.input.multiline.flags, r.ImGui_InputTextFlags_CtrlEnterForNewLine());
      rv,widgets.input.multiline.text = r.ImGui_InputTextMultiline(ctx, '##source', widgets.input.multiline.text, -FLT_MIN, r.ImGui_GetTextLineHeight(ctx) * 16, widgets.input.multiline.flags)
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Filtered Text Input') then
      -- TODO
      -- struct TextFilters
      -- {
      --     // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i'
      --     static int FilterImGuiLetters(ImGuiInputTextCallbackData* data)
      --     {
      --         if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar))
      --             return 0;
      --         return 1;
      --     }
      -- };

      rv,widgets.input.buf[1] = r.ImGui_InputText(ctx, 'default',     widgets.input.buf[1]);
      rv,widgets.input.buf[2] = r.ImGui_InputText(ctx, 'decimal',     widgets.input.buf[2], r.ImGui_InputTextFlags_CharsDecimal())
      rv,widgets.input.buf[3] = r.ImGui_InputText(ctx, 'hexadecimal', widgets.input.buf[3], r.ImGui_InputTextFlags_CharsHexadecimal() | r.ImGui_InputTextFlags_CharsUppercase())
      rv,widgets.input.buf[4] = r.ImGui_InputText(ctx, 'uppercase',   widgets.input.buf[4], r.ImGui_InputTextFlags_CharsUppercase())
      rv,widgets.input.buf[5] = r.ImGui_InputText(ctx, 'no blank',    widgets.input.buf[5], r.ImGui_InputTextFlags_CharsNoBlank())
      -- static char buf6[64] = ""; r.ImGui_InputText("\"imgui\" letters", buf6, 64, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Password Input') then
      rv,widgets.input.password = r.ImGui_InputText(ctx, 'password', widgets.input.password, r.ImGui_InputTextFlags_Password())
      r.ImGui_SameLine(ctx); demo.HelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
      rv,widgets.input.password = r.ImGui_InputTextWithHint(ctx, 'password (w/ hint)', '<password>', widgets.input.password, r.ImGui_InputTextFlags_Password());
      rv,widgets.input.password = r.ImGui_InputText(ctx, 'password (clear)', widgets.input.password)
      r.ImGui_TreePop(ctx)
    end

-- TODO
--         if (r.ImGui_TreeNode("Completion, History, Edit Callbacks"))
--         {
--             struct Funcs
--             {
--                 static int MyCallback(ImGuiInputTextCallbackData* data)
--                 {
--                     if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
--                     {
--                         data->InsertChars(data->CursorPos, "..");
--                     }
--                     else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
--                     {
--                         if (data->EventKey == ImGuiKey_UpArrow)
--                         {
--                             data->DeleteChars(0, data->BufTextLen);
--                             data->InsertChars(0, "Pressed Up!");
--                             data->SelectAll();
--                         }
--                         else if (data->EventKey == ImGuiKey_DownArrow)
--                         {
--                             data->DeleteChars(0, data->BufTextLen);
--                             data->InsertChars(0, "Pressed Down!");
--                             data->SelectAll();
--                         }
--                     }
--                     else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
--                     {
--                         // Toggle casing of first character
--                         char c = data->Buf[0];
--                         if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) data->Buf[0] ^= 32;
--                         data->BufDirty = true;
--
--                         // Increment a counter
--                         int* p_int = (int*)data->UserData;
--                         *p_int = *p_int + 1;
--                     }
--                     return 0;
--                 }
--             };
--             static char buf1[64];
--             r.ImGui_InputText("Completion", buf1, 64, ImGuiInputTextFlags_CallbackCompletion, Funcs::MyCallback);
--             r.ImGui_SameLine(); HelpMarker("Here we append \"..\" each time Tab is pressed. See 'Examples>Console' for a more meaningful demonstration of using this callback.");
--
--             static char buf2[64];
--             r.ImGui_InputText("History", buf2, 64, ImGuiInputTextFlags_CallbackHistory, Funcs::MyCallback);
--             r.ImGui_SameLine(); HelpMarker("Here we replace and select text each time Up/Down are pressed. See 'Examples>Console' for a more meaningful demonstration of using this callback.");
--
--             static char buf3[64];
--             static int edit_count = 0;
--             r.ImGui_InputText("Edit", buf3, 64, ImGuiInputTextFlags_CallbackEdit, Funcs::MyCallback, (void*)&edit_count);
--             r.ImGui_SameLine(); HelpMarker("Here we toggle the casing of the first character on every edits + count edits.");
--             r.ImGui_SameLine(); r.ImGui_Text("(%d)", edit_count);
--
--             r.ImGui_TreePop();
--         }
--
--         if (r.ImGui_TreeNode("Resize Callback"))
--         {
--             // To wire InputText() with std::string or any other custom string type,
--             // you can use the ImGuiInputTextFlags_CallbackResize flag + create a custom ImGui_InputText() wrapper
--             // using your preferred type. See misc/cpp/imgui_stdlib.h for an implementation of this using std::string.
--             HelpMarker(
--                 "Using ImGuiInputTextFlags_CallbackResize to wire your custom string type to InputText().\n\n"
--                 "See misc/cpp/imgui_stdlib.h for an implementation of this for std::string.");
--             struct Funcs
--             {
--                 static int MyResizeCallback(ImGuiInputTextCallbackData* data)
--                 {
--                     if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
--                     {
--                         ImVector<char>* my_str = (ImVector<char>*)data->UserData;
--                         IM_ASSERT(my_str->begin() == data->Buf);
--                         my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
--                         data->Buf = my_str->begin();
--                     }
--                     return 0;
--                 }
--
--                 // Note: Because ImGui_ is a namespace you would typically add your own function into the namespace.
--                 // For example, you code may declare a function 'ImGui_InputText(const char* label, MyString* my_str)'
--                 static bool MyInputTextMultiline(const char* label, ImVector<char>* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
--                 {
--                     IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
--                     return r.ImGui_InputTextMultiline(label, my_str->begin(), (size_t)my_str->size(), size, flags | ImGuiInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
--                 }
--             };
--
--             // For this demo we are using ImVector as a string container.
--             // Note that because we need to store a terminating zero character, our size/capacity are 1 more
--             // than usually reported by a typical string class.
--             static ImVector<char> my_str;
--             if (my_str.empty())
--                 my_str.push_back(0);
--             Funcs::MyInputTextMultiline("##MyStr", &my_str, ImVec2(-FLT_MIN, r.ImGui_GetTextLineHeight() * 16));
--             r.ImGui_Text("Data: %p\nSize: %d\nCapacity: %d", (void*)my_str.begin(), my_str.size(), my_str.capacity());
--             r.ImGui_TreePop();
--         }

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Tabs') then
    if not widgets.tabs then
      widgets.tabs = {
        flags1  = r.ImGui_TabBarFlags_Reorderable(),
        opened  = { true, true, true, true },
        flags2  = r.ImGui_TabBarFlags_AutoSelectNewTabs() |
                  r.ImGui_TabBarFlags_Reorderable() |
                  r.ImGui_TabBarFlags_FittingPolicyResizeDown(),
        active  = { 1, 2, 3 },
        next_id = 4,
        show_leading_button  = true,
        show_trailing_button = true,
      }
    end

    local fitting_policy_mask = r.ImGui_TabBarFlags_FittingPolicyResizeDown() |
                                r.ImGui_TabBarFlags_FittingPolicyScroll()

    if r.ImGui_TreeNode(ctx, 'Basic') then
      if r.ImGui_BeginTabBar(ctx, 'MyTabBar', r.ImGui_TabBarFlags_None()) then
        if r.ImGui_BeginTabItem(ctx, 'Avocado') then
          r.ImGui_Text(ctx, 'This is the Avocado tab!\nblah blah blah blah blah')
          r.ImGui_EndTabItem(ctx)
        end
        if r.ImGui_BeginTabItem(ctx, 'Broccoli') then
          r.ImGui_Text(ctx, 'This is the Broccoli tab!\nblah blah blah blah blah')
          r.ImGui_EndTabItem(ctx)
        end
        if r.ImGui_BeginTabItem(ctx, 'Cucumber') then
          r.ImGui_Text(ctx, 'This is the Cucumber tab!\nblah blah blah blah blah')
          r.ImGui_EndTabItem(ctx)
        end
        r.ImGui_EndTabBar(ctx)
      end
      r.ImGui_Separator(ctx)
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Advanced & Close Button') then
      -- Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
      rv,widgets.tabs.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_Reorderable', widgets.tabs.flags1, r.ImGui_TabBarFlags_Reorderable())
      rv,widgets.tabs.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_AutoSelectNewTabs', widgets.tabs.flags1, r.ImGui_TabBarFlags_AutoSelectNewTabs())
      rv,widgets.tabs.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_TabListPopupButton', widgets.tabs.flags1, r.ImGui_TabBarFlags_TabListPopupButton())
      rv,widgets.tabs.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_NoCloseWithMiddleMouseButton', widgets.tabs.flags1, r.ImGui_TabBarFlags_NoCloseWithMiddleMouseButton())

      if widgets.tabs.flags1 & fitting_policy_mask == 0 then
        widgets.tabs.flags1 = widgets.tabs.flags1 | r.ImGui_TabBarFlags_FittingPolicyResizeDown() -- was FittingPolicyDefault_
      end
      if r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyResizeDown', widgets.tabs.flags1, r.ImGui_TabBarFlags_FittingPolicyResizeDown()) then
        widgets.tabs.flags1 = widgets.tabs.flags1 & ~fitting_policy_mask | r.ImGui_TabBarFlags_FittingPolicyResizeDown()
      end
      if r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyScroll', widgets.tabs.flags1, r.ImGui_TabBarFlags_FittingPolicyScroll()) then
        widgets.tabs.flags1 = widgets.tabs.flags1 & ~fitting_policy_mask | r.ImGui_TabBarFlags_FittingPolicyScroll()
      end

      -- Tab Bar
      local names = { 'Artichoke', 'Beetroot', 'Celery', 'Daikon' }
      for n,opened in ipairs(widgets.tabs.opened) do
        if n > 1 then r.ImGui_SameLine(ctx); end
        rv,widgets.tabs.opened[n] = r.ImGui_Checkbox(ctx, names[n], opened)
      end

      -- Passing a bool* to BeginTabItem() is similar to passing one to Begin():
      -- the underlying bool will be set to false when the tab is closed.
      if r.ImGui_BeginTabBar(ctx, 'MyTabBar', widgets.tabs.flags1) then
        for n,opened in ipairs(widgets.tabs.opened) do
          if opened then
            rv,widgets.tabs.opened[n] = r.ImGui_BeginTabItem(ctx, names[n], opened, r.ImGui_TabItemFlags_None())
            if rv then
              r.ImGui_Text(ctx, ('This is the %s tab!'):format(names[n]))
              if n & 1 then
                r.ImGui_Text(ctx, 'I am an odd tab.')
              end
              r.ImGui_EndTabItem(ctx)
            end
          end
        end
        r.ImGui_EndTabBar(ctx)
      end
      r.ImGui_Separator(ctx)
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'TabItemButton & Leading/Trailing flags') then
      -- TabItemButton() and Leading/Trailing flags are distinct features which we will demo together.
      -- (It is possible to submit regular tabs with Leading/Trailing flags, or TabItemButton tabs without Leading/Trailing flags...
      -- but they tend to make more sense together)
      rv,widgets.tabs.show_leading_button = r.ImGui_Checkbox(ctx, 'Show Leading TabItemButton()', widgets.tabs.show_leading_button)
      rv,widgets.tabs.show_trailing_button = r.ImGui_Checkbox(ctx, 'Show Trailing TabItemButton()', widgets.tabs.show_trailing_button)

      -- Expose some other flags which are useful to showcase how they interact with Leading/Trailing tabs
      rv,widgets.tabs.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_TabListPopupButton', widgets.tabs.flags2, r.ImGui_TabBarFlags_TabListPopupButton())
      if r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyResizeDown', widgets.tabs.flags2, r.ImGui_TabBarFlags_FittingPolicyResizeDown()) then
        widgets.tabs.flags2 = widgets.tabs.flags2 & ~fitting_policy_mask | r.ImGui_TabBarFlags_FittingPolicyResizeDown()
      end
      if r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyScroll', widgets.tabs.flags2, r.ImGui_TabBarFlags_FittingPolicyScroll()) then
        widgets.tabs.flags2 = widgets.tabs.flags2 & ~fitting_policy_mask | r.ImGui_TabBarFlags_FittingPolicyScroll()
      end

      if r.ImGui_BeginTabBar(ctx, 'MyTabBar', widgets.tabs.flags2) then
        -- Demo a Leading TabItemButton(): click the '?' button to open a menu
        if widgets.tabs.show_leading_button then
          if r.ImGui_TabItemButton(ctx, '?', r.ImGui_TabItemFlags_Leading() | r.ImGui_TabItemFlags_NoTooltip()) then
            r.ImGui_OpenPopup(ctx, 'MyHelpMenu')
          end
        end
        if r.ImGui_BeginPopup(ctx, 'MyHelpMenu') then
          r.ImGui_Selectable(ctx, 'Hello!')
          r.ImGui_EndPopup(ctx)
        end

        -- Demo Trailing Tabs: click the "+" button to add a new tab (in your app you may want to use a font icon instead of the "+")
        -- Note that we submit it before the regular tabs, but because of the ImGuiTabItemFlags_Trailing flag it will always appear at the end.
        if widgets.tabs.show_trailing_button then
          if r.ImGui_TabItemButton(ctx, '+', r.ImGui_TabItemFlags_Trailing() | r.ImGui_TabItemFlags_NoTooltip()) then
            -- add new tab
            table.insert(widgets.tabs.active, widgets.tabs.next_id)
            widgets.tabs.next_id = widgets.tabs.next_id + 1
          end
        end

        -- Submit our regular tabs
        local n = 1
        while n <= #widgets.tabs.active do
          local open = true
          local name = ('%04d'):format(widgets.tabs.active[n]-1)
          rv,open = r.ImGui_BeginTabItem(ctx, name, open, r.ImGui_TabItemFlags_None())
          if rv then
            r.ImGui_Text(ctx, ('This is the %s tab!'):format(name))
            r.ImGui_EndTabItem(ctx)
          end

          if not open then
            table.remove(widgets.tabs.active, n)
          else
            n = n + 1
          end
        end

        r.ImGui_EndTabBar(ctx)
      end
      r.ImGui_Separator(ctx)
      r.ImGui_TreePop(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Plots Widgets') then
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

    rv,widgets.plots.animate = r.ImGui_Checkbox(ctx, 'Animate', widgets.plots.animate)

    r.ImGui_PlotLines(ctx, 'Frame Times', widgets.plots.frame_times)

    -- Fill an array of contiguous float values to plot
    if not widgets.plots.animate or widgets.plots.plot1.refresh_time == 0.0 then
      widgets.plots.plot1.refresh_time = r.ImGui_GetTime(ctx)
    end
    while widgets.plots.plot1.refresh_time < r.ImGui_GetTime(ctx) do -- Create data at fixed 60 Hz rate for the demo
      widgets.plots.plot1.data[widgets.plots.plot1.offset] = math.cos(widgets.plots.plot1.phase)
      widgets.plots.plot1.offset = (widgets.plots.plot1.offset % PLOT1_SIZE) + 1
      widgets.plots.plot1.phase = widgets.plots.plot1.phase + (0.10 * widgets.plots.plot1.offset)
      widgets.plots.plot1.refresh_time = widgets.plots.plot1.refresh_time + (1.0 / 60.0)
    end

    -- Plots can display overlay texts
    -- (in this example, we will display an average value)
    local average = 0.0
    for n = 1, PLOT1_SIZE do
      average = average + widgets.plots.plot1.data[n]
    end
    average = average / PLOT1_SIZE

    local overlay = ('avg %f'):format(average)
    r.ImGui_PlotLines(ctx, 'Lines', widgets.plots.plot1.data, widgets.plots.plot1.offset - 1, overlay, -1.0, 1.0, 0, 80.0)

    r.ImGui_PlotHistogram(ctx, 'Histogram', widgets.plots.frame_times, 0, nil, 0.0, 1.0, 0, 80.0)
    r.ImGui_Separator(ctx)

    r.ImGui_SetNextItemWidth(ctx, 100)
    rv,widgets.plots.plot2.func = r.ImGui_Combo(ctx, 'func', widgets.plots.plot2.func, 'Sin\31Saw\31')
    local funcChanged = rv
    r.ImGui_SameLine(ctx)
    rv,widgets.plots.plot2.size = r.ImGui_SliderInt(ctx, 'Sample count', widgets.plots.plot2.size, 1, 400)

    -- Use functions to generate output
    if funcChanged or rv or widgets.plots.plot2.fill then
      widgets.plots.plot2.fill = false -- fill the first time
      widgets.plots.plot2.data = reaper.new_array(widgets.plots.plot2.size)
      for n = 1, widgets.plots.plot2.size do
        widgets.plots.plot2.data[n] = plot2_funcs[widgets.plots.plot2.func + 1](n - 1)
      end
    end

    r.ImGui_PlotLines(ctx, 'Lines', widgets.plots.plot2.data, 0, nil, -1.0, 1.0, 0, 80)
    r.ImGui_PlotHistogram(ctx, 'Histogram', widgets.plots.plot2.data, 0, nil, -1.0, 1.0, 0, 80)
    r.ImGui_Separator(ctx)

    -- Animate a simple progress bar
    if widgets.plots.animate then
      widgets.plots.progress = widgets.plots.progress +
        (widgets.plots.progress_dir * 0.4 * r.ImGui_GetDeltaTime(ctx))
      if widgets.plots.progress >= 1.1 then
        widgets.plots.progress = 1.1
        widgets.plots.progress_dir = widgets.plots.progress_dir * -1
      elseif widgets.plots.progress <= -0.1 then
        widgets.plots.progress = -0.1
        widgets.plots.progress_dir = widgets.plots.progress_dir * -1
      end
    end

    -- Typically we would use (-1.0f,0.0f) or (-FLT_MIN,0.0f) to use all available width,
    -- or (width,0.0f) for a specified width. (0.0f,0.0f) uses ItemWidth.
    r.ImGui_ProgressBar(ctx, widgets.plots.progress, 0.0, 0.0)
    r.ImGui_SameLine(ctx, 0.0, ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemInnerSpacing())})[1])
    r.ImGui_Text(ctx, 'Progress Bar')

    local progress_saturated = demo.clamp(widgets.plots.progress, 0.0, 1.0);
    local buf = ('%d/%d'):format(math.floor(progress_saturated * 1753), 1753)
    r.ImGui_ProgressBar(ctx, widgets.plots.progress, 0.0, 0.0, buf);

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Color/Picker Widgets') then
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
    rv,widgets.colors.alpha_preview      = r.ImGui_Checkbox(ctx, 'With Alpha Preview',      widgets.colors.alpha_preview)
    rv,widgets.colors.alpha_half_preview = r.ImGui_Checkbox(ctx, 'With Half Alpha Preview', widgets.colors.alpha_half_preview)
    rv,widgets.colors.drag_and_drop      = r.ImGui_Checkbox(ctx, 'With Drag and Drop',      widgets.colors.drag_and_drop)
    rv,widgets.colors.options_menu       = r.ImGui_Checkbox(ctx, 'With Options Menu',       widgets.colors.options_menu)
    r.ImGui_SameLine(ctx); demo.HelpMarker('Right-click on the individual color widget to show options.')
    -- r.ImGui_Checkbox("With HDR", &hdr); r.ImGui_SameLine(); HelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");
    local misc_flags = --(widgets.colors.hdr and r.ImGui_ColorEditFlags_HDR() or 0) |
    (widgets.colors.drag_and_drop and 0 or r.ImGui_ColorEditFlags_NoDragDrop()) |
    (widgets.colors.alpha_half_preview and r.ImGui_ColorEditFlags_AlphaPreviewHalf()
    or (widgets.colors.alpha_preview and r.ImGui_ColorEditFlags_AlphaPreview() or 0)) |
    (widgets.colors.options_menu  and 0 or r.ImGui_ColorEditFlags_NoOptions())

    r.ImGui_Text(ctx, 'Color widget:')
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'Click on the color square to open a color picker.\n\z
       CTRL+click on individual component to input value.\n')
    local argb = demo.RgbaToArgb(widgets.colors.rgba)
    rv,argb = r.ImGui_ColorEdit3(ctx, 'MyColor##1', argb, misc_flags)
    if rv then
      widgets.colors.rgba = demo.ArgbToRgba(argb)
    end

    r.ImGui_Text(ctx, 'Color widget HSV with Alpha:')
    rv,widgets.colors.rgba = r.ImGui_ColorEdit4(ctx, 'MyColor##2', widgets.colors.rgba, r.ImGui_ColorEditFlags_DisplayHSV() | misc_flags)

    r.ImGui_Text(ctx, 'Color widget with Float Display:')
    rv,widgets.colors.rgba = r.ImGui_ColorEdit4(ctx, 'MyColor##2f', widgets.colors.rgba, r.ImGui_ColorEditFlags_Float() | misc_flags)

    r.ImGui_Text(ctx, 'Color button with Picker:')
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'With the ImGuiColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\n\z
       With the ImGuiColorEditFlags_NoLabel flag you can pass a non-empty label which will only \z
       be used for the tooltip and picker popup.')
    rv,widgets.colors.rgba = r.ImGui_ColorEdit4(ctx, 'MyColor##3', widgets.colors.rgba, r.ImGui_ColorEditFlags_NoInputs() | r.ImGui_ColorEditFlags_NoLabel() | misc_flags);

    r.ImGui_Text(ctx, 'Color button with Custom Picker Popup:')

    -- Generate a default palette. The palette will persist and can be edited.
    if not widgets.colors.saved_palette then
      widgets.colors.saved_palette = {}
      for n = 0, 31 do
        local color = r.ImGui_ColorConvertHSVtoRGB(n / 31.0, 0.8, 0.8)
        table.insert(widgets.colors.saved_palette, color)
      end
    end

    local open_popup = r.ImGui_ColorButton(ctx, 'MyColor##3b', widgets.colors.rgba, misc_flags)
    r.ImGui_SameLine(ctx, 0, ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemInnerSpacing())})[1])
    open_popup = r.ImGui_Button(ctx, 'Palette') or open_popup
    if open_popup then
      r.ImGui_OpenPopup(ctx, 'mypicker')
      widgets.colors.backup_color = widgets.colors.rgba
    end
    if r.ImGui_BeginPopup(ctx, 'mypicker') then
      r.ImGui_Text(ctx, 'MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!')
      r.ImGui_Separator(ctx)
      rv,widgets.colors.rgba = r.ImGui_ColorPicker4(ctx, '##picker', widgets.colors.rgba, misc_flags | r.ImGui_ColorEditFlags_NoSidePreview() | r.ImGui_ColorEditFlags_NoSmallPreview())
      r.ImGui_SameLine(ctx)

      r.ImGui_BeginGroup(ctx) -- Lock X position
      r.ImGui_Text(ctx, 'Current')
      r.ImGui_ColorButton(ctx, '##current', widgets.colors.rgba,
        r.ImGui_ColorEditFlags_NoPicker() |
        r.ImGui_ColorEditFlags_AlphaPreviewHalf(), 60, 40)
      r.ImGui_Text(ctx, 'Previous')
      if r.ImGui_ColorButton(ctx, '##previous', widgets.colors.backup_color,
          r.ImGui_ColorEditFlags_NoPicker() |
          r.ImGui_ColorEditFlags_AlphaPreviewHalf(), 60, 40) then
        widgets.colors.rgba = widgets.colors.backup_color
      end
      r.ImGui_Separator(ctx)
      r.ImGui_Text(ctx, 'Palette')
      local palette_button_flags = r.ImGui_ColorEditFlags_NoAlpha()  |
                                   r.ImGui_ColorEditFlags_NoPicker() |
                                   r.ImGui_ColorEditFlags_NoTooltip()
      for n,c in ipairs(widgets.colors.saved_palette) do
        r.ImGui_PushID(ctx, n)
        if ((n - 1) % 8) ~= 0 then
          r.ImGui_SameLine(ctx, 0.0, ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing())})[2])
        end

        if r.ImGui_ColorButton(ctx, '##palette', c, palette_button_flags, 20, 20) then
          widgets.colors.rgba = (c << 8) | (widgets.colors.rgba & 0xFF) -- Preserve alpha!
        end

        -- Allow user to drop colors into each palette entry. Note that ColorButton() is already a
        -- drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
        if r.ImGui_BeginDragDropTarget(ctx) then
          local drop_color
          rv,drop_color = r.ImGui_AcceptDragDropPayloadRGB(ctx)
          if rv then
            widgets.colors.saved_palette[n] = drop_color
          end
          rv,drop_color = r.ImGui_AcceptDragDropPayloadRGBA(ctx)
          if rv then
            widgets.colors.saved_palette[n] = drop_color >> 8
          end
          r.ImGui_EndDragDropTarget(ctx)
        end

        r.ImGui_PopID(ctx)
      end
      r.ImGui_EndGroup(ctx)
      r.ImGui_EndPopup(ctx)
    end

    r.ImGui_Text(ctx, 'Color button only:')
    rv,widgets.colors.no_border = r.ImGui_Checkbox(ctx, 'ImGuiColorEditFlags_NoBorder', widgets.colors.no_border)
    r.ImGui_ColorButton(ctx, 'MyColor##3c', widgets.colors.rgba,
      misc_flags | (widgets.colors.no_border and r.ImGui_ColorEditFlags_NoBorder() or 0),
      80, 80)

    r.ImGui_Text(ctx, 'Color picker:')
    rv,widgets.colors.alpha = r.ImGui_Checkbox(ctx, 'With Alpha', widgets.colors.alpha)
    rv,widgets.colors.alpha_bar = r.ImGui_Checkbox(ctx, 'With Alpha Bar', widgets.colors.alpha_bar)
    rv,widgets.colors.side_preview = r.ImGui_Checkbox(ctx, 'With Side Preview', widgets.colors.side_preview)
    if widgets.colors.side_preview then
      r.ImGui_SameLine(ctx)
      rv,widgets.colors.ref_color = r.ImGui_Checkbox(ctx, 'With Ref Color', widgets.colors.ref_color)
      if widgets.colors.ref_color then
        r.ImGui_SameLine(ctx)
        rv,widgets.colors.ref_color_rgba = r.ImGui_ColorEdit4(ctx, '##RefColor',
          widgets.colors.ref_color_rgba, r.ImGui_ColorEditFlags_NoInputs() | misc_flags)
      end
    end
    rv,widgets.colors.display_mode = r.ImGui_Combo(ctx, 'Display Mode', widgets.colors.display_mode,
      'Auto/Current\31None\31RGB Only\31HSV Only\31Hex Only\31')
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      "ColorEdit defaults to displaying RGB inputs if you don't specify a display mode, \z
       but the user can change it with a right-click.\n\nColorPicker defaults to displaying RGB+HSV+Hex \z
       if you don't specify a display mode.\n\nYou can change the defaults using SetColorEditOptions().");
    rv,widgets.colors.picker_mode = r.ImGui_Combo(ctx, 'Picker Mode', widgets.colors.picker_mode,
      'Auto/Current\31Hue bar + SV rect\31Hue wheel + SV triangle\31')
    r.ImGui_SameLine(ctx); demo.HelpMarker('User can right-click the picker to change mode.');
    local flags = misc_flags
    if not widgets.colors.alpha         then flags = flags | r.ImGui_ColorEditFlags_NoAlpha()        end
    if widgets.colors.alpha_bar         then flags = flags | r.ImGui_ColorEditFlags_AlphaBar()       end
    if not widgets.colors.side_preview  then flags = flags | r.ImGui_ColorEditFlags_NoSidePreview()  end
    if widgets.colors.picker_mode  == 1 then flags = flags | r.ImGui_ColorEditFlags_PickerHueBar()   end
    if widgets.colors.picker_mode  == 2 then flags = flags | r.ImGui_ColorEditFlags_PickerHueWheel() end
    if widgets.colors.display_mode == 1 then flags = flags | r.ImGui_ColorEditFlags_NoInputs()       end -- Disable all RGB/HSV/Hex displays
    if widgets.colors.display_mode == 2 then flags = flags | r.ImGui_ColorEditFlags_DisplayRGB()     end -- Override display mode
    if widgets.colors.display_mode == 3 then flags = flags | r.ImGui_ColorEditFlags_DisplayHSV()     end
    if widgets.colors.display_mode == 4 then flags = flags | r.ImGui_ColorEditFlags_DisplayHex()     end

    local color = widgets.colors.alpha and widgets.colors.rgba or demo.RgbaToArgb(widgets.colors.rgba)
    local ref_color = widgets.colors.alpha and widgets.colors.ref_color_rgba or demo.RgbaToArgb(widgets.colors.ref_color_rgba)
    rv,color = r.ImGui_ColorPicker4(ctx, 'MyColor##4', color, flags,
      widgets.colors.ref_color and ref_color or nil)
    if rv then
      widgets.colors.rgba = widgets.colors.alpha and color or demo.ArgbToRgba(color)
    end

    r.ImGui_Text(ctx, 'Set defaults in code:')
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      "SetColorEditOptions() is designed to allow you to set boot-time default.\n\z
       We don't have Push/Pop functions because you can force options on a per-widget basis if needed,\z
       and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid\z
       encouraging you to persistently save values that aren't forward-compatible.")
    if r.ImGui_Button(ctx, 'Default: Uint8 + HSV + Hue Bar') then
      r.ImGui_SetColorEditOptions(ctx, r.ImGui_ColorEditFlags_Uint8() | r.ImGui_ColorEditFlags_DisplayHSV() | r.ImGui_ColorEditFlags_PickerHueBar())
    end
    if r.ImGui_Button(ctx, 'Default: Float + Hue Wheel') then -- (NOTE: removed HDR for ReaImGui as we use uint32 for color i/o)
      r.ImGui_SetColorEditOptions(ctx, r.ImGui_ColorEditFlags_Float() | r.ImGui_ColorEditFlags_PickerHueWheel())
    end

    -- HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
    r.ImGui_Spacing(ctx)
    r.ImGui_Text(ctx, 'HSV encoded colors')
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'By default, colors are given to ColorEdit and ColorPicker in RGB, but ImGuiColorEditFlags_InputHSV \z
       allows you to store colors as HSV and pass them to ColorEdit and ColorPicker as HSV. This comes with the \z
       added benefit that you can manipulate hue values with the picker even when saturation or value are zero.')
    r.ImGui_Text(ctx, 'Color widget with InputHSV:')
    rv,widgets.colors.hsva = r.ImGui_ColorEdit4(ctx, 'HSV shown as RGB##1', widgets.colors.hsva,
      r.ImGui_ColorEditFlags_DisplayRGB() | r.ImGui_ColorEditFlags_InputHSV() | r.ImGui_ColorEditFlags_Float())
    rv,widgets.colors.hsva = r.ImGui_ColorEdit4(ctx, 'HSV shown as HSV##1', widgets.colors.hsva,
      r.ImGui_ColorEditFlags_DisplayHSV() | r.ImGui_ColorEditFlags_InputHSV() | r.ImGui_ColorEditFlags_Float())

    local raw_hsv = widgets.colors.raw_hsv
    raw_hsv[1] = (widgets.colors.hsva >> 24 & 0xFF) / 255.0 -- H
    raw_hsv[2] = (widgets.colors.hsva >> 16 & 0xFF) / 255.0 -- S
    raw_hsv[3] = (widgets.colors.hsva >>  8 & 0xFF) / 255.0 -- V
    raw_hsv[4] = (widgets.colors.hsva       & 0xFF) / 255.0 -- A
    if r.ImGui_DragDoubleN(ctx, 'Raw HSV values', raw_hsv, 0.01, 0.0, 1.0) then
      widgets.colors.hsva =
        (demo.round(raw_hsv[1] * 0xFF) << 24) |
        (demo.round(raw_hsv[2] * 0xFF) << 16) |
        (demo.round(raw_hsv[3] * 0xFF) <<  8) |
        (demo.round(raw_hsv[4] * 0xFF)      )
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Drag/Slider Flags') then
    if not widgets.sliders then
      widgets.sliders = {
        flags    = r.ImGui_SliderFlags_None(),
        drag_d   = 0.5,
        drag_i   = 50,
        slider_d = 0.5,
        slider_i = 50,
      }
    end

    -- Demonstrate using advanced flags for DragXXX and SliderXXX functions. Note that the flags are the same!
    rv,widgets.sliders.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiSliderFlags_AlwaysClamp', widgets.sliders.flags, r.ImGui_SliderFlags_AlwaysClamp())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Always clamp value to min/max bounds (if any) when input manually with CTRL+Click.')
    rv,widgets.sliders.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiSliderFlags_Logarithmic', widgets.sliders.flags, r.ImGui_SliderFlags_Logarithmic())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Enable logarithmic editing (more precision for small values).')
    rv,widgets.sliders.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiSliderFlags_NoRoundToFormat', widgets.sliders.flags, r.ImGui_SliderFlags_NoRoundToFormat())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Disable rounding underlying value to match precision of the format string (e.g. %.3f values are rounded to those 3 digits).')
    rv,widgets.sliders.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiSliderFlags_NoInput', widgets.sliders.flags, r.ImGui_SliderFlags_NoInput())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Disable CTRL+Click or Enter key allowing to input text directly into the widget.')

    local DBL_MIN, DBL_MAX = 2.22507e-308, 1.79769e+308

    -- Drags
    r.ImGui_Text(ctx, ('Underlying double value: %f'):format(widgets.sliders.drag_d))
    rv,widgets.sliders.drag_d = r.ImGui_DragDouble(ctx, 'DragDouble (0 -> 1)', widgets.sliders.drag_d, 0.005, 0.0, 1.0, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_d = r.ImGui_DragDouble(ctx, 'DragDouble (0 -> +inf)', widgets.sliders.drag_d, 0.005, 0.0, DBL_MAX, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_d = r.ImGui_DragDouble(ctx, 'DragDouble (-inf -> 1)', widgets.sliders.drag_d, 0.005, -DBL_MAX, 1.0, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_d = r.ImGui_DragDouble(ctx, 'DragDouble (-inf -> +inf)', widgets.sliders.drag_d, 0.005, -DBL_MAX, DBL_MAX, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.drag_i = r.ImGui_DragInt(ctx, 'DragInt (0 -> 100)', widgets.sliders.drag_i, 0.5, 0, 100, '%d', widgets.sliders.flags)

    -- Sliders
    r.ImGui_Text(ctx, ('Underlying float value: %f'):format(widgets.sliders.slider_d))
    rv,widgets.sliders.slider_d = r.ImGui_SliderDouble(ctx, 'SliderDouble (0 -> 1)', widgets.sliders.slider_d, 0.0, 1.0, '%.3f', widgets.sliders.flags)
    rv,widgets.sliders.slider_i = r.ImGui_SliderInt(ctx, 'SliderInt (0 -> 100)', widgets.sliders.slider_i, 0, 100, '%d', widgets.sliders.flags)

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Range Widgets') then
    if not widgets.range then
      widgets.range = {
        begin_f = 10.0,
        end_f   = 90.0,
        begin_i = 100,
        end_i   = 1000,
      }
    end

    rv,widgets.range.begin_f,widgets.range.end_f = r.ImGui_DragFloatRange2(ctx, 'range float', widgets.range.begin_f, widgets.range.end_f, 0.25, 0.0, 100.0, 'Min: %.1f %%', "Max: %.1f %%", r.ImGui_SliderFlags_AlwaysClamp())
    rv,widgets.range.begin_i,widgets.range.end_i = r.ImGui_DragIntRange2(ctx, 'range int', widgets.range.begin_i, widgets.range.end_i, 5, 0, 1000, 'Min: %d units', "Max: %d units")
    rv,widgets.range.begin_i,widgets.range.end_i = r.ImGui_DragIntRange2(ctx, 'range int (no bounds)', widgets.range.begin_i, widgets.range.end_i, 5, 0, 0, 'Min: %d units', "Max: %d units")
    r.ImGui_TreePop(ctx)
  end

--     if (r.ImGui_TreeNode("Data Types"))
--     {
--         // DragScalar/InputScalar/SliderScalar functions allow various data types
--         // - signed/unsigned
--         // - 8/16/32/64-bits
--         // - integer/float/double
--         // To avoid polluting the public API with all possible combinations, we use the ImGuiDataType enum
--         // to pass the type, and passing all arguments by pointer.
--         // This is the reason the test code below creates local variables to hold "zero" "one" etc. for each types.
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
--         r.ImGui_Text("Drags:");
--         r.ImGui_Checkbox("Clamp integers to 0..50", &drag_clamp);
--         r.ImGui_SameLine(); HelpMarker(
--             "As with every widgets in dear imgui, we never modify values unless there is a user interaction.\n"
--             "You can override the clamping limits by using CTRL+Click to input a value.");
--         r.ImGui_DragScalar("drag s8",        ImGuiDataType_S8,     &s8_v,  drag_speed, drag_clamp ? &s8_zero  : NULL, drag_clamp ? &s8_fifty  : NULL);
--         r.ImGui_DragScalar("drag u8",        ImGuiDataType_U8,     &u8_v,  drag_speed, drag_clamp ? &u8_zero  : NULL, drag_clamp ? &u8_fifty  : NULL, "%u ms");
--         r.ImGui_DragScalar("drag s16",       ImGuiDataType_S16,    &s16_v, drag_speed, drag_clamp ? &s16_zero : NULL, drag_clamp ? &s16_fifty : NULL);
--         r.ImGui_DragScalar("drag u16",       ImGuiDataType_U16,    &u16_v, drag_speed, drag_clamp ? &u16_zero : NULL, drag_clamp ? &u16_fifty : NULL, "%u ms");
--         r.ImGui_DragScalar("drag s32",       ImGuiDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL);
--         r.ImGui_DragScalar("drag u32",       ImGuiDataType_U32,    &u32_v, drag_speed, drag_clamp ? &u32_zero : NULL, drag_clamp ? &u32_fifty : NULL, "%u ms");
--         r.ImGui_DragScalar("drag s64",       ImGuiDataType_S64,    &s64_v, drag_speed, drag_clamp ? &s64_zero : NULL, drag_clamp ? &s64_fifty : NULL);
--         r.ImGui_DragScalar("drag u64",       ImGuiDataType_U64,    &u64_v, drag_speed, drag_clamp ? &u64_zero : NULL, drag_clamp ? &u64_fifty : NULL);
--         r.ImGui_DragScalar("drag float",     ImGuiDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f");
--         r.ImGui_DragScalar("drag float log", ImGuiDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f", ImGuiSliderFlags_Logarithmic);
--         r.ImGui_DragScalar("drag double",    ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, NULL,     "%.10f grams");
--         r.ImGui_DragScalar("drag double log",ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, &f64_one, "0 < %.10f < 1", ImGuiSliderFlags_Logarithmic);
--
--         r.ImGui_Text("Sliders");
--         r.ImGui_SliderScalar("slider s8 full",       ImGuiDataType_S8,     &s8_v,  &s8_min,   &s8_max,   "%d");
--         r.ImGui_SliderScalar("slider u8 full",       ImGuiDataType_U8,     &u8_v,  &u8_min,   &u8_max,   "%u");
--         r.ImGui_SliderScalar("slider s16 full",      ImGuiDataType_S16,    &s16_v, &s16_min,  &s16_max,  "%d");
--         r.ImGui_SliderScalar("slider u16 full",      ImGuiDataType_U16,    &u16_v, &u16_min,  &u16_max,  "%u");
--         r.ImGui_SliderScalar("slider s32 low",       ImGuiDataType_S32,    &s32_v, &s32_zero, &s32_fifty,"%d");
--         r.ImGui_SliderScalar("slider s32 high",      ImGuiDataType_S32,    &s32_v, &s32_hi_a, &s32_hi_b, "%d");
--         r.ImGui_SliderScalar("slider s32 full",      ImGuiDataType_S32,    &s32_v, &s32_min,  &s32_max,  "%d");
--         r.ImGui_SliderScalar("slider u32 low",       ImGuiDataType_U32,    &u32_v, &u32_zero, &u32_fifty,"%u");
--         r.ImGui_SliderScalar("slider u32 high",      ImGuiDataType_U32,    &u32_v, &u32_hi_a, &u32_hi_b, "%u");
--         r.ImGui_SliderScalar("slider u32 full",      ImGuiDataType_U32,    &u32_v, &u32_min,  &u32_max,  "%u");
--         r.ImGui_SliderScalar("slider s64 low",       ImGuiDataType_S64,    &s64_v, &s64_zero, &s64_fifty,"%I64d");
--         r.ImGui_SliderScalar("slider s64 high",      ImGuiDataType_S64,    &s64_v, &s64_hi_a, &s64_hi_b, "%I64d");
--         r.ImGui_SliderScalar("slider s64 full",      ImGuiDataType_S64,    &s64_v, &s64_min,  &s64_max,  "%I64d");
--         r.ImGui_SliderScalar("slider u64 low",       ImGuiDataType_U64,    &u64_v, &u64_zero, &u64_fifty,"%I64u ms");
--         r.ImGui_SliderScalar("slider u64 high",      ImGuiDataType_U64,    &u64_v, &u64_hi_a, &u64_hi_b, "%I64u ms");
--         r.ImGui_SliderScalar("slider u64 full",      ImGuiDataType_U64,    &u64_v, &u64_min,  &u64_max,  "%I64u ms");
--         r.ImGui_SliderScalar("slider float low",     ImGuiDataType_Float,  &f32_v, &f32_zero, &f32_one);
--         r.ImGui_SliderScalar("slider float low log", ImGuiDataType_Float,  &f32_v, &f32_zero, &f32_one,  "%.10f", ImGuiSliderFlags_Logarithmic);
--         r.ImGui_SliderScalar("slider float high",    ImGuiDataType_Float,  &f32_v, &f32_lo_a, &f32_hi_a, "%e");
--         r.ImGui_SliderScalar("slider double low",    ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f grams");
--         r.ImGui_SliderScalar("slider double low log",ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f", ImGuiSliderFlags_Logarithmic);
--         r.ImGui_SliderScalar("slider double high",   ImGuiDataType_Double, &f64_v, &f64_lo_a, &f64_hi_a, "%e grams");
--
--         r.ImGui_Text("Sliders (reverse)");
--         r.ImGui_SliderScalar("slider s8 reverse",    ImGuiDataType_S8,   &s8_v,  &s8_max,    &s8_min, "%d");
--         r.ImGui_SliderScalar("slider u8 reverse",    ImGuiDataType_U8,   &u8_v,  &u8_max,    &u8_min, "%u");
--         r.ImGui_SliderScalar("slider s32 reverse",   ImGuiDataType_S32,  &s32_v, &s32_fifty, &s32_zero, "%d");
--         r.ImGui_SliderScalar("slider u32 reverse",   ImGuiDataType_U32,  &u32_v, &u32_fifty, &u32_zero, "%u");
--         r.ImGui_SliderScalar("slider s64 reverse",   ImGuiDataType_S64,  &s64_v, &s64_fifty, &s64_zero, "%I64d");
--         r.ImGui_SliderScalar("slider u64 reverse",   ImGuiDataType_U64,  &u64_v, &u64_fifty, &u64_zero, "%I64u ms");
--
--         static bool inputs_step = true;
--         r.ImGui_Text("Inputs");
--         r.ImGui_Checkbox("Show step buttons", &inputs_step);
--         r.ImGui_InputScalar("input s8",      ImGuiDataType_S8,     &s8_v,  inputs_step ? &s8_one  : NULL, NULL, "%d");
--         r.ImGui_InputScalar("input u8",      ImGuiDataType_U8,     &u8_v,  inputs_step ? &u8_one  : NULL, NULL, "%u");
--         r.ImGui_InputScalar("input s16",     ImGuiDataType_S16,    &s16_v, inputs_step ? &s16_one : NULL, NULL, "%d");
--         r.ImGui_InputScalar("input u16",     ImGuiDataType_U16,    &u16_v, inputs_step ? &u16_one : NULL, NULL, "%u");
--         r.ImGui_InputScalar("input s32",     ImGuiDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%d");
--         r.ImGui_InputScalar("input s32 hex", ImGuiDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
--         r.ImGui_InputScalar("input u32",     ImGuiDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%u");
--         r.ImGui_InputScalar("input u32 hex", ImGuiDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
--         r.ImGui_InputScalar("input s64",     ImGuiDataType_S64,    &s64_v, inputs_step ? &s64_one : NULL);
--         r.ImGui_InputScalar("input u64",     ImGuiDataType_U64,    &u64_v, inputs_step ? &u64_one : NULL);
--         r.ImGui_InputScalar("input float",   ImGuiDataType_Float,  &f32_v, inputs_step ? &f32_one : NULL);
--         r.ImGui_InputScalar("input double",  ImGuiDataType_Double, &f64_v, inputs_step ? &f64_one : NULL);
--
--         r.ImGui_TreePop();
--     }

  if r.ImGui_TreeNode(ctx, 'Multi-component Widgets') then
    if not widgets.multi_component then
      widgets.multi_component = {
        vec4d = { 0.10, 0.20, 0.30, 0.44 },
        vec4i = { 1, 5, 100, 255 },
        vec4a = reaper.new_array({ 0.10, 0.20, 0.30, 0.44 }),
      }
    end

    local vec4d = widgets.multi_component.vec4d
    local vec4i = widgets.multi_component.vec4i

    rv,vec4d[1],vec4d[2] = r.ImGui_InputDouble2(ctx, 'input double2', vec4d[1], vec4d[2])
    rv,vec4d[1],vec4d[2] = r.ImGui_DragDouble2(ctx, 'drag double2', vec4d[1], vec4d[2], 0.01, 0.0, 1.0)
    rv,vec4d[1],vec4d[2] = r.ImGui_SliderDouble2(ctx, 'slider double2', vec4d[1], vec4d[2], 0.0, 1.0)
    rv,vec4i[1],vec4i[2] = r.ImGui_InputInt2(ctx, 'input int2', vec4i[1], vec4i[2])
    rv,vec4i[1],vec4i[2] = r.ImGui_DragInt2(ctx, 'drag int2', vec4i[1], vec4i[2], 1, 0, 255)
    rv,vec4i[1],vec4i[2] = r.ImGui_SliderInt2(ctx, 'slider int2', vec4i[1], vec4i[2], 0, 255)
    r.ImGui_Spacing(ctx)

    rv,vec4d[1],vec4d[2],vec4d[3] = r.ImGui_InputDouble3(ctx, 'input double3', vec4d[1], vec4d[2], vec4d[3])
    rv,vec4d[1],vec4d[2],vec4d[3] = r.ImGui_DragDouble3(ctx, 'drag double3', vec4d[1], vec4d[2], vec4d[3], 0.01, 0.0, 1.0)
    rv,vec4d[1],vec4d[2],vec4d[3] = r.ImGui_SliderDouble3(ctx, 'slider double3', vec4d[1], vec4d[2], vec4d[3], 0.0, 1.0)
    rv,vec4i[1],vec4i[2],vec4i[3] = r.ImGui_InputInt3(ctx, 'input int3', vec4i[1], vec4i[2], vec4i[3])
    rv,vec4i[1],vec4i[2],vec4i[3] = r.ImGui_DragInt3(ctx, 'drag int3', vec4i[1], vec4i[2], vec4i[3], 1, 0, 255)
    rv,vec4i[1],vec4i[2],vec4i[3] = r.ImGui_SliderInt3(ctx, 'slider int3', vec4i[1], vec4i[2], vec4i[3], 0, 255)
    r.ImGui_Spacing(ctx)

    rv,vec4d[1],vec4d[2],vec4d[3],vec4d[4] = r.ImGui_InputDouble4(ctx, 'input double4', vec4d[1], vec4d[2], vec4d[3], vec4d[4])
    rv,vec4d[1],vec4d[2],vec4d[3],vec4d[4] = r.ImGui_DragDouble4(ctx, 'drag double4', vec4d[1], vec4d[2], vec4d[3], vec4d[4], 0.01, 0.0, 1.0)
    rv,vec4d[1],vec4d[2],vec4d[3],vec4d[4] = r.ImGui_SliderDouble4(ctx, 'slider double4', vec4d[1], vec4d[2], vec4d[3], vec4d[4], 0.0, 1.0)
    rv,vec4i[1],vec4i[2],vec4i[3],vec4i[4] = r.ImGui_InputInt4(ctx, 'input int4', vec4i[1], vec4i[2], vec4i[3], vec4i[4])
    rv,vec4i[1],vec4i[2],vec4i[3],vec4i[4] = r.ImGui_DragInt4(ctx, 'drag int4', vec4i[1], vec4i[2], vec4i[3], vec4i[4], 1, 0, 255)
    rv,vec4i[1],vec4i[2],vec4i[3],vec4i[4] = r.ImGui_SliderInt4(ctx, 'slider int4', vec4i[1], vec4i[2], vec4i[3], vec4i[4], 0, 255)
    r.ImGui_Spacing(ctx)

    r.ImGui_InputDoubleN(ctx, 'input reaper.array', widgets.multi_component.vec4a)
    r.ImGui_DragDoubleN(ctx, 'drag reaper.array', widgets.multi_component.vec4a, 0.01, 0.0, 1.0)
    r.ImGui_SliderDoubleN(ctx, 'slider reaper.array', widgets.multi_component.vec4a, 0.0, 1.0)

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Vertical Sliders') then
    if not widgets.vsliders then
      widgets.vsliders = {
        int_value = 0,
        values    = { 0.0,  0.60, 0.35, 0.9, 0.70, 0.20, 0.0 },
        values2   = { 0.20, 0.80, 0.40, 0.25 },
      }
    end

    local spacing = 4
    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing(), spacing, spacing)

    rv,widgets.vsliders.int_value = r.ImGui_VSliderInt(ctx, '##int', 18, 160, widgets.vsliders.int_value, 0, 5)
    r.ImGui_SameLine(ctx)

    r.ImGui_PushID(ctx, 'set1')
    for i,v in ipairs(widgets.vsliders.values) do
      if i > 1 then r.ImGui_SameLine(ctx) end
      r.ImGui_PushID(ctx, i)
      local frameBg        = reaper.ImGui_ColorConvertHSVtoRGB((i-1) / 7.0, 0.5, 0.5, 1.0)
      local frameBgHovered = reaper.ImGui_ColorConvertHSVtoRGB((i-1) / 7.0, 0.6, 0.5, 1.0)
      local frameBgActive  = reaper.ImGui_ColorConvertHSVtoRGB((i-1) / 7.0, 0.7, 0.5, 1.0)
      local sliderGrab     = reaper.ImGui_ColorConvertHSVtoRGB((i-1) / 7.0, 0.9, 0.9, 1.0)
      r.ImGui_PushStyleColor(ctx, r.ImGui_Col_FrameBg(), frameBg)
      r.ImGui_PushStyleColor(ctx, r.ImGui_Col_FrameBgHovered(), frameBgHovered)
      r.ImGui_PushStyleColor(ctx, r.ImGui_Col_FrameBgActive(), frameBgActive)
      r.ImGui_PushStyleColor(ctx, r.ImGui_Col_SliderGrab(), sliderGrab)
      rv,widgets.vsliders.values[i] = r.ImGui_VSliderDouble(ctx, '##v', 18, 160, v, 0.0, 1.0, ' ')
      if r.ImGui_IsItemActive(ctx) or r.ImGui_IsItemHovered(ctx) then
        r.ImGui_SetTooltip(ctx, ('%.3f'):format(v))
      end
      r.ImGui_PopStyleColor(ctx, 4)
      r.ImGui_PopID(ctx)
    end
    r.ImGui_PopID(ctx)

    r.ImGui_SameLine(ctx)
    r.ImGui_PushID(ctx, 'set2')
    local rows = 3
    local small_slider_w, small_slider_h = 18, (160.0 - (rows - 1) * spacing) / rows
    for nx,v2 in ipairs(widgets.vsliders.values2) do
      if nx > 1 then r.ImGui_SameLine(ctx) end
      r.ImGui_BeginGroup(ctx)
      for ny = 0, rows - 1 do
        r.ImGui_PushID(ctx, nx * rows + ny)
        rv,v2 = r.ImGui_VSliderDouble(ctx, '##v', small_slider_w, small_slider_h, v2, 0.0, 1.0, ' ')
        if rv then
          widgets.vsliders.values2[nx] = v2
        end
        if r.ImGui_IsItemActive(ctx) or r.ImGui_IsItemHovered(ctx) then
          r.ImGui_SetTooltip(ctx, ('%.3f'):format(v2))
        end
        r.ImGui_PopID(ctx)
      end
      r.ImGui_EndGroup(ctx)
    end
    r.ImGui_PopID(ctx)

    r.ImGui_SameLine(ctx)
    r.ImGui_PushID(ctx, 'set3')
    for i = 1, 4 do
      local v = widgets.vsliders.values[i]
      if i > 1 then r.ImGui_SameLine(ctx) end
      r.ImGui_PushID(ctx, i)
      r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_GrabMinSize(), 40)
      rv,widgets.vsliders.values[i] = r.ImGui_VSliderDouble(ctx, '##v', 40, 160, v, 0.0, 1.0, '%.2f\nsec')
      r.ImGui_PopStyleVar(ctx)
      r.ImGui_PopID(ctx)
    end
    r.ImGui_PopID(ctx)
    r.ImGui_PopStyleVar(ctx)
    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Drag and Drop') then
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
      }
    end

    if r.ImGui_TreeNode(ctx, 'Drag and drop in standard widgets') then
      -- ColorEdit widgets automatically act as drag source and drag target.
      -- They are using standardized payload types accessible using
      -- ImGui_AcceptDragDropPayloadRGB or ImGui_AcceptDragDropPayloadRGBA
      -- to allow your own widgets to use colors in their drag and drop interaction.
      -- Also see 'Demo->Widgets->Color/Picker Widgets->Palette' demo.
      demo.HelpMarker('You can drag from the color squares.')
      rv,widgets.dragdrop.color1 = r.ImGui_ColorEdit3(ctx, 'color 1', widgets.dragdrop.color1)
      rv,widgets.dragdrop.color2 = r.ImGui_ColorEdit4(ctx, 'color 2', widgets.dragdrop.color2)
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Drag and drop to copy/swap items') then
      local mode_copy, mode_move, mode_swap = 0, 1, 2
      if r.ImGui_RadioButton(ctx, 'Copy', widgets.dragdrop.mode == mode_copy) then widgets.dragdrop.mode = mode_copy end r.ImGui_SameLine(ctx)
      if r.ImGui_RadioButton(ctx, 'Move', widgets.dragdrop.mode == mode_move) then widgets.dragdrop.mode = mode_move end r.ImGui_SameLine(ctx)
      if r.ImGui_RadioButton(ctx, 'Swap', widgets.dragdrop.mode == mode_swap) then widgets.dragdrop.mode = mode_swap end
      for n,name in ipairs(widgets.dragdrop.names) do
        r.ImGui_PushID(ctx, n)
        if ((n-1) % 3) ~= 0 then
          r.ImGui_SameLine(ctx)
        end
        r.ImGui_Button(ctx, name, 60, 60)

        -- Our buttons are both drag sources and drag targets here!
        if r.ImGui_BeginDragDropSource(ctx, r.ImGui_DragDropFlags_None()) then
          -- Set payload to carry the index of our item (could be anything)
          r.ImGui_SetDragDropPayload(ctx, 'DND_DEMO_CELL', tostring(n))

          -- Display preview (could be anything, e.g. when dragging an image we could decide to display
          -- the filename and a small preview of the image, etc.)
          if widgets.dragdrop.mode == mode_copy then r.ImGui_Text(ctx, ('Copy %s'):format(name)) end
          if widgets.dragdrop.mode == mode_move then r.ImGui_Text(ctx, ('Move %s'):format(name)) end
          if widgets.dragdrop.mode == mode_swap then r.ImGui_Text(ctx, ('Swap %s'):format(name)) end
          r.ImGui_EndDragDropSource(ctx)
        end
        if r.ImGui_BeginDragDropTarget(ctx) then
          local payload
          rv,payload = r.ImGui_AcceptDragDropPayload(ctx, 'DND_DEMO_CELL')
          if rv then
            local payload_n = tonumber(payload)
            if widgets.dragdrop.mode == mode_copy then
              widgets.dragdrop.names[n] = widgets.dragdrop.names[payload_n]
            end
            if widgets.dragdrop.mode == mode_move then
              widgets.dragdrop.names[n] = widgets.dragdrop.names[payload_n]
              widgets.dragdrop.names[payload_n] = '';
            end
            if widgets.dragdrop.mode == mode_swap then
              widgets.dragdrop.names[n] = widgets.dragdrop.names[payload_n]
              widgets.dragdrop.names[payload_n] = name
            end
          end
          r.ImGui_EndDragDropTarget(ctx)
        end
        r.ImGui_PopID(ctx)
      end
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Drag to reorder items (simple)') then
      -- Simple reordering
      demo.HelpMarker(
        "We don't use the drag and drop api at all here! \z
         Instead we query when the item is held but not hovered, and order items accordingly.")
      for n,item in ipairs(widgets.dragdrop.items) do
        r.ImGui_Selectable(ctx, item)

        if r.ImGui_IsItemActive(ctx) and not r.ImGui_IsItemHovered(ctx) then
          local n_next = n + (({r.ImGui_GetMouseDragDelta(ctx, r.ImGui_MouseButton_Left())})[2] < 0 and -1 or 1)
          if n_next >= 1 and n_next < #widgets.dragdrop.items then
            widgets.dragdrop.items[n] = widgets.dragdrop.items[n_next]
            widgets.dragdrop.items[n_next] = item
            r.ImGui_ResetMouseDragDelta(ctx, r.ImGui_MouseButton_Left())
          end
        end
      end
      r.ImGui_TreePop(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Querying Status (Edited/Active/Focused/Hovered etc.)') then
    if not widgets.query then
      widgets.query = {
        item_type   = 1,
        b           = false,
        color       = 0xFF8000FF,
        str         = '',
        current     = 1,
        d4a         = { 1.0, 0.5, 0.0, 1.0 },
        embed_all_inside_a_child_window = false,
        test_window = false,
      }
    end

    -- Select an item type
    rv,widgets.query.item_type = r.ImGui_Combo(ctx, 'Item Type', widgets.query.item_type,
      'Text\31Button\31Button (w/ repeat)\31Checkbox\31SliderDouble\31\z
       InputText\31InputDouble\31InputDouble3\31ColorEdit\31MenuItem\31\z
       TreeNode\31TreeNode (w/ double-click)\31Combo\31ListBox\31')

    r.ImGui_SameLine(ctx)
    demo.HelpMarker(
      'Testing how various types of items are interacting with the IsItemXXX \z
       functions. Note that the bool return value of most ImGui function is \z
       generally equivalent to calling r.ImGui_IsItemHovered().')

    -- Submit selected item item so we can query their status in the code following it.
    local item_type = widgets.query.item_type
    if item_type == 0  then -- Testing text items with no identifier/interaction
      r.ImGui_Text(ctx, 'ITEM: Text')
    end
    if item_type == 1  then -- Testing button
      rv = r.ImGui_Button(ctx, 'ITEM: Button')
    end
    if item_type == 2  then -- Testing button (with repeater)
      r.ImGui_PushButtonRepeat(ctx, true)
      rv = r.ImGui_Button(ctx, 'ITEM: Button')
      r.ImGui_PopButtonRepeat(ctx)
    end
    if item_type == 3  then -- Testing checkbox
      rv,widgets.query.b = r.ImGui_Checkbox(ctx, 'ITEM: Checkbox', widgets.query.b)
    end
    if item_type == 4  then -- Testing basic item
      rv,widgets.query.d4a[1] = r.ImGui_SliderDouble(ctx, 'ITEM: SliderDouble', widgets.query.d4a[1], 0.0, 1.0)
    end
    if item_type == 5  then -- Testing input text (which handles tabbing)
      rv,widgets.query.str = r.ImGui_InputText(ctx, 'ITEM: InputText', widgets.query.str)
    end
    if item_type == 6  then -- Testing +/- buttons on scalar input
      rv,widgets.query.d4a[1] = r.ImGui_InputDouble(ctx, 'ITEM: InputDouble', widgets.query.d4a[1], 1.0)
    end
    if item_type == 7  then -- Testing multi-component items (IsItemXXX flags are reported merged)
      local d4a = widgets.query.d4a
      rv,d4a[1],d4a[2],d4a[3] = r.ImGui_InputDouble3(ctx, 'ITEM: InputDouble3', d4a[1], d4a[2], d4a[3])
    end
    if item_type == 8  then -- Testing multi-component items (IsItemXXX flags are reported merged)
      rv,widgets.query.color = r.ImGui_ColorEdit4(ctx, 'ITEM: ColorEdit', widgets.query.color)
    end
    if item_type == 9  then -- Testing menu item (they use ImGuiButtonFlags_PressedOnRelease button policy)
      rv = r.ImGui_MenuItem(ctx, 'ITEM: MenuItem')
    end
    if item_type == 10 then -- Testing tree node
      rv = r.ImGui_TreeNode(ctx, 'ITEM: TreeNode')
      if rv then r.ImGui_TreePop(ctx) end
    end
    if item_type == 11 then -- Testing tree node with ImGuiButtonFlags_PressedOnDoubleClick button policy.
      rv = r.ImGui_TreeNode(ctx, 'ITEM: TreeNode w/ ImGuiTreeNodeFlags_OpenOnDoubleClick',
        r.ImGui_TreeNodeFlags_OpenOnDoubleClick() | r.ImGui_TreeNodeFlags_NoTreePushOnOpen())
    end
    if item_type == 12 then
      rv,widgets.query.current = r.ImGui_Combo(ctx, 'ITEM: Combo', widgets.query.current, 'Apple\31Banana\31Cherry\31Kiwi\31')
    end
    if item_type == 13 then
      rv,widgets.query.current = r.ImGui_ListBox(ctx, 'ITEM: ListBox', widgets.query.current, 'Apple\31Banana\31Cherry\31Kiwi\31')
    end

    -- Display the values of IsItemHovered() and other common item state functions.
    -- Note that the ImGuiHoveredFlags_XXX flags can be combined.
    -- Because BulletText is an item itself and that would affect the output of IsItemXXX functions,
    -- we query every state in a single call to avoid storing them and to simplify the code.
    r.ImGui_BulletText(ctx, ([[Return value = %s
IsItemFocused() = %s
IsItemHovered() = %s
IsItemHovered(_AllowWhenBlockedByPopup) = %s
IsItemHovered(_AllowWhenBlockedByActiveItem) = %s
IsItemHovered(_AllowWhenOverlapped) = %s
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
      r.ImGui_IsItemFocused(ctx),
      r.ImGui_IsItemHovered(ctx),
      r.ImGui_IsItemHovered(ctx, r.ImGui_HoveredFlags_AllowWhenBlockedByPopup()),
      r.ImGui_IsItemHovered(ctx, r.ImGui_HoveredFlags_AllowWhenBlockedByActiveItem()),
      r.ImGui_IsItemHovered(ctx, r.ImGui_HoveredFlags_AllowWhenOverlapped()),
      r.ImGui_IsItemHovered(ctx, r.ImGui_HoveredFlags_RectOnly()),
      r.ImGui_IsItemActive(ctx),
      r.ImGui_IsItemEdited(ctx),
      r.ImGui_IsItemActivated(ctx),
      r.ImGui_IsItemDeactivated(ctx),
      r.ImGui_IsItemDeactivatedAfterEdit(ctx),
      r.ImGui_IsItemVisible(ctx),
      r.ImGui_IsItemClicked(ctx),
      r.ImGui_IsItemToggledOpen(ctx),
      ({r.ImGui_GetItemRectMin(ctx)})[1], ({r.ImGui_GetItemRectMin(ctx)})[2],
      ({r.ImGui_GetItemRectMax(ctx)})[1], ({r.ImGui_GetItemRectMax(ctx)})[2],
      ({r.ImGui_GetItemRectSize(ctx)})[1], ({r.ImGui_GetItemRectSize(ctx)})[2]
    ))

    rv,widgets.query.embed_all_inside_a_child_window =
      r.ImGui_Checkbox(ctx, 'Embed everything inside a child window (for additional testing)',
      widgets.query.embed_all_inside_a_child_window)
    if widgets.query.embed_all_inside_a_child_window then
      r.ImGui_BeginChild(ctx, 'outer_child', 0, r.ImGui_GetFontSize(ctx) * 20.0, true)
    end

    -- Testing IsWindowFocused() function with its various flags.
    -- Note that the ImGuiFocusedFlags_XXX flags can be combined.
    r.ImGui_BulletText(ctx, ([[IsWindowFocused() = %s
IsWindowFocused(_ChildWindows) = %s
IsWindowFocused(_ChildWindows|_RootWindow) = %s
IsWindowFocused(_RootWindow) = %s
IsWindowFocused(_AnyWindow) = %s]]):format(
      r.ImGui_IsWindowFocused(ctx),
      r.ImGui_IsWindowFocused(ctx, r.ImGui_FocusedFlags_ChildWindows()),
      r.ImGui_IsWindowFocused(ctx, r.ImGui_FocusedFlags_ChildWindows() | r.ImGui_FocusedFlags_RootWindow()),
      r.ImGui_IsWindowFocused(ctx, r.ImGui_FocusedFlags_RootWindow()),
      r.ImGui_IsWindowFocused(ctx, r.ImGui_FocusedFlags_AnyWindow())))

    -- Testing IsWindowHovered() function with its various flags.
    -- Note that the ImGuiHoveredFlags_XXX flags can be combined.
    r.ImGui_BulletText(ctx, ([[IsWindowHovered() = %s
IsWindowHovered(_AllowWhenBlockedByPopup) = %s
IsWindowHovered(_AllowWhenBlockedByActiveItem) = %s
IsWindowHovered(_ChildWindows) = %s
IsWindowHovered(_ChildWindows|_RootWindow) = %s
IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %s
IsWindowHovered(_RootWindow) = %s
IsWindowHovered(_AnyWindow) = %s]]):format(
      r.ImGui_IsWindowHovered(ctx),
      r.ImGui_IsWindowHovered(ctx, r.ImGui_HoveredFlags_AllowWhenBlockedByPopup()),
      r.ImGui_IsWindowHovered(ctx, r.ImGui_HoveredFlags_AllowWhenBlockedByActiveItem()),
      r.ImGui_IsWindowHovered(ctx, r.ImGui_HoveredFlags_ChildWindows()),
      r.ImGui_IsWindowHovered(ctx, r.ImGui_HoveredFlags_ChildWindows() | r.ImGui_HoveredFlags_RootWindow()),
      r.ImGui_IsWindowHovered(ctx, r.ImGui_HoveredFlags_ChildWindows() | r.ImGui_HoveredFlags_AllowWhenBlockedByPopup()),
      r.ImGui_IsWindowHovered(ctx, r.ImGui_HoveredFlags_RootWindow()),
      r.ImGui_IsWindowHovered(ctx, r.ImGui_HoveredFlags_AnyWindow())))

    r.ImGui_BeginChild(ctx, 'child', 0, 50, true)
    r.ImGui_Text(ctx, 'This is another child window for testing the _ChildWindows flag.')
    r.ImGui_EndChild(ctx)
    if widgets.query.embed_all_inside_a_child_window then
      r.ImGui_EndChild(ctx)
    end

    local unused_str = 'This widget is only here to be able to tab-out of the widgets above.'
    r.ImGui_InputText(ctx, 'unused', unused_str, r.ImGui_InputTextFlags_ReadOnly())

    -- Calling IsItemHovered() after begin returns the hovered status of the title bar.
    -- This is useful in particular if you want to create a context menu associated to the title bar of a window.
    rv,widgets.query.test_window = r.ImGui_Checkbox(ctx, 'Hovered/Active tests after Begin() for title bar testing', widgets.query.test_window)
    if widgets.query.test_window then
      rv,widgets.query.test_window = r.ImGui_Begin(ctx, 'Title bar Hovered/Active tests', widgets.query.test_window)
      if r.ImGui_BeginPopupContextItem(ctx) then -- <-- This is using IsItemHovered()
        if r.ImGui_MenuItem(ctx, 'Close') then widgets.query.test_window = false end
        r.ImGui_EndPopup(ctx)
      end
      r.ImGui_Text(ctx,
        ('IsItemHovered() after begin = %s (== is title bar hovered)\n\z
          IsItemActive() after begin = %s (== is window being clicked/moved)\n')
        :format(r.ImGui_IsItemHovered(ctx), r.ImGui_IsItemActive(ctx)))
      r.ImGui_End(ctx)
    end

    r.ImGui_TreePop(ctx)
  end
end

function demo.ShowDemoWindowLayout()
  if not r.ImGui_CollapsingHeader(ctx, 'Layout & Scrolling') then
    return
  end

  local rv

  if r.ImGui_TreeNode(ctx, 'Child windows') then
    if not layout.child then
      layout.child = {
        disable_mouse_wheel = false,
        disable_menu        = false,
        offset_x            = 0,
      }
    end

    demo.HelpMarker('Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window.')
    rv,layout.child.disable_mouse_wheel = r.ImGui_Checkbox(ctx, 'Disable Mouse Wheel', layout.child.disable_mouse_wheel)
    rv,layout.child.disable_menu = r.ImGui_Checkbox(ctx, 'Disable Menu', layout.child.disable_menu)

    -- Child 1: no border, enable horizontal scrollbar
    local window_flags = r.ImGui_WindowFlags_HorizontalScrollbar()
    if layout.child.disable_mouse_wheel then
      window_flags = window_flags | r.ImGui_WindowFlags_NoScrollWithMouse()
    end
    r.ImGui_BeginChild(ctx, 'ChildL', r.ImGui_GetWindowContentRegionWidth(ctx) * 0.5, 260, false, window_flags)
    for i = 0, 99 do
      r.ImGui_Text(ctx, ('%04d: scrollable region'):format(i))
    end
    r.ImGui_EndChild(ctx)

    r.ImGui_SameLine(ctx)

    -- Child 2: rounded border
    window_flags = r.ImGui_WindowFlags_None()
    if layout.child.disable_mouse_wheel then
      window_flags = window_flags | r.ImGui_WindowFlags_NoScrollWithMouse()
    end
    if not layout.child.disable_menu then
      window_flags = window_flags | r.ImGui_WindowFlags_MenuBar()
    end
    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_ChildRounding(), 5.0)
    r.ImGui_BeginChild(ctx, 'ChildR', 0, 260, true, window_flags)
    if not layout.child.disable_menu and r.ImGui_BeginMenuBar(ctx) then
      if r.ImGui_BeginMenu(ctx, 'Menu') then
        demo.ShowExampleMenuFile()
        r.ImGui_EndMenu(ctx)
      end
      r.ImGui_EndMenuBar(ctx)
    end
    if r.ImGui_BeginTable(ctx, 'split', 2, r.ImGui_TableFlags_Resizable()--[[ | r.ImGuiTableFlags_NoSavedSettings()]]) then
      for i = 0, 99 do
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Button(ctx, ('%03d'):format(i), -FLT_MIN, 0.0)
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_EndChild(ctx)
    r.ImGui_PopStyleVar(ctx)

    r.ImGui_Separator(ctx)

    -- Demonstrate a few extra things
    -- - Changing ImGuiCol_ChildBg (which is transparent black in default styles)
    -- - Using SetCursorPos() to position child window (the child window is an item from the POV of parent window)
    --   You can also call SetNextWindowPos() to position the child window. The parent window will effectively
    --   layout from this position.
    -- - Using r.ImGui_GetItemRectMin/Max() to query the "item" state (because the child window is an item from
    --   the POV of the parent window). See 'Demo->Querying Status (Active/Focused/Hovered etc.)' for details.
    r.ImGui_SetNextItemWidth(ctx, 100)
    rv,layout.child.offset_x = r.ImGui_DragInt(ctx, 'Offset X', layout.child.offset_x, 1.0, -1000, 1000)

    r.ImGui_SetCursorPosX(ctx, r.ImGui_GetCursorPosX(ctx) + layout.child.offset_x)
    r.ImGui_PushStyleColor(ctx, r.ImGui_Col_ChildBg(), 0xFF000064)
    r.ImGui_BeginChild(ctx, 'Red', 200, 100, true, r.ImGui_WindowFlags_None())
    for n = 0, 49 do
      r.ImGui_Text(ctx, ('Some test %d'):format(n))
    end
    r.ImGui_EndChild(ctx)
    r.ImGui_PopStyleColor(ctx)
    local child_is_hovered = r.ImGui_IsItemHovered(ctx)
    local child_rect_min_x,child_rect_min_y = r.ImGui_GetItemRectMin(ctx)
    local child_rect_max_x,child_rect_max_y = r.ImGui_GetItemRectMax(ctx)
    r.ImGui_Text(ctx, ('Hovered: %s'):format(child_is_hovered))
    r.ImGui_Text(ctx, ('Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)')
      :format(child_rect_min_x, child_rect_min_y, child_rect_max_x, child_rect_max_y))

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Widgets Width') then
    if not layout.width then
      layout.width = {
        d = 0.0,
        show_indented_items = true,
      }
    end

    -- Use SetNextItemWidth() to set the width of a single upcoming item.
    -- Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
    -- In real code use you'll probably want to choose width values that are proportional to your font size
    -- e.g. Using '20.0f * GetFontSize()' as width instead of '200.0f', etc.

    rv,layout.width.show_indented_items = r.ImGui_Checkbox(ctx, 'Show indented items', layout.width.show_indented_items)

    r.ImGui_Text(ctx, 'SetNextItemWidth/PushItemWidth(100)')
    r.ImGui_SameLine(ctx); demo.HelpMarker('Fixed width.')
    r.ImGui_PushItemWidth(ctx, 100)
    rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float##1b', layout.width.d)
    if layout.width.show_indented_items then
      r.ImGui_Indent(ctx)
      rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float (indented)##1b', layout.width.d)
      r.ImGui_Unindent(ctx)
    end
    r.ImGui_PopItemWidth(ctx)

    r.ImGui_Text(ctx, 'SetNextItemWidth/PushItemWidth(-100)')
    r.ImGui_SameLine(ctx); demo.HelpMarker('Align to right edge minus 100')
    r.ImGui_PushItemWidth(ctx, -100)
    rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float##2a', layout.width.d)
    if layout.width.show_indented_items then
      r.ImGui_Indent(ctx)
      rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float (indented)##2b', layout.width.d)
      r.ImGui_Unindent(ctx)
    end
    r.ImGui_PopItemWidth(ctx)

    r.ImGui_Text(ctx, 'SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5f)')
    r.ImGui_SameLine(ctx); demo.HelpMarker('Half of available width.\n(~ right-cursor_pos)\n(works within a column set)')
    r.ImGui_PushItemWidth(ctx, ({r.ImGui_GetContentRegionAvail(ctx)})[1] * 0.5)
    rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float##3a', layout.width.d)
    if layout.width.show_indented_items then
      r.ImGui_Indent(ctx)
      rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float (indented)##3b', layout.width.d)
      r.ImGui_Unindent(ctx)
    end
    r.ImGui_PopItemWidth(ctx)

    r.ImGui_Text(ctx, 'SetNextItemWidth/PushItemWidth(-GetContentRegionAvail().x * 0.5f)')
    r.ImGui_SameLine(ctx); demo.HelpMarker('Align to right edge minus half')
    r.ImGui_PushItemWidth(ctx, -({r.ImGui_GetContentRegionAvail(ctx)})[1] * 0.5)
    rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float##4a', layout.width.d)
    if layout.width.show_indented_items then
      r.ImGui_Indent(ctx)
      rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float (indented)##4b', layout.width.d)
      r.ImGui_Unindent(ctx)
    end
    r.ImGui_PopItemWidth(ctx)

    -- Demonstrate using PushItemWidth to surround three items.
    -- Calling SetNextItemWidth() before each of them would have the same effect.
    r.ImGui_Text(ctx, 'SetNextItemWidth/PushItemWidth(-FLT_MIN)')
    r.ImGui_SameLine(ctx); demo.HelpMarker('Align to right edge')
    r.ImGui_PushItemWidth(ctx, -FLT_MIN)
    rv,layout.width.d = r.ImGui_DragDouble(ctx, '##float5a', layout.width.d)
    if layout.width.show_indented_items then
      r.ImGui_Indent(ctx)
      rv,layout.width.d = r.ImGui_DragDouble(ctx, 'float (indented)##5b', layout.width.d)
      r.ImGui_Unindent(ctx)
    end
    r.ImGui_PopItemWidth(ctx)

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Basic Horizontal Layout') then
    if not layout.horizontal then
      layout.horizontal = {
        c1 = false, c2 = false, c3 = false, c4 = false,
        d0 = 1.0, d1 = 2.0, d2 = 3.0,
        item = -1,
        selection = { 0, 1, 2, 3 },
      }
    end

    r.ImGui_TextWrapped(ctx, '(Use r.ImGui_SameLine() to keep adding items to the right of the preceding item)');

    -- Text
    r.ImGui_Text(ctx, 'Two items: Hello'); r.ImGui_SameLine(ctx)
    r.ImGui_TextColored(ctx, 0xFFFF00FF, 'Sailor')

    -- Adjust spacing
    r.ImGui_Text(ctx, 'More spacing: Hello'); r.ImGui_SameLine(ctx, 0, 20)
    r.ImGui_TextColored(ctx, 0xFFFF00FF, 'Sailor')

    -- Button
    r.ImGui_AlignTextToFramePadding(ctx)
    r.ImGui_Text(ctx, 'Normal buttons'); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Banana'); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Apple'); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Corniflower')

    -- Button
    r.ImGui_Text(ctx, 'Small buttons'); r.ImGui_SameLine(ctx)
    r.ImGui_SmallButton(ctx, 'Like this one'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'can fit within a text block.')

    -- Aligned to arbitrary position. Easy/cheap column.
    r.ImGui_Text(ctx, 'Aligned')
    r.ImGui_SameLine(ctx, 150); r.ImGui_Text(ctx, 'x=150')
    r.ImGui_SameLine(ctx, 300); r.ImGui_Text(ctx, 'x=300')
    r.ImGui_Text(ctx, 'Aligned')
    r.ImGui_SameLine(ctx, 150); r.ImGui_SmallButton(ctx, 'x=150')
    r.ImGui_SameLine(ctx, 300); r.ImGui_SmallButton(ctx, 'x=300')

    -- Checkbox
    rv,layout.horizontal.c1 = r.ImGui_Checkbox(ctx, 'My',     layout.horizontal.c1); r.ImGui_SameLine(ctx)
    rv,layout.horizontal.c2 = r.ImGui_Checkbox(ctx, 'Tailor', layout.horizontal.c2); r.ImGui_SameLine(ctx)
    rv,layout.horizontal.c3 = r.ImGui_Checkbox(ctx, 'Is',     layout.horizontal.c3); r.ImGui_SameLine(ctx)
    rv,layout.horizontal.c4 = r.ImGui_Checkbox(ctx, 'Rich',   layout.horizontal.c4)

    -- Various
    r.ImGui_PushItemWidth(ctx, 80)
    local items = 'AAAA\31BBBB\31CCCC\31DDDD\31'
    rv,layout.horizontal.item = r.ImGui_Combo(ctx, 'Combo', layout.horizontal.item, items);   r.ImGui_SameLine(ctx)
    rv,layout.horizontal.d0 = r.ImGui_SliderDouble(ctx, 'X', layout.horizontal.d0, 0.0, 5.0); r.ImGui_SameLine(ctx)
    rv,layout.horizontal.d1 = r.ImGui_SliderDouble(ctx, 'Y', layout.horizontal.d1, 0.0, 5.0); r.ImGui_SameLine(ctx)
    rv,layout.horizontal.d2 = r.ImGui_SliderDouble(ctx, 'Z', layout.horizontal.d2, 0.0, 5.0)
    r.ImGui_PopItemWidth(ctx)

    r.ImGui_PushItemWidth(ctx, 80)
    r.ImGui_Text(ctx, 'Lists:')
    for i,sel in ipairs(layout.horizontal.selection) do
      if i > 1 then r.ImGui_SameLine(ctx) end
      r.ImGui_PushID(ctx, i)
      rv,layout.horizontal.selection[i] = r.ImGui_ListBox(ctx, '', sel, items)
      r.ImGui_PopID(ctx)
      --if r.ImGui_IsItemHovered(ctx) then r.ImGui_SetTooltip(ctx, ('ListBox %d hovered'):format(i)) end
    end
    r.ImGui_PopItemWidth(ctx)

    -- Dummy
    local button_sz = { 40, 40 }
    r.ImGui_Button(ctx, 'A', table.unpack(button_sz)); r.ImGui_SameLine(ctx)
    r.ImGui_Dummy(ctx, table.unpack(button_sz)); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx,'B', table.unpack(button_sz))

    -- Manually wrapping
    -- (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
    r.ImGui_Text(ctx, 'Manually wrapping:')
    local item_spacing_x = ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing())})[1]
    local buttons_count = 20
    local window_visible_x2 = ({r.ImGui_GetWindowPos(ctx)})[1] + ({r.ImGui_GetWindowContentRegionMax(ctx)})[1]
    for n = 0, buttons_count - 1 do
      r.ImGui_PushID(ctx, n)
      r.ImGui_Button(ctx, 'Box', table.unpack(button_sz))
      local last_button_x2 = r.ImGui_GetItemRectMax(ctx)
      local next_button_x2 = last_button_x2 + item_spacing_x + button_sz[1] -- Expected position if next button was on same line
      if n + 1 < buttons_count and next_button_x2 < window_visible_x2 then
        r.ImGui_SameLine(ctx)
      end
      r.ImGui_PopID(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Groups') then
    if not widgets.groups then
      widgets.groups = {
        values = reaper.new_array({ 0.5, 0.20, 0.80, 0.60, 0.25 }),
      }
    end

    demo.HelpMarker(
      'BeginGroup() basically locks the horizontal position for new line. \z
       EndGroup() bundles the whole group so that you can use "item" functions such as \z
       IsItemHovered()/IsItemActive() or SameLine() etc. on the whole group.')
    r.ImGui_BeginGroup(ctx)
    r.ImGui_BeginGroup(ctx)
    r.ImGui_Button(ctx, 'AAA')
    r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'BBB')
    r.ImGui_SameLine(ctx)
    r.ImGui_BeginGroup(ctx)
    r.ImGui_Button(ctx, 'CCC')
    r.ImGui_Button(ctx, 'DDD')
    r.ImGui_EndGroup(ctx)
    r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'EEE')
    r.ImGui_EndGroup(ctx)
    if r.ImGui_IsItemHovered(ctx) then
      r.ImGui_SetTooltip(ctx, 'First group hovered')
    end

    -- Capture the group size and create widgets using the same size
    local size = {r.ImGui_GetItemRectSize(ctx)}
    local item_spacing_x = ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing())})[1]

    r.ImGui_PlotHistogram(ctx, '##values', widgets.groups.values, 0, nil, 0.0, 1.0, table.unpack(size))

    r.ImGui_Button(ctx, 'ACTION', (size[1] - item_spacing_x) * 0.5, size[2])
    r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'REACTION', (size[1] - item_spacing_x) * 0.5, size[2])
    r.ImGui_EndGroup(ctx)
    r.ImGui_SameLine(ctx)

    r.ImGui_Button(ctx, 'LEVERAGE\nBUZZWORD', table.unpack(size))
    r.ImGui_SameLine(ctx)

    if r.ImGui_BeginListBox(ctx, 'List', table.unpack(size)) then
      r.ImGui_Selectable(ctx, 'Selected', true)
      r.ImGui_Selectable(ctx, 'Not Selected', false)
      r.ImGui_EndListBox(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Text Baseline Alignment') then
    r.ImGui_BulletText(ctx, 'Text baseline:')
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'This is testing the vertical alignment that gets applied on text to keep it aligned with widgets. \z
       Lines only composed of text or "small" widgets use less vertical space than lines with framed widgets.')
    r.ImGui_Indent(ctx)

    r.ImGui_Text(ctx, 'KO Blahblah'); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Some framed item'); r.ImGui_SameLine(ctx)
    demo.HelpMarker('Baseline of button will look misaligned with text..')

    -- If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
    -- (because we don't know what's coming after the Text() statement, we need to move the text baseline
    -- down by FramePadding.y ahead of time)
    r.ImGui_AlignTextToFramePadding(ctx)
    r.ImGui_Text(ctx, 'OK Blahblah'); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Some framed item'); r.ImGui_SameLine(ctx)
    demo.HelpMarker('We call AlignTextToFramePadding() to vertically align the text baseline by +FramePadding.y')

    -- SmallButton() uses the same vertical padding as Text
    r.ImGui_Button(ctx, 'TEST##1'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'TEST'); r.ImGui_SameLine(ctx)
    r.ImGui_SmallButton(ctx, 'TEST##2')

    -- If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
    r.ImGui_AlignTextToFramePadding(ctx)
    r.ImGui_Text(ctx, 'Text aligned to framed item'); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Item##1'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Item'); r.ImGui_SameLine(ctx)
    r.ImGui_SmallButton(ctx, 'Item##2'); r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Item##3')

    r.ImGui_Unindent(ctx)

    r.ImGui_Spacing(ctx)

    r.ImGui_BulletText(ctx, 'Multi-line text:')
    r.ImGui_Indent(ctx)
    r.ImGui_Text(ctx, 'One\nTwo\nThree'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Hello\nWorld'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Banana')

    r.ImGui_Text(ctx, 'Banana'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Hello\nWorld'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'One\nTwo\nThree')

    r.ImGui_Button(ctx, 'HOP##1'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Banana'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Hello\nWorld'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Banana')

    r.ImGui_Button(ctx, 'HOP##2'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Hello\nWorld'); r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Banana')
    r.ImGui_Unindent(ctx)

    r.ImGui_Spacing(ctx)

    r.ImGui_BulletText(ctx, 'Misc items:')
    r.ImGui_Indent(ctx)

    -- SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of previous Button.
    r.ImGui_Button(ctx, '80x80', 80, 80)
    r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, '50x50', 50, 50)
    r.ImGui_SameLine(ctx)
    r.ImGui_Button(ctx, 'Button()')
    r.ImGui_SameLine(ctx)
    r.ImGui_SmallButton(ctx, 'SmallButton()')

    -- Tree
    local spacing = ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemInnerSpacing())})[1]
    r.ImGui_Button(ctx, 'Button##1')
    r.ImGui_SameLine(ctx, 0.0, spacing)
    if r.ImGui_TreeNode(ctx, 'Node##1') then
      -- Placeholder tree data
      for i = 0, 5 do
        r.ImGui_BulletText(ctx, ('Item %d..'):format(i))
      end
      r.ImGui_TreePop(ctx)
    end

    -- Vertically align text node a bit lower so it'll be vertically centered with upcoming widget.
    -- Otherwise you can use SmallButton() (smaller fit).
    r.ImGui_AlignTextToFramePadding(ctx)

    -- Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add
    -- other contents below the node.
    local node_open = r.ImGui_TreeNode(ctx, 'Node##2')
    r.ImGui_SameLine(ctx, 0.0, spacing); r.ImGui_Button(ctx, 'Button##2')
    if node_open then
      -- Placeholder tree data
      for i = 0, 5 do
        r.ImGui_BulletText(ctx, ('Item %d..'):format(i))
      end
      r.ImGui_TreePop(ctx)
    end

    -- Bullet
    r.ImGui_Button(ctx, 'Button##3')
    r.ImGui_SameLine(ctx, 0.0, spacing)
    r.ImGui_BulletText(ctx, 'Bullet text')

    r.ImGui_AlignTextToFramePadding(ctx)
    r.ImGui_BulletText(ctx, 'Node')
    r.ImGui_SameLine(ctx, 0.0, spacing); r.ImGui_Button(ctx, 'Button##4')
    r.ImGui_Unindent(ctx)

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Scrolling') then
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

    rv,layout.scrolling.enable_extra_decorations = r.ImGui_Checkbox(ctx, 'Decoration', layout.scrolling.enable_extra_decorations)

    rv,layout.scrolling.enable_track = r.ImGui_Checkbox(ctx, 'Track', layout.scrolling.enable_track)
    r.ImGui_PushItemWidth(ctx, 100)
    r.ImGui_SameLine(ctx, 140)
    rv,layout.scrolling.track_item = r.ImGui_DragInt(ctx, '##item', layout.scrolling.track_item, 0.25, 0, 99, 'Item = %d')
    if rv then
      layout.scrolling.enable_track = true
    end

    local scroll_to_off = r.ImGui_Button(ctx, 'Scroll Offset')
    r.ImGui_SameLine(ctx, 140)
    rv,layout.scrolling.scroll_to_off_px = r.ImGui_DragDouble(ctx, '##off', layout.scrolling.scroll_to_off_px, 1.00, 0, FLT_MAX, '+%.0f px')
    if rv then
      scroll_to_off = true
    end

    local scroll_to_pos = r.ImGui_Button(ctx, 'Scroll To Pos')
    r.ImGui_SameLine(ctx, 140)
    rv,layout.scrolling.scroll_to_pos_px = r.ImGui_DragDouble(ctx, '##pos', layout.scrolling.scroll_to_pos_px, 1.00, -10, FLT_MAX, 'X/Y = %.0f px')
    if rv then
      scroll_to_pos = true
    end
    r.ImGui_PopItemWidth(ctx)

    if scroll_to_off or scroll_to_pos then
      layout.scrolling.enable_track = false
    end

    local names = { "Top", "25%", "Center", "75%", "Bottom" }
    local item_spacing_x = ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing())})[1]
    local child_w = (({r.ImGui_GetContentRegionAvail(ctx)})[1] - 4 * item_spacing_x) / #names
    local child_flags = layout.scrolling.enable_extra_decorations and r.ImGui_WindowFlags_MenuBar() or r.ImGui_WindowFlags_None()
    if child_w < 1.0 then
      child_w = 1.0
    end
    r.ImGui_PushID(ctx, '##VerticalScrolling')
    for i,name in ipairs(names) do
      if i > 1 then r.ImGui_SameLine(ctx) end
      r.ImGui_BeginGroup(ctx)
      r.ImGui_Text(ctx, name)

      local child_is_visible = r.ImGui_BeginChild(ctx, i, child_w, 200.0, true, child_flags)
      if r.ImGui_BeginMenuBar(ctx) then
        r.ImGui_Text(ctx, 'abc')
        r.ImGui_EndMenuBar(ctx)
      end
      if scroll_to_off then
        r.ImGui_SetScrollY(ctx, layout.scrolling.scroll_to_off_px)
      end
      if scroll_to_pos then
        r.ImGui_SetScrollFromPosY(ctx, ({r.ImGui_GetCursorStartPos(ctx)})[2] + layout.scrolling.scroll_to_pos_px, (i - 1) * 0.25)
      end
      if child_is_visible then -- Avoid calling SetScrollHereY when running with culled items
        for item = 0, 99 do
          if layout.scrolling.enable_track and item == layout.scrolling.track_item then
            r.ImGui_TextColored(ctx, 0xFFFF00FF, ('Item %d'):format(item))
            r.ImGui_SetScrollHereY(ctx, (i - 1) * 0.25) -- 0.0f:top, 0.5f:center, 1.0f:bottom
          else
            r.ImGui_Text(ctx, ('Item %d'):format(item))
          end
        end
      end
      local scroll_y = r.ImGui_GetScrollY(ctx)
      local scroll_max_y = r.ImGui_GetScrollMaxY(ctx)
      r.ImGui_EndChild(ctx)
      r.ImGui_Text(ctx, ('%.0f/%.0f'):format(scroll_y, scroll_max_y))
      r.ImGui_EndGroup(ctx)
    end
    r.ImGui_PopID(ctx)

    -- Horizontal scroll functions
    r.ImGui_Spacing(ctx)
    demo.HelpMarker(
      "Use SetScrollHereX() or SetScrollFromPosX() to scroll to a given horizontal position.\n\n\z
       Because the clipping rectangle of most window hides half worth of WindowPadding on the \z
       left/right, using SetScrollFromPosX(+1) will usually result in clipped text whereas the \z
       equivalent SetScrollFromPosY(+1) wouldn't.")
    r.ImGui_PushID(ctx, '##HorizontalScrolling')
    local scrollbar_size = ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ScrollbarSize())})[1]
    local window_padding_y = ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_WindowPadding())})[2]
    local child_height = r.ImGui_GetTextLineHeight(ctx) + scrollbar_size + window_padding_y * 2.0
    local child_flags = r.ImGui_WindowFlags_HorizontalScrollbar()
    if layout.scrolling.enable_extra_decorations then
      child_flags = child_flags | r.ImGui_WindowFlags_AlwaysVerticalScrollbar()
    end
    for i,name in ipairs(names) do
      local child_is_visible = r.ImGui_BeginChild(ctx, i, -100, child_height, true, child_flags)
      if scroll_to_off then
        r.ImGui_SetScrollX(layout.scrolling.scroll_to_off_px)
      end
      if scroll_to_pos then
        r.ImGui_SetScrollFromPosX(ctx, ({r.ImGui_GetCursorStartPos(ctx)})[1] + scroll_to_pos_px, (i - 1) * 0.25)
      end
      if child_is_visible then -- Avoid calling SetScrollHereY when running with culled items
        for item = 0, 99 do
          if layout.scrolling.enable_track and item == layout.scrolling.track_item then
            r.ImGui_TextColored(ctx, 0xFFFF00FF, ('Item %d'):format(item))
            r.ImGui_SetScrollHereX(ctx, (i - 1) * 0.25) -- 0.0f:left, 0.5f:center, 1.0f:right
          else
            r.ImGui_Text(ctx, ('Item %d'):format(item))
          end
          r.ImGui_SameLine(ctx)
        end
      end
      local scroll_x = r.ImGui_GetScrollX(ctx)
      local scroll_max_x = r.ImGui_GetScrollMaxX(ctx)
      r.ImGui_EndChild(ctx)
      r.ImGui_SameLine(ctx)
      r.ImGui_Text(ctx, ('%s\n%.0f/%.0f'):format(name, scroll_x, scroll_max_x))
      r.ImGui_Spacing(ctx)
    end
    r.ImGui_PopID(ctx)

    -- Miscellaneous Horizontal Scrolling Demo
    demo.HelpMarker(
      'Horizontal scrolling for a window is enabled via the ImGuiWindowFlags_HorizontalScrollbar flag.\n\n\z
       You may want to also explicitly specify content width by using SetNextWindowContentWidth() before Begin().')
    rv,layout.scrolling.lines = r.ImGui_SliderInt(ctx, 'Lines', layout.scrolling.lines, 1, 15)
    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_FrameRounding(), 3.0)
    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_FramePadding(), 2.0, 1.0)
    local scrolling_child_width = r.ImGui_GetFrameHeightWithSpacing(ctx) * 7 + 30
    r.ImGui_BeginChild(ctx, 'scrolling', 0, scrolling_child_width, true, r.ImGui_WindowFlags_HorizontalScrollbar())
    for line = 0, layout.scrolling.lines - 1 do
      -- Display random stuff. For the sake of this trivial demo we are using basic Button() + SameLine()
      -- If you want to create your own time line for a real application you may be better off manipulating
      -- the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets
      -- yourself. You may also want to use the lower-level ImDrawList API.
      local num_buttons = 10 + ((line & 1 ~= 0) and line * 9 or line * 3)
      for n = 0, num_buttons - 1 do
        if n > 0 then r.ImGui_SameLine(ctx) end
        r.ImGui_PushID(ctx, n + line * 1000)
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
        local hue = n * 0.05;
        local button_color = r.ImGui_ColorConvertHSVtoRGB(hue, 0.6, 0.6, 1.0)
        local hovered_color = r.ImGui_ColorConvertHSVtoRGB(hue, 0.7, 0.7, 1.0)
        local active_color = r.ImGui_ColorConvertHSVtoRGB(hue, 0.8, 0.8, 1.0)
        r.ImGui_PushStyleColor(ctx, r.ImGui_Col_Button(), button_color)
        r.ImGui_PushStyleColor(ctx, r.ImGui_Col_ButtonHovered(), hovered_color)
        r.ImGui_PushStyleColor(ctx, r.ImGui_Col_ButtonActive(), active_color)
        r.ImGui_Button(ctx, label, 40.0 + math.sin(line + n) * 20.0, 0.0)
        r.ImGui_PopStyleColor(ctx, 3)
        r.ImGui_PopID(ctx)
      end
    end
    local scroll_x = r.ImGui_GetScrollX(ctx)
    local scroll_max_x = r.ImGui_GetScrollMaxX(ctx)
    r.ImGui_EndChild(ctx)
    r.ImGui_PopStyleVar(ctx, 2)
    local scroll_x_delta = 0.0
    r.ImGui_SmallButton(ctx, '<<')
    if r.ImGui_IsItemActive(ctx) then
      scroll_x_delta = (0 - r.ImGui_GetDeltaTime(ctx)) * 1000.0
    end
    r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Scroll from code'); r.ImGui_SameLine(ctx)
    r.ImGui_SmallButton(ctx, '>>')
    if r.ImGui_IsItemActive(ctx) then
      scroll_x_delta = r.ImGui_GetDeltaTime(ctx) * 1000.0
    end
    r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, ('%.0f/%.0f'):format(scroll_x, scroll_max_x))
    if scroll_x_delta ~= 0.0 then
      -- Demonstrate a trick: you can use Begin to set yourself in the context of another window
      -- (here we are already out of your child window)
      r.ImGui_BeginChild(ctx, 'scrolling')
      r.ImGui_SetScrollX(ctx, r.ImGui_GetScrollX(ctx) + scroll_x_delta)
      r.ImGui_EndChild(ctx)
    end
    r.ImGui_Spacing(ctx)

    rv,layout.scrolling.show_horizontal_contents_size_demo_window =
      r.ImGui_Checkbox(ctx, 'Show Horizontal contents size demo window',
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
        r.ImGui_SetNextWindowContentSize(ctx, layout.horizontal_window.contents_size_x, 0.0)
      end
      rv,layout.scrolling.show_horizontal_contents_size_demo_window =
        r.ImGui_Begin(ctx, 'Horizontal contents size demo window',
          layout.scrolling.show_horizontal_contents_size_demo_window,
          layout.horizontal_window.show_h_scrollbar and r.ImGui_WindowFlags_HorizontalScrollbar() or r.ImGui_WindowFlags_None())
      r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing(), 2, 0)
      r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_FramePadding(), 2, 0)
      demo.HelpMarker("Test of different widgets react and impact the work rectangle growing when horizontal scrolling is enabled.\n\nUse 'Metrics->Tools->Show windows rectangles' to visualize rectangles.")
      rv,layout.horizontal_window.show_h_scrollbar =
        r.ImGui_Checkbox(ctx, 'H-scrollbar', layout.horizontal_window.show_h_scrollbar)
      rv,layout.horizontal_window.show_button =
        r.ImGui_Checkbox(ctx, 'Button', layout.horizontal_window.show_button)             -- Will grow contents size (unless explicitly overwritten)
      rv,layout.horizontal_window.show_tree_nodes =
        r.ImGui_Checkbox(ctx, 'Tree nodes', layout.horizontal_window.show_tree_nodes)     -- Will grow contents size and display highlight over full width
      rv,layout.horizontal_window.show_text_wrapped =
        r.ImGui_Checkbox(ctx, 'Text wrapped', layout.horizontal_window.show_text_wrapped) -- Will grow and use contents size
      rv,layout.horizontal_window.show_columns =
        r.ImGui_Checkbox(ctx, 'Columns', layout.horizontal_window.show_columns)           -- Will use contents size
      rv,layout.horizontal_window.show_tab_bar =
        r.ImGui_Checkbox(ctx, 'Tab bar', layout.horizontal_window.show_tab_bar)           -- Will use contents size
      rv,layout.horizontal_window.show_child =
        r.ImGui_Checkbox(ctx, 'Child', layout.horizontal_window.show_child)               -- Will grow and use contents size
      rv,layout.horizontal_window.explicit_content_size =
        r.ImGui_Checkbox(ctx, 'Explicit content size', layout.horizontal_window.explicit_content_size)
      r.ImGui_Text(ctx, ('Scroll %.1f/%.1f %.1f/%.1f'):format(r.ImGui_GetScrollX(ctx), r.ImGui_GetScrollMaxX(ctx), r.ImGui_GetScrollY(ctx), r.ImGui_GetScrollMaxY(ctx)))
      if layout.horizontal_window.explicit_content_size then
        r.ImGui_SameLine(ctx)
        r.ImGui_SetNextItemWidth(ctx, 100)
        rv,layout.horizontal_window.contents_size_x =
          r.ImGui_DragDouble(ctx, '##csx', layout.horizontal_window.contents_size_x)
        local x, y = r.ImGui_GetCursorScreenPos(ctx)
        local draw_list = r.ImGui_GetWindowDrawList(ctx)
        r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + 10, y + 10, 0xFFFFFFFF)
        r.ImGui_DrawList_AddRectFilled(draw_list, x + layout.horizontal_window.contents_size_x - 10, y, x + layout.horizontal_window.contents_size_x, y + 10, 0xFFFFFFFF)
        r.ImGui_Dummy(ctx, 0, 10)
      end
      r.ImGui_PopStyleVar(ctx, 2)
      r.ImGui_Separator(ctx)
      if layout.horizontal_window.show_button then
        r.ImGui_Button(ctx, 'this is a 300-wide button', 300, 0)
      end
      if layout.horizontal_window.show_tree_nodes then
        local open = true
        if r.ImGui_TreeNode(ctx, 'this is a tree node') then
          if r.ImGui_TreeNode(ctx, 'another one of those tree node...') then
            r.ImGui_Text(ctx, 'Some tree contents')
            r.ImGui_TreePop(ctx)
          end
          r.ImGui_TreePop(ctx)
        end
        r.ImGui_CollapsingHeader(ctx, 'CollapsingHeader', open)
      end
      if layout.horizontal_window.show_text_wrapped then
        r.ImGui_TextWrapped(ctx, 'This text should automatically wrap on the edge of the work rectangle.')
      end
      if layout.horizontal_window.show_columns then
        r.ImGui_Text(ctx, 'Tables:')
        if r.ImGui_BeginTable(ctx, 'table', 4, r.ImGui_TableFlags_Borders()) then
          for n = 0, 3 do
            r.ImGui_TableNextColumn(ctx)
            r.ImGui_Text(ctx, ('Width %.2f'):format(({r.ImGui_GetContentRegionAvail(ctx)})[1]))
          end
          r.ImGui_EndTable(ctx)
        end
        -- r.ImGui_Text(ctx, 'Columns:')
        -- r.ImGui_Columns(ctx, 4)
        -- for n = 0, 3 do
        --   r.ImGui_Text(ctx, ('Width %.2f'):format(r.ImGui_GetColumnWidth()))
        --   r.ImGui_NextColumn(ctx)
        -- end
        -- r.ImGui_Columns(ctx, 1)
      end
      if layout.horizontal_window.show_tab_bar and r.ImGui_BeginTabBar(ctx, 'Hello') then
        if r.ImGui_BeginTabItem(ctx, 'OneOneOne') then r.ImGui_EndTabItem(ctx) end
        if r.ImGui_BeginTabItem(ctx, 'TwoTwoTwo') then r.ImGui_EndTabItem(ctx) end
        if r.ImGui_BeginTabItem(ctx, 'ThreeThreeThree') then r.ImGui_EndTabItem(ctx) end
        if r.ImGui_BeginTabItem(ctx, 'FourFourFour') then r.ImGui_EndTabItem(ctx) end
        r.ImGui_EndTabBar(ctx)
      end
      if layout.horizontal_window.show_child then
        r.ImGui_BeginChild(ctx, 'child', 0, 0, true)
        r.ImGui_EndChild(ctx)
      end
      r.ImGui_End(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Clipping') then
    if not layout.clipping then
      layout.clipping = {
        size   = { 100.0, 100.0 },
        offset = {  30.0,  30.0 },
      }
    end

    rv,layout.clipping.size[1],layout.clipping.size[2] =
      r.ImGui_DragDouble2(ctx, 'size', layout.clipping.size[1], layout.clipping.size[2],
      0.5, 1.0, 200.0, "%.0f")
    r.ImGui_TextWrapped(ctx, '(Click and drag to scroll)')

    for n = 0, 2 do
      if n > 0 then r.ImGui_SameLine(ctx) end
      r.ImGui_PushID(ctx, n)
      r.ImGui_BeginGroup(ctx) -- Lock X position

      r.ImGui_InvisibleButton(ctx, '##empty', table.unpack(layout.clipping.size))
      if r.ImGui_IsItemActive(ctx) and r.ImGui_IsMouseDragging(ctx, r.ImGui_MouseButton_Left()) then
        local mouse_delta = {r.ImGui_GetMouseDelta(ctx)}
        layout.clipping.offset[1] = layout.clipping.offset[1] + mouse_delta[1]
        layout.clipping.offset[2] = layout.clipping.offset[2] + mouse_delta[2]
      end
      local p0 = {r.ImGui_GetItemRectMin(ctx)}
      local p1 = {r.ImGui_GetItemRectMax(ctx)}
      local text_str = "Line 1 hello\nLine 2 clip me!"
      local text_pos = { p0[1] + layout.clipping.offset[1], p0[2] + layout.clipping.offset[2] }

      local draw_list = r.ImGui_GetWindowDrawList(ctx)
      if n == 0 then
        demo.HelpMarker(
          'Using r.ImGui_PushClipRect():\n\z
           Will alter ImGui hit-testing logic + ImDrawList rendering.\n\z
           (use this if you want your clipping rectangle to affect interactions)')
        r.ImGui_PushClipRect(ctx, p0[1], p0[2], p1[1], p1[2], true)
        r.ImGui_DrawList_AddRectFilled(draw_list, p0[1], p0[2], p1[1], p1[2], 0x5a5a78ff)
        r.ImGui_DrawList_AddText(draw_list, text_pos[1], text_pos[2], 0xffffffff, text_str)
        r.ImGui_PopClipRect(ctx)
      elseif n == 1 then
        demo.HelpMarker(
          'Using ImDrawList::PushClipRect():\n\z
           Will alter ImDrawList rendering only.\n\z
           (use this as a shortcut if you are only using ImDrawList calls)')
        r.ImGui_DrawList_PushClipRect(draw_list, p0[1], p0[2], p1[1], p1[2], true)
        r.ImGui_DrawList_AddRectFilled(draw_list, p0[1], p0[2], p1[1], p1[2], 0x5a5a78ff)
        r.ImGui_DrawList_AddText(draw_list, text_pos[1], text_pos[2], 0xffffffff, text_str)
        r.ImGui_DrawList_PopClipRect(draw_list)
      -- TODO
      -- elseif n == 2 then
      --   demo.HelpMarker(
      --     'Using ImDrawList::AddText() with a fine ClipRect:\n\z
      --      Will alter only this specific ImDrawList::AddText() rendering.\n\z
      --      (this is often used internally to avoid altering the clipping rectangle and minimize draw calls)')
      --   local clip_rect = { p0[1], p0[2], p1[1], p1[2] }
      --   r.ImGui_DrawList_AddRectFilled(draw_list, p0[1], p0[2], p1[1], p1[2], 0x5a5a78ff)
      --   r.ImGui_DrawList_AddTextEx(draw_list, r.ImGui_GetFont(ctx), r.ImGui_GetFontSize(ctx),
      --     text_pos[1], text_pos[2], 0xffffffff, text_str, nil, 0.0, table.unpack(clip_rect))
      end
      r.ImGui_EndGroup(ctx)
      r.ImGui_PopID(ctx)
    end

    r.ImGui_TreePop(ctx)
  end
end

function demo.ShowDemoWindowPopups()
  if not r.ImGui_CollapsingHeader(ctx, 'Popups & Modal windows') then
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
  --   bool my_tool_is_active = false; if (r.ImGui_Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
  -- Typical use for popups:
  --   if (r.ImGui_Button("Open")) r.ImGui_OpenPopup("MyPopup"); if (r.ImGui_BeginPopup("MyPopup") { [...] EndPopup(); }

  -- With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
  -- This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.

  if r.ImGui_TreeNode(ctx, 'Popups') then
    if not popups.popups then
      popups.popups = {
        selected_fish = -1,
        toggles = { true, false, false, false, false },
      }
    end

    r.ImGui_TextWrapped(ctx,
      'When a popup is active, it inhibits interacting with windows that are behind the popup. \z
       Clicking outside the popup closes it.');

    local names = { 'Bream', 'Haddock', 'Mackerel', 'Pollock', 'Tilefish' }

    -- Simple selection popup (if you want to show the current selection inside the Button itself,
    -- you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
    if r.ImGui_Button(ctx, 'Select..') then
      r.ImGui_OpenPopup(ctx, 'my_select_popup')
    end
    r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, names[popups.popups.selected_fish] or '<None>')
    if r.ImGui_BeginPopup(ctx, 'my_select_popup') then
      r.ImGui_Text(ctx, 'Aquarium')
      r.ImGui_Separator(ctx)
      for i,fish in ipairs(names) do
        if r.ImGui_Selectable(ctx, fish) then
          popups.popups.selected_fish = i
        end
      end
      r.ImGui_EndPopup(ctx)
    end

    -- Showing a menu with toggles
    if r.ImGui_Button(ctx, 'Toggle..') then
      r.ImGui_OpenPopup(ctx, 'my_toggle_popup')
    end
    if r.ImGui_BeginPopup(ctx, 'my_toggle_popup') then
      for i,fish in ipairs(names) do
        rv,popups.popups.toggles[i] = r.ImGui_MenuItem(ctx, fish, '', popups.popups.toggles[i]);
      end
      if r.ImGui_BeginMenu(ctx, 'Sub-menu') then
        r.ImGui_MenuItem(ctx, 'Click me')
        r.ImGui_EndMenu(ctx)
      end

      r.ImGui_Separator(ctx)
      r.ImGui_Text(ctx, 'Tooltip here')
      if r.ImGui_IsItemHovered(ctx) then
        r.ImGui_SetTooltip(ctx, 'I am a tooltip over a popup')
      end

      if r.ImGui_Button(ctx, 'Stacked Popup') then
        r.ImGui_OpenPopup(ctx, 'another popup')
      end
      if r.ImGui_BeginPopup(ctx, 'another popup') then
        for i,fish in ipairs(names) do
          rv,popups.popups.toggles[i] = r.ImGui_MenuItem(ctx, fish, '', popups.popups.toggles[i])
        end
        if r.ImGui_BeginMenu(ctx, 'Sub-menu') then
          r.ImGui_MenuItem(ctx, 'Click me');
          if r.ImGui_Button(ctx, 'Stacked Popup') then
            r.ImGui_OpenPopup(ctx, 'another popup')
          end
          if r.ImGui_BeginPopup(ctx, 'another popup') then
            r.ImGui_Text(ctx, 'I am the last one here.')
            r.ImGui_EndPopup(ctx)
          end
          r.ImGui_EndMenu(ctx)
        end
        r.ImGui_EndPopup(ctx)
      end
      r.ImGui_EndPopup(ctx)
    end

    -- Call the more complete ShowExampleMenuFile which we use in various places of this demo
    if r.ImGui_Button(ctx, 'File Menu..') then
      r.ImGui_OpenPopup(ctx, 'my_file_popup')
    end
    if r.ImGui_BeginPopup(ctx, 'my_file_popup') then
      demo.ShowExampleMenuFile()
      r.ImGui_EndPopup(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Context menus') then
    if not popups.context then
      popups.context = {
        value = 0.5,
        name  = 'Label1',
      }
    end

    -- BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
    --    if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
    --       OpenPopup(id);
    --    return BeginPopup(id);
    -- For more advanced uses you may want to replicate and customize this code.
    -- See details in BeginPopupContextItem().
    r.ImGui_Text(ctx, ('Value = %.6f (<-- right-click here)'):format(popups.context.value))
    if r.ImGui_BeginPopupContextItem(ctx, 'item context menu') then
      if r.ImGui_Selectable(ctx, 'Set to zero') then popups.context.value = 0.0      end
      if r.ImGui_Selectable(ctx, 'Set to PI')   then popups.context.value = 3.141592 end
      r.ImGui_SetNextItemWidth(ctx, -FLT_MIN)
      rv,popups.context.value = r.ImGui_DragDouble(ctx, '##Value', popups.context.value, 0.1, 0.0, 0.0)
      r.ImGui_EndPopup(ctx)
    end

    -- We can also use OpenPopupOnItemClick() which is the same as BeginPopupContextItem() but without the
    -- Begin() call. So here we will make it that clicking on the text field with the right mouse button (1)
    -- will toggle the visibility of the popup above.
    r.ImGui_Text(ctx, '(You can also right-click me to open the same popup as above.)')
    r.ImGui_OpenPopupOnItemClick(ctx, 'item context menu', 1)

    -- When used after an item that has an ID (e.g.Button), we can skip providing an ID to BeginPopupContextItem().
    -- BeginPopupContextItem() will use the last item ID as the popup ID.
    -- In addition here, we want to include your editable label inside the button label.
    -- We use the ### operator to override the ID (read FAQ about ID for details)
    r.ImGui_Button(ctx, ("Button: %s###Button"):format(popups.context.name)) -- ### operator override ID ignoring the preceding label
    if r.ImGui_BeginPopupContextItem(ctx) then
      r.ImGui_Text(ctx, 'Edit name:')
      rv,popups.context.name = r.ImGui_InputText(ctx, '##edit', popups.context.name)
      if r.ImGui_Button(ctx, 'Close') then
        r.ImGui_CloseCurrentPopup(ctx)
      end
      r.ImGui_EndPopup(ctx)
    end
    r.ImGui_SameLine(ctx); r.ImGui_Text(ctx, '(<-- right-click here)')

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Modals') then
    if not popups.modal then
      popups.modal = {
        dont_ask_me_next_time = false,
        item  = 1,
        color = 0x66b30080,
      }
    end

    r.ImGui_TextWrapped(ctx, 'Modal windows are like popups but the user cannot close them by clicking outside.')

    if r.ImGui_Button(ctx, 'Delete..') then
      r.ImGui_OpenPopup(ctx, 'Delete?')
    end

    -- Always center this window when appearing
    local center = {r.ImGui_Viewport_GetCenter(r.ImGui_GetMainViewport(ctx))}
    r.ImGui_SetNextWindowPos(ctx, center[1], center[2], r.ImGui_Cond_Appearing(), 0.5, 0.5)

    if r.ImGui_BeginPopupModal(ctx, 'Delete?', nil, r.ImGui_WindowFlags_AlwaysAutoResize()) then
      r.ImGui_Text(ctx, 'All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n')
      r.ImGui_Separator(ctx)

      --static int unused_i = 0;
      --r.ImGui_Combo("Combo", &unused_i, "Delete\0Delete harder\0");

      r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_FramePadding(), 0, 0)
      rv,popups.modal.dont_ask_me_next_time =
        r.ImGui_Checkbox(ctx, "Don't ask me next time", popups.modal.dont_ask_me_next_time)
      r.ImGui_PopStyleVar(ctx)

      if r.ImGui_Button(ctx, 'OK', 120, 0) then r.ImGui_CloseCurrentPopup(ctx) end
      r.ImGui_SetItemDefaultFocus(ctx)
      r.ImGui_SameLine(ctx)
      if r.ImGui_Button(ctx, 'Cancel', 120, 0) then r.ImGui_CloseCurrentPopup(ctx) end
      r.ImGui_EndPopup(ctx)
    end

    if r.ImGui_Button(ctx, 'Stacked modals..') then
      r.ImGui_OpenPopup(ctx, 'Stacked 1')
    end
    if r.ImGui_BeginPopupModal(ctx, 'Stacked 1', nil, r.ImGui_WindowFlags_MenuBar()) then
      if r.ImGui_BeginMenuBar(ctx) then
        if r.ImGui_BeginMenu(ctx, 'File') then
          if r.ImGui_MenuItem(ctx, 'Some menu item') then end
          r.ImGui_EndMenu(ctx)
        end
        r.ImGui_EndMenuBar(ctx)
      end
      r.ImGui_Text(ctx, 'Hello from Stacked The First\nUsing style.Colors[ImGuiCol_ModalWindowDimBg] behind it.')

      -- Testing behavior of widgets stacking their own regular popups over the modal.
      rv,popups.modal.item  = r.ImGui_Combo(ctx, 'Combo', popups.modal.item, 'aaaa\31bbbb\31cccc\31dddd\31eeee\31')
      rv,popups.modal.color = r.ImGui_ColorEdit4(ctx, 'color', popups.modal.color)

      if r.ImGui_Button(ctx, 'Add another modal..') then
        r.ImGui_OpenPopup(ctx, 'Stacked 2')
      end

      -- Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close button which
      -- will close the popup. Note that the visibility state of popups is owned by imgui, so the input value
      -- of the bool actually doesn't matter here.
      local unused_open = true
      if r.ImGui_BeginPopupModal(ctx, 'Stacked 2', unused_open) then
        r.ImGui_Text(ctx, 'Hello from Stacked The Second!')
        if r.ImGui_Button(ctx, 'Close') then
          r.ImGui_CloseCurrentPopup(ctx)
        end
        r.ImGui_EndPopup(ctx)
      end

      if r.ImGui_Button(ctx, 'Close') then
        r.ImGui_CloseCurrentPopup(ctx)
      end
      r.ImGui_EndPopup(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  if r.ImGui_TreeNode(ctx, 'Menus inside a regular window') then
    r.ImGui_TextWrapped(ctx, "Below we are testing adding menu items to a regular window. It's rather unusual but should work!")
    r.ImGui_Separator(ctx)

    -- Note: As a quirk in this very specific example, we want to differentiate the parent of this menu from the
    -- parent of the various popup menus above. To do so we are encloding the items in a PushID()/PopID() block
    -- to make them two different menusets. If we don't, opening any popup above and hovering our menu here would
    -- open it. This is because once a menu is active, we allow to switch to a sibling menu by just hovering on it,
    -- which is the desired behavior for regular menus.
    r.ImGui_PushID(ctx, 'foo')
    r.ImGui_MenuItem(ctx, 'Menu item', 'CTRL+M')
    if r.ImGui_BeginMenu(ctx, 'Menu inside a regular window') then
      demo.ShowExampleMenuFile()
      r.ImGui_EndMenu(ctx)
    end
    r.ImGui_PopID(ctx)
    r.ImGui_Separator(ctx)
    r.ImGui_TreePop(ctx)
  end
end

local MyItemColumnID_ID          = 4
local MyItemColumnID_Name        = 5
local MyItemColumnID_Quantity    = 6
local MyItemColumnID_Description = 7

function demo.CompareTableItems(a, b)
  local next_id = 0
  while true do
    local ok, col_user_id, col_idx, sort_order, sort_direction = r.ImGui_TableGetColumnSortSpecs(ctx, next_id)
    if not ok then break end
    next_id = next_id + 1

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

    local is_ascending = sort_direction == r.ImGui_SortDirection_Ascending()
    if a[key] < b[key] then
      return is_ascending
    elseif a[key] > b[key] then
      return not is_ascending
    end
  end

  -- table.sort is instable so always return a way to differenciate items.
  -- Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
  return a.id < b.id
end

-- Make the UI compact because there are so many fields
function demo.PushStyleCompact()
  local frame_padding = {r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_FramePadding())}
  local item_spacing = {r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_FramePadding())}
  r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_FramePadding(), frame_padding[1], frame_padding[2] * 0.60)
  r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing(), item_spacing[1], item_spacing[2] * 0.60)
end

function demo.PopStyleCompact()
  r.ImGui_PopStyleVar(ctx, 2)
end

-- Show a combo box with a choice of sizing policies
function demo.EditTableSizingFlags(flags)
  local policies = {
    {
      value   = r.ImGui_TableFlags_None(),
      name    = 'Default',
      tooltip = 'Use default sizing policy:\n- ImGuiTableFlags_SizingFixedFit if ScrollX is on or if host window has ImGuiWindowFlags_AlwaysAutoResize.\n- ImGuiTableFlags_SizingStretchSame otherwise.',
    },
    {
      value   = r.ImGui_TableFlags_SizingFixedFit(),
      name    = 'ImGuiTableFlags_SizingFixedFit',
      tooltip = 'Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching contents width.',
    },
    {
      value   = r.ImGui_TableFlags_SizingFixedSame(),
      name    = 'ImGuiTableFlags_SizingFixedSame',
      tooltip = 'Columns are all the same width, matching the maximum contents width.\nImplicitly disable ImGuiTableFlags_Resizable and enable ImGuiTableFlags_NoKeepColumnsVisible.',
    },
    {
      value   = r.ImGui_TableFlags_SizingStretchProp(),
      name    = 'ImGuiTableFlags_SizingStretchProp',
      tooltip = 'Columns default to _WidthStretch with weights proportional to their widths.',
    },
    {
      value   = r.ImGui_TableFlags_SizingStretchSame(),
      name    = 'ImGuiTableFlags_SizingStretchSame',
      tooltip = 'Columns default to _WidthStretch with same weights.',
    },
  }

  local sizing_mask = r.ImGui_TableFlags_SizingFixedFit()    |
                      r.ImGui_TableFlags_SizingFixedSame()   |
                      r.ImGui_TableFlags_SizingStretchProp() |
                      r.ImGui_TableFlags_SizingStretchSame()
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
  if r.ImGui_BeginCombo(ctx, 'Sizing Policy', preview_text) then
    for n,policy in ipairs(policies) do
      if r.ImGui_Selectable(ctx, policy.name, idx == n) then
        flags = (flags & ~sizing_mask) | policy.value
      end
    end
    r.ImGui_EndCombo(ctx)
  end
  r.ImGui_SameLine(ctx)
  r.ImGui_TextDisabled(ctx, '(?)')
  if r.ImGui_IsItemHovered(ctx) then
    r.ImGui_BeginTooltip(ctx)
    r.ImGui_PushTextWrapPos(ctx, r.ImGui_GetFontSize(ctx) * 50.0)
    for m,policy in ipairs(policies) do
      r.ImGui_Separator(ctx)
      r.ImGui_Text(ctx, ('%s:'):format(policy.name))
      r.ImGui_Separator(ctx)
      local indent_spacing = r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_IndentSpacing())
      r.ImGui_SetCursorPosX(ctx, r.ImGui_GetCursorPosX(ctx) + indent_spacing * 0.5)
      r.ImGui_Text(ctx, policy.tooltip)
    end
    r.ImGui_PopTextWrapPos(ctx)
    r.ImGui_EndTooltip(ctx)
  end

  return flags
end

function demo.EditTableColumnsFlags(flags)
  local rv
  local width_mask = r.ImGui_TableColumnFlags_WidthStretch() |
                     r.ImGui_TableColumnFlags_WidthFixed()

  rv,flags = r.ImGui_CheckboxFlags(ctx, '_DefaultHide', flags, r.ImGui_TableColumnFlags_DefaultHide())
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_DefaultSort', flags, r.ImGui_TableColumnFlags_DefaultSort())
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_WidthStretch', flags, r.ImGui_TableColumnFlags_WidthStretch())
  if rv then
    flags = flags & ~(width_mask ^ r.ImGui_TableColumnFlags_WidthStretch())
  end
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_WidthFixed', flags, r.ImGui_TableColumnFlags_WidthFixed())
  if rv then
    flags = flags & ~(width_mask ^ r.ImGui_TableColumnFlags_WidthFixed())
  end
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoResize', flags, r.ImGui_TableColumnFlags_NoResize());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoReorder', flags, r.ImGui_TableColumnFlags_NoReorder());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoHide', flags, r.ImGui_TableColumnFlags_NoHide());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoClip', flags, r.ImGui_TableColumnFlags_NoClip());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoSort', flags, r.ImGui_TableColumnFlags_NoSort());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoSortAscending', flags, r.ImGui_TableColumnFlags_NoSortAscending());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoSortDescending', flags, r.ImGui_TableColumnFlags_NoSortDescending());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_NoHeaderWidth', flags, r.ImGui_TableColumnFlags_NoHeaderWidth());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_PreferSortAscending', flags, r.ImGui_TableColumnFlags_PreferSortAscending());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_PreferSortDescending', flags, r.ImGui_TableColumnFlags_PreferSortDescending());
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_IndentEnable', flags, r.ImGui_TableColumnFlags_IndentEnable()); r.ImGui_SameLine(ctx); demo.HelpMarker("Default for column 0")
  rv,flags = r.ImGui_CheckboxFlags(ctx, '_IndentDisable', flags, r.ImGui_TableColumnFlags_IndentDisable()); r.ImGui_SameLine(ctx); demo.HelpMarker("Default for column >0")

  return flags
end

function demo.ShowTableColumnsStatusFlags(flags)
  r.ImGui_CheckboxFlags(ctx, '_IsEnabled', flags, r.ImGui_TableColumnFlags_IsEnabled())
  r.ImGui_CheckboxFlags(ctx, '_IsVisible', flags, r.ImGui_TableColumnFlags_IsVisible())
  r.ImGui_CheckboxFlags(ctx, '_IsSorted',  flags, r.ImGui_TableColumnFlags_IsSorted())
  r.ImGui_CheckboxFlags(ctx, '_IsHovered', flags, r.ImGui_TableColumnFlags_IsHovered())
end

function demo.ShowDemoWindowTables()
  -- r.ImGui_SetNextItemOpen(ctx, true, r.ImGui_Cond_Once())
  if not r.ImGui_CollapsingHeader(ctx, 'Tables') then
    return
  end

  local rv

  -- Using those as a base value to create width/height that are factor of the size of our font
  local TEXT_BASE_WIDTH  = r.ImGui_CalcTextSize(ctx, 'A')
  local TEXT_BASE_HEIGHT = r.ImGui_GetTextLineHeightWithSpacing(ctx)

  r.ImGui_PushID(ctx, 'Tables')

  local open_action = -1
  if r.ImGui_Button(ctx, 'Open all') then
    open_action = 1;
  end
  r.ImGui_SameLine(ctx)
  if r.ImGui_Button(ctx, 'Close all') then
    open_action = 0;
  end
  r.ImGui_SameLine(ctx)

  if tables.disable_indent == nil then
    tables.disable_indent = false
  end

  -- Options
  rv,tables.disable_indent = r.ImGui_Checkbox(ctx, 'Disable tree indentation', tables.disable_indent)
  r.ImGui_SameLine(ctx)
  demo.HelpMarker('Disable the indenting of tree nodes so demo tables can use the full window width.')
  r.ImGui_Separator(ctx)
  if tables.disable_indent then
    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_IndentSpacing(), 0.0)
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
      r.ImGui_SetNextItemOpen(ctx, open_action ~= 0)
    end
  end

  -- Demos
  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Basic') then
    -- Here we will showcase three different ways to output a table.
    -- They are very simple variations of a same thing!

    -- [Method 1] Using TableNextRow() to create a new row, and TableSetColumnIndex() to select the column.
    -- In many situations, this is the most flexible and easy to use pattern.
    demo.HelpMarker('Using TableNextRow() + calling TableSetColumnIndex() _before_ each cell, in a loop.')
    if r.ImGui_BeginTable(ctx, 'table1', 3) then
      for row = 0, 3 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('Row %d Column %d'):format(row, column))
        end
      end
      r.ImGui_EndTable(ctx)
    end

    -- [Method 2] Using TableNextColumn() called multiple times, instead of using a for loop + TableSetColumnIndex().
    -- This is generally more convenient when you have code manually submitting the contents of each columns.
    demo.HelpMarker('Using TableNextRow() + calling TableNextColumn() _before_ each cell, manually.')
    if r.ImGui_BeginTable(ctx, 'table2', 3) then
      for row = 0, 3 do
        r.ImGui_TableNextRow(ctx)
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, ('Row %d'):format(row))
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, 'Some contents')
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, '123.456')
      end
      r.ImGui_EndTable(ctx)
    end

    -- [Method 3] We call TableNextColumn() _before_ each cell. We never call TableNextRow(),
    -- as TableNextColumn() will automatically wrap around and create new roes as needed.
    -- This is generally more convenient when your cells all contains the same type of data.
    demo.HelpMarker(
      "Only using TableNextColumn(), which tends to be convenient for tables where every cells contains the same type of contents.\n\z
       This is also more similar to the old NextColumn() function of the Columns API, and provided to facilitate the Columns->Tables API transition.")
    if r.ImGui_BeginTable(ctx, 'table3', 3) then
      for item = 0, 13 do
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, ('Item %d'):format(item))
      end
      r.ImGui_EndTable(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Borders, background') then
    if not tables.borders_bg then
      tables.borders_bg = {
        flags = r.ImGui_TableFlags_Borders() | r.ImGui_TableFlags_RowBg(),
        display_headers = false,
        contents_type = 0,
      }
    end
    -- Expose a few Borders related flags interactively

    demo.PushStyleCompact()
    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.borders_bg.flags, r.ImGui_TableFlags_RowBg())
    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Borders', tables.borders_bg.flags, r.ImGui_TableFlags_Borders())
    r.ImGui_SameLine(ctx); demo.HelpMarker('ImGuiTableFlags_Borders\n = ImGuiTableFlags_BordersInnerV\n | ImGuiTableFlags_BordersOuterV\n | ImGuiTableFlags_BordersInnerV\n | ImGuiTableFlags_BordersOuterH')
    r.ImGui_Indent(ctx)

    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersH', tables.borders_bg.flags, r.ImGui_TableFlags_BordersH())
    r.ImGui_Indent(ctx)
    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterH', tables.borders_bg.flags, r.ImGui_TableFlags_BordersOuterH())
    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerH', tables.borders_bg.flags, r.ImGui_TableFlags_BordersInnerH())
    r.ImGui_Unindent(ctx)

    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.borders_bg.flags, r.ImGui_TableFlags_BordersV())
    r.ImGui_Indent(ctx)
    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.borders_bg.flags, r.ImGui_TableFlags_BordersOuterV())
    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.borders_bg.flags, r.ImGui_TableFlags_BordersInnerV())
    r.ImGui_Unindent(ctx)

    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuter', tables.borders_bg.flags, r.ImGui_TableFlags_BordersOuter())
    rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInner', tables.borders_bg.flags, r.ImGui_TableFlags_BordersInner())
    r.ImGui_Unindent(ctx)

    r.ImGui_AlignTextToFramePadding(ctx); r.ImGui_Text(ctx, 'Cell contents:')
    r.ImGui_SameLine(ctx); rv,tables.borders_bg.contents_type = r.ImGui_RadioButtonEx(ctx, 'Text', tables.borders_bg.contents_type, 0)
    r.ImGui_SameLine(ctx); rv,tables.borders_bg.contents_type = r.ImGui_RadioButtonEx(ctx, 'FillButton', tables.borders_bg.contents_type, 1)
    rv,tables.borders_bg.display_headers = r.ImGui_Checkbox(ctx, 'Display headers', tables.borders_bg.display_headers)
    -- rv,tables.borders_bg.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBody', tables.borders_bg.flags, r.ImGui_TableFlags_NoBordersInBody()); r.ImGui_SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body (borders will always appears in Headers');
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table1', 3, tables.borders_bg.flags) then
      -- Display headers so we can inspect their interaction with borders.
      -- (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
      if tables.borders_bg.display_headers then
        r.ImGui_TableSetupColumn(ctx, 'One')
        r.ImGui_TableSetupColumn(ctx, 'Two')
        r.ImGui_TableSetupColumn(ctx, 'Three')
        r.ImGui_TableHeadersRow(ctx)
      end

      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          local buf = ('Hello %d,%d'):format(column, row)
          if tables.borders_bg.contents_type == 0 then
            r.ImGui_Text(ctx, buf)
          else
            r.ImGui_Button(ctx, buf, -FLT_MIN, 0.0)
          end
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Resizable, stretch') then
    if not tables.resz_stretch then
      tables.resz_stretch = {
        flags = r.ImGui_TableFlags_SizingStretchSame() |
                r.ImGui_TableFlags_Resizable() |
                r.ImGui_TableFlags_BordersOuter() |
                r.ImGui_TableFlags_BordersV() |
                r.ImGui_TableFlags_ContextMenuInBody(),
      }
    end

    -- By default, if we don't enable ScrollX the sizing policy for each columns is "Stretch"
    -- Each columns maintain a sizing weight, and they will occupy all available width.
    demo.PushStyleCompact()
    rv,tables.resz_stretch.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.resz_stretch.flags, r.ImGui_TableFlags_Resizable())
    rv,tables.resz_stretch.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.resz_stretch.flags, r.ImGui_TableFlags_BordersV())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Using the _Resizable flag automatically enables the _BordersInnerV flag as well, this is why the resize borders are still showing when unchecking this.')
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table1', 3, tables.resz_stretch.flags) then
      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('Hello %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Resizable, fixed') then
    if not tables.resz_fixed then
      tables.resz_fixed = {
        flags = r.ImGui_TableFlags_SizingFixedFit() |
                r.ImGui_TableFlags_Resizable() |
                r.ImGui_TableFlags_BordersOuter() |
                r.ImGui_TableFlags_BordersV() |
                r.ImGui_TableFlags_ContextMenuInBody(),
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
    rv,tables.resz_fixed.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.resz_fixed.flags, r.ImGui_TableFlags_NoHostExtendX())
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table1', 3, tables.resz_fixed.flags) then
      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('Hello %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, "Resizable, mixed") then
    if not tables.resz_mixed then
      tables.resz_mixed = {
        flags = r.ImGui_TableFlags_SizingFixedFit() |
                r.ImGui_TableFlags_RowBg() | r.ImGui_TableFlags_Borders() |
                r.ImGui_TableFlags_Resizable() |
                r.ImGui_TableFlags_Reorderable() | r.ImGui_TableFlags_Hideable()
      }
    end
    demo.HelpMarker(
      'Using TableSetupColumn() to alter resizing policy on a per-column basis.\n\n\z
       When combining Fixed and Stretch columns, generally you only want one, maybe two trailing columns to use _WidthStretch.')

    if r.ImGui_BeginTable(ctx, 'table1', 3, tables.resz_mixed.flags) then
      r.ImGui_TableSetupColumn(ctx, 'AAA', r.ImGui_TableColumnFlags_WidthFixed())
      r.ImGui_TableSetupColumn(ctx, 'BBB', r.ImGui_TableColumnFlags_WidthFixed())
      r.ImGui_TableSetupColumn(ctx, 'CCC', r.ImGui_TableColumnFlags_WidthStretch())
      r.ImGui_TableHeadersRow(ctx)
      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('%s %d,%d'):format(column == 2 and 'Stretch' or 'Fixed', column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end
    if r.ImGui_BeginTable(ctx, 'table2', 6, tables.resz_mixed.flags) then
      r.ImGui_TableSetupColumn(ctx, 'AAA', r.ImGui_TableColumnFlags_WidthFixed())
      r.ImGui_TableSetupColumn(ctx, 'BBB', r.ImGui_TableColumnFlags_WidthFixed())
      r.ImGui_TableSetupColumn(ctx, 'CCC', r.ImGui_TableColumnFlags_WidthFixed() | r.ImGui_TableColumnFlags_DefaultHide())
      r.ImGui_TableSetupColumn(ctx, 'DDD', r.ImGui_TableColumnFlags_WidthStretch())
      r.ImGui_TableSetupColumn(ctx, 'EEE', r.ImGui_TableColumnFlags_WidthStretch())
      r.ImGui_TableSetupColumn(ctx, 'FFF', r.ImGui_TableColumnFlags_WidthStretch() | r.ImGui_TableColumnFlags_DefaultHide())
      r.ImGui_TableHeadersRow(ctx)
      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 5 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('%s %d,%d'):format(column >= 3 and 'Stretch' or 'Fixed', column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Reorderable, hideable, with headers') then
    if not tables.reorder then
      tables.reorder = {
        flags = r.ImGui_TableFlags_Resizable() |
                r.ImGui_TableFlags_Reorderable() |
                r.ImGui_TableFlags_Hideable() |
                r.ImGui_TableFlags_BordersOuter() |
                r.ImGui_TableFlags_BordersV()
      }
    end

    demo.HelpMarker(
      'Click and drag column headers to reorder columns.\n\n\z
       Right-click on a header to open a context menu.')
    demo.PushStyleCompact()
    rv,tables.reorder.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.reorder.flags, r.ImGui_TableFlags_Resizable())
    rv,tables.reorder.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Reorderable', tables.reorder.flags, r.ImGui_TableFlags_Reorderable())
    rv,tables.reorder.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Hideable', tables.reorder.flags, r.ImGui_TableFlags_Hideable())
    -- rv,tables.reorder.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBody', tables.reorder.flags, r.ImGui_TableFlags_NoBordersInBody())
    -- rv,tables.reorder.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBodyUntilResize', tables.reorder.flags, r.ImGui_TableFlags_NoBordersInBodyUntilResize()); r.ImGui_SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body until hovered for resize (borders will always appears in Headers)')
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table1', 3, tables.reorder.flags) then
      -- Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
      -- (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
      r.ImGui_TableSetupColumn(ctx, 'One')
      r.ImGui_TableSetupColumn(ctx, 'Two')
      r.ImGui_TableSetupColumn(ctx, 'Three')
      r.ImGui_TableHeadersRow(ctx)
      for row = 0, 5 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('Hello %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end

    -- Use outer_size.x == 0.0f instead of default to make the table as tight as possible (only valid when no scrolling and no stretch column)
    if r.ImGui_BeginTable(ctx, 'table2', 3, tables.reorder.flags | r.ImGui_TableFlags_SizingFixedFit(), 0.0, 0.0) then
      r.ImGui_TableSetupColumn(ctx, 'One')
      r.ImGui_TableSetupColumn(ctx, 'Two')
      r.ImGui_TableSetupColumn(ctx, 'Three')
      r.ImGui_TableHeadersRow(ctx)
      for row = 0, 5 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('Fixed %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Padding') then
    if not tables.padding then
      tables.padding = {
        flags1 = r.ImGui_TableFlags_BordersV(),
        show_headers = false,

        flags2 = r.ImGui_TableFlags_Borders() | r.ImGui_TableFlags_RowBg(),
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
       In this demo we don't show horizontal borders to emphasis how they don't affect default horizontal padding.")

    demo.PushStyleCompact()
    rv,tables.padding.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_PadOuterX', tables.padding.flags1, r.ImGui_TableFlags_PadOuterX())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Enable outer-most padding (default if ImGuiTableFlags_BordersOuterV is set)');
    rv,tables.padding.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadOuterX', tables.padding.flags1, r.ImGui_TableFlags_NoPadOuterX())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Disable outer-most padding (default if ImGuiTableFlags_BordersOuterV is not set)');
    rv,tables.padding.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadInnerX', tables.padding.flags1, r.ImGui_TableFlags_NoPadInnerX())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off)')
    rv,tables.padding.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.padding.flags1, r.ImGui_TableFlags_BordersOuterV())
    rv,tables.padding.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.padding.flags1, r.ImGui_TableFlags_BordersInnerV())
    rv,tables.padding.show_headers = r.ImGui_Checkbox(ctx, 'show_headers', tables.padding.show_headers)
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table_padding', 3, tables.padding.flags1) then
      if tables.padding.show_headers then
        r.ImGui_TableSetupColumn(ctx, 'One')
        r.ImGui_TableSetupColumn(ctx, 'Two')
        r.ImGui_TableSetupColumn(ctx, 'Three')
        r.ImGui_TableHeadersRow(ctx)
      end

      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          if row == 0 then
            r.ImGui_Text(ctx, ('Avail %.2f'):format(({r.ImGui_GetContentRegionAvail(ctx)})[1]))
          else
            local buf = ('Hello %d,%d'):format(column, row)
            r.ImGui_Button(ctx, buf, -FLT_MIN, 0.0)
          end
          --if (r.ImGui_TableGetColumnFlags() & ImGuiTableColumnFlags_IsHovered)
          --  r.ImGui_TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 100, 0, 255));
        end
      end
      r.ImGui_EndTable(ctx)
    end

    -- Second example: set style.CellPadding to (0.0) or a custom value.
    -- FIXME-TABLE: Vertical border effectively not displayed the same way as horizontal one...
    demo.HelpMarker('Setting style.CellPadding to (0,0) or a custom value.')

    demo.PushStyleCompact()
    rv,tables.padding.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Borders', tables.padding.flags2, r.ImGui_TableFlags_Borders())
    rv,tables.padding.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersH', tables.padding.flags2, r.ImGui_TableFlags_BordersH())
    rv,tables.padding.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.padding.flags2, r.ImGui_TableFlags_BordersV())
    rv,tables.padding.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInner', tables.padding.flags2, r.ImGui_TableFlags_BordersInner())
    rv,tables.padding.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuter', tables.padding.flags2, r.ImGui_TableFlags_BordersOuter())
    rv,tables.padding.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.padding.flags2, r.ImGui_TableFlags_RowBg())
    rv,tables.padding.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.padding.flags2, r.ImGui_TableFlags_Resizable())
    rv,tables.padding.show_widget_frame_bg = r.ImGui_Checkbox(ctx, 'show_widget_frame_bg', tables.padding.show_widget_frame_bg)
    rv,tables.padding.cell_padding[1],tables.padding.cell_padding[2] =
      r.ImGui_SliderDouble2(ctx, 'CellPadding', tables.padding.cell_padding[1],
      tables.padding.cell_padding[2], 0.0, 10.0, '%.0f')
    demo.PopStyleCompact()

    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_CellPadding(), table.unpack(tables.padding.cell_padding))
    if r.ImGui_BeginTable(ctx, 'table_padding_2', 3, tables.padding.flags2) then
      if not tables.padding.show_widget_frame_bg then
        r.ImGui_PushStyleColor(ctx, r.ImGui_Col_FrameBg(), 0)
      end
      for cell = 1, 3 * 5 do
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_SetNextItemWidth(ctx, -FLT_MIN)
        r.ImGui_PushID(ctx, cell)
        rv,tables.padding.text_bufs[cell] = r.ImGui_InputText(ctx, '##cell', tables.padding.text_bufs[cell])
        r.ImGui_PopID(ctx)
      end
      if not tables.padding.show_widget_frame_bg then
        r.ImGui_PopStyleColor(ctx)
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_PopStyleVar(ctx)

    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Sizing policies') then
    if not tables.sz_policies then
      tables.sz_policies = {
        flags1 = r.ImGui_TableFlags_BordersV()      |
                 r.ImGui_TableFlags_BordersOuterH() |
                 r.ImGui_TableFlags_RowBg()         |
                 r.ImGui_TableFlags_ContextMenuInBody(),
        sizing_policy_flags = {
          r.ImGui_TableFlags_SizingFixedFit(),
          r.ImGui_TableFlags_SizingFixedSame(),
          r.ImGui_TableFlags_SizingStretchProp(),
          r.ImGui_TableFlags_SizingStretchSame(),
        },

        flags2 = r.ImGui_TableFlags_ScrollY() |
                 r.ImGui_TableFlags_Borders() |
                 r.ImGui_TableFlags_RowBg()   |
                 r.ImGui_TableFlags_Resizable(),
        contents_type = 0,
        column_count  = 3,
        text_buf      = '',
      }
    end

    demo.PushStyleCompact()
    rv,tables.sz_policies.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.sz_policies.flags1, r.ImGui_TableFlags_Resizable())
    rv,tables.sz_policies.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.sz_policies.flags1, r.ImGui_TableFlags_NoHostExtendX())
    demo.PopStyleCompact()

    for table_n,sizing_flags in ipairs(tables.sz_policies.sizing_policy_flags) do
      r.ImGui_PushID(ctx, table_n)
      r.ImGui_SetNextItemWidth(ctx, TEXT_BASE_WIDTH * 30)
      sizing_flags = demo.EditTableSizingFlags(sizing_flags)
      tables.sz_policies.sizing_policy_flags[table_n] = sizing_flags

      -- To make it easier to understand the different sizing policy,
      -- For each policy: we display one table where the columns have equal contents width, and one where the columns have different contents width.
      if r.ImGui_BeginTable(ctx, 'table1', 3, sizing_flags | tables.sz_policies.flags1) then
        for row = 0, 2 do
          r.ImGui_TableNextRow(ctx)
          r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'Oh dear')
          r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'Oh dear')
          r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'Oh dear')
        end
        r.ImGui_EndTable(ctx)
      end
      if r.ImGui_BeginTable(ctx, 'table2', 3, sizing_flags | tables.sz_policies.flags1) then
        for row = 0, 2 do
          r.ImGui_TableNextRow(ctx)
          r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'AAAA')
          r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'BBBBBBBB')
          r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'CCCCCCCCCCCC')
        end
        r.ImGui_EndTable(ctx)
      end
      r.ImGui_PopID(ctx)
    end

    r.ImGui_Spacing(ctx)
    r.ImGui_Text(ctx, 'Advanced')
    r.ImGui_SameLine(ctx)
    demo.HelpMarker('This section allows you to interact and see the effect of various sizing policies depending on whether Scroll is enabled and the contents of your columns.')

    demo.PushStyleCompact()
    r.ImGui_PushID(ctx, 'Advanced')
    r.ImGui_PushItemWidth(ctx, TEXT_BASE_WIDTH * 30)
    tables.sz_policies.flags2 = demo.EditTableSizingFlags(tables.sz_policies.flags2)
    rv,tables.sz_policies.contents_type = r.ImGui_Combo(ctx, 'Contents', tables.sz_policies.contents_type, 'Show width\31Short Text\31Long Text\31Button\31Fill Button\31InputText\31')
    if tables.sz_policies.contents_type == 4 then -- fill button
      r.ImGui_SameLine(ctx)
      demo.HelpMarker('Be mindful that using right-alignment (e.g. size.x = -FLT_MIN) creates a feedback loop where contents width can feed into auto-column width can feed into contents width.');
    end
    rv,tables.sz_policies.column_count = r.ImGui_DragInt(ctx, 'Columns', tables.sz_policies.column_count, 0.1, 1, 64, '%d', r.ImGui_SliderFlags_AlwaysClamp())
    rv,tables.sz_policies.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.sz_policies.flags2, r.ImGui_TableFlags_Resizable())
    rv,tables.sz_policies.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_PreciseWidths', tables.sz_policies.flags2, r.ImGui_TableFlags_PreciseWidths())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.')
    rv,tables.sz_policies.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.sz_policies.flags2, r.ImGui_TableFlags_ScrollX())
    rv,tables.sz_policies.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.sz_policies.flags2, r.ImGui_TableFlags_ScrollY())
    rv,tables.sz_policies.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoClip', tables.sz_policies.flags2, r.ImGui_TableFlags_NoClip())
    r.ImGui_PopItemWidth(ctx)
    r.ImGui_PopID(ctx)
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table2', tables.sz_policies.column_count, tables.sz_policies.flags2, 0.0, TEXT_BASE_HEIGHT * 7) then
      for cell = 1, 10 * tables.sz_policies.column_count do
        r.ImGui_TableNextColumn(ctx)
        local column = r.ImGui_TableGetColumnIndex(ctx)
        local row = r.ImGui_TableGetRowIndex(ctx)

        r.ImGui_PushID(ctx, cell)
        local label = ('Hello %d,%d'):format(column, row)
        local contents_type = tables.sz_policies.contents_type
        if contents_type == 1 then -- short text
          r.ImGui_Text(ctx, label)
        elseif contents_type == 2 then -- long text
          r.ImGui_Text(ctx, ('Some %s text %d,%d\nOver two lines..'):format(column == 0 and 'long' or 'longeeer', column, row))
        elseif contents_type == 0 then -- show width
          r.ImGui_Text(ctx, ('W: %.1f'):format(({r.ImGui_GetContentRegionAvail(ctx)})[1]))
        elseif contents_type == 3 then -- button
          r.ImGui_Button(ctx, label)
        elseif contents_type == 4 then -- fill button
          r.ImGui_Button(ctx, label, -FLT_MIN, 0.0)
        elseif contents_type == 5 then -- input text
          r.ImGui_SetNextItemWidth(ctx, -FLT_MIN)
          rv,tables.sz_policies.text_buf = r.ImGui_InputText(ctx, '##', tables.sz_policies.text_buf)
        end
        r.ImGui_PopID(ctx)
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Vertical scrolling, with clipping') then
    if not tables.vertical then
      tables.vertical = {
        flags = r.ImGui_TableFlags_ScrollY()      |
                r.ImGui_TableFlags_RowBg()        |
                r.ImGui_TableFlags_BordersOuter() |
                r.ImGui_TableFlags_BordersV()     |
                r.ImGui_TableFlags_Resizable()    |
                r.ImGui_TableFlags_Reorderable()  |
                r.ImGui_TableFlags_Hideable(),
      }
    end

    demo.HelpMarker('Here we activate ScrollY, which will create a child window container to allow hosting scrollable contents.\n\nWe also demonstrate using ImGuiListClipper to virtualize the submission of many items.');

    demo.PushStyleCompact()
    rv,tables.vertical.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.vertical.flags, r.ImGui_TableFlags_ScrollY())
    demo.PopStyleCompact()

    -- When using ScrollX or ScrollY we need to specify a size for our table container!
    -- Otherwise by default the table will fit all available space, like a BeginChild() call.
    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 8 }
    if r.ImGui_BeginTable(ctx, 'table_scrolly', 3, tables.vertical.flags, table.unpack(outer_size)) then
      r.ImGui_TableSetupScrollFreeze(ctx, 0, 1); -- Make top row always visible
      r.ImGui_TableSetupColumn(ctx, 'One', r.ImGui_TableColumnFlags_None())
      r.ImGui_TableSetupColumn(ctx, 'Two', r.ImGui_TableColumnFlags_None())
      r.ImGui_TableSetupColumn(ctx, 'Three', r.ImGui_TableColumnFlags_None())
      r.ImGui_TableHeadersRow(ctx)

      -- Demonstrate using clipper for large vertical lists
      local clipper = r.ImGui_CreateListClipper(ctx)
      r.ImGui_ListClipper_Begin(clipper, 1000)
      while r.ImGui_ListClipper_Step(clipper) do
        local display_start, display_end = r.ImGui_ListClipper_GetDisplayRange(clipper)
        for row = display_start, display_end - 1 do
          r.ImGui_TableNextRow(ctx)
          for column = 0, 2 do
            r.ImGui_TableSetColumnIndex(ctx, column)
            r.ImGui_Text(ctx, ('Hello %d,%d'):format(column, row))
          end
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Horizontal scrolling') then
    if not tables.horizontal then
      tables.horizontal = {
        flags1 = r.ImGui_TableFlags_ScrollX()      |
                 r.ImGui_TableFlags_ScrollY()      |
                 r.ImGui_TableFlags_RowBg()        |
                 r.ImGui_TableFlags_BordersOuter() |
                 r.ImGui_TableFlags_BordersV()     |
                 r.ImGui_TableFlags_Resizable()    |
                 r.ImGui_TableFlags_Reorderable()  |
                 r.ImGui_TableFlags_Hideable(),
        freeze_cols = 1,
        freeze_rows = 1,

        flags2 = r.ImGui_TableFlags_SizingStretchSame() |
                 r.ImGui_TableFlags_ScrollX()           |
                 r.ImGui_TableFlags_ScrollY()           |
                 r.ImGui_TableFlags_BordersOuter()      |
                 r.ImGui_TableFlags_RowBg()             |
                 r.ImGui_TableFlags_ContextMenuInBody(),
        inner_width = 1000.0,
      }
    end

    demo.HelpMarker(
      "When ScrollX is enabled, the default sizing policy becomes ImGuiTableFlags_SizingFixedFit, \z
       as automatically stretching columns doesn't make much sense with horizontal scrolling.\n\n\z
       Also note that as of the current version, you will almost always want to enable ScrollY along with ScrollX,\z
       because the container window won't automatically extend vertically to fix contents (this may be improved in future versions).")

    demo.PushStyleCompact()
    rv,tables.horizontal.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.horizontal.flags1, r.ImGui_TableFlags_Resizable())
    rv,tables.horizontal.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.horizontal.flags1, r.ImGui_TableFlags_ScrollX())
    rv,tables.horizontal.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.horizontal.flags1, r.ImGui_TableFlags_ScrollY())
    r.ImGui_SetNextItemWidth(ctx, r.ImGui_GetFrameHeight(ctx))
    rv,tables.horizontal.freeze_cols = r.ImGui_DragInt(ctx, 'freeze_cols', tables.horizontal.freeze_cols, 0.2, 0, 9, nil, r.ImGui_SliderFlags_NoInput())
    r.ImGui_SetNextItemWidth(ctx, r.ImGui_GetFrameHeight(ctx))
    rv,tables.horizontal.freeze_rows = r.ImGui_DragInt(ctx, 'freeze_rows', tables.horizontal.freeze_rows, 0.2, 0, 9, nil, r.ImGui_SliderFlags_NoInput())
    demo.PopStyleCompact()

    -- When using ScrollX or ScrollY we need to specify a size for our table container!
    -- Otherwise by default the table will fit all available space, like a BeginChild() call.
    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 8 }
    if r.ImGui_BeginTable(ctx, 'table_scrollx', 7, tables.horizontal.flags1, table.unpack(outer_size)) then
      r.ImGui_TableSetupScrollFreeze(ctx, tables.horizontal.freeze_cols, tables.horizontal.freeze_rows)
      r.ImGui_TableSetupColumn(ctx, 'Line #', r.ImGui_TableColumnFlags_NoHide()) -- Make the first column not hideable to match our use of TableSetupScrollFreeze()
      r.ImGui_TableSetupColumn(ctx, 'One')
      r.ImGui_TableSetupColumn(ctx, 'Two')
      r.ImGui_TableSetupColumn(ctx, 'Three')
      r.ImGui_TableSetupColumn(ctx, 'Four')
      r.ImGui_TableSetupColumn(ctx, 'Five')
      r.ImGui_TableSetupColumn(ctx, 'Six')
      r.ImGui_TableHeadersRow(ctx)
      for row = 0, 19 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 6 do
          -- Both TableNextColumn() and TableSetColumnIndex() return true when a column is visible or performing width measurement.
          -- Because here we know that:
          -- - A) all our columns are contributing the same to row height
          -- - B) column 0 is always visible,
          -- We only always submit this one column and can skip others.
          -- More advanced per-column clipping behaviors may benefit from polling the status flags via TableGetColumnFlags().
          if r.ImGui_TableSetColumnIndex(ctx, column) or column == 0 then
            if column == 0 then
              r.ImGui_Text(ctx, ('Line %d'):format(row))
            else
              r.ImGui_Text(ctx, ('Hello world %d,%d'):format(column, row))
            end
          end
        end
      end
      r.ImGui_EndTable(ctx)
    end

    r.ImGui_Spacing(ctx)
    r.ImGui_Text(ctx, 'Stretch + ScrollX')
    r.ImGui_SameLine(ctx)
    demo.HelpMarker(
      "Showcase using Stretch columns + ScrollX together: \z
       this is rather unusual and only makes sense when specifying an 'inner_width' for the table!\n\z
       Without an explicit value, inner_width is == outer_size.x and therefore using Stretch columns + ScrollX together doesn't make sense.")
    demo.PushStyleCompact()
    r.ImGui_PushID(ctx, 'flags3')
    r.ImGui_PushItemWidth(ctx, TEXT_BASE_WIDTH * 30)
    rv,tables.horizontal.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.horizontal.flags2, r.ImGui_TableFlags_ScrollX())
    rv,tables.horizontal.inner_width = r.ImGui_DragDouble(ctx, 'inner_width', tables.horizontal.inner_width, 1.0, 0.0, FLT_MAX, '%.1f')
    r.ImGui_PopItemWidth(ctx)
    r.ImGui_PopID(ctx)
    demo.PopStyleCompact()
    if r.ImGui_BeginTable(ctx, 'table2', 7, tables.horizontal.flags2, outer_size[1], outer_size[2], tables.horizontal.inner_width) then
      for cell = 1, 20 * 7 do
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, ('Hello world %d,%d'):format(r.ImGui_TableGetColumnIndex(ctx), r.ImGui_TableGetRowIndex(ctx)))
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Columns flags') then
    if not tables.col_flags then
      tables.col_flags = {
        columns = {
          { name='One',   flags=r.ImGui_TableColumnFlags_DefaultSort(), flags_out=0 },
          { name='Two',   flags=r.ImGui_TableColumnFlags_None(),        flags_out=0 },
          { name='Three', flags=r.ImGui_TableColumnFlags_DefaultHide(), flags_out=0 },
        },
      }
    end

    -- Create a first table just to show all the options/flags we want to make visible in our example!
    if r.ImGui_BeginTable(ctx, 'table_columns_flags_checkboxes', #tables.col_flags.columns, r.ImGui_TableFlags_None()) then
      demo.PushStyleCompact()
      for i,column in ipairs(tables.col_flags.columns) do
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_PushID(ctx, i)
        r.ImGui_AlignTextToFramePadding(ctx) -- FIXME-TABLE: Workaround for wrong text baseline propagation
        r.ImGui_Text(ctx, ("'%s'"):format(column.name))
        r.ImGui_Spacing(ctx)
        r.ImGui_Text(ctx, 'Input flags:')
        column.flags = demo.EditTableColumnsFlags(column.flags)
        r.ImGui_Spacing(ctx)
        r.ImGui_Text(ctx, 'Output flags:')
        demo.ShowTableColumnsStatusFlags(column.flags_out)
        r.ImGui_PopID(ctx)
      end
      demo.PopStyleCompact()
      r.ImGui_EndTable(ctx)
    end

    -- Create the real table we care about for the example!
    -- We use a scrolling table to be able to showcase the difference between the _IsEnabled and _IsVisible flags above, otherwise in
    -- a non-scrolling table columns are always visible (unless using ImGuiTableFlags_NoKeepColumnsVisible + resizing the parent window down)
    local flags = r.ImGui_TableFlags_SizingFixedFit() |
                  r.ImGui_TableFlags_ScrollX()        |
                  r.ImGui_TableFlags_ScrollY()        |
                  r.ImGui_TableFlags_RowBg()          |
                  r.ImGui_TableFlags_BordersOuter()   |
                  r.ImGui_TableFlags_BordersV()       |
                  r.ImGui_TableFlags_Resizable()      |
                  r.ImGui_TableFlags_Reorderable()    |
                  r.ImGui_TableFlags_Hideable()       |
                  r.ImGui_TableFlags_Sortable()
    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 9 }
    if r.ImGui_BeginTable(ctx, 'table_columns_flags', #tables.col_flags.columns, flags, table.unpack(outer_size)) then
      for i,column in ipairs(tables.col_flags.columns) do
        r.ImGui_TableSetupColumn(ctx, column.name, column.flags)
      end
      r.ImGui_TableHeadersRow(ctx)
      for i,column in ipairs(tables.col_flags.columns) do
        column.flags_out = r.ImGui_TableGetColumnFlags(ctx, i - 1)
      end
      local indent_step = TEXT_BASE_WIDTH / 2
      for row = 0, 7 do
        r.ImGui_Indent(ctx, indent_step); -- Add some indentation to demonstrate usage of per-column IndentEnable/IndentDisable flags.
        r.ImGui_TableNextRow(ctx)
        for column = 0, #tables.col_flags.columns - 1 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('%s %s'):format(column == 0 and 'Indented' or 'Hello', r.ImGui_TableGetColumnName(ctx, column)))
        end
      end
      r.ImGui_Unindent(ctx, indent_step * 8.0)

      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Columns widths') then
    if not tables.col_widths then
      tables.col_widths = {
        flags1 = r.ImGui_TableFlags_Borders(), --|
                 -- r.ImGui_TableFlags_NoBordersInBodyUntilResize(),
        flags2 = r.ImGui_TableFlags_None(),
      }
    end
    demo.HelpMarker('Using TableSetupColumn() to setup default width.')

    demo.PushStyleCompact()
    rv,tables.col_widths.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.col_widths.flags1, r.ImGui_TableFlags_Resizable())
    -- rv,tables.col_widths.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBodyUntilResize', tables.col_widths.flags1, r.ImGui_TableFlags_NoBordersInBodyUntilResize())
    demo.PopStyleCompact()
    if r.ImGui_BeginTable(ctx, 'table1', 3, tables.col_widths.flags1) then
      -- We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
      r.ImGui_TableSetupColumn(ctx, 'one', r.ImGui_TableColumnFlags_WidthFixed(), 100.0) -- Default to 100.0f
      r.ImGui_TableSetupColumn(ctx, 'two', r.ImGui_TableColumnFlags_WidthFixed(), 200.0) -- Default to 200.0f
      r.ImGui_TableSetupColumn(ctx, 'three', r.ImGui_TableColumnFlags_WidthFixed());     -- Default to auto
      r.ImGui_TableHeadersRow(ctx)
      for row = 0, 3 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          if row == 0 then
            r.ImGui_Text(ctx, ('(w: %5.1f)'):format(({r.ImGui_GetContentRegionAvail(ctx)})[1]))
          else
            r.ImGui_Text(ctx, ('Hello %d,%d'):format(column, row))
          end
        end
      end
      r.ImGui_EndTable(ctx)
    end

    demo.HelpMarker("Using TableSetupColumn() to setup explicit width.\n\nUnless _NoKeepColumnsVisible is set, fixed columns with set width may still be shrunk down if there's not enough space in the host.")

    demo.PushStyleCompact()
    rv,tables.col_widths.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoKeepColumnsVisible', tables.col_widths.flags2, r.ImGui_TableFlags_NoKeepColumnsVisible())
    rv,tables.col_widths.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.col_widths.flags2, r.ImGui_TableFlags_BordersInnerV())
    rv,tables.col_widths.flags2 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.col_widths.flags2, r.ImGui_TableFlags_BordersOuterV())
    demo.PopStyleCompact()
    if r.ImGui_BeginTable(ctx, 'table2', 4, tables.col_widths.flags2) then
      -- We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
      r.ImGui_TableSetupColumn(ctx, '', r.ImGui_TableColumnFlags_WidthFixed(), 100.0);
      r.ImGui_TableSetupColumn(ctx, '', r.ImGui_TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 15.0)
      r.ImGui_TableSetupColumn(ctx, '', r.ImGui_TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 30.0)
      r.ImGui_TableSetupColumn(ctx, '', r.ImGui_TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 15.0)
      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 3 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          if row == 0 then
            r.ImGui_Text(ctx, ('(w: %5.1f)'):format(({r.ImGui_GetContentRegionAvail(ctx)})[1]))
          else
            r.ImGui_Text(ctx, ('Hello %d,%d'):format(column, row))
          end
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Nested tables') then
    demo.HelpMarker('This demonstrate embedding a table into another table cell.')

    local flags = r.ImGui_TableFlags_Borders() | r.ImGui_TableFlags_Resizable() | r.ImGui_TableFlags_Reorderable() | r.ImGui_TableFlags_Hideable()
    if r.ImGui_BeginTable(ctx, 'table_nested1', 2, flags) then
      r.ImGui_TableSetupColumn(ctx, 'A0')
      r.ImGui_TableSetupColumn(ctx, 'A1')
      r.ImGui_TableHeadersRow(ctx)

      r.ImGui_TableNextColumn(ctx)
      r.ImGui_Text(ctx, 'A0 Row 0')

      local rows_height = TEXT_BASE_HEIGHT * 2
      if r.ImGui_BeginTable(ctx, 'table_nested2', 2, flags) then
        r.ImGui_TableSetupColumn(ctx, 'B0')
        r.ImGui_TableSetupColumn(ctx, 'B1')
        r.ImGui_TableHeadersRow(ctx)

        r.ImGui_TableNextRow(ctx, r.ImGui_TableRowFlags_None(), rows_height)
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, 'B0 Row 0')
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, 'B0 Row 1')
        r.ImGui_TableNextRow(ctx, r.ImGui_TableRowFlags_None(), rows_height)
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, 'B1 Row 0')
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, 'B1 Row 1')

        r.ImGui_EndTable(ctx)
      end

      r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'A0 Row 1')
      r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'A1 Row 0')
      r.ImGui_TableNextColumn(ctx); r.ImGui_Text(ctx, 'A1 Row 1')
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Row height') then
    demo.HelpMarker("You can pass a 'min_row_height' to TableNextRow().\n\nRows are padded with 'style.CellPadding.y' on top and bottom, so effectively the minimum row height will always be >= 'style.CellPadding.y * 2.0f'.\n\nWe cannot honor a _maximum_ row height as that would requires a unique clipping rectangle per row.");
    if r.ImGui_BeginTable(ctx, 'table_row_height', 1, r.ImGui_TableFlags_BordersOuter() | r.ImGui_TableFlags_BordersInnerV()) then
      for row = 0, 9 do
        local min_row_height = TEXT_BASE_HEIGHT * 0.30 * row
        r.ImGui_TableNextRow(ctx, r.ImGui_TableRowFlags_None(), min_row_height)
        r.ImGui_TableNextColumn(ctx)
        r.ImGui_Text(ctx, ('min_row_height = %.2f'):format(min_row_height))
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Outer size') then
    if not tables.outer_sz then
      tables.outer_sz = {
        flags = r.ImGui_TableFlags_Borders() |
                r.ImGui_TableFlags_Resizable() |
                r.ImGui_TableFlags_ContextMenuInBody() |
                r.ImGui_TableFlags_RowBg() |
                r.ImGui_TableFlags_SizingFixedFit() |
                r.ImGui_TableFlags_NoHostExtendX(),
      }
    end

    -- Showcasing use of ImGuiTableFlags_NoHostExtendX and ImGuiTableFlags_NoHostExtendY
    -- Important to that note how the two flags have slightly different behaviors!
    r.ImGui_Text(ctx, 'Using NoHostExtendX and NoHostExtendY:')
    demo.PushStyleCompact()
    rv,tables.outer_sz.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.outer_sz.flags, r.ImGui_TableFlags_NoHostExtendX())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.')
    rv,tables.outer_sz.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendY', tables.outer_sz.flags, r.ImGui_TableFlags_NoHostExtendY())
    r.ImGui_SameLine(ctx); demo.HelpMarker('Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.')
    demo.PopStyleCompact()

    local outer_size = { 0.0, TEXT_BASE_HEIGHT * 5.5 }
    if r.ImGui_BeginTable(ctx, 'table1', 3, tables.outer_sz.flags, table.unpack(outer_size)) then
      for row = 0, 9 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_SameLine(ctx)
    r.ImGui_Text(ctx, 'Hello!')

    r.ImGui_Spacing(ctx)

    local flags = r.ImGui_TableFlags_Borders() | r.ImGui_TableFlags_RowBg()
    r.ImGui_Text(ctx, 'Using explicit size:')
    if r.ImGui_BeginTable(ctx, 'table2', 3, flags, TEXT_BASE_WIDTH * 30, 0.0) then
      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_SameLine(ctx)
    if r.ImGui_BeginTable(ctx, 'table3', 3, flags, TEXT_BASE_WIDTH * 30, 0.0) then
      for row = 0, 2 do
        r.ImGui_TableNextRow(ctx, 0, TEXT_BASE_HEIGHT * 1.5)
        for column = 0, 2 do
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end

    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Background color') then
    if not tables.bg_col then
      tables.bg_col = {
        flags         = r.ImGui_TableFlags_RowBg(),
        row_bg_type   = 1,
        row_bg_target = 1,
        cell_bg_type  = 1,
      }
    end

    demo.PushStyleCompact()
    rv,tables.bg_col.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Borders', tables.bg_col.flags, r.ImGui_TableFlags_Borders())
    rv,tables.bg_col.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.bg_col.flags, r.ImGui_TableFlags_RowBg())
    r.ImGui_SameLine(ctx); demo.HelpMarker('ImGuiTableFlags_RowBg automatically sets RowBg0 to alternative colors pulled from the Style.')
    rv,tables.bg_col.row_bg_type = r.ImGui_Combo(ctx, 'row bg type', tables.bg_col.row_bg_type, "None\31Red\31Gradient\31")
    rv,tables.bg_col.row_bg_target = r.ImGui_Combo(ctx, 'row bg target', tables.bg_col.row_bg_target, "RowBg0\31RowBg1\31"); r.ImGui_SameLine(ctx); demo.HelpMarker('Target RowBg0 to override the alternating odd/even colors,\nTarget RowBg1 to blend with them.')
    rv,tables.bg_col.cell_bg_type = r.ImGui_Combo(ctx, 'cell bg type', tables.bg_col.cell_bg_type, 'None\31Blue\31'); r.ImGui_SameLine(ctx); demo.HelpMarker('We are colorizing cells to B1->C2 here.')
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table1', 5, tables.bg_col.flags) then
      for row = 0, 5 do
        r.ImGui_TableNextRow(ctx)

        -- Demonstrate setting a row background color with 'r.ImGui_TableSetBgColor(ImGuiTableBgTarget_RowBgX, ...)'
        -- We use a transparent color so we can see the one behind in case our target is RowBg1 and RowBg0 was already targeted by the ImGuiTableFlags_RowBg flag.
        if tables.bg_col.row_bg_type ~= 0 then
          local row_bg_color
          if tables.bg_col.row_bg_type == 1 then -- flat
            row_bg_color = 0xb34d4da6
          else -- gradient
            row_bg_color = 0x333333a6
            row_bg_color = row_bg_color + (demo.round((row * 0.1) * 0xFF) << 24)
          end
          r.ImGui_TableSetBgColor(ctx, r.ImGui_TableBgTarget_RowBg0() + tables.bg_col.row_bg_target, row_bg_color)
        end

        -- Fill cells
        for column = 0, 4 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('%c%c'):format(string.byte('A') + row, string.byte('0') + column))

          -- Change background of Cells B1->C2
          -- Demonstrate setting a cell background color with 'r.ImGui_TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
          -- (the CellBg color will be blended over the RowBg and ColumnBg colors)
          -- We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
          if row >= 1 and row <= 2 and column >= 1 and column <= 2 and tables.bg_col.cell_bg_type == 1 then
            r.ImGui_TableSetBgColor(ctx, r.ImGui_TableBgTarget_CellBg(), 0x4d4db3a6)
          end
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Tree view') then
    local flags = r.ImGui_TableFlags_BordersV()      |
                  r.ImGui_TableFlags_BordersOuterH() |
                  r.ImGui_TableFlags_Resizable()     |
                  r.ImGui_TableFlags_RowBg()--         |
                  -- r.ImGui_TableFlags_NoBordersInBody()

    if r.ImGui_BeginTable(ctx, '3ways', 3, flags) then
      -- The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
      r.ImGui_TableSetupColumn(ctx, 'Name', r.ImGui_TableColumnFlags_NoHide())
      r.ImGui_TableSetupColumn(ctx, 'Size', r.ImGui_TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 12.0)
      r.ImGui_TableSetupColumn(ctx, 'Type', r.ImGui_TableColumnFlags_WidthFixed(), TEXT_BASE_WIDTH * 18.0)
      r.ImGui_TableHeadersRow(ctx)

      -- Simple storage to output a dummy file-system.
      local nodes = {
        { name="Root",                          type="Folder",      size=-1,     child_idx= 1,  child_count= 3 }, -- 0
        { name="Music",                         type="Folder",      size=-1,     child_idx= 4,  child_count= 2 }, -- 1
        { name="Textures",                      type="Folder",      size=-1,     child_idx= 6,  child_count= 3 }, -- 2
        { name="desktop.ini",                   type="System file", size= 1024,   child_idx=-1, child_count=-1 }, -- 3
        { name="File1_a.wav",                   type="Audio file",  size= 123000, child_idx=-1, child_count=-1 }, -- 4
        { name="File1_b.wav",                   type="Audio file",  size= 456000, child_idx=-1, child_count=-1 }, -- 5
        { name="Image001.png",                  type="Image file",  size= 203128, child_idx=-1, child_count=-1 }, -- 6
        { name="Copy of Image001.png",          type="Image file",  size= 203256, child_idx=-1, child_count=-1 }, -- 7
        { name="Copy of Image001 (Final2).png", type="Image file",  size= 203512, child_idx=-1, child_count=-1 }, -- 8
      }

      local function DisplayNode(node)
        r.ImGui_TableNextRow(ctx)
        r.ImGui_TableNextColumn(ctx)
        local is_folder = node.child_count > 0
        if is_folder then
          local open = r.ImGui_TreeNode(ctx, node.name, r.ImGui_TreeNodeFlags_SpanFullWidth())
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_TextDisabled(ctx, '--')
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, node.type)
          if open then
            for child_n = 1, node.child_count do
              DisplayNode(nodes[node.child_idx + child_n])
            end
            r.ImGui_TreePop(ctx)
          end
        else
          r.ImGui_TreeNode(ctx, node.name, r.ImGui_TreeNodeFlags_Leaf() | r.ImGui_TreeNodeFlags_Bullet() | r.ImGui_TreeNodeFlags_NoTreePushOnOpen() | r.ImGui_TreeNodeFlags_SpanFullWidth())
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, ('%d'):format(node.size))
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, node.type)
        end
      end

      DisplayNode(nodes[1])

      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Item width') then
    if not tables.item_width then
      tables.item_width = {
        dummy_d = 0.0,
      }
    end

    demo.HelpMarker(
      "Showcase using PushItemWidth() and how it is preserved on a per-column basis.\n\n\z
       Note that on auto-resizing non-resizable fixed columns, querying the content width for e.g. right-alignment doesn't make sense.")
    if r.ImGui_BeginTable(ctx, 'table_item_width', 3, r.ImGui_TableFlags_Borders()) then
      r.ImGui_TableSetupColumn(ctx, 'small')
      r.ImGui_TableSetupColumn(ctx, 'half')
      r.ImGui_TableSetupColumn(ctx, 'right-align')
      r.ImGui_TableHeadersRow(ctx)

      for row = 0, 2 do
        r.ImGui_TableNextRow(ctx)
        if row == 0 then
          -- Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
          r.ImGui_TableSetColumnIndex(ctx, 0)
          r.ImGui_PushItemWidth(ctx, TEXT_BASE_WIDTH * 3.0) -- Small
          r.ImGui_TableSetColumnIndex(ctx, 1)
          r.ImGui_PushItemWidth(ctx, 0 - ({r.ImGui_GetContentRegionAvail(ctx)})[1] * 0.5)
          r.ImGui_TableSetColumnIndex(ctx, 2)
          r.ImGui_PushItemWidth(ctx, -FLT_MIN) -- Right-aligned
        end

        -- Draw our contents
        r.ImGui_PushID(ctx, row)
        r.ImGui_TableSetColumnIndex(ctx, 0)
        rv,tables.item_width.dummy_d = r.ImGui_SliderDouble(ctx, 'double0', tables.item_width.dummy_d, 0.0, 1.0)
        r.ImGui_TableSetColumnIndex(ctx, 1)
        rv,tables.item_width.dummy_d = r.ImGui_SliderDouble(ctx, 'double1', tables.item_width.dummy_d, 0.0, 1.0)
        r.ImGui_TableSetColumnIndex(ctx, 2)
        rv,tables.item_width.dummy_d = r.ImGui_SliderDouble(ctx, 'double2', tables.item_width.dummy_d, 0.0, 1.0)
        r.ImGui_PopID(ctx)
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  -- Demonstrate using TableHeader() calls instead of TableHeadersRow()
  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Custom headers') then
    if not tables.headers then
      tables.headers = {
        column_selected = { false, false, false },
      }
    end

    local COLUMNS_COUNT = 3
    if r.ImGui_BeginTable(ctx, 'table_custom_headers', COLUMNS_COUNT, r.ImGui_TableFlags_Borders() | r.ImGui_TableFlags_Reorderable() | r.ImGui_TableFlags_Hideable()) then
      r.ImGui_TableSetupColumn(ctx, 'Apricot')
      r.ImGui_TableSetupColumn(ctx, 'Banana')
      r.ImGui_TableSetupColumn(ctx, 'Cherry')

      -- Instead of calling TableHeadersRow() we'll submit custom headers ourselves
      r.ImGui_TableNextRow(ctx, r.ImGui_TableRowFlags_Headers())
      for column = 0, COLUMNS_COUNT - 1 do
        r.ImGui_TableSetColumnIndex(ctx, column)
        local column_name = r.ImGui_TableGetColumnName(ctx, column) -- Retrieve name passed to TableSetupColumn()
        r.ImGui_PushID(ctx, column)
        r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_FramePadding(), 0, 0)
        rv,tables.headers.column_selected[column + 1] =
          r.ImGui_Checkbox(ctx, '##checkall', tables.headers.column_selected[column + 1])
        r.ImGui_PopStyleVar(ctx)
        r.ImGui_SameLine(ctx, 0.0, ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemInnerSpacing())})[1])
        r.ImGui_TableHeader(ctx, column_name)
        r.ImGui_PopID(ctx)
      end

      for row = 0, 4 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, 2 do
          local buf = ('Cell %d,%d'):format(column, row)
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Selectable(ctx, buf, tables.headers.column_selected[column + 1])
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  -- Demonstrate creating custom context menus inside columns, while playing it nice with context menus provided by TableHeadersRow()/TableHeader()
  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Context menus') then
    if not tables.ctx_menus then
      tables.ctx_menus = {
        flags1 = r.ImGui_TableFlags_Resizable()   |
                 r.ImGui_TableFlags_Reorderable() |
                 r.ImGui_TableFlags_Hideable()    |
                 r.ImGui_TableFlags_Borders()     |
                 r.ImGui_TableFlags_ContextMenuInBody()
      }
    end
    demo.HelpMarker('By default, right-clicking over a TableHeadersRow()/TableHeader() line will open the default context-menu.\nUsing ImGuiTableFlags_ContextMenuInBody we also allow right-clicking over columns body.')

    demo.PushStyleCompact()
    rv,tables.ctx_menus.flags1 = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ContextMenuInBody', tables.ctx_menus.flags1, r.ImGui_TableFlags_ContextMenuInBody())
    demo.PopStyleCompact()

    -- Context Menus: first example
    -- [1.1] Right-click on the TableHeadersRow() line to open the default table context menu.
    -- [1.2] Right-click in columns also open the default table context menu (if ImGuiTableFlags_ContextMenuInBody is set)
    local COLUMNS_COUNT = 3
    if r.ImGui_BeginTable(ctx, 'table_context_menu', COLUMNS_COUNT, tables.ctx_menus.flags1) then
      r.ImGui_TableSetupColumn(ctx, 'One')
      r.ImGui_TableSetupColumn(ctx, 'Two')
      r.ImGui_TableSetupColumn(ctx, 'Three')

      -- [1.1]] Right-click on the TableHeadersRow() line to open the default table context menu.
      r.ImGui_TableHeadersRow(ctx)

      -- Submit dummy contents
      for row = 0, 3 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, COLUMNS_COUNT - 1 do
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('Cell %d,%d'):format(column, row))
        end
      end
      r.ImGui_EndTable(ctx)
    end

    -- Context Menus: second example
    -- [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
    -- [2.2] Right-click on the ".." to open a custom popup
    -- [2.3] Right-click in columns to open another custom popup
    demo.HelpMarker('Demonstrate mixing table context menu (over header), item context button (over button) and custom per-colum context menu (over column body).')
    local flags2 = r.ImGui_TableFlags_Resizable()      |
                   r.ImGui_TableFlags_SizingFixedFit() |
                   r.ImGui_TableFlags_Reorderable()    |
                   r.ImGui_TableFlags_Hideable()       |
                   r.ImGui_TableFlags_Borders()
    if r.ImGui_BeginTable(ctx, 'table_context_menu_2', COLUMNS_COUNT, flags2) then
      r.ImGui_TableSetupColumn(ctx, 'One')
      r.ImGui_TableSetupColumn(ctx, 'Two')
      r.ImGui_TableSetupColumn(ctx, 'Three')

      -- [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
      r.ImGui_TableHeadersRow(ctx)
      for row = 0, 3 do
        r.ImGui_TableNextRow(ctx)
        for column = 0, COLUMNS_COUNT - 1 do
          -- Submit dummy contents
          r.ImGui_TableSetColumnIndex(ctx, column)
          r.ImGui_Text(ctx, ('Cell %d,%d'):format(column, row))
          r.ImGui_SameLine(ctx)

          -- [2.2] Right-click on the ".." to open a custom popup
          r.ImGui_PushID(ctx, row * COLUMNS_COUNT + column)
          r.ImGui_SmallButton(ctx, "..")
          if r.ImGui_BeginPopupContextItem(ctx) then
            r.ImGui_Text(ctx, ('This is the popup for Button("..") in Cell %d,%d'):format(column, row))
            if r.ImGui_Button(ctx, 'Close') then
              r.ImGui_CloseCurrentPopup(ctx)
            end
            r.ImGui_EndPopup(ctx)
          end
          r.ImGui_PopID(ctx)
        end
      end

      -- [2.3] Right-click anywhere in columns to open another custom popup
      -- (instead of testing for !IsAnyItemHovered() we could also call OpenPopup() with ImGuiPopupFlags_NoOpenOverExistingPopup
      -- to manage popup priority as the popups triggers, here "are we hovering a column" are overlapping)
      local hovered_column = -1
      for column = 0, COLUMNS_COUNT do
        r.ImGui_PushID(ctx, column)
        if (r.ImGui_TableGetColumnFlags(ctx, column) & r.ImGui_TableColumnFlags_IsHovered()) ~= 0 then
          hovered_column = column
        end
        if hovered_column == column and not r.ImGui_IsAnyItemHovered(ctx) and r.ImGui_IsMouseReleased(ctx, 1) then
          r.ImGui_OpenPopup(ctx, 'MyPopup')
        end
        if r.ImGui_BeginPopup(ctx, 'MyPopup') then
          if column == COLUMNS_COUNT then
            r.ImGui_Text(ctx, 'This is a custom popup for unused space after the last column.')
          else
            r.ImGui_Text(ctx, ('This is a custom popup for Column %d'):format(column))
          end
          if r.ImGui_Button(ctx, 'Close') then
            r.ImGui_CloseCurrentPopup(ctx)
          end
          r.ImGui_EndPopup(ctx)
        end
        r.ImGui_PopID(ctx)
      end

      r.ImGui_EndTable(ctx)
      r.ImGui_Text(ctx, ('Hovered column: %d'):format(hovered_column))
    end
    r.ImGui_TreePop(ctx)
  end

  -- Demonstrate creating multiple tables with the same ID
  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Synced instances') then
    demo.HelpMarker('Multiple tables with the same identifier will share their settings, width, visibility, order etc.')
    local flags = r.ImGui_TableFlags_Resizable() |
                  r.ImGui_TableFlags_Reorderable() |
                  r.ImGui_TableFlags_Hideable() |
                  r.ImGui_TableFlags_Borders() |
                  r.ImGui_TableFlags_SizingFixedFit()-- |
                  --r.ImGui_TableFlags_NoSavedSettings()
    for n = 0, 2 do
      local buf = ('Synced Table %d'):format(n)
      local open = r.ImGui_CollapsingHeader(ctx, buf, nil, r.ImGui_TreeNodeFlags_DefaultOpen())
      if open and r.ImGui_BeginTable(ctx, 'Table', 3, flags) then
        r.ImGui_TableSetupColumn(ctx, 'One')
        r.ImGui_TableSetupColumn(ctx, 'Two')
        r.ImGui_TableSetupColumn(ctx, 'Three')
        r.ImGui_TableHeadersRow(ctx)
        for cell = 0, 9 do
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, ('this cell %d'):format(cell))
        end
        r.ImGui_EndTable(ctx)
      end
    end
    r.ImGui_TreePop(ctx)
  end

  -- Demonstrate using Sorting facilities
  -- This is a simplified version of the "Advanced" example, where we mostly focus on the code necessary to handle sorting.
  -- Note that the "Advanced" example also showcase manually triggering a sort (e.g. if item quantities have been modified)
  local template_items_names = {
    'Banana', 'Apple', 'Cherry', 'Watermelon', 'Grapefruit', 'Strawberry', 'Mango',
    'Kiwi', 'Orange', 'Pineapple', 'Blueberry', 'Plum', 'Coconut', 'Pear', 'Apricot'
  }
  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Sorting') then
    if not tables.sorting then
      tables.sorting = {
        flags = r.ImGui_TableFlags_Resizable()       |
                r.ImGui_TableFlags_Reorderable()     |
                r.ImGui_TableFlags_Hideable()        |
                r.ImGui_TableFlags_Sortable()        |
                r.ImGui_TableFlags_SortMulti()       |
                r.ImGui_TableFlags_RowBg()           |
                r.ImGui_TableFlags_BordersOuter()    |
                r.ImGui_TableFlags_BordersV()        |
                -- r.ImGui_TableFlags_NoBordersInBody() |
                r.ImGui_TableFlags_ScrollY(),
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
    rv,tables.sorting.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_SortMulti', tables.sorting.flags, r.ImGui_TableFlags_SortMulti())
    r.ImGui_SameLine(ctx); demo.HelpMarker('When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).')
    rv,tables.sorting.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_SortTristate', tables.sorting.flags, r.ImGui_TableFlags_SortTristate())
    r.ImGui_SameLine(ctx); demo.HelpMarker('When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).')
    demo.PopStyleCompact()

    if r.ImGui_BeginTable(ctx, 'table_sorting', 4, tables.sorting.flags, 0.0, TEXT_BASE_HEIGHT * 15, 0.0) then
      -- Declare columns
      -- We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
      -- This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
      -- Demonstrate using a mixture of flags among available sort-related flags:
      -- - ImGuiTableColumnFlags_DefaultSort
      -- - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending / ImGuiTableColumnFlags_NoSortDescending
      -- - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
      r.ImGui_TableSetupColumn(ctx, 'ID',       r.ImGui_TableColumnFlags_DefaultSort()          | r.ImGui_TableColumnFlags_WidthFixed(),   0.0, MyItemColumnID_ID)
      r.ImGui_TableSetupColumn(ctx, 'Name',                                                       r.ImGui_TableColumnFlags_WidthFixed(),   0.0, MyItemColumnID_Name)
      r.ImGui_TableSetupColumn(ctx, 'Action',   r.ImGui_TableColumnFlags_NoSort()               | r.ImGui_TableColumnFlags_WidthFixed(),   0.0, MyItemColumnID_Action)
      r.ImGui_TableSetupColumn(ctx, 'Quantity', r.ImGui_TableColumnFlags_PreferSortDescending() | r.ImGui_TableColumnFlags_WidthStretch(), 0.0, MyItemColumnID_Quantity)
      r.ImGui_TableSetupScrollFreeze(ctx, 0, 1) -- Make row always visible
      r.ImGui_TableHeadersRow(ctx)

      -- Sort our data if sort specs have been changed!
      if r.ImGui_TableNeedSort(ctx) then
        table.sort(tables.sorting.items, demo.CompareTableItems)
      end

      -- Demonstrate using clipper for large vertical lists
      local clipper = r.ImGui_CreateListClipper(ctx)
      r.ImGui_ListClipper_Begin(clipper, #tables.sorting.items)
      while r.ImGui_ListClipper_Step(clipper) do
        local display_start, display_end = r.ImGui_ListClipper_GetDisplayRange(clipper)
        for row_n = display_start, display_end - 1 do
          -- Display a data item
          local item = tables.sorting.items[row_n + 1]
          r.ImGui_PushID(ctx, item.id)
          r.ImGui_TableNextRow(ctx)
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, ('%04d'):format(item.id))
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, item.name)
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_SmallButton(ctx, 'None')
          r.ImGui_TableNextColumn(ctx)
          r.ImGui_Text(ctx, ('%d'):format(item.quantity))
          r.ImGui_PopID(ctx)
        end
      end
      r.ImGui_EndTable(ctx)
    end
    r.ImGui_TreePop(ctx)
  end

  -- In this example we'll expose most table flags and settings.
  -- For specific flags and settings refer to the corresponding section for more detailed explanation.
  -- This section is mostly useful to experiment with combining certain flags or settings with each others.
  -- r.ImGui_SetNextItemOpen(ctx, true, r.ImGui_Cond_Once()) -- [DEBUG]
  DoOpenAction()
  if r.ImGui_TreeNode(ctx, 'Advanced') then
    if not tables.advanced then
      tables.advanced = {
        items = {},
        flags = r.ImGui_TableFlags_Resizable()       |
                r.ImGui_TableFlags_Reorderable()     |
                r.ImGui_TableFlags_Hideable()        |
                r.ImGui_TableFlags_Sortable()        |
                r.ImGui_TableFlags_SortMulti()       |
                r.ImGui_TableFlags_RowBg()           |
                r.ImGui_TableFlags_Borders()         |
                -- r.ImGui_TableFlags_NoBordersInBody() |
                r.ImGui_TableFlags_ScrollX()         |
                r.ImGui_TableFlags_ScrollY()         |
                r.ImGui_TableFlags_SizingFixedFit(),
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
    -- r.ImGui_SetNextItemOpen(ctx, true, r.ImGui_Cond_Once()) -- FIXME-TABLE: Enabling this results in initial clipped first pass on table which tend to affects column sizing
    if r.ImGui_TreeNode(ctx, 'Options') then
      -- Make the UI compact because there are so many fields
      demo.PushStyleCompact()
      r.ImGui_PushItemWidth(ctx, TEXT_BASE_WIDTH * 28.0)

      if r.ImGui_TreeNode(ctx, 'Features:', r.ImGui_TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Resizable', tables.advanced.flags, r.ImGui_TableFlags_Resizable())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Reorderable', tables.advanced.flags, r.ImGui_TableFlags_Reorderable())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Hideable', tables.advanced.flags, r.ImGui_TableFlags_Hideable())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_Sortable', tables.advanced.flags, r.ImGui_TableFlags_Sortable())
        -- rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoSavedSettings', tables.advanced.flags, r.ImGui_TableFlags_NoSavedSettings())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ContextMenuInBody', tables.advanced.flags, r.ImGui_TableFlags_ContextMenuInBody())
        r.ImGui_TreePop(ctx)
      end

      if r.ImGui_TreeNode(ctx, 'Decorations:', r.ImGui_TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_RowBg', tables.advanced.flags, r.ImGui_TableFlags_RowBg())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersV', tables.advanced.flags, r.ImGui_TableFlags_BordersV())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterV', tables.advanced.flags, r.ImGui_TableFlags_BordersOuterV())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerV', tables.advanced.flags, r.ImGui_TableFlags_BordersInnerV())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersH', tables.advanced.flags, r.ImGui_TableFlags_BordersH())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersOuterH', tables.advanced.flags, r.ImGui_TableFlags_BordersOuterH())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_BordersInnerH', tables.advanced.flags, r.ImGui_TableFlags_BordersInnerH())
        -- rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBody', tables.advanced.flags, r.ImGui_TableFlags_NoBordersInBody()) r.ImGui_SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body (borders will always appears in Headers')
        -- rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoBordersInBodyUntilResize', tables.advanced.flags, r.ImGui_TableFlags_NoBordersInBodyUntilResize()) r.ImGui_SameLine(ctx); demo.HelpMarker('Disable vertical borders in columns Body until hovered for resize (borders will always appears in Headers)')
        r.ImGui_TreePop(ctx)
      end

      if r.ImGui_TreeNode(ctx, 'Sizing:', r.ImGui_TreeNodeFlags_DefaultOpen()) then
        tables.advanced.flags = demo.EditTableSizingFlags(tables.advanced.flags)
        r.ImGui_SameLine(ctx); demo.HelpMarker('In the Advanced demo we override the policy of each column so those table-wide settings have less effect that typical.');
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendX', tables.advanced.flags, r.ImGui_TableFlags_NoHostExtendX())
        r.ImGui_SameLine(ctx); demo.HelpMarker('Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.');
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoHostExtendY', tables.advanced.flags, r.ImGui_TableFlags_NoHostExtendY())
        r.ImGui_SameLine(ctx); demo.HelpMarker('Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.');
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoKeepColumnsVisible', tables.advanced.flags, r.ImGui_TableFlags_NoKeepColumnsVisible())
        r.ImGui_SameLine(ctx); demo.HelpMarker('Only available if ScrollX is disabled.');
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_PreciseWidths', tables.advanced.flags, r.ImGui_TableFlags_PreciseWidths())
        r.ImGui_SameLine(ctx); demo.HelpMarker('Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.')
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoClip', tables.advanced.flags, r.ImGui_TableFlags_NoClip())
        r.ImGui_SameLine(ctx); demo.HelpMarker('Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with ScrollFreeze options.')
        r.ImGui_TreePop(ctx)
      end

      if r.ImGui_TreeNode(ctx, 'Padding:', r.ImGui_TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_PadOuterX',   tables.advanced.flags, r.ImGui_TableFlags_PadOuterX())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadOuterX', tables.advanced.flags, r.ImGui_TableFlags_NoPadOuterX())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_NoPadInnerX', tables.advanced.flags, r.ImGui_TableFlags_NoPadInnerX())
        r.ImGui_TreePop(ctx)
      end

      if r.ImGui_TreeNode(ctx, 'Scrolling:', r.ImGui_TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollX', tables.advanced.flags, r.ImGui_TableFlags_ScrollX())
        r.ImGui_SameLine(ctx)
        r.ImGui_SetNextItemWidth(ctx, r.ImGui_GetFrameHeight(ctx))
        rv,tables.advanced.freeze_cols = r.ImGui_DragInt(ctx, 'freeze_cols', tables.advanced.freeze_cols, 0.2, 0, 9, nil, r.ImGui_SliderFlags_NoInput())
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_ScrollY', tables.advanced.flags, r.ImGui_TableFlags_ScrollY())
        r.ImGui_SameLine(ctx)
        r.ImGui_SetNextItemWidth(ctx, r.ImGui_GetFrameHeight(ctx))
        rv,tables.advanced.freeze_rows = r.ImGui_DragInt(ctx, 'freeze_rows', tables.advanced.freeze_rows, 0.2, 0, 9, nil, r.ImGui_SliderFlags_NoInput())
        r.ImGui_TreePop(ctx)
      end

      if r.ImGui_TreeNode(ctx, 'Sorting:', r.ImGui_TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_SortMulti', tables.advanced.flags, r.ImGui_TableFlags_SortMulti())
        r.ImGui_SameLine(ctx); demo.HelpMarker('When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).')
        rv,tables.advanced.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiTableFlags_SortTristate', tables.advanced.flags, r.ImGui_TableFlags_SortTristate())
        r.ImGui_SameLine(ctx); demo.HelpMarker('When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).')
        r.ImGui_TreePop(ctx)
      end

      if r.ImGui_TreeNode(ctx, 'Other:', r.ImGui_TreeNodeFlags_DefaultOpen()) then
        rv,tables.advanced.show_headers = r.ImGui_Checkbox(ctx, 'show_headers', tables.advanced.show_headers)
        rv,tables.advanced.show_wrapped_text = r.ImGui_Checkbox(ctx, 'show_wrapped_text', tables.advanced.show_wrapped_text)

        rv,tables.advanced.outer_size_value[1],tables.advanced.outer_size_value[2] =
          r.ImGui_DragDouble2(ctx, '##OuterSize', table.unpack(tables.advanced.outer_size_value))
        r.ImGui_SameLine(ctx, 0.0, ({r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemInnerSpacing())})[1]);
        rv,tables.advanced.outer_size_enabled = r.ImGui_Checkbox(ctx, 'outer_size', tables.advanced.outer_size_enabled)
        r.ImGui_SameLine(ctx)
        demo.HelpMarker(
          'If scrolling is disabled (ScrollX and ScrollY not set):\n\z
           - The table is output directly in the parent window.\n\z
           - OuterSize.x < 0.0f will right-align the table.\n\z
           - OuterSize.x = 0.0f will narrow fit the table unless there are any Stretch column.\n\z
           - OuterSize.y then becomes the minimum size for the table, which will extend vertically if there are more rows (unless NoHostExtendY is set).')

        -- From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
        -- To facilitate toying with this demo we will actually pass 0.0f to the BeginTable() when ScrollX is disabled.
        rv,tables.advanced.inner_width_with_scroll = r.ImGui_DragDouble(ctx, 'inner_width (when ScrollX active)', tables.advanced.inner_width_with_scroll, 1.0, 0.0, FLT_MAX)

        rv,tables.advanced.row_min_height = r.ImGui_DragDouble(ctx, 'row_min_height', tables.advanced.row_min_height, 1.0, 0.0, FLT_MAX)
        r.ImGui_SameLine(ctx); demo.HelpMarker('Specify height of the Selectable item.')

        rv,tables.advanced.items_count = r.ImGui_DragInt(ctx, 'items_count', tables.advanced.items_count, 0.1, 0, 9999)
        rv,tables.advanced.contents_type = r.ImGui_Combo(ctx, 'items_type (first column)', tables.advanced.contents_type,
          'Text\31Button\31SmallButton\31FillButton\31Selectable\31Selectable (span row)\31')
        -- //filter.Draw('filter');
        r.ImGui_TreePop(ctx)
      end

      r.ImGui_PopItemWidth(ctx)
      demo.PopStyleCompact()
      r.ImGui_Spacing(ctx)
      r.ImGui_TreePop(ctx)
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

    -- const ImDrawList* parent_draw_list = r.ImGui_GetWindowDrawList();
    -- const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
    -- local table_scroll_cur, table_scroll_max, table_draw_list -- For debug display

    -- Submit table
    local inner_width_to_use = (tables.advanced.flags & r.ImGui_TableFlags_ScrollX()) ~= 0 and tables.advanced.inner_width_with_scroll or 0.0
    local w, h = 0, 0
    if tables.advanced.outer_size_enabled then
      w, h = table.unpack(tables.advanced.outer_size_value)
    end
    if r.ImGui_BeginTable(ctx, 'table_advanced', 6, tables.advanced.flags, w, h, inner_width_to_use) then
      -- Declare columns
      -- We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
      -- This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
      r.ImGui_TableSetupColumn(ctx, 'ID',           r.ImGui_TableColumnFlags_DefaultSort() | r.ImGui_TableColumnFlags_WidthFixed() | r.ImGui_TableColumnFlags_NoHide(), 0.0, MyItemColumnID_ID)
      r.ImGui_TableSetupColumn(ctx, 'Name',         r.ImGui_TableColumnFlags_WidthFixed(), 0.0, MyItemColumnID_Name)
      r.ImGui_TableSetupColumn(ctx, 'Action',       r.ImGui_TableColumnFlags_NoSort() | r.ImGui_TableColumnFlags_WidthFixed(), 0.0, MyItemColumnID_Action)
      r.ImGui_TableSetupColumn(ctx, 'Quantity',     r.ImGui_TableColumnFlags_PreferSortDescending(), 0.0, MyItemColumnID_Quantity)
      r.ImGui_TableSetupColumn(ctx, 'Description',  (tables.advanced.flags & r.ImGui_TableFlags_NoHostExtendX()) ~= 0 and 0 or r.ImGui_TableColumnFlags_WidthStretch(), 0.0, MyItemColumnID_Description)
      r.ImGui_TableSetupColumn(ctx, 'Hidden',       r.ImGui_TableColumnFlags_DefaultHide() | r.ImGui_TableColumnFlags_NoSort())
      r.ImGui_TableSetupScrollFreeze(ctx, tables.advanced.freeze_cols, tables.advanced.freeze_rows)

      -- Sort our data if sort specs have been changed!
      local specs_dirty, has_specs = r.ImGui_TableNeedSort(ctx)
      if has_specs and (specs_dirty or tables.advanced.items_need_sort) then
        table.sort(tables.advanced.items, demo.CompareTableItems)
        tables.advanced.items_need_sort = false
      end

      -- Take note of whether we are currently sorting based on the Quantity field,
      -- we will use this to trigger sorting when we know the data of this column has been modified.
      local sorts_specs_using_quantity = (r.ImGui_TableGetColumnFlags(ctx, 3) & r.ImGui_TableColumnFlags_IsSorted()) ~= 0

      -- Show headers
      if tables.advanced.show_headers then
        r.ImGui_TableHeadersRow(ctx)
      end

      -- Show data
      r.ImGui_PushButtonRepeat(ctx, true)

      -- Demonstrate using clipper for large vertical lists
      local clipper = r.ImGui_CreateListClipper(ctx)
      r.ImGui_ListClipper_Begin(clipper, #tables.advanced.items)
      while r.ImGui_ListClipper_Step(clipper) do
        local display_start, display_end = r.ImGui_ListClipper_GetDisplayRange(clipper)
        for row_n = display_start, display_end - 1 do
          local item = tables.advanced.items[row_n + 1]
          -- //if (!filter.PassFilter(item->Name))
          -- //    continue;

          r.ImGui_PushID(ctx, item.id);
          r.ImGui_TableNextRow(ctx, r.ImGui_TableRowFlags_None(), tables.advanced.row_min_height)

          -- For the demo purpose we can select among different type of items submitted in the first column
          r.ImGui_TableSetColumnIndex(ctx, 0)
          local label = ('%04d'):format(item.id)
          local contents_type = tables.advanced.contents_type
          if contents_type == 0 then -- text
              r.ImGui_Text(ctx, label)
          elseif contents_type == 1 then -- button
              r.ImGui_Button(ctx, label)
          elseif contents_type == 2 then -- small button
              r.ImGui_SmallButton(ctx, label)
          elseif contents_type == 3 then -- fill button
              r.ImGui_Button(ctx, label, -FLT_MIN, 0.0)
          elseif contents_type == 4 or contents_type == 5 then -- selectable/selectable (span row)
            local selectable_flags = contents_type == 5 and r.ImGui_SelectableFlags_SpanAllColumns() | r.ImGui_SelectableFlags_AllowItemOverlap() or r.ImGui_SelectableFlags_None()
            if r.ImGui_Selectable(ctx, label, item.is_selected, selectable_flags, 0, tables.advanced.row_min_height) then
              if (r.ImGui_GetKeyMods(ctx) & r.ImGui_KeyModFlags_Ctrl()) ~= 0 then
                item.is_selected = not item.is_selected
              else
                for _,it in ipairs(tables.advanced.items) do
                  it.is_selected = it == item
                end
              end
            end
          end

          if r.ImGui_TableSetColumnIndex(ctx, 1) then
            r.ImGui_Text(ctx, item.name)
          end

          -- Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
          -- and we are currently sorting on the column showing the Quantity.
          -- To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
          -- You will probably need a more advanced system in your code if you want to automatically sort when a specific entry changes.
          if r.ImGui_TableSetColumnIndex(ctx, 2) then
            if r.ImGui_SmallButton(ctx, 'Chop') then item.quantity = item.quantity + 1 end
            if sorts_specs_using_quantity and r.ImGui_IsItemDeactivated(ctx) then tables.advanced.items_need_sort = true end
            r.ImGui_SameLine(ctx)
            if r.ImGui_SmallButton(ctx, 'Eat')  then item.quantity = item.quantity - 1 end
            if sorts_specs_using_quantity and r.ImGui_IsItemDeactivated(ctx) then tables.advanced.items_need_sort = true end
          end

          if r.ImGui_TableSetColumnIndex(ctx, 3) then
            r.ImGui_Text(ctx, ('%d'):format(item.quantity))
          end

          r.ImGui_TableSetColumnIndex(ctx, 4)
          if tables.advanced.show_wrapped_text then
            r.ImGui_TextWrapped(ctx, 'Lorem ipsum dolor sit amet')
          else
            r.ImGui_Text(ctx, 'Lorem ipsum dolor sit amet')
          end

          if r.ImGui_TableSetColumnIndex(ctx, 5) then
            r.ImGui_Text(ctx, '1234')
          end

          r.ImGui_PopID(ctx)
        end
      end
      r.ImGui_PopButtonRepeat(ctx)

      -- Store some info to display debug details below
      -- table_scroll_cur = { r.ImGui_GetScrollX(ctx), r.ImGui_GetScrollY(ctx) }
      -- table_scroll_max = { r.ImGui_GetScrollMaxX(ctx), r.ImGui_GetScrollMaxY(ctx) }
      -- table_draw_list  = r.ImGui_GetWindowDrawList(ctx)
      r.ImGui_EndTable(ctx)
    end
    -- static bool show_debug_details = false;
    -- r.ImGui_Checkbox("Debug details", &show_debug_details);
    -- if (show_debug_details && table_draw_list)
    -- {
    --     r.ImGui_SameLine(0.0f, 0.0f);
    --     const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
    --     if (table_draw_list == parent_draw_list)
    --         r.ImGui_Text(": DrawCmd: +%d (in same window)",
    --             table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
    --     else
    --         r.ImGui_Text(": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
    --             table_draw_list_draw_cmd_count - 1, table_scroll_cur.x, table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
    -- }
    r.ImGui_TreePop(ctx)
  end

  r.ImGui_PopID(ctx)

  -- demo.ShowDemoWindowColumns()

  if tables.disable_indent then
    r.ImGui_PopStyleVar(ctx)
  end
end

-- // Demonstrate old/legacy Columns API!
-- // [2020: Columns are under-featured and not maintained. Prefer using the more flexible and powerful BeginTable() API!]
-- static void ShowDemoWindowColumns()
-- {
--     bool open = r.ImGui_TreeNode("Legacy Columns API");
--     r.ImGui_SameLine();
--     HelpMarker("Columns() is an old API! Prefer using the more flexible and powerful BeginTable() API!");
--     if (!open)
--         return;
--
--     // Basic columns
--     if (r.ImGui_TreeNode("Basic"))
--     {
--         r.ImGui_Text("Without border:");
--         r.ImGui_Columns(3, "mycolumns3", false);  // 3-ways, no border
--         r.ImGui_Separator();
--         for (int n = 0; n < 14; n++)
--         {
--             char label[32];
--             sprintf(label, "Item %d", n);
--             if (r.ImGui_Selectable(label)) {}
--             //if (r.ImGui_Button(label, ImVec2(-FLT_MIN,0.0f))) {}
--             r.ImGui_NextColumn();
--         }
--         r.ImGui_Columns(1);
--         r.ImGui_Separator();
--
--         r.ImGui_Text("With border:");
--         r.ImGui_Columns(4, "mycolumns"); // 4-ways, with border
--         r.ImGui_Separator();
--         r.ImGui_Text("ID"); r.ImGui_NextColumn();
--         r.ImGui_Text("Name"); r.ImGui_NextColumn();
--         r.ImGui_Text("Path"); r.ImGui_NextColumn();
--         r.ImGui_Text("Hovered"); r.ImGui_NextColumn();
--         r.ImGui_Separator();
--         const char* names[3] = { "One", "Two", "Three" };
--         const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
--         static int selected = -1;
--         for (int i = 0; i < 3; i++)
--         {
--             char label[32];
--             sprintf(label, "%04d", i);
--             if (r.ImGui_Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
--                 selected = i;
--             bool hovered = r.ImGui_IsItemHovered();
--             r.ImGui_NextColumn();
--             r.ImGui_Text(names[i]); r.ImGui_NextColumn();
--             r.ImGui_Text(paths[i]); r.ImGui_NextColumn();
--             r.ImGui_Text("%d", hovered); r.ImGui_NextColumn();
--         }
--         r.ImGui_Columns(1);
--         r.ImGui_Separator();
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Borders"))
--     {
--         // NB: Future columns API should allow automatic horizontal borders.
--         static bool h_borders = true;
--         static bool v_borders = true;
--         static int columns_count = 4;
--         const int lines_count = 3;
--         r.ImGui_SetNextItemWidth(r.ImGui_GetFontSize() * 8);
--         r.ImGui_DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d columns");
--         if (columns_count < 2)
--             columns_count = 2;
--         r.ImGui_SameLine();
--         r.ImGui_Checkbox("horizontal", &h_borders);
--         r.ImGui_SameLine();
--         r.ImGui_Checkbox("vertical", &v_borders);
--         r.ImGui_Columns(columns_count, NULL, v_borders);
--         for (int i = 0; i < columns_count * lines_count; i++)
--         {
--             if (h_borders && r.ImGui_GetColumnIndex() == 0)
--                 r.ImGui_Separator();
--             r.ImGui_Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
--             r.ImGui_Text("Width %.2f", r.ImGui_GetColumnWidth());
--             r.ImGui_Text("Avail %.2f", r.ImGui_GetContentRegionAvail().x);
--             r.ImGui_Text("Offset %.2f", r.ImGui_GetColumnOffset());
--             r.ImGui_Text("Long text that is likely to clip");
--             r.ImGui_Button("Button", ImVec2(-FLT_MIN, 0.0f));
--             r.ImGui_NextColumn();
--         }
--         r.ImGui_Columns(1);
--         if (h_borders)
--             r.ImGui_Separator();
--         r.ImGui_TreePop();
--     }
--
--     // Create multiple items in a same cell before switching to next column
--     if (r.ImGui_TreeNode("Mixed items"))
--     {
--         r.ImGui_Columns(3, "mixed");
--         r.ImGui_Separator();
--
--         r.ImGui_Text("Hello");
--         r.ImGui_Button("Banana");
--         r.ImGui_NextColumn();
--
--         r.ImGui_Text("ImGui");
--         r.ImGui_Button("Apple");
--         static float foo = 1.0f;
--         r.ImGui_InputFloat("red", &foo, 0.05f, 0, "%.3f");
--         r.ImGui_Text("An extra line here.");
--         r.ImGui_NextColumn();
--
--         r.ImGui_Text("Sailor");
--         r.ImGui_Button("Corniflower");
--         static float bar = 1.0f;
--         r.ImGui_InputFloat("blue", &bar, 0.05f, 0, "%.3f");
--         r.ImGui_NextColumn();
--
--         if (r.ImGui_CollapsingHeader("Category A")) { r.ImGui_Text("Blah blah blah"); } r.ImGui_NextColumn();
--         if (r.ImGui_CollapsingHeader("Category B")) { r.ImGui_Text("Blah blah blah"); } r.ImGui_NextColumn();
--         if (r.ImGui_CollapsingHeader("Category C")) { r.ImGui_Text("Blah blah blah"); } r.ImGui_NextColumn();
--         r.ImGui_Columns(1);
--         r.ImGui_Separator();
--         r.ImGui_TreePop();
--     }
--
--     // Word wrapping
--     if (r.ImGui_TreeNode("Word-wrapping"))
--     {
--         r.ImGui_Columns(2, "word-wrapping");
--         r.ImGui_Separator();
--         r.ImGui_TextWrapped("The quick brown fox jumps over the lazy dog.");
--         r.ImGui_TextWrapped("Hello Left");
--         r.ImGui_NextColumn();
--         r.ImGui_TextWrapped("The quick brown fox jumps over the lazy dog.");
--         r.ImGui_TextWrapped("Hello Right");
--         r.ImGui_Columns(1);
--         r.ImGui_Separator();
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Horizontal Scrolling"))
--     {
--         r.ImGui_SetNextWindowContentSize(ImVec2(1500.0f, 0.0f));
--         ImVec2 child_size = ImVec2(0, r.ImGui_GetFontSize() * 20.0f);
--         r.ImGui_BeginChild("##ScrollingRegion", child_size, false, ImGuiWindowFlags_HorizontalScrollbar);
--         r.ImGui_Columns(10);
--
--         // Also demonstrate using clipper for large vertical lists
--         int ITEMS_COUNT = 2000;
--         ImGuiListClipper clipper;
--         clipper.Begin(ITEMS_COUNT);
--         while (clipper.Step())
--         {
--             for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
--                 for (int j = 0; j < 10; j++)
--                 {
--                     r.ImGui_Text("Line %d Column %d...", i, j);
--                     r.ImGui_NextColumn();
--                 }
--         }
--         r.ImGui_Columns(1);
--         r.ImGui_EndChild();
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Tree"))
--     {
--         r.ImGui_Columns(2, "tree", true);
--         for (int x = 0; x < 3; x++)
--         {
--             bool open1 = r.ImGui_TreeNode((void*)(intptr_t)x, "Node%d", x);
--             r.ImGui_NextColumn();
--             r.ImGui_Text("Node contents");
--             r.ImGui_NextColumn();
--             if (open1)
--             {
--                 for (int y = 0; y < 3; y++)
--                 {
--                     bool open2 = r.ImGui_TreeNode((void*)(intptr_t)y, "Node%d.%d", x, y);
--                     r.ImGui_NextColumn();
--                     r.ImGui_Text("Node contents");
--                     if (open2)
--                     {
--                         r.ImGui_Text("Even more contents");
--                         if (r.ImGui_TreeNode("Tree in column"))
--                         {
--                             r.ImGui_Text("The quick brown fox jumps over the lazy dog");
--                             r.ImGui_TreePop();
--                         }
--                     }
--                     r.ImGui_NextColumn();
--                     if (open2)
--                         r.ImGui_TreePop();
--                 }
--                 r.ImGui_TreePop();
--             }
--         }
--         r.ImGui_Columns(1);
--         r.ImGui_TreePop();
--     }
--
--     r.ImGui_TreePop();
-- }

function demo.ShowDemoWindowMisc()
  local rv

--     if (r.ImGui_CollapsingHeader("Filtering"))
--     {
--         // Helper class to easy setup a text filter.
--         // You may want to implement a more feature-full filtering scheme in your own application.
--         static ImGuiTextFilter filter;
--         r.ImGui_Text("Filter usage:\n"
--                     "  \"\"         display all lines\n"
--                     "  \"xxx\"      display lines containing \"xxx\"\n"
--                     "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
--                     "  \"-xxx\"     hide lines containing \"xxx\"");
--         filter.Draw();
--         const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
--         for (int i = 0; i < IM_ARRAYSIZE(lines); i++)
--             if (filter.PassFilter(lines[i]))
--                 r.ImGui_BulletText("%s", lines[i]);
--     }

  if r.ImGui_CollapsingHeader(ctx, 'Inputs, Navigation & Focus') then
    -- Display ImGuiIO output flags
    -- r.ImGui_Text("WantCaptureMouse: %d", io.WantCaptureMouse);
    -- r.ImGui_Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
    -- r.ImGui_Text("WantTextInput: %d", io.WantTextInput);
    -- r.ImGui_Text("WantSetMousePos: %d", io.WantSetMousePos);
    -- r.ImGui_Text("NavActive: %d, NavVisible: %d", io.NavActive, io.NavVisible);

    -- Display Mouse state
    if r.ImGui_TreeNode(ctx, 'Mouse State') then
      if r.ImGui_IsMousePosValid(ctx) then
        r.ImGui_Text(ctx, ('Mouse pos: (%g, %g)'):format(r.ImGui_GetMousePos(ctx)))
      else
        r.ImGui_Text(ctx, 'Mouse pos: <INVALID>')
      end
      r.ImGui_Text(ctx, ('Mouse delta: (%g, %g)'):format(r.ImGui_GetMouseDelta(ctx)))

      local buttons = { r.ImGui_MouseButton_Left(), r.ImGui_MouseButton_Right(), r.ImGui_MouseButton_Middle() }
      local function MouseState(stateFunc)
        for _,button in ipairs(buttons) do
          if stateFunc(ctx, button) then
            r.ImGui_SameLine(ctx)
            r.ImGui_Text(ctx, ('b%d'):format(button))
          end
        end
      end
      r.ImGui_Text(ctx, 'Mouse down:')
      for _,button in ipairs(buttons) do
        if r.ImGui_IsMouseDown(ctx, button) then
          local duration = r.ImGui_GetMouseDownDuration(ctx, button)
          r.ImGui_SameLine(ctx)
          r.ImGui_Text(ctx, ('b%d (%.02f secs)'):format(button, duration))
        end
      end
      r.ImGui_Text(ctx, 'Mouse clicked:');  MouseState(r.ImGui_IsMouseClicked)
      r.ImGui_Text(ctx, 'Mouse dblclick:'); MouseState(r.ImGui_IsMouseDoubleClicked)
      r.ImGui_Text(ctx, 'Mouse released:'); MouseState(r.ImGui_IsMouseReleased)
      r.ImGui_Text(ctx, ('Mouse wheel: %.1f'):format(r.ImGui_GetMouseWheel(ctx)))
      -- r.ImGui_Text(cxt, ('Pen Pressure: %.1f'):format(r.ImGui_GetPenPressure(ctx))) -- Note: currently unused
      r.ImGui_TreePop(ctx)
    end

    -- Display Keyboard/Mouse state
    if r.ImGui_TreeNode(ctx, 'Keyboard & Navigation State') then
      local max_key = 512
      r.ImGui_Text(ctx, 'Keys down:')
      for i = 0, max_key - 1 do
        if r.ImGui_IsKeyDown(ctx, i) then
          local duration = r.ImGui_GetKeyDownDuration(ctx, i)
          r.ImGui_SameLine(ctx)
          r.ImGui_Text(ctx, ('%d (0x%X) (%.02f secs)'):format(i, i, duration))
        end
      end
      local function KeyboardState(stateFunc)
        for i = 0, max_key - 1 do
          if stateFunc(ctx, i) then
            r.ImGui_SameLine(ctx)
            r.ImGui_Text(ctx, ('%d (0x%X)'):format(i, i))
          end
        end
      end
      r.ImGui_Text(ctx, 'Keys pressed:'); KeyboardState(r.ImGui_IsKeyPressed)
      r.ImGui_Text(ctx, 'Keys release:'); KeyboardState(r.ImGui_IsKeyReleased)
      local mods = r.ImGui_GetKeyMods(ctx)
      r.ImGui_Text(ctx, ('Keys mods: %s%s%s%s'):format(
        (mods & r.ImGui_KeyModFlags_Ctrl())  ~= 0  and 'CTRL '   or '',
        (mods & r.ImGui_KeyModFlags_Shift()) ~= 0  and 'SHIFT '  or '',
        (mods & r.ImGui_KeyModFlags_Alt())   ~= 0  and 'ALT '    or '',
        (mods & r.ImGui_KeyModFlags_Super()) ~= 0  and 'SUPER '  or ''))

      r.ImGui_Text(ctx, 'Chars queue:')
      local next_id = 0
      while true do
        local rv, c = r.ImGui_GetInputQueueCharacter(ctx, next_id)
        if not rv then break end
        next_id = next_id + 1
        r.ImGui_SameLine(ctx)
        r.ImGui_Text(ctx, ("'%s' (0x%04X)"):format(utf8.char(c), c))
      end

      -- r.ImGui_Text(ctx, 'NavInputs down:');     for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f)              { r.ImGui_SameLine(ctx); r.ImGui_Text(ctx, '[%d] %.2f (%.02f secs)', i, io.NavInputs[i], io.NavInputsDownDuration[i]); }
      -- r.ImGui_Text('NavInputs pressed:');  for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { r.ImGui_SameLine(ctx); r.ImGui_Text(ctx, '[%d]', i); }

      -- r.ImGui_Button("Hovering me sets the\nkeyboard capture flag");
      -- if (r.ImGui_IsItemHovered())
      --     r.ImGui_CaptureKeyboardFromApp(true);
      -- r.ImGui_SameLine();
      -- r.ImGui_Button("Holding me clears the\nthe keyboard capture flag");
      -- if (r.ImGui_IsItemActive())
      --     r.ImGui_CaptureKeyboardFromApp(false);

      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Tabbing') then
      if not misc.tabbing then
        misc.tabbing = {
          buf = 'hello',
        }
      end

      r.ImGui_Text(ctx, 'Use TAB/SHIFT+TAB to cycle through keyboard editable fields.')
      rv,misc.tabbing.buf = r.ImGui_InputText(ctx, '1', misc.tabbing.buf)
      rv,misc.tabbing.buf = r.ImGui_InputText(ctx, '2', misc.tabbing.buf)
      rv,misc.tabbing.buf = r.ImGui_InputText(ctx, '3', misc.tabbing.buf)
      r.ImGui_PushAllowKeyboardFocus(ctx, false)
      rv,misc.tabbing.buf = r.ImGui_InputText(ctx, '4 (tab skip)', misc.tabbing.buf)
      -- r.ImGui_SameLine(ctx); demo.HelpMarker('Use r.ImGui_PushAllowKeyboardFocus(bool) to disable tabbing through certain widgets.')
      r.ImGui_PopAllowKeyboardFocus(ctx)
      rv,misc.tabbing.buf = r.ImGui_InputText(ctx, '5', misc.tabbing.buf)
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Focus from code') then
      if not misc.focus then
        misc.focus = {
          buf = 'click on a button to set focus',
          d3  = { 0.0, 0.0, 0.0 }
        }
      end

      local focus_1 = r.ImGui_Button(ctx, 'Focus on 1'); r.ImGui_SameLine(ctx)
      local focus_2 = r.ImGui_Button(ctx, 'Focus on 2'); r.ImGui_SameLine(ctx)
      local focus_3 = r.ImGui_Button(ctx, 'Focus on 3')
      local has_focus = 0

      if focus_1 then r.ImGui_SetKeyboardFocusHere(ctx) end
      rv,misc.focus.buf = r.ImGui_InputText(ctx, '1', misc.focus.buf)
      if r.ImGui_IsItemActive(ctx) then has_focus = 1 end

      if focus_2 then r.ImGui_SetKeyboardFocusHere(ctx) end
      rv,misc.focus.buf = r.ImGui_InputText(ctx, '2', misc.focus.buf)
      if r.ImGui_IsItemActive(ctx) then has_focus = 2 end

      r.ImGui_PushAllowKeyboardFocus(ctx, false)
      if focus_3 then r.ImGui_SetKeyboardFocusHere(ctx) end
      rv,misc.focus.buf = r.ImGui_InputText(ctx, '3 (tab skip)', misc.focus.buf)
      if r.ImGui_IsItemActive(ctx) then has_focus = 3 end
      r.ImGui_PopAllowKeyboardFocus(ctx)

      if has_focus > 0 then
        r.ImGui_Text(ctx, ('Item with focus: %d'):format(has_focus))
      else
        r.ImGui_Text(ctx, 'Item with focus: <none>')
      end

      -- Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
      local focus_ahead = -1
      if r.ImGui_Button(ctx, 'Focus on X') then focus_ahead = 0 end r.ImGui_SameLine(ctx)
      if r.ImGui_Button(ctx, 'Focus on Y') then focus_ahead = 1 end r.ImGui_SameLine(ctx)
      if r.ImGui_Button(ctx, 'Focus on Z') then focus_ahead = 2 end
      if focus_ahead ~= -1 then r.ImGui_SetKeyboardFocusHere(ctx, focus_ahead) end
      rv,misc.focus.d3[1],misc.focus.d3[2],misc.focus.d3[3] =
        r.ImGui_SliderDouble3(ctx, 'Float3', misc.focus.d3[1], misc.focus.d3[2], misc.focus.d3[3], 0.0, 1.0)

      r.ImGui_TextWrapped(ctx, 'NB: Cursor & selection are preserved when refocusing last used item in code.')
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Dragging') then
      r.ImGui_TextWrapped(ctx, 'You can use r.ImGui_GetMouseDragDelta(0) to query for the dragged amount on any widget.')
      for button = 0, 2 do
        r.ImGui_Text(ctx, ('IsMouseDragging(%d):'):format(button))
        r.ImGui_Text(ctx, ('  w/ default threshold: %s,'):format(r.ImGui_IsMouseDragging(ctx, button)))
        r.ImGui_Text(ctx, ('  w/ zero threshold: %s,'):format(r.ImGui_IsMouseDragging(ctx, button, 0.0)))
        r.ImGui_Text(ctx, ('  w/ large threshold: %s,'):format(r.ImGui_IsMouseDragging(ctx, button, 20.0)))
      end

      r.ImGui_Button(ctx, 'Drag Me')
      if r.ImGui_IsItemActive(ctx) then
        -- Draw a line between the button and the mouse cursor
        local draw_list = r.ImGui_GetForegroundDrawList(ctx)
        local mouse_pos = { r.ImGui_GetMousePos(ctx) }
        local click_pos = { r.ImGui_GetMouseClickedPos(ctx, 0) }
        local color = r.ImGui_GetColor(ctx, r.ImGui_Col_Button())
        r.ImGui_DrawList_AddLine(draw_list, click_pos[1], click_pos[2], mouse_pos[1], mouse_pos[2], color, 4.0)
      end

      -- Drag operations gets "unlocked" when the mouse has moved past a certain threshold
      -- (the default threshold is stored in io.MouseDragThreshold). You can request a lower or higher
      -- threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta().
      local value_raw = { r.ImGui_GetMouseDragDelta(ctx, 0, 0, r.ImGui_MouseButton_Left(), 0.0) }
      local value_with_lock_threshold = { r.ImGui_GetMouseDragDelta(ctx, 0, 0, r.ImGui_MouseButton_Left()) }
      local mouse_delta = { r.ImGui_GetMouseDelta(ctx) }
      r.ImGui_Text(ctx, 'GetMouseDragDelta(0):')
      r.ImGui_Text(ctx, ('  w/ default threshold: (%.1f, %.1f)'):format(table.unpack(value_with_lock_threshold)))
      r.ImGui_Text(ctx, ('  w/ zero threshold: (%.1f, %.1f)'):format(table.unpack(value_raw)))
      r.ImGui_Text(ctx, ('GetMouseDelta() (%.1f, %.1f)'):format(table.unpack(mouse_delta)))
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Mouse cursors') then
      local mouse_cursors_names = { 'Arrow', 'TextInput', 'ResizeAll', 'ResizeNS', 'ResizeEW', 'ResizeNESW', 'ResizeNWSE', 'Hand', 'NotAllowed' }

      local current = r.ImGui_GetMouseCursor(ctx)
      r.ImGui_Text(ctx, ('Current mouse cursor = %d: %s'):format(current, mouse_cursors_names[current + 1]))
      r.ImGui_Text(ctx, 'Hover to see mouse cursors:')
      r.ImGui_SameLine(ctx); demo.HelpMarker(
        'Your application can render a different mouse cursor based on what r.ImGui_GetMouseCursor() returns. \z
         If software cursor rendering (io.MouseDrawCursor) is set ImGui will draw the right cursor for you, \z
         otherwise your backend needs to handle it.')
      for i,name in ipairs(mouse_cursors_names) do
        local label = ('Mouse cursor %d: %s'):format(i - 1, name)
        r.ImGui_Bullet(ctx); r.ImGui_Selectable(ctx, label, false)
        if r.ImGui_IsItemHovered(ctx) then
          r.ImGui_SetMouseCursor(ctx, i - 1)
        end
      end
      r.ImGui_TreePop(ctx)
    end
  end
end

-------------------------------------------------------------------------------
-- [SECTION] About Window / ShowAboutWindow()
-- Access from Dear ImGui Demo -> Tools -> About
-------------------------------------------------------------------------------

function demo.ShowAboutWindow()
  local rv,open = r.ImGui_Begin(ctx, 'About Dear ImGui', true, r.ImGui_WindowFlags_AlwaysAutoResize())
  if not rv then
    r.ImGui_End(ctx)
    return open
  end
  r.ImGui_Separator(ctx)
  r.ImGui_Text(ctx, ('Dear ImGui %s'):format(IMGUI_VERSION))
  r.ImGui_Separator(ctx)
  r.ImGui_Text(ctx, 'By Omar Cornut and all Dear ImGui contributors.')
  r.ImGui_Text(ctx, 'Dear ImGui is licensed under the MIT License.')

  r.ImGui_Spacing(ctx)

  r.ImGui_Separator(ctx)
  r.ImGui_Text(ctx, ('reaper_imgui %s'):format(REAIMGUI_VERSION))
  r.ImGui_Separator(ctx)
  r.ImGui_Text(ctx, 'By Christian Fillion and contributors.')
  r.ImGui_Text(ctx, 'ReaImGui is licensed under the LGPL License.')
  r.ImGui_End(ctx)

  return open
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
function demo.ShowExampleAppMainMenuBar()
  if r.ImGui_BeginMainMenuBar(ctx) then
    if r.ImGui_BeginMenu(ctx, 'File') then
      demo.ShowExampleMenuFile()
      r.ImGui_EndMenu(ctx)
    end
    if r.ImGui_BeginMenu(ctx, 'Edit') then
      if r.ImGui_MenuItem(ctx, 'Undo', 'CTRL+Z') then end
      if r.ImGui_MenuItem(ctx, 'Redo', 'CTRL+Y', false, false) then end -- Disabled item
      r.ImGui_Separator(ctx)
      if r.ImGui_MenuItem(ctx, 'Cut', 'CTRL+X') then end
      if r.ImGui_MenuItem(ctx, 'Copy', 'CTRL+C') then end
      if r.ImGui_MenuItem(ctx, 'Paste', 'CTRL+V') then end
      r.ImGui_EndMenu(ctx)
    end
    r.ImGui_EndMainMenuBar(ctx)
  end
end

-- Note that shortcuts are currently provided for display only
-- (future version will add explicit flags to BeginMenu() to request processing shortcuts)
function demo.ShowExampleMenuFile()
  local rv

  r.ImGui_MenuItem(ctx, '(demo menu)', nil, false, false)
  if r.ImGui_MenuItem(ctx, 'New') then end
  if r.ImGui_MenuItem(ctx, 'Open', 'Ctrl+O') then end
  if r.ImGui_BeginMenu(ctx, 'Open Recent') then
    r.ImGui_MenuItem(ctx, 'fish_hat.c');
    r.ImGui_MenuItem(ctx, 'fish_hat.inl');
    r.ImGui_MenuItem(ctx, 'fish_hat.h');
    if r.ImGui_BeginMenu(ctx,'More..') then
      r.ImGui_MenuItem(ctx, 'Hello')
      r.ImGui_MenuItem(ctx, 'Sailor')
      if r.ImGui_BeginMenu(ctx, 'Recurse..') then
        demo.ShowExampleMenuFile()
        r.ImGui_EndMenu(ctx)
      end
      r.ImGui_EndMenu(ctx)
      end
    r.ImGui_EndMenu(ctx)
  end
  if r.ImGui_MenuItem(ctx, 'Save', 'Ctrl+S') then end
  if r.ImGui_MenuItem(ctx, 'Save As..') then end

  r.ImGui_Separator(ctx)
  if r.ImGui_BeginMenu(ctx, 'Options') then
    rv,demo.menu.enabled = r.ImGui_MenuItem(ctx, 'Enabled', "", demo.menu.enabled)
    r.ImGui_BeginChild(ctx, 'child', 0, 60, true)
    for i = 0, 9 do
      r.ImGui_Text(ctx, ('Scrolling Text %d'):format(i))
    end
    r.ImGui_EndChild(ctx)
    rv,demo.menu.f = r.ImGui_SliderDouble(ctx, 'Value', demo.menu.f, 0.0, 1.0)
    rv,demo.menu.f = r.ImGui_InputDouble(ctx, 'Input', demo.menu.f, 0.1)
    rv,demo.menu.n = r.ImGui_Combo(ctx, 'Combo', demo.menu.n, 'Yes\31No\31Maybe\31')
    r.ImGui_EndMenu(ctx)
  end

  if r.ImGui_BeginMenu(ctx, 'Colors') then
    local sz = r.ImGui_GetTextLineHeight(ctx)
    local draw_list = r.ImGui_GetWindowDrawList(ctx)
    for i = 0, r.ImGui_Col_ModalWindowDimBg() do
      local name = r.ImGui_GetStyleColorName(i)
      local x, y = r.ImGui_GetCursorScreenPos(ctx)
      r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, r.ImGui_GetColor(ctx, i))
      r.ImGui_Dummy(ctx, sz, sz)
      r.ImGui_SameLine(ctx)
      r.ImGui_MenuItem(ctx, name)
    end
    r.ImGui_EndMenu(ctx)
  end

  -- Here we demonstrate appending again to the "Options" menu (which we already created above)
  -- Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
  -- In a real code-base using it would make senses to use this feature from very different code locations.
  if r.ImGui_BeginMenu(ctx, 'Options') then -- <-- Append!
    rv,demo.menu.b = r.ImGui_Checkbox(ctx, 'SomeOption', demo.menu.b)
    r.ImGui_EndMenu(ctx)
  end

  if r.ImGui_BeginMenu(ctx, 'Disabled', false) then -- Disabled
    error('never called')
  end
  if r.ImGui_MenuItem(ctx, 'Checked', nil, true) then end
  if r.ImGui_MenuItem(ctx, 'Quit', 'Alt+F4') then end
end

-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Debug Console / ShowExampleAppConsole()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
-- // For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
-- struct ExampleAppConsole
-- {
--     char                  InputBuf[256];
--     ImVector<char*>       Items;
--     ImVector<const char*> Commands;
--     ImVector<char*>       History;
--     int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
--     ImGuiTextFilter       Filter;
--     bool                  AutoScroll;
--     bool                  ScrollToBottom;
--
--     ExampleAppConsole()
--     {
--         ClearLog();
--         memset(InputBuf, 0, sizeof(InputBuf));
--         HistoryPos = -1;
--
--         // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
--         Commands.push_back("HELP");
--         Commands.push_back("HISTORY");
--         Commands.push_back("CLEAR");
--         Commands.push_back("CLASSIFY");
--         AutoScroll = true;
--         ScrollToBottom = false;
--         AddLog("Welcome to Dear ImGui!");
--     }
--     ~ExampleAppConsole()
--     {
--         ClearLog();
--         for (int i = 0; i < History.Size; i++)
--             free(History[i]);
--     }
--
--     // Portable helpers
--     static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
--     static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
--     static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
--     static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }
--
--     void    ClearLog()
--     {
--         for (int i = 0; i < Items.Size; i++)
--             free(Items[i]);
--         Items.clear();
--     }
--
--     void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
--     {
--         // FIXME-OPT
--         char buf[1024];
--         va_list args;
--         va_start(args, fmt);
--         vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
--         buf[IM_ARRAYSIZE(buf)-1] = 0;
--         va_end(args);
--         Items.push_back(Strdup(buf));
--     }
--
--     void    Draw(const char* title, bool* p_open)
--     {
--         r.ImGui_SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
--         if (!r.ImGui_Begin(title, p_open))
--         {
--             r.ImGui_End();
--             return;
--         }
--
--         // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
--         // So e.g. IsItemHovered() will return true when hovering the title bar.
--         // Here we create a context menu only available from the title bar.
--         if (r.ImGui_BeginPopupContextItem())
--         {
--             if (r.ImGui_MenuItem("Close Console"))
--                 *p_open = false;
--             r.ImGui_EndPopup();
--         }
--
--         r.ImGui_TextWrapped(
--             "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
--             "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
--         r.ImGui_TextWrapped("Enter 'HELP' for help.");
--
--         // TODO: display items starting from the bottom
--
--         if (r.ImGui_SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
--         r.ImGui_SameLine();
--         if (r.ImGui_SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
--         r.ImGui_SameLine();
--         if (r.ImGui_SmallButton("Clear"))           { ClearLog(); }
--         r.ImGui_SameLine();
--         bool copy_to_clipboard = r.ImGui_SmallButton("Copy");
--         //static float t = 0.0f; if (r.ImGui_GetTime() - t > 0.02f) { t = r.ImGui_GetTime(); AddLog("Spam %f", t); }
--
--         r.ImGui_Separator();
--
--         // Options menu
--         if (r.ImGui_BeginPopup("Options"))
--         {
--             r.ImGui_Checkbox("Auto-scroll", &AutoScroll);
--             r.ImGui_EndPopup();
--         }
--
--         // Options, Filter
--         if (r.ImGui_Button("Options"))
--             r.ImGui_OpenPopup("Options");
--         r.ImGui_SameLine();
--         Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
--         r.ImGui_Separator();
--
--         // Reserve enough left-over height for 1 separator + 1 input text
--         const float footer_height_to_reserve = r.ImGui_GetStyle().ItemSpacing.y + r.ImGui_GetFrameHeightWithSpacing();
--         r.ImGui_BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
--         if (r.ImGui_BeginPopupContextWindow())
--         {
--             if (r.ImGui_Selectable("Clear")) ClearLog();
--             r.ImGui_EndPopup();
--         }
--
--         // Display every line as a separate entry so we can change their color or add custom widgets.
--         // If you only want raw text you can use r.ImGui_TextUnformatted(log.begin(), log.end());
--         // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
--         // to only process visible items. The clipper will automatically measure the height of your first item and then
--         // "seek" to display only items in the visible area.
--         // To use the clipper we can replace your standard loop:
--         //      for (int i = 0; i < Items.Size; i++)
--         //   With:
--         //      ImGuiListClipper clipper;
--         //      clipper.Begin(Items.Size);
--         //      while (clipper.Step())
--         //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
--         // - That your items are evenly spaced (same height)
--         // - That you have cheap random access to your elements (you can access them given their index,
--         //   without processing all the ones before)
--         // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
--         // We would need random-access on the post-filtered list.
--         // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
--         // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
--         // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
--         // to improve this example code!
--         // If your items are of variable height:
--         // - Split them into same height items would be simpler and facilitate random-seeking into your list.
--         // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
--         r.ImGui_PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
--         if (copy_to_clipboard)
--             r.ImGui_LogToClipboard();
--         for (int i = 0; i < Items.Size; i++)
--         {
--             const char* item = Items[i];
--             if (!Filter.PassFilter(item))
--                 continue;
--
--             // Normally you would store more information in your item than just a string.
--             // (e.g. make Items[] an array of structure, store color/type etc.)
--             ImVec4 color;
--             bool has_color = false;
--             if (strstr(item, "[error]"))          { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
--             else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
--             if (has_color)
--                 r.ImGui_PushStyleColor(ImGuiCol_Text, color);
--             r.ImGui_TextUnformatted(item);
--             if (has_color)
--                 r.ImGui_PopStyleColor();
--         }
--         if (copy_to_clipboard)
--             r.ImGui_LogFinish();
--
--         if (ScrollToBottom || (AutoScroll && r.ImGui_GetScrollY() >= r.ImGui_GetScrollMaxY()))
--             r.ImGui_SetScrollHereY(1.0f);
--         ScrollToBottom = false;
--
--         r.ImGui_PopStyleVar();
--         r.ImGui_EndChild();
--         r.ImGui_Separator();
--
--         // Command-line
--         bool reclaim_focus = false;
--         ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
--         if (r.ImGui_InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
--         {
--             char* s = InputBuf;
--             Strtrim(s);
--             if (s[0])
--                 ExecCommand(s);
--             strcpy(s, "");
--             reclaim_focus = true;
--         }
--
--         // Auto-focus on window apparition
--         r.ImGui_SetItemDefaultFocus();
--         if (reclaim_focus)
--             r.ImGui_SetKeyboardFocusHere(-1); // Auto focus previous widget
--
--         r.ImGui_End();
--     }
--
--     void    ExecCommand(const char* command_line)
--     {
--         AddLog("# %s\n", command_line);
--
--         // Insert into history. First find match and delete it so it can be pushed to the back.
--         // This isn't trying to be smart or optimal.
--         HistoryPos = -1;
--         for (int i = History.Size - 1; i >= 0; i--)
--             if (Stricmp(History[i], command_line) == 0)
--             {
--                 free(History[i]);
--                 History.erase(History.begin() + i);
--                 break;
--             }
--         History.push_back(Strdup(command_line));
--
--         // Process command
--         if (Stricmp(command_line, "CLEAR") == 0)
--         {
--             ClearLog();
--         }
--         else if (Stricmp(command_line, "HELP") == 0)
--         {
--             AddLog("Commands:");
--             for (int i = 0; i < Commands.Size; i++)
--                 AddLog("- %s", Commands[i]);
--         }
--         else if (Stricmp(command_line, "HISTORY") == 0)
--         {
--             int first = History.Size - 10;
--             for (int i = first > 0 ? first : 0; i < History.Size; i++)
--                 AddLog("%3d: %s\n", i, History[i]);
--         }
--         else
--         {
--             AddLog("Unknown command: '%s'\n", command_line);
--         }
--
--         // On command input, we scroll to bottom even if AutoScroll==false
--         ScrollToBottom = true;
--     }
--
--     // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
--     static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
--     {
--         ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
--         return console->TextEditCallback(data);
--     }
--
--     int     TextEditCallback(ImGuiInputTextCallbackData* data)
--     {
--         //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
--         switch (data->EventFlag)
--         {
--         case ImGuiInputTextFlags_CallbackCompletion:
--             {
--                 // Example of TEXT COMPLETION
--
--                 // Locate beginning of current word
--                 const char* word_end = data->Buf + data->CursorPos;
--                 const char* word_start = word_end;
--                 while (word_start > data->Buf)
--                 {
--                     const char c = word_start[-1];
--                     if (c == ' ' || c == '\t' || c == ',' || c == ';')
--                         break;
--                     word_start--;
--                 }
--
--                 // Build a list of candidates
--                 ImVector<const char*> candidates;
--                 for (int i = 0; i < Commands.Size; i++)
--                     if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
--                         candidates.push_back(Commands[i]);
--
--                 if (candidates.Size == 0)
--                 {
--                     // No match
--                     AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
--                 }
--                 else if (candidates.Size == 1)
--                 {
--                     // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
--                     data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
--                     data->InsertChars(data->CursorPos, candidates[0]);
--                     data->InsertChars(data->CursorPos, " ");
--                 }
--                 else
--                 {
--                     // Multiple matches. Complete as much as we can..
--                     // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
--                     int match_len = (int)(word_end - word_start);
--                     for (;;)
--                     {
--                         int c = 0;
--                         bool all_candidates_matches = true;
--                         for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
--                             if (i == 0)
--                                 c = toupper(candidates[i][match_len]);
--                             else if (c == 0 || c != toupper(candidates[i][match_len]))
--                                 all_candidates_matches = false;
--                         if (!all_candidates_matches)
--                             break;
--                         match_len++;
--                     }
--
--                     if (match_len > 0)
--                     {
--                         data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
--                         data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
--                     }
--
--                     // List matches
--                     AddLog("Possible matches:\n");
--                     for (int i = 0; i < candidates.Size; i++)
--                         AddLog("- %s\n", candidates[i]);
--                 }
--
--                 break;
--             }
--         case ImGuiInputTextFlags_CallbackHistory:
--             {
--                 // Example of HISTORY
--                 const int prev_history_pos = HistoryPos;
--                 if (data->EventKey == ImGuiKey_UpArrow)
--                 {
--                     if (HistoryPos == -1)
--                         HistoryPos = History.Size - 1;
--                     else if (HistoryPos > 0)
--                         HistoryPos--;
--                 }
--                 else if (data->EventKey == ImGuiKey_DownArrow)
--                 {
--                     if (HistoryPos != -1)
--                         if (++HistoryPos >= History.Size)
--                             HistoryPos = -1;
--                 }
--
--                 // A better implementation would preserve the data on the current input line along with cursor position.
--                 if (prev_history_pos != HistoryPos)
--                 {
--                     const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
--                     data->DeleteChars(0, data->BufTextLen);
--                     data->InsertChars(0, history_str);
--                 }
--             }
--         }
--         return 0;
--     }
-- };
--
-- static void ShowExampleAppConsole(bool* p_open)
-- {
--     static ExampleAppConsole console;
--     console.Draw("Example: Console", p_open);
-- }

-------------------------------------------------------------------------------
-- [SECTION] Example App: Debug Log / ShowExampleAppLog()
-------------------------------------------------------------------------------

-- Usage:
--   local my_log = ExampleAppLog:new(ctx)
--   my_log:add_log('Hello %d world\n', 123)
--   my_log:draw('title')

local ExampleAppLog = {}
function ExampleAppLog:new(ctx)
  obj = {
    ctx          = ctx,
    lines        = {},
    -- filter       = ImGuiTextFilter,
    auto_scroll  = true, -- Keep scrolling if already at the bottom.
  }
  setmetatable(obj, self)
  self.__index = self
  return obj
end

function ExampleAppLog.clear(self)
  self.lines = {}
end

function ExampleAppLog.add_log(self, fmt, ...)
  local text = fmt:format(...)
  for line in text:gmatch("[^\r\n]+") do
    table.insert(self.lines, line)
  end
end

function ExampleAppLog.draw(self, title, p_open)
  local rv,p_open = r.ImGui_Begin(self.ctx, title, p_open)
  if not rv then
    r.ImGui_End(self.ctx)
    return p_open
  end

  -- Options menu
  if r.ImGui_BeginPopup(self.ctx, 'Options') then
    rv,self.auto_scroll = r.ImGui_Checkbox(self.ctx, 'Auto-scroll', self.auto_scroll)
    r.ImGui_EndPopup(self.ctx)
  end

  -- Main window
  if r.ImGui_Button(self.ctx, 'Options') then
    r.ImGui_OpenPopup(self.ctx, 'Options')
  end
  r.ImGui_SameLine(self.ctx)
  local clear = r.ImGui_Button(self.ctx, 'Clear')
  r.ImGui_SameLine(self.ctx)
  local copy = r.ImGui_Button(self.ctx, 'Copy')
  -- r.ImGui_SameLine(self.ctx)
  -- Filter.Draw(ctx, 'Filter', -100.0)

  r.ImGui_Separator(self.ctx)
  r.ImGui_BeginChild(self.ctx, 'scrolling', 0, 0, false, r.ImGui_WindowFlags_HorizontalScrollbar())

  if clear then
    self:clear()
  end
  if copy then
    r.ImGui_LogToClipboard(self.ctx)
  end

  r.ImGui_PushStyleVar(self.ctx, r.ImGui_StyleVar_ItemSpacing(), 0, 0)
  -- const char* buf = Buf.begin();
  -- const char* buf_end = Buf.end();
  -- if (Filter.IsActive())
  -- {
  --     // In this example we don't use the clipper when Filter is enabled.
  --     // This is because we don't have a random access on the result on our filter.
  --     // A real application processing logs with ten of thousands of entries may want to store the result of
  --     // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
  --     for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
  --     {
  --         const char* line_start = buf + LineOffsets[line_no];
  --         const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
  --         if (Filter.PassFilter(line_start, line_end))
  --             r.ImGui_TextUnformatted(line_start, line_end);
  --     }
  -- }
  -- else
  -- {
    -- The simplest and easy way to display the entire buffer:
    --   r.ImGui_TextUnformatted(buf_begin, buf_end);
    -- And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
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
    local clipper = r.ImGui_CreateListClipper(self.ctx)
    r.ImGui_ListClipper_Begin(clipper, #self.lines)
    while r.ImGui_ListClipper_Step(clipper) do
      local display_start, display_end = r.ImGui_ListClipper_GetDisplayRange(clipper)
      for line_no = display_start, display_end - 1 do
        r.ImGui_Text(self.ctx, self.lines[line_no + 1])
      end
    end
    r.ImGui_ListClipper_End(clipper)
  -- }
  r.ImGui_PopStyleVar(self.ctx)

  if self.auto_scroll and r.ImGui_GetScrollY(self.ctx) >= r.ImGui_GetScrollMaxY(self.ctx) then
    r.ImGui_SetScrollHereY(self.ctx, 1.0)
  end

  r.ImGui_EndChild(self.ctx)
  r.ImGui_End(self.ctx)
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
  r.ImGui_SetNextWindowSize(ctx, 500, 400, r.ImGui_Cond_FirstUseEver())
  local rv,open = r.ImGui_Begin(ctx, 'Example: Log', true)
  if r.ImGui_SmallButton(ctx, '[Debug] Add 5 entries') then
    local categories = { "info", "warn", "error" }
    local words = { "Bumfuzzled", "Cattywampus", "Snickersnee",
                    "Abibliophobia", "Absquatulate", "Nincompoop",
                    "Pauciloquent" }
    for n = 0, 5 - 1 do
      local category = categories[(app.log.counter % #categories) + 1]
      local word = words[(app.log.counter % #words) + 1]
      app.log:add_log("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
          r.ImGui_GetFrameCount(ctx), category, r.ImGui_GetTime(ctx), word)
      app.log.counter = app.log.counter + 1
    end
  end
  r.ImGui_End(ctx)

  -- Actually call in the regular Log helper (which will Begin() into the same window as we just did)
  return app.log:draw("Example: Log", open)
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

  r.ImGui_SetNextWindowSize(ctx, 500, 440, r.ImGui_Cond_FirstUseEver())
  local rv,open = r.ImGui_Begin(ctx, 'Example: Simple layout', true, r.ImGui_WindowFlags_MenuBar())
  if not rv then
    r.ImGui_End(ctx)
    return open
  end

  if r.ImGui_BeginMenuBar(ctx) then
    if r.ImGui_BeginMenu(ctx, 'File') then
      if r.ImGui_MenuItem(ctx, 'Close') then open = false end
      r.ImGui_EndMenu(ctx)
    end
    r.ImGui_EndMenuBar(ctx)
  end

  -- Left
  r.ImGui_BeginChild(ctx, 'left pane', 150, 0, true)
  for i = 0, 100 - 1 do
    if r.ImGui_Selectable(ctx, ('MyObject %d'):format(i), app.layout.selected == i) then
      app.layout.selected = i
    end
  end
  r.ImGui_EndChild(ctx)
  r.ImGui_SameLine(ctx)

  -- Right
  r.ImGui_BeginGroup(ctx)
  r.ImGui_BeginChild(ctx, 'item view', 0, -r.ImGui_GetFrameHeightWithSpacing(ctx)) -- Leave room for 1 line below us
  r.ImGui_Text(ctx, ("MyObject: %d"):format(app.layout.selected))
  r.ImGui_Separator(ctx)
  if r.ImGui_BeginTabBar(ctx, '##Tabs', r.ImGui_TabBarFlags_None()) then
    if r.ImGui_BeginTabItem(ctx, 'Description') then
      r.ImGui_TextWrapped(ctx, 'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ')
      r.ImGui_EndTabItem(ctx)
    end
    if r.ImGui_BeginTabItem(ctx, 'Details') then
      r.ImGui_Text(ctx, 'ID: 0123456789')
      r.ImGui_EndTabItem(ctx)
    end
    r.ImGui_EndTabBar(ctx)
  end
  r.ImGui_EndChild(ctx)
  if r.ImGui_Button(ctx, 'Revert') then end
  r.ImGui_SameLine(ctx)
  if r.ImGui_Button(ctx, 'Save') then end
  r.ImGui_EndGroup(ctx)
  r.ImGui_End(ctx)
  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
-------------------------------------------------------------------------------

function demo.ShowPlaceholderObject(prefix, uid)
  local rv

  -- Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
  r.ImGui_PushID(ctx, uid)

  -- Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add vertical spacing to make the tree lines equal high.
  r.ImGui_TableNextRow(ctx)
  r.ImGui_TableSetColumnIndex(ctx, 0)
  r.ImGui_AlignTextToFramePadding(ctx)
  local node_open = r.ImGui_TreeNodeEx(ctx, 'Object', ('%s_%u'):format(prefix, uid))
  r.ImGui_TableSetColumnIndex(ctx, 1)
  r.ImGui_Text(ctx, 'my sailor is rich')

  if node_open then
    for i = 0, #app.property_editor.placeholder_members - 1 do
      r.ImGui_PushID(ctx, i) -- Use field index as identifier.
      if i < 2 then
        demo.ShowPlaceholderObject('Child', 424242)
      else
        -- Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
        r.ImGui_TableNextRow(ctx)
        r.ImGui_TableSetColumnIndex(ctx, 0)
        r.ImGui_AlignTextToFramePadding(ctx)
        local flags = r.ImGui_TreeNodeFlags_Leaf()             |
                      r.ImGui_TreeNodeFlags_NoTreePushOnOpen() |
                      r.ImGui_TreeNodeFlags_Bullet()
        r.ImGui_TreeNodeEx(ctx, 'Field', ('Field_%d'):format(i), flags)

        r.ImGui_TableSetColumnIndex(ctx, 1)
        r.ImGui_SetNextItemWidth(ctx, -FLT_MIN)
        if i >= 5 then
          rv,app.property_editor.placeholder_members[i] =
            r.ImGui_InputDouble(ctx, '##value', app.property_editor.placeholder_members[i], 1.0)
        else
          rv,app.property_editor.placeholder_members[i] =
            r.ImGui_DragDouble(ctx, '##value', app.property_editor.placeholder_members[i], 0.01)
        end
      end
      r.ImGui_PopID(ctx)
    end
    r.ImGui_TreePop(ctx)
  end
  r.ImGui_PopID(ctx)
end

-- Demonstrate create a simple property editor.
function demo.ShowExampleAppPropertyEditor()
  if not app.property_editor then
    app.property_editor = {
      placeholder_members = { 0.0, 0.0, 1.0, 3.1416, 100.0, 999.0, 0.0, 0.0 },
    }
  end

  r.ImGui_SetNextWindowSize(ctx, 430, 450, r.ImGui_Cond_FirstUseEver())
  local rv,open = r.ImGui_Begin(ctx, 'Example: Property editor', true)
  if not rv then
    r.ImGui_End(ctx)
    return open
  end

  demo.HelpMarker(
    'This example shows how you may implement a property editor using two columns.\n\z
     All objects/fields data are dummies here.\n\z
     Remember that in many simple cases, you can use r.ImGui_SameLine(xxx) to position\n\z
     your cursor horizontally instead of using the Columns() API.')

  r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_FramePadding(), 2, 2)
  if r.ImGui_BeginTable(ctx, 'split', 2, r.ImGui_TableFlags_BordersOuter() | r.ImGui_TableFlags_Resizable()) then
    -- Iterate placeholder objects (all the same data)
    for obj_i = 0, 4 - 1 do
      demo.ShowPlaceholderObject('Object', obj_i)
      -- r.ImGui_Separator(ctx)
    end
    r.ImGui_EndTable(ctx)
  end
  r.ImGui_PopStyleVar(ctx)
  r.ImGui_End(ctx)
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

  r.ImGui_SetNextWindowSize(ctx, 520, 600, r.ImGui_Cond_FirstUseEver())
  local rv, open = r.ImGui_Begin(ctx, 'Example: Long text display', true)
  if not rv then
    r.ImGui_End(ctx)
    return open
  end

  r.ImGui_Text(ctx, 'Printing unusually long amount of text.')
  rv,app.long_text.test_type = r.ImGui_Combo(ctx, 'Test type', app.long_text.test_type,
    'Single call to Text()\31\z
     Multiple calls to Text(), clipped\31\z
     Multiple calls to Text(), not clipped (slow)\31')
  r.ImGui_Text(ctx, ('Buffer contents: %d lines, %d bytes'):format(app.long_text.lines, app.long_text.log:len()))
  if r.ImGui_Button(ctx, 'Clear') then app.long_text.log = ''; app.long_text.lines = 0 end
  r.ImGui_SameLine(ctx)
  if r.ImGui_Button(ctx, 'Add 1000 lines') then
    local newLines = ''
    for i = 0, 1000 - 1 do
      newLines = newLines .. ("%i The quick brown fox jumps over the lazy dog\n"):format(app.long_text.lines + i)
    end
    app.long_text.log = app.long_text.log .. newLines
    app.long_text.lines = app.long_text.lines + 1000
  end
  r.ImGui_BeginChild(ctx, 'Log')
  if app.long_text.test_type == 0 then
    -- Single call to TextUnformatted() with a big buffer
    r.ImGui_Text(ctx, app.long_text.log)
  elseif app.long_text.test_type == 1 then
    -- Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGui_ListClipper helper.
    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing(), 0, 0);
    local clipper = r.ImGui_CreateListClipper(ctx);
    r.ImGui_ListClipper_Begin(clipper, app.long_text.lines)
    while r.ImGui_ListClipper_Step(clipper) do
      local display_start, display_end = r.ImGui_ListClipper_GetDisplayRange(clipper)
      for i = display_start, display_end - 1 do
        r.ImGui_Text(ctx, ('%i The quick brown fox jumps over the lazy dog'):format(i))
      end
    end
    r.ImGui_PopStyleVar(ctx)
  elseif app.long_text.test_type == 2 then
    -- Multiple calls to Text(), not clipped (slow)
    r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_ItemSpacing(), 0, 0);
    for i = 0, app.long_text.lines do
      r.ImGui_Text(ctx, ('%i The quick brown fox jumps over the lazy dog'):format(i))
    end
    r.ImGui_PopStyleVar(ctx)
  end
  r.ImGui_EndChild(ctx)
  r.ImGui_End(ctx)
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

  local rv,open = r.ImGui_Begin(ctx, 'Example: Auto-resizing window', true, r.ImGui_WindowFlags_AlwaysAutoResize())
  if not rv then
    r.ImGui_End(ctx)
    return open
  end

  r.ImGui_Text(ctx,
    "Window will resize every-frame to the size of its content.\n\z
     Note that you probably don't want to query the window size to\n\z
     output your content because that would create a feedback loop.")
  rv,app.auto_resize.lines = r.ImGui_SliderInt(ctx, 'Number of lines', app.auto_resize.lines, 1, 20)
  for i = 1, app.auto_resize.lines do
    r.ImGui_Text(ctx, ("%sThis is line %d"):format((' '):rep(i * 4), i)) -- Pad with space to extend size horizontally
  end
  r.ImGui_End(ctx)
  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
-------------------------------------------------------------------------------

-- Demonstrate creating a window with custom resize constraints.
function demo.ShowExampleAppConstrainedResize()
  -- struct CustomConstraints
  -- {
  --   // Helper functions to demonstrate programmatic constraints
  --   static void Square(ImGuiSizeCallbackData* data) { data->DesiredSize.x = data->DesiredSize.y = IM_MAX(data->DesiredSize.x, data->DesiredSize.y); }
  --   static void Step(ImGuiSizeCallbackData* data)   { float step = (float)(int)(intptr_t)data->UserData; data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step); }
  -- };

  if not app.constrained_resize then
    app.constrained_resize = {
      auto_resize   = false,
      type          = 0,
      display_lines = 10,
    }
  end

  if app.constrained_resize.type == 0 then r.ImGui_SetNextWindowSizeConstraints(ctx, -1, 0, -1, FLT_MAX)         end -- Vertical only
  if app.constrained_resize.type == 1 then r.ImGui_SetNextWindowSizeConstraints(ctx, 0, -1, FLT_MAX, -1)         end -- Horizontal only
  if app.constrained_resize.type == 2 then r.ImGui_SetNextWindowSizeConstraints(ctx, 100, 100, FLT_MAX, FLT_MAX) end -- Width > 100, Height > 100
  if app.constrained_resize.type == 3 then r.ImGui_SetNextWindowSizeConstraints(ctx, 400, -1, 500, -1)           end -- Width 400-500
  if app.constrained_resize.type == 4 then r.ImGui_SetNextWindowSizeConstraints(ctx, -1, 400, -1, 500)           end -- Height 400-500
  -- if (type == 5) r.ImGui_SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);                     // Always Square
  -- if (type == 6) r.ImGui_SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)(intptr_t)100); // Fixed Step

  local flags = app.constrained_resize.auto_resize and r.ImGui_WindowFlags_AlwaysAutoResize() or 0
  local rv,open = r.ImGui_Begin(ctx, 'Example: Constrained Resize', true, flags)
  if rv then
    if r.ImGui_Button(ctx, '200x200') then r.ImGui_SetWindowSize(ctx, 200, 200) end r.ImGui_SameLine(ctx)
    if r.ImGui_Button(ctx, '500x500') then r.ImGui_SetWindowSize(ctx, 500, 500) end r.ImGui_SameLine(ctx)
    if r.ImGui_Button(ctx, '800x200') then r.ImGui_SetWindowSize(ctx, 800, 200) end
    r.ImGui_SetNextItemWidth(ctx, 200)
    rv,app.constrained_resize.type = r.ImGui_Combo(ctx, 'Constraint', app.constrained_resize.type,
      "Resize vertical only\31\z
       Resize horizontal only\31\z
       Width > 100, Height > 100\31\z
       Width 400-500\31\z
       Height 400-500\31")
       --Custom: Always Square\31\z
       --Custom: Fixed Steps (100)\31")
    r.ImGui_SetNextItemWidth(ctx, 200)
    rv,app.constrained_resize.display_lines = r.ImGui_DragInt(ctx, 'Lines', app.constrained_resize.display_lines, 0.2, 1, 100)
    rv,app.constrained_resize.auto_resize = r.ImGui_Checkbox(ctx, 'Auto-resize', app.constrained_resize.auto_resize)
    for i = 1, app.constrained_resize.display_lines do
      r.ImGui_Text(ctx, ('%sHello, sailor! Making this line long enough for the example.'):format((' '):rep(i * 4)))
    end
  end
  r.ImGui_End(ctx)
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
      PAD    = 10.0,
      corner = 0,
    }
  end

  local window_flags = r.ImGui_WindowFlags_NoDecoration()       |
                       r.ImGui_WindowFlags_AlwaysAutoResize()   |
                       -- r.ImGui_WindowFlags_NoSavedSettings()    |
                       r.ImGui_WindowFlags_NoFocusOnAppearing() |
                       r.ImGui_WindowFlags_NoNav()

  if app.simple_overlay.corner ~= -1 then
    local viewport = r.ImGui_GetMainViewport(ctx)
    local work_pos = {r.ImGui_Viewport_GetWorkPos(viewport)} -- Use work area to avoid menu-bar/task-bar, if any!
    local work_size = {r.ImGui_Viewport_GetWorkSize(viewport)}
    local window_pos_x, window_pos_y, window_pos_pivot_x, window_pos_pivot_y
    window_pos_x = app.simple_overlay.corner & 1 ~= 0 and work_pos[1] + work_size[1] - app.simple_overlay.PAD or work_pos[1] + app.simple_overlay.PAD
    window_pos_y = app.simple_overlay.corner & 2 ~= 0 and work_pos[2] + work_size[2] - app.simple_overlay.PAD or work_pos[2] + app.simple_overlay.PAD
    window_pos_pivot_x = app.simple_overlay.corner & 1 ~= 0 and 1.0 or 0.0
    window_pos_pivot_y = app.simple_overlay.corner & 2 ~= 0 and 1.0 or 0.0
    r.ImGui_SetNextWindowPos(ctx, window_pos_x, window_pos_y, r.ImGui_Cond_Always(), window_pos_pivot_x, window_pos_pivot_y)
    window_flags = window_flags | r.ImGui_WindowFlags_NoMove()
  end

  r.ImGui_SetNextWindowBgAlpha(ctx, 0.35) -- Transparent background

  local rv,open = r.ImGui_Begin(ctx, 'Example: Simple overlay', true, window_flags)
  if rv then
    r.ImGui_Text(ctx, 'Simple overlay\nin the corner of the screen.\n(right-click to change position)')
    r.ImGui_Separator(ctx)
    if r.ImGui_IsMousePosValid(ctx) then
      r.ImGui_Text(ctx, ('Mouse Position: (%.1f,%.1f)'):format(r.ImGui_GetMousePos(ctx)))
    else
      r.ImGui_Text(ctx, 'Mouse Position: <invalid>')
    end
    if r.ImGui_BeginPopupContextWindow(ctx) then
      if r.ImGui_MenuItem(ctx, 'Custom',       nil, app.simple_overlay.corner == -1) then app.simple_overlay.corner = -1 end
      if r.ImGui_MenuItem(ctx, 'Top-left',     nil, app.simple_overlay.corner ==  0) then app.simple_overlay.corner =  0 end
      if r.ImGui_MenuItem(ctx, 'Top-right',    nil, app.simple_overlay.corner ==  1) then app.simple_overlay.corner =  1 end
      if r.ImGui_MenuItem(ctx, 'Bottom-left',  nil, app.simple_overlay.corner ==  2) then app.simple_overlay.corner =  2 end
      if r.ImGui_MenuItem(ctx, 'Bottom-right', nil, app.simple_overlay.corner ==  3) then app.simple_overlay.corner =  3 end
      if r.ImGui_MenuItem(ctx, 'Close') then open = false end
      r.ImGui_EndPopup(ctx)
    end
  end
  r.ImGui_End(ctx)
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
      flags = r.ImGui_WindowFlags_NoDecoration() | r.ImGui_WindowFlags_NoMove() |
              r.ImGui_WindowFlags_NoResize()-- | r.ImGui_WindowFlags_NoSavedSettings(),
    }
  end

  -- We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
  -- Based on your use case you may want one of the other.
  local viewport = r.ImGui_GetMainViewport(ctx)
  local getViewportPos  = app.fullscreen.use_work_area and r.ImGui_Viewport_GetWorkPos or r.ImGui_Viewport_GetPos
  local getViewportSize = app.fullscreen.use_work_area and r.ImGui_Viewport_GetWorkSize or r.ImGui_Viewport_GetSize
  r.ImGui_SetNextWindowPos(ctx, getViewportPos(viewport))
  r.ImGui_SetNextWindowSize(ctx, getViewportSize(viewport))

  local rv,open = r.ImGui_Begin(ctx, 'Example: Fullscreen window', true, app.fullscreen.flags)
  if rv then
    rv,app.fullscreen.use_work_area = r.ImGui_Checkbox(ctx, 'Use work area instead of main area', app.fullscreen.use_work_area)
    r.ImGui_SameLine(ctx)
    demo.HelpMarker('Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the difference.');

    rv,app.fullscreen.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiWindowFlags_NoBackground', app.fullscreen.flags, r.ImGui_WindowFlags_NoBackground())
    rv,app.fullscreen.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiWindowFlags_NoDecoration', app.fullscreen.flags, r.ImGui_WindowFlags_NoDecoration())
    r.ImGui_Indent(ctx)
    rv,app.fullscreen.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiWindowFlags_NoTitleBar', app.fullscreen.flags, r.ImGui_WindowFlags_NoTitleBar())
    rv,app.fullscreen.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiWindowFlags_NoCollapse', app.fullscreen.flags, r.ImGui_WindowFlags_NoCollapse())
    rv,app.fullscreen.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiWindowFlags_NoScrollbar', app.fullscreen.flags, r.ImGui_WindowFlags_NoScrollbar())
    r.ImGui_Unindent(ctx)

    if r.ImGui_Button(ctx, 'Close this window') then
      open = false
    end
  end
  r.ImGui_End(ctx)
  return open
end

-------------------------------------------------------------------------------
-- [SECTION] Example App: Manipulating window titles / ShowExampleAppWindowTitles()
-------------------------------------------------------------------------------

-- Demonstrate using "##" and "###" in identifiers to manipulate ID generation.
-- This apply to all regular items as well.
-- Read FAQ section "How can I have multiple widgets with the same label?" for details.
function demo.ShowExampleAppWindowTitles()
  local viewport = r.ImGui_GetMainViewport(ctx)
  local base_pos = {r.ImGui_Viewport_GetPos(viewport)}

  -- By default, Windows are uniquely identified by their title.
  -- You can use the "##" and "###" markers to manipulate the display/ID.

  -- Using "##" to display same title but have unique identifier.
  r.ImGui_SetNextWindowPos(ctx, base_pos[1] + 100, base_pos[2] + 100, r.ImGui_Cond_FirstUseEver())
  r.ImGui_Begin(ctx, 'Same title as another window##1')
  r.ImGui_Text(ctx, 'This is window 1.\nMy title is the same as window 2, but my identifier is unique.')
  r.ImGui_End(ctx)

  r.ImGui_SetNextWindowPos(ctx, base_pos[1] + 100, base_pos[2] + 200, r.ImGui_Cond_FirstUseEver())
  r.ImGui_Begin(ctx, 'Same title as another window##2')
  r.ImGui_Text(ctx, 'This is window 2.\nMy title is the same as window 1, but my identifier is unique.')
  r.ImGui_End(ctx)

  -- Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
  r.ImGui_SetNextWindowPos(ctx, base_pos[1] + 100, base_pos[2] + 300, r.ImGui_Cond_FirstUseEver())
  spinners = {'|', '/', '-', '\\'}
  local spinner = math.floor(r.ImGui_GetTime(ctx) / 0.25) & 3
  r.ImGui_Begin(ctx, ("Animated title %s %d###AnimatedTitle"):format(spinners[spinner+1], r.ImGui_GetFrameCount(ctx)))
  r.ImGui_Text(ctx, "This window has a changing title.")
  r.ImGui_End(ctx)
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

  local rv,open = r.ImGui_Begin(ctx, 'Example: Custom rendering', true)
  if not rv then
    r.ImGui_End(ctx)
    return open
  end

  if r.ImGui_BeginTabBar(ctx, '##TabBar') then
    if r.ImGui_BeginTabItem(ctx, 'Primitives') then
      r.ImGui_PushItemWidth(ctx, -r.ImGui_GetFontSize(ctx) * 15)
      local draw_list = r.ImGui_GetWindowDrawList(ctx)

      -- Draw gradients
      -- (note that those are currently exacerbating our sRGB/Linear issues)
      -- Calling r.ImGui_GetColor[Ex]() multiplies the given colors by the current Style Alpha
      r.ImGui_Text(ctx, 'Gradients')
      local gradient_size = {r.ImGui_CalcItemWidth(ctx), r.ImGui_GetFrameHeight(ctx)}

      local p0 = {r.ImGui_GetCursorScreenPos(ctx)}
      local p1 = {p0[1] + gradient_size[1], p0[2] + gradient_size[2]}
      local col_a = r.ImGui_GetColorEx(ctx, 0x000000FF)
      local col_b = r.ImGui_GetColorEx(ctx, 0xFFFFFFFF)
      r.ImGui_DrawList_AddRectFilledMultiColor(draw_list, p0[1], p0[2], p1[1], p1[2], col_a, col_b, col_b, col_a)
      r.ImGui_InvisibleButton(ctx, '##gradient1', gradient_size[1], gradient_size[2])

      local p0 = {r.ImGui_GetCursorScreenPos(ctx)}
      local p1 = {p0[1] + gradient_size[1], p0[2] + gradient_size[2]}
      local col_a = r.ImGui_GetColorEx(ctx, 0x00FF00FF)
      local col_b = r.ImGui_GetColorEx(ctx, 0xFF0000FF)
      r.ImGui_DrawList_AddRectFilledMultiColor(draw_list, p0[1], p0[2], p1[1], p1[2], col_a, col_b, col_b, col_a)
      r.ImGui_InvisibleButton(ctx, '##gradient2', gradient_size[1], gradient_size[2])

      -- Draw a bunch of primitives
      local item_inner_spacing_x = r.ImGui_GetStyleVar(ctx, r.ImGui_StyleVar_ItemInnerSpacing())
      r.ImGui_Text(ctx, 'All primitives')
      rv,app.rendering.sz = r.ImGui_DragDouble(ctx, 'Size', app.rendering.sz, 0.2, 2.0, 100.0, '%.0f')
      rv,app.rendering.thickness = r.ImGui_DragDouble(ctx, 'Thickness', app.rendering.thickness, 0.05, 1.0, 8.0, '%.02f')
      rv,app.rendering.ngon_sides = r.ImGui_SliderInt(ctx, 'N-gon sides', app.rendering.ngon_sides, 3, 12)
      rv,app.rendering.circle_segments_override = r.ImGui_Checkbox(ctx, '##circlesegmentoverride', app.rendering.circle_segments_override)
      r.ImGui_SameLine(ctx, 0.0, item_inner_spacing_x)
      rv,app.rendering.circle_segments_override_v = r.ImGui_SliderInt(ctx, 'Circle segments override', app.rendering.circle_segments_override_v, 3, 40)
      if rv then app.rendering.circle_segments_override = true end
      rv,app.rendering.curve_segments_override = r.ImGui_Checkbox(ctx, '##curvessegmentoverride', app.rendering.curve_segments_override)
      r.ImGui_SameLine(ctx, 0.0, item_inner_spacing_x)
      rv,app.rendering.curve_segments_override_v = r.ImGui_SliderInt(ctx, 'Curves segments override', app.rendering.curve_segments_override_v, 3, 40)
      if rv then app.rendering.curve_segments_override = true end
      rv,app.rendering.col = r.ImGui_ColorEdit4(ctx, 'Color', app.rendering.col)

      local p = {r.ImGui_GetCursorScreenPos(ctx)}
      local spacing = 10.0
      local corners_tl_br = r.ImGui_DrawFlags_RoundCornersTopLeft() | r.ImGui_DrawFlags_RoundCornersBottomRight()
      local col = app.rendering.col
      local sz = app.rendering.sz
      local rounding = sz / 5.0
      local circle_segments = app.rendering.circle_segments_override and app.rendering.circle_segments_override_v or 0
      local curve_segments = app.rendering.curve_segments_override and app.rendering.curve_segments_override_v or 0
      local x = p[1] + 4.0
      local y = p[2] + 4.0
      for n = 1, 2 do
        -- First line uses a thickness of 1.0, second line uses the configurable thickness
        local th = n == 1 and 1.0 or app.rendering.thickness
        r.ImGui_DrawList_AddNgon(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, app.rendering.ngon_sides, th); x = x + sz + spacing  -- N-gon
        r.ImGui_DrawList_AddCircle(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, circle_segments, th);        x = x + sz + spacing  -- Circle
        r.ImGui_DrawList_AddRect(draw_list, x, y, x + sz, y + sz, col, 0.0, r.ImGui_DrawFlags_None(), th);      x = x + sz + spacing  -- Square
        r.ImGui_DrawList_AddRect(draw_list, x, y, x + sz, y + sz, col, rounding, r.ImGui_DrawFlags_None(), th); x = x + sz + spacing  -- Square with all rounded corners
        r.ImGui_DrawList_AddRect(draw_list, x, y, x + sz, y + sz, col, rounding, corners_tl_br, th);            x = x + sz + spacing  -- Square with two rounded corners
        r.ImGui_DrawList_AddTriangle(draw_list, x+sz*0.5, y, x+sz, y+sz-0.5, x, y+sz-0.5, col, th);             x = x + sz + spacing  -- Triangle
        -- r.ImGui_DrawList_AddTriangle(draw_list, x+sz*0.2, y, x, y+sz-0.5, x+sz*0.4, y+sz-0.5, col, th);      x = x + sz*0.4 + spacing -- Thin triangle
        r.ImGui_DrawList_AddLine(draw_list, x, y, x + sz, y, col, th);                                          x = x + sz + spacing  -- Horizontal line (note: drawing a filled rectangle will be faster!)
        r.ImGui_DrawList_AddLine(draw_list, x, y, x, y + sz, col, th);                                          x = x +      spacing  -- Vertical line (note: drawing a filled rectangle will be faster!)
        r.ImGui_DrawList_AddLine(draw_list, x, y, x + sz, y + sz, col, th);                                     x = x + sz + spacing  -- Diagonal line

        -- Quadratic Bezier Curve (3 control points)
        local cp3 = {{x, y + sz * 0.6}, {x + sz * 0.5, y - sz * 0.4}, {x + sz, y + sz}}
        r.ImGui_DrawList_AddBezierQuadratic(draw_list,
          cp3[1][1], cp3[1][2], cp3[2][1], cp3[2][2], cp3[3][1], cp3[3][2],
          col, th, curve_segments)
        x = x + sz + spacing

        -- Cubic Bezier Curve (4 control points)
        local cp4 = {{x, y}, {x + sz * 1.3, y + sz * 0.3}, {x + sz - sz * 1.3, y + sz - sz * 0.3}, {x + sz, y + sz}}
        r.ImGui_DrawList_AddBezierCubic(draw_list,
          cp4[1][1], cp4[1][2], cp4[2][1], cp4[2][2], cp4[3][1], cp4[3][2], cp4[4][1], cp4[4][2],
          col, th, curve_segments)

        x = p[1] + 4
        y = y + sz + spacing
      end
      r.ImGui_DrawList_AddNgonFilled(draw_list, x + sz * 0.5, y + sz * 0.5, sz*0.5, col, app.rendering.ngon_sides); x = x + sz + spacing  -- N-gon
      r.ImGui_DrawList_AddCircleFilled(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, circle_segments);            x = x + sz + spacing  -- Circle
      r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, col);                                         x = x + sz + spacing  -- Square
      r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, col, 10.0);                                   x = x + sz + spacing  -- Square with all rounded corners
      r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + sz, y + sz, col, 10.0, corners_tl_br);                    x = x + sz + spacing  -- Square with two rounded corners
      r.ImGui_DrawList_AddTriangleFilled(draw_list, x+sz*0.5, y, x+sz, y+sz-0.5, x, y+sz-0.5, col);                 x = x + sz + spacing  -- Triangle
      -- r.ImGui_DrawList_AddTriangleFilled(draw_list, x+sz*0.2, y, x, y+sz-0.5, x+sz*0.4, y+sz-0.5, col);          x = x + sz*0.4 + spacing -- Thin triangle
      r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + sz, y + app.rendering.thickness, col);                    x = x + sz + spacing  -- Horizontal line (faster than AddLine, but only handle integer thickness)
      r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + app.rendering.thickness, y + sz, col);                    x = x + spacing * 2.0 -- Vertical line (faster than AddLine, but only handle integer thickness)
      r.ImGui_DrawList_AddRectFilled(draw_list, x, y, x + 1, y + 1, col);                                           x = x + sz            -- Pixel (faster than AddLine)
      r.ImGui_DrawList_AddRectFilledMultiColor(draw_list, x, y, x + sz, y + sz, 0x000000ff, 0xff0000ff, 0xffff00ff, 0x00ff00ff)

      r.ImGui_Dummy(ctx, (sz + spacing) * 10.2, (sz + spacing) * 3.0)
      r.ImGui_PopItemWidth(ctx)
      r.ImGui_EndTabItem(ctx)
    end

    if r.ImGui_BeginTabItem(ctx, 'Canvas') then
      rv,app.rendering.opt_enable_grid =
        r.ImGui_Checkbox(ctx, 'Enable grid', app.rendering.opt_enable_grid)
      rv,app.rendering.opt_enable_context_menu =
        r.ImGui_Checkbox(ctx, 'Enable context menu', app.rendering.opt_enable_context_menu)
      r.ImGui_Text(ctx, 'Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.')

      -- Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
      -- Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
      -- To use a child window instead we could use, e.g:
      --      r.ImGui_PushStyleVar(ctx, r.ImGui_StyleVar_WindowPadding(), 0, 0) -- Disable padding
      --      r.ImGui_PushStyleColor(ctx, r.ImGui_Col_ChildBg(), 0x323232ff)    -- Set a background color
      --      r.ImGui_BeginChild(ctx, 'canvas', 0.0, 0.0, true, r.ImGui_WindowFlags_NoMove())
      --      r.ImGui_PopStyleColor(ctx)
      --      r.ImGui_PopStyleVar(ctx)
      --      [...]
      --      r.ImGui_EndChild(ctx)

      -- Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
      local canvas_p0 = {r.ImGui_GetCursorScreenPos(ctx)}      -- ImDrawList API uses screen coordinates!
      local canvas_sz = {r.ImGui_GetContentRegionAvail(ctx)}   -- Resize canvas to what's available
      if canvas_sz[1] < 50.0 then canvas_sz[1] = 50.0 end
      if canvas_sz[2] < 50.0 then canvas_sz[2] = 50.0 end
      local canvas_p1 = {canvas_p0[1] + canvas_sz[1], canvas_p0[2] + canvas_sz[2]}

      -- Draw border and background color
      local mouse_pos = {r.ImGui_GetMousePos(ctx)}
      local draw_list = r.ImGui_GetWindowDrawList(ctx)
      r.ImGui_DrawList_AddRectFilled(draw_list, canvas_p0[1], canvas_p0[2], canvas_p1[1], canvas_p1[2], 0x323232ff)
      r.ImGui_DrawList_AddRect(draw_list, canvas_p0[1], canvas_p0[2], canvas_p1[1], canvas_p1[2], 0xffffffff)

      -- This will catch our interactions
      r.ImGui_InvisibleButton(ctx, 'canvas', canvas_sz[1], canvas_sz[2], r.ImGui_ButtonFlags_MouseButtonLeft() | r.ImGui_ButtonFlags_MouseButtonRight())
      local is_hovered = r.ImGui_IsItemHovered(ctx) -- Hovered
      local is_active = r.ImGui_IsItemActive(ctx)   -- Held
      local origin = {canvas_p0[1] + app.rendering.scrolling[1], canvas_p0[2] + app.rendering.scrolling[2]} -- Lock scrolled origin
      local mouse_pos_in_canvas = {mouse_pos[1] - origin[1], mouse_pos[2] - origin[2]}

      -- Add first and second point
      if is_hovered and not app.rendering.adding_line and r.ImGui_IsMouseClicked(ctx, r.ImGui_MouseButton_Left()) then
        table.insert(app.rendering.points, mouse_pos_in_canvas)
        table.insert(app.rendering.points, mouse_pos_in_canvas)
        app.rendering.adding_line = true
      end
      if app.rendering.adding_line then
        app.rendering.points[#app.rendering.points] = mouse_pos_in_canvas
        if not r.ImGui_IsMouseDown(ctx, r.ImGui_MouseButton_Left()) then
          app.rendering.adding_line = false
        end
      end

      -- Pan (we use a zero mouse threshold when there's no context menu)
      -- You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
      local mouse_threshold_for_pan = app.rendering.opt_enable_context_menu and -1.0 or 0.0
      if is_active and r.ImGui_IsMouseDragging(ctx, r.ImGui_MouseButton_Right(), mouse_threshold_for_pan) then
        local mouse_delta = {r.ImGui_GetMouseDelta(ctx)}
        app.rendering.scrolling[1] = app.rendering.scrolling[1] + mouse_delta[1]
        app.rendering.scrolling[2] = app.rendering.scrolling[2] + mouse_delta[2]
      end

      local removeLastLine = function()
        table.remove(app.rendering.points)
        table.remove(app.rendering.points)
      end

      -- Context menu (under default mouse threshold)
      local drag_delta = {r.ImGui_GetMouseDragDelta(ctx, 0, 0, r.ImGui_MouseButton_Right())}
      if app.rendering.opt_enable_context_menu and r.ImGui_IsMouseReleased(ctx, r.ImGui_MouseButton_Right()) and drag_delta[1] == 0.0 and drag_delta[2] == 0.0 then
        r.ImGui_OpenPopupOnItemClick(ctx, 'context')
      end
      if r.ImGui_BeginPopup(ctx, 'context') then
        if app.rendering.adding_line then
          removeLastLine()
          app.rendering.adding_line = false
        end
        if r.ImGui_MenuItem(ctx, 'Remove one', nil, false, #app.rendering.points > 0) then removeLastLine() end
        if r.ImGui_MenuItem(ctx, 'Remove all', nil, false, #app.rendering.points > 0) then app.rendering.points = {} end
        r.ImGui_EndPopup(ctx)
      end

      -- Draw grid + all lines in the canvas
      r.ImGui_DrawList_PushClipRect(draw_list, canvas_p0[1], canvas_p0[2], canvas_p1[1], canvas_p1[2], true)
      if app.rendering.opt_enable_grid then
        local GRID_STEP = 64.0
        local x = math.fmod(app.rendering.scrolling[1], GRID_STEP)
        while x < canvas_sz[1] do
          r.ImGui_DrawList_AddLine(draw_list, canvas_p0[1] + x, canvas_p0[2], canvas_p0[1] + x, canvas_p1[2], 0xc8c8c828)
          x = x + GRID_STEP
        end
        local y = math.fmod(app.rendering.scrolling[2], GRID_STEP)
        while y < canvas_sz[2] do
          r.ImGui_DrawList_AddLine(draw_list, canvas_p0[1], canvas_p0[2] + y, canvas_p1[1], canvas_p0[2] + y, 0xc8c8c828)
          y = y + GRID_STEP
        end
      end
      local n = 1
      while n < #app.rendering.points do
        r.ImGui_DrawList_AddLine(draw_list,
          origin[1] + app.rendering.points[n][1], origin[2] + app.rendering.points[n][2],
          origin[1] + app.rendering.points[n + 1][1], origin[2] + app.rendering.points[n + 1][2],
          0xffff00ff, 2.0)
        n = n + 2
      end
      r.ImGui_DrawList_PopClipRect(draw_list)

      r.ImGui_EndTabItem(ctx)
    end

    if r.ImGui_BeginTabItem(ctx, 'BG/FG draw lists') then
      rv,app.rendering.draw_bg = r.ImGui_Checkbox(ctx, 'Draw in Background draw list', app.rendering.draw_bg)
      r.ImGui_SameLine(ctx); demo.HelpMarker('The Background draw list will be rendered below every Dear ImGui windows.')
      rv,app.rendering.draw_fg = r.ImGui_Checkbox(ctx, 'Draw in Foreground draw list', app.rendering.draw_fg)
      r.ImGui_SameLine(ctx); demo.HelpMarker('The Foreground draw list will be rendered over every Dear ImGui windows.')
      local window_pos = {r.ImGui_GetWindowPos(ctx)}
      local window_size = {r.ImGui_GetWindowSize(ctx)}
      local window_center = {window_pos[1] + window_size[1] * 0.5, window_pos[2] + window_size[2] * 0.5}
      if app.rendering.draw_bg then
        r.ImGui_DrawList_AddCircle(r.ImGui_GetBackgroundDrawList(ctx),
          window_center[1], window_center[2], window_size[1] * 0.6,
          0xFF0000c8, nil, 10 + 4)
      end
      if app.rendering.draw_fg then
        r.ImGui_DrawList_AddCircle(r.ImGui_GetForegroundDrawList(ctx),
          window_center[1], window_center[2], window_size[2] * 0.6,
          0x00FF00c8, nil, 10)
      end
      r.ImGui_EndTabItem(ctx)
    end

    r.ImGui_EndTabBar(ctx)
  end

  r.ImGui_End(ctx)
  return open
end

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
--         r.ImGui_PushID(doc);
--         r.ImGui_Text("Document \"%s\"", doc->Name);
--         r.ImGui_PushStyleColor(ImGuiCol_Text, doc->Color);
--         r.ImGui_TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
--         r.ImGui_PopStyleColor();
--         if (r.ImGui_Button("Modify", ImVec2(100, 0)))
--             doc->Dirty = true;
--         r.ImGui_SameLine();
--         if (r.ImGui_Button("Save", ImVec2(100, 0)))
--             doc->DoSave();
--         r.ImGui_ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
--         r.ImGui_PopID();
--     }
--
--     // Display context menu for the Document
--     static void DisplayContextMenu(MyDocument* doc)
--     {
--         if (!r.ImGui_BeginPopupContextItem())
--             return;
--
--         char buf[256];
--         sprintf(buf, "Save %s", doc->Name);
--         if (r.ImGui_MenuItem(buf, "CTRL+S", false, doc->Open))
--             doc->DoSave();
--         if (r.ImGui_MenuItem("Close", "CTRL+W", false, doc->Open))
--             doc->DoQueueClose();
--         r.ImGui_EndPopup();
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
--             r.ImGui_SetTabItemClosed(doc->Name);
--         doc->OpenPrev = doc->Open;
--     }
-- }
--
-- void ShowExampleAppDocuments(bool* p_open)
-- {
--     static ExampleAppDocuments app;
--
--     // Options
--     static bool opt_reorderable = true;
--     static ImGuiTabBarFlags opt_fitting_flags = ImGuiTabBarFlags_FittingPolicyDefault_;
--
--     bool window_contents_visible = r.ImGui_Begin("Example: Documents", p_open, ImGuiWindowFlags_MenuBar);
--     if (!window_contents_visible)
--     {
--         r.ImGui_End();
--         return;
--     }
--
--     // Menu
--     if (r.ImGui_BeginMenuBar())
--     {
--         if (r.ImGui_BeginMenu("File"))
--         {
--             int open_count = 0;
--             for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--                 open_count += app.Documents[doc_n].Open ? 1 : 0;
--
--             if (r.ImGui_BeginMenu("Open", open_count < app.Documents.Size))
--             {
--                 for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--                 {
--                     MyDocument* doc = &app.Documents[doc_n];
--                     if (!doc->Open)
--                         if (r.ImGui_MenuItem(doc->Name))
--                             doc->DoOpen();
--                 }
--                 r.ImGui_EndMenu();
--             }
--             if (r.ImGui_MenuItem("Close All Documents", NULL, false, open_count > 0))
--                 for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--                     app.Documents[doc_n].DoQueueClose();
--             if (r.ImGui_MenuItem("Exit", "Alt+F4")) {}
--             r.ImGui_EndMenu();
--         }
--         r.ImGui_EndMenuBar();
--     }
--
--     // [Debug] List documents with one checkbox for each
--     for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--     {
--         MyDocument* doc = &app.Documents[doc_n];
--         if (doc_n > 0)
--             r.ImGui_SameLine();
--         r.ImGui_PushID(doc);
--         if (r.ImGui_Checkbox(doc->Name, &doc->Open))
--             if (!doc->Open)
--                 doc->DoForceClose();
--         r.ImGui_PopID();
--     }
--
--     r.ImGui_Separator();
--
--     // Submit Tab Bar and Tabs
--     {
--         ImGuiTabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? ImGuiTabBarFlags_Reorderable : 0);
--         if (r.ImGui_BeginTabBar("##tabs", tab_bar_flags))
--         {
--             if (opt_reorderable)
--                 NotifyOfDocumentsClosedElsewhere(app);
--
--             // [DEBUG] Stress tests
--             //if ((r.ImGui_GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
--             //if (r.ImGui_GetIO().KeyCtrl) r.ImGui_SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..
--
--             // Submit Tabs
--             for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
--             {
--                 MyDocument* doc = &app.Documents[doc_n];
--                 if (!doc->Open)
--                     continue;
--
--                 ImGuiTabItemFlags tab_flags = (doc->Dirty ? ImGuiTabItemFlags_UnsavedDocument : 0);
--                 bool visible = r.ImGui_BeginTabItem(doc->Name, &doc->Open, tab_flags);
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
--                     r.ImGui_EndTabItem();
--                 }
--             }
--
--             r.ImGui_EndTabBar();
--         }
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
--             if (!r.ImGui_IsPopupOpen("Save?"))
--                 r.ImGui_OpenPopup("Save?");
--             if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
--             {
--                 r.ImGui_Text("Save change to the following items?");
--                 float item_height = ImGui::GetTextLineHeightWithSpacing();
--                 if (ImGui::BeginChildFrame(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height)))
--                 {
--                     for (int n = 0; n < close_queue.Size; n++)
--                         if (close_queue[n]->Dirty)
--                             r.ImGui_Text("%s", close_queue[n]->Name);
--                     ImGui::EndChildFrame();
--                 }
--
--                 ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
--                 if (ImGui::Button("Yes", button_size))
--                 {
--                     for (int n = 0; n < close_queue.Size; n++)
--                     {
--                         if (close_queue[n]->Dirty)
--                             close_queue[n]->DoSave();
--                         close_queue[n]->DoForceClose();
--                     }
--                     close_queue.clear();
--                     r.ImGui_CloseCurrentPopup();
--                 }
--                 r.ImGui_SameLine();
--                 if (ImGui::Button("No", button_size))
--                 {
--                     for (int n = 0; n < close_queue.Size; n++)
--                         close_queue[n]->DoForceClose();
--                     close_queue.clear();
--                     r.ImGui_CloseCurrentPopup();
--                 }
--                 r.ImGui_SameLine();
--                 if (ImGui::Button("Cancel", button_size))
--                 {
--                     close_queue.clear();
--                     r.ImGui_CloseCurrentPopup();
--                 }
--                 r.ImGui_EndPopup();
--             }
--         }
--     }
--
--     r.ImGui_End();
-- }

reaper.defer(demo.loop)
