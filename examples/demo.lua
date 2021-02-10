-- Lua/ReaImGui port of Dear ImGui's C++ demo code (v1.81)

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
// [SECTION] Style Editor / ShowStyleEditor()
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

--[[
// Play it nice with Windows users (Update: May 2018, Notepad now supports Unix-style carriage returns!)
#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif
--]]

demo = { open = true }

local r = reaper
local ctx = r.ImGui_CreateContext('ImGui Demo', 300, 300, 590, 740)

function demo.loop()
  if r.ImGui_IsCloseRequested(ctx) then
    r.ImGui_DestroyContext(ctx)
    return
  end

  if demo.open then
    demo.open = demo.ShowDemoWindow(demo.open)
  end

  -- UpdateContext() must be called at least once per window per timer cycle
  r.ImGui_UpdateContext(ctx)

  r.defer(demo.loop)
end

function demo.clamp(v, mn, mx)
  if v < mn then return mn end
  if v > mx then return mx end
  return v
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
  -- metrics      = false
  -- style_editor = false
  -- about        = false

  -- Window flags (accessible from the "Configuration" section)
  no_titlebar = false,
  no_scrollbar = false,
  no_menu = false,
  no_move = false,
  no_resize = false,
  no_collapse = false,
  no_close = false,
  no_nav = false,
  no_background = false,
  no_bring_to_front = false,
}

-- Demonstrate most Dear ImGui features (this is big function!)
-- You may execute this function to experiment with the UI and understand what it does.
-- You may then search for keywords in the code when you are interested by a specific feature.
function demo.ShowDemoWindow(open)
  local rv

  if show_app.main_menu_bar      then show_app.main_menu_bar      = ShowExampleAppMainMenuBar()       end
  if show_app.documents          then show_app.documents          = ShowExampleAppDocuments()         end
  if show_app.console            then show_app.console            = ShowExampleAppConsole()           end
  if show_app.log                then show_app.log                = ShowExampleAppLog()               end
  if show_app.layout             then show_app.layout             = ShowExampleAppLayout()            end
  if show_app.property_editor    then show_app.property_editor    = ShowExampleAppPropertyEditor()    end
  if show_app.long_text          then show_app.long_text          = ShowExampleAppLongText()          end
  if show_app.auto_resize        then show_app.auto_resize        = ShowExampleAppAutoResize()        end
  if show_app.constrained_resize then show_app.constrained_resize = ShowExampleAppConstrainedResize() end
  if show_app.simple_overlay     then show_app.simple_overlay     = ShowExampleAppSimpleOverlay()     end
  if show_app.fullscreen         then show_app.fullscreen         = ShowExampleAppFullscreen()        end
  if show_app.window_titles      then show_app.window_titles      = ShowExampleAppWindowTitles()      end
  if show_app.custom_rendering   then show_app.custom_rendering   = ShowExampleAppCustomRendering()   end

  -- if (show_app_metrics)       { r.ImGui_ShowMetricsWindow(&show_app_metrics); }
  -- if (show_app_about)         { r.ImGui_ShowAboutWindow(&show_app_about); }
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
  local p_open, new_open = open
  if demo.no_close          then p_open = nil end -- Don't pass our bool* to Begin

  -- We specify a default position/size in case there's no data in the .ini file.
  -- We only do it to make the demo applications a little more welcoming, but typically this isn't required.
  -- v1.81 TODO const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
  -- ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
  r.ImGui_SetNextWindowPos(ctx, 20, 20, r.ImGui_Cond_FirstUseEver());
  r.ImGui_SetNextWindowSize(ctx, 550, 680, r.ImGui_Cond_FirstUseEver());

  -- Main body of the Demo window starts here.
  rv, new_open = r.ImGui_Begin(ctx, 'Dear ImGui Demo', p_open, window_flags)
  if not demo.no_close then open = new_open end
  if not rv then
    -- Early out if the window is collapsed, as an optimization.
    r.ImGui_End(ctx)
    return open
  end

  -- Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.

  -- e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)
  --r.ImGui_PushItemWidth(-r.ImGui_GetWindowWidth() * 0.35f);

  -- TODO this one
  -- e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
  --r.ImGui_PushItemWidth(r.ImGui_GetFontSize() * -12);

  -- Menu Bar
  if r.ImGui_BeginMenuBar(ctx) then
    if r.ImGui_BeginMenu(ctx, 'Menu') then
      --ShowExampleMenuFile() TODO
      r.ImGui_EndMenu(ctx)
    end
    if r.ImGui_BeginMenu(ctx, 'Examples') then
      rv,show_app.main_menu_bar =
        r.ImGui_MenuItem(ctx, 'Main menu bar', nil, show_app.main_menu_bar)
      rv,show_app.console =
        r.ImGui_MenuItem(ctx, 'Console', nil, show_app.console)
      rv,show_app.log =
        r.ImGui_MenuItem(ctx, 'Log', nil, show_app.log)
      rv,show_app.layout =
        r.ImGui_MenuItem(ctx, 'Simple layout',nil, show_app.layout)
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
        r.ImGui_MenuItem(ctx, 'Documents', nil, show_app.documents)
      r.ImGui_EndMenu(ctx)
    end
    if r.ImGui_BeginMenu(ctx, 'Tools') then
      rv,show_app.metrics =
        r.ImGui_MenuItem(ctx, 'Metrics/Debugger', nil, show_app.metrics)
      rv,show_app.style_editor =
        r.ImGui_MenuItem(ctx, 'Style Editor', nil, show_app.style_editor)
      rv,show_app.about =
        r.ImGui_MenuItem(ctx, 'About Dear ImGui', nil, show_app.about)
      r.ImGui_EndMenu(ctx)
    end
    r.ImGui_EndMenuBar(ctx)
  end

  local IMGUI_VERSION = 'FOOBAR' -- TODO
  r.ImGui_Text(ctx, ('dear imgui says hello. (%s)'):format(IMGUI_VERSION));
  r.ImGui_Spacing(ctx);

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

--     if (r.ImGui_CollapsingHeader("Configuration"))
--     {
--         ImGuiIO& io = r.ImGui_GetIO();
--
--         if (r.ImGui_TreeNode("Configuration##2"))
--         {
--             r.ImGui_CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",    &io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
--             r.ImGui_SameLine(); HelpMarker("Enable keyboard controls.");
--             r.ImGui_CheckboxFlags("io.ConfigFlags: NavEnableGamepad",     &io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
--             r.ImGui_SameLine(); HelpMarker("Enable gamepad controls. Require backend to set io.BackendFlags |= ImGuiBackendFlags_HasGamepad.\n\nRead instructions in imgui.cpp for details.");
--             r.ImGui_CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos", &io.ConfigFlags, ImGuiConfigFlags_NavEnableSetMousePos);
--             r.ImGui_SameLine(); HelpMarker("Instruct navigation to move the mouse cursor. See comment for ImGuiConfigFlags_NavEnableSetMousePos.");
--             r.ImGui_CheckboxFlags("io.ConfigFlags: NoMouse",              &io.ConfigFlags, ImGuiConfigFlags_NoMouse);
--             if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)
--             {
--                 // The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to fix it:
--                 if (fmodf((float)r.ImGui_GetTime(), 0.40f) < 0.20f)
--                 {
--                     r.ImGui_SameLine();
--                     r.ImGui_Text("<<PRESS SPACE TO DISABLE>>");
--                 }
--                 if (r.ImGui_IsKeyPressed(r.ImGui_GetKeyIndex(ImGuiKey_Space)))
--                     io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
--             }
--             r.ImGui_CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", &io.ConfigFlags, ImGuiConfigFlags_NoMouseCursorChange);
--             r.ImGui_SameLine(); HelpMarker("Instruct backend to not alter mouse cursor shape and visibility.");
--             r.ImGui_Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
--             r.ImGui_SameLine(); HelpMarker("Enable blinking cursor (optional as some users consider it to be distracting)");
--             r.ImGui_Checkbox("io.ConfigDragClickToInputText", &io.ConfigDragClickToInputText);
--             r.ImGui_SameLine(); HelpMarker("Enable turning DragXXX widgets into text input with a simple mouse click-release (without moving).");
--             r.ImGui_Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
--             r.ImGui_SameLine(); HelpMarker("Enable resizing of windows from their edges and from the lower-left corner.\nThis requires (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback.");
--             r.ImGui_Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
--             r.ImGui_Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
--             r.ImGui_SameLine(); HelpMarker("Instruct Dear ImGui to render a mouse cursor itself. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");
--             r.ImGui_Text("Also see Style->Rendering for rendering options.");
--             r.ImGui_TreePop();
--             r.ImGui_Separator();
--         }
--
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
--
--         if (r.ImGui_TreeNode("Capture/Logging"))
--         {
--             HelpMarker(
--                 "The logging API redirects all text output so you can easily capture the content of "
--                 "a window or a block. Tree nodes can be automatically expanded.\n"
--                 "Try opening any of the contents below in this window and then click one of the \"Log To\" button.");
--             r.ImGui_LogButtons();
--
--             HelpMarker("You can also call r.ImGui_LogText() to output directly to the log without a visual output.");
--             if (r.ImGui_Button("Copy \"Hello, world!\" to clipboard"))
--             {
--                 r.ImGui_LogToClipboard();
--                 r.ImGui_LogText("Hello, world!");
--                 r.ImGui_LogFinish();
--             }
--             r.ImGui_TreePop();
--         }
--     }

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
--     ShowDemoWindowLayout();
--     ShowDemoWindowPopups();
--     ShowDemoWindowTables();
--     ShowDemoWindowMisc();
--
--     // End of ShowDemoWindow()
--     r.ImGui_PopItemWidth();
  r.ImGui_End(ctx)
  return open
end

widgets = {
  basic = {
    clicked = 0,
    check   = true,
    radio   = 0,
    counter = 0,
    curitem = 0,
    str0    = 'Hello, world!',
    str1    = '',
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
    elem    = 1,
    col1    = 0xff0033,  -- 0xRRGGBB
    col2    = 0x66b2007f,-- 0xRRGGBBAA
    listcur = 0,
  },
  trees = {
    base_flags = r.ImGui_TreeNodeFlags_OpenOnArrow() |
                 r.ImGui_TreeNodeFlags_OpenOnDoubleClick() |
                 r.ImGui_TreeNodeFlags_SpanAvailWidth(),
    align_label_with_current_x_position = false,
    test_drag_and_drop = false,
    selection_mask = 1 << 2,
  },
  cheads = {
    closable_group = true,
  },
  text = {
    wrap_width = 200.0,
    utf8 = '日本語',
  },
  combos = {
    flags = r.ImGui_ComboFlags_None(),
    current_item1 = 1,
    current_item2 = 0,
    current_item3 = -1,
  },
  selectables = {
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
  },
  text = {
    multiline = {
      text = [[/*
 The Pentium F00F bug, shorthand for F0 0F C7 C8,
 the hexadecimal encoding of one offending instruction,
 more formally, the invalid operand with locked CMPXCHG8B
 instruction bug, is a design flaw in the majority of
 Intel Pentium, Pentium MMX, and Pentium OverDrive
 processors (all in the P5 microarchitecture).
*/\n\n"
label:\n"
	lock cmpxchg8b eax
]],
    },
    flags = r.ImGui_InputTextFlags_AllowTabInput(),
    buf = { '', '', '', '', '' },
    password = 'hunter2',
  },
  tabs = {
    flags1  = r.ImGui_TabBarFlags_Reorderable(),
    opened  = { true, true, true, true },
    flags2  = r.ImGui_TabBarFlags_AutoSelectNewTabs() |
              r.ImGui_TabBarFlags_Reorderable() |
              r.ImGui_TabBarFlags_FittingPolicyResizeDown(),
    active  = { 1, 2, 3 },
    next_id = 4,
    show_leading_button  = true,
    show_trailing_button = true,
  },
  plots = {
    animate = true,
    plot1 = {
      offset       = 1,
      refresh_time = 0.0,
      phase        = 0.0,
    },
    plot2 = {
      func = 0,
      size = 70,
      fill = true,
    },
    progress     = 0.0,
    progress_dir = 1,
  },
}

local tooltip_curve = reaper.new_array({ 0.6, 0.1, 1.0, 0.5, 0.92, 0.1, 0.2 })
local vec4a         = reaper.new_array({ 0.10, 0.20, 0.30, 0.44 })
local frame_times   = reaper.new_array({ 0.6, 0.1, 1.0, 0.5, 0.92, 0.1, 0.2 })
local PLOT1_SIZE    = 90
local plot1         = reaper.new_array(PLOT1_SIZE)
plot1.clear()
local plot2         = reaper.new_array(1)
local plot2_funcs   = {
  function(i) return math.sin(i * 0.1) end, -- sin
  function(i) return (i & 1) == 1 and 1.0 or -1.0 end, --saw
}

function demo.ShowDemoWindowWidgets()
  if not r.ImGui_CollapsingHeader(ctx, 'Widgets') then
    return
  end

  local rv

  if r.ImGui_TreeNode(ctx, 'Basic') then
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

    -- TODO
--         // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
--         for (int i = 0; i < 7; i++)
--         {
--             if (i > 0)
--                 r.ImGui_SameLine();
--             r.ImGui_PushID(i);
--             r.ImGui_PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
--             r.ImGui_PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
--             r.ImGui_PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
--             r.ImGui_Button("Click");
--             r.ImGui_PopStyleColor(3);
--             r.ImGui_PopID();
--         }
--
    -- Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
    -- (otherwise a Text+SameLine+Button sequence will have the text a little too high by default!)
    -- See 'Demo->Layout->Text Baseline Alignment' for details.
    r.ImGui_AlignTextToFramePadding(ctx)
    r.ImGui_Text(ctx, 'Hold to repeat:')
    r.ImGui_SameLine(ctx)

    -- Arrow buttons with Repeater
    -- local spacing = r.ImGui_GetStyle().ItemInnerSpacing.x TODO
    local spacing = 1
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
      r.ImGui_PlotLines(ctx, 'Curve', tooltip_curve, 0)
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

    r.ImGui_InputDoubleN(ctx, 'input reaper.array', vec4a)

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

--             static float angle = 0.0f;
--             r.ImGui_SliderAngle("slider angle", &angle);

    -- Using the format string to display a name instead of an integer.
    -- Here we completely omit '%d' from the format string, so it'll only display a name.
    -- This technique can also be used with DragInt().
    local elements = { 'Fire', 'Earth', 'Air', 'Water' }
    local current_elem = elements[widgets.basic.elem] or 'Unknown'
    rv,widgets.basic.elem = r.ImGui_SliderInt(ctx, 'slider enum', widgets.basic.elem, 1, #elements, current_elem);
    r.ImGui_SameLine(ctx);
    demo.HelpMarker(
      'Using the format string parameter to display a name instead \z
       of the underlying integer.'
    )

    foo = widgets.basic.col1
    rv,widgets.basic.col1 = r.ImGui_ColorEdit(ctx, 'color 1', widgets.basic.col1, r.ImGui_ColorEditFlags_NoAlpha())
    r.ImGui_SameLine(ctx); demo.HelpMarker(
      'Click on the color square to open a color picker.\n\z
       Click and hold to use drag and drop.\n\z
       Right-click on the color square to show options.\n\z
       CTRL+click on individual component to input value.'
    )

    rv, widgets.basic.col2 = r.ImGui_ColorEdit(ctx, 'color 2', widgets.basic.col2)

    -- Using the _simplified_ one-liner ListBox() api here
    -- See "List boxes" section for examples of how to use the more flexible BeginListBox()/EndListBox() api.
    local items = 'Apple\31Banana\31Cherry\31Kiwi\31Mango\31Orange\31Pineapple\31Strawberry\31Watermelon\31'
    rv,widgets.basic.listcur = r.ImGui_ListBox(ctx, 'listbox\n(single select)', widgets.basic.listcur, items, 4);
    r.ImGui_SameLine(ctx);
    demo.HelpMarker(
      'Using the simplified one-liner ListBox API here.\n\z
       Refer to the "List boxes" section below for an explanation of how to use\z
       the more flexible and general BeginListBox/EndListBox API.'
    )

    r.ImGui_TreePop(ctx);
  end

--     // Testing ImGuiOnceUponAFrame helper.
--     //static ImGuiOnceUponAFrame once;
--     //for (int i = 0; i < 5; i++)
--     //    if (once)
--     //        r.ImGui_Text("This will be displayed only once.");

  if r.ImGui_TreeNode(ctx, 'Trees') then
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
        -- if r.ImGui_GetIO().KeyCtrl then TODO
        --   widgets.trees.selection_mask ^= (1 << node_clicked)           -- CTRL+click to toggle
        if false then
        else-- if (widgets.trees.selection_mask & (1 << node_clicked)) == 0 -- Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
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
    if r.ImGui_TreeNode(ctx, 'Colorful Text') then
      -- Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
      r.ImGui_TextColored(ctx, 0xFF00FFFF, 'Pink')
      r.ImGui_TextColored(ctx, 0xFFFF00FF, 'Yellow')
      r.ImGui_TextDisabled(ctx, 'Disabled');
      r.ImGui_SameLine(ctx); demo.HelpMarker('The TextDisabled color is stored in ImGuiStyle.') -- TODO style
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Word Wrapping') then
      -- Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
      r.ImGui_TextWrapped(ctx,
        'This text should automatically wrap on the edge of the window. The current implementation ' ..
        'for text wrapping follows simple rules suitable for English and possibly other languages.')
      r.ImGui_Spacing(ctx)

      rv,widgets.text.wrap_width = r.ImGui_SliderDouble(ctx,'Wrap width', widgets.text.wrap_width, -20, 600, '%.0f')

--             ImDrawList* draw_list = r.ImGui_GetWindowDrawList();
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
        r.ImGui_DrawList_AddRect(ctx, text_min_x, text_min_y, text_max_x, text_max_y, 0xFFFF00FF)
        r.ImGui_DrawList_AddRectFilled(ctx, marker_min_x, marker_min_y, marker_max_x, marker_max_y, 0xFF00FFFF)

        r.ImGui_PopTextWrapPos(ctx)
      end

      r.ImGui_TreePop(ctx)
    end

    -- Not supported by the default built-in font
    -- if r.ImGui_TreeNode(ctx, 'UTF-8 Text') then
    --   -- UTF-8 test with Japanese characters
    --   -- (Needs a suitable font? Try "Google Noto" or "Arial Unicode". See docs/FONTS.md for details.)
    --   -- so you can safely copy & paste garbled characters into another application.
    --   r.ImGui_TextWrapped(ctx,
    --     'CJK text will only appears if the font was loaded with the appropriate CJK character ranges. ' ..
    --     'Call io.Font->AddFontFromFileTTF() manually to load extra character ranges. ' ..
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

  -- TODO!
  -- if (ImGui::TreeNode("List boxes"))
  -- {
  --     // Using the generic BeginListBox() API, you have full control over how to display the combo contents.
  --     // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
  --     // stored in the object itself, etc.)
  --     const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
  --     static int item_current_idx = 0; // Here we store our selection data as an index.
  --     if (ImGui::BeginListBox("listbox 1"))
  --     {
  --         for (int n = 0; n < IM_ARRAYSIZE(items); n++)
  --         {
  --             const bool is_selected = (item_current_idx == n);
  --             if (ImGui::Selectable(items[n], is_selected))
  --                 item_current_idx = n;
  --
  --             // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
  --             if (is_selected)
  --                 ImGui::SetItemDefaultFocus();
  --         }
  --         ImGui::EndListBox();
  --     }
  --
  --     // Custom size: use all width, 5 items tall
  --     ImGui::Text("Full-width:");
  --     if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
  --     {
  --         for (int n = 0; n < IM_ARRAYSIZE(items); n++)
  --         {
  --             const bool is_selected = (item_current_idx == n);
  --             if (ImGui::Selectable(items[n], is_selected))
  --                 item_current_idx = n;
  --
  --             // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
  --             if (is_selected)
  --                 ImGui::SetItemDefaultFocus();
  --         }
  --         ImGui::EndListBox();
  --     }
  --
  --     ImGui::TreePop();
  -- }

  if r.ImGui_TreeNode(ctx, 'Selectables') then
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
          -- if not r.ImGui_GetIO().KeyCtrl then -- Clear selection when CTRL is not held TODO
          --   for j = 1, #multiple do
          --     widgets.selectables.multiple[j] = false
          --   end
          -- end
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
    if r.ImGui_TreeNode(ctx, 'Multi-line Text Input') then
      rv,widgets.text.multiline.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiInputTextFlags_ReadOnly', widgets.text.multiline.flags, r.ImGui_InputTextFlags_ReadOnly());
      rv,widgets.text.multiline.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiInputTextFlags_AllowTabInput', widgets.text.multiline.flags, r.ImGui_InputTextFlags_AllowTabInput());
      rv,widgets.text.multiline.flags = r.ImGui_CheckboxFlags(ctx, 'ImGuiInputTextFlags_CtrlEnterForNewLine', widgets.text.multiline.flags, r.ImGui_InputTextFlags_CtrlEnterForNewLine());
      rv,widgets.text.multiline.text = r.ImGui_InputTextMultiline(ctx, '##source', widgets.text.multiline.text, -1, r.ImGui_GetTextLineHeight(ctx) * 16, widgets.text.multiline.flags)
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

      rv,widgets.text.buf[1] = r.ImGui_InputText(ctx, 'default',     widgets.text.buf[1]);
      rv,widgets.text.buf[2] = r.ImGui_InputText(ctx, 'decimal',     widgets.text.buf[2], r.ImGui_InputTextFlags_CharsDecimal())
      rv,widgets.text.buf[3] = r.ImGui_InputText(ctx, 'hexadecimal', widgets.text.buf[3], r.ImGui_InputTextFlags_CharsHexadecimal() | r.ImGui_InputTextFlags_CharsUppercase())
      rv,widgets.text.buf[4] = r.ImGui_InputText(ctx, 'uppercase',   widgets.text.buf[4], r.ImGui_InputTextFlags_CharsUppercase())
      rv,widgets.text.buf[5] = r.ImGui_InputText(ctx, 'no blank',    widgets.text.buf[5], r.ImGui_InputTextFlags_CharsNoBlank())
      -- static char buf6[64] = ""; r.ImGui_InputText("\"imgui\" letters", buf6, 64, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);
      r.ImGui_TreePop(ctx)
    end

    if r.ImGui_TreeNode(ctx, 'Password Input') then
      rv,widgets.text.password = r.ImGui_InputText(ctx, 'password', widgets.text.password, r.ImGui_InputTextFlags_Password())
      r.ImGui_SameLine(ctx); demo.HelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
      rv,widgets.text.password = r.ImGui_InputTextWithHint(ctx, 'password (w/ hint)', '<password>', widgets.text.password, r.ImGui_InputTextFlags_Password());
      rv,widgets.text.password = r.ImGui_InputText(ctx, 'password (clear)', widgets.text.password)
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
    local fittingPolicyMask = r.ImGui_TabBarFlags_FittingPolicyResizeDown() |
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

      if widgets.tabs.flags1 & fittingPolicyMask == 0 then
        widgets.tabs.flags1 = widgets.tabs.flags1 | r.ImGui_TabBarFlags_FittingPolicyResizeDown() -- was FittingPolicyDefault_
      end
      if r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyResizeDown', widgets.tabs.flags1, r.ImGui_TabBarFlags_FittingPolicyResizeDown()) then
        widgets.tabs.flags1 = widgets.tabs.flags1 & ~fittingPolicyMask | r.ImGui_TabBarFlags_FittingPolicyResizeDown()
      end
      if r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyScroll', widgets.tabs.flags1, r.ImGui_TabBarFlags_FittingPolicyScroll()) then
        widgets.tabs.flags1 = widgets.tabs.flags1 & ~fittingPolicyMask | r.ImGui_TabBarFlags_FittingPolicyScroll()
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
        widgets.tabs.flags2 = widgets.tabs.flags2 & ~fittingPolicyMask | r.ImGui_TabBarFlags_FittingPolicyResizeDown()
      end
      if r.ImGui_CheckboxFlags(ctx, 'ImGuiTabBarFlags_FittingPolicyScroll', widgets.tabs.flags2, r.ImGui_TabBarFlags_FittingPolicyScroll()) then
        widgets.tabs.flags2 = widgets.tabs.flags2 & ~fittingPolicyMask | r.ImGui_TabBarFlags_FittingPolicyScroll()
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
    rv,widgets.plots.animate = r.ImGui_Checkbox(ctx, 'Animate', widgets.plots.animate)

    r.ImGui_PlotLines(ctx, 'Frame Times', frame_times)

    -- Fill an array of contiguous float values to plot
    if not widgets.plots.animate or widgets.plots.plot1.refresh_time == 0.0 then
      widgets.plots.plot1.refresh_time = r.ImGui_GetTime(ctx)
    end
    while widgets.plots.plot1.refresh_time < r.ImGui_GetTime(ctx) do -- Create data at fixed 60 Hz rate for the demo
      plot1[widgets.plots.plot1.offset] = math.cos(widgets.plots.plot1.phase)
      widgets.plots.plot1.offset = (widgets.plots.plot1.offset % PLOT1_SIZE) + 1
      widgets.plots.plot1.phase = widgets.plots.plot1.phase + (0.10 * widgets.plots.plot1.offset)
      widgets.plots.plot1.refresh_time = widgets.plots.plot1.refresh_time + (1.0 / 60.0)
    end

    -- Plots can display overlay texts
    -- (in this example, we will display an average value)
    local average = 0.0
    for n = 1, PLOT1_SIZE do
      average = average + plot1[n]
    end
    average = average / PLOT1_SIZE

    local overlay = ('avg %f'):format(average)
    r.ImGui_PlotLines(ctx, 'Lines', plot1, widgets.plots.plot1.offset - 1, overlay, -1.0, 1.0, 0, 80.0)

    r.ImGui_PlotHistogram(ctx, 'Histogram', frame_times, 0, nil, 0.0, 1.0, 0, 80.0)
    r.ImGui_Separator(ctx)

    r.ImGui_SetNextItemWidth(ctx, 100)
    rv,widgets.plots.plot2.func = r.ImGui_Combo(ctx, 'func', widgets.plots.plot2.func, 'Sin\31Saw\31')
    local funcChanged = rv
    r.ImGui_SameLine(ctx)
    rv,widgets.plots.plot2.size = r.ImGui_SliderInt(ctx, 'Sample count', widgets.plots.plot2.size, 1, 400)

    -- Use functions to generate output
    if funcChanged or rv or widgets.plots.plot2.fill then
      widgets.plots.plot2.fill = false -- fill the first time
      plot2 = reaper.new_array(widgets.plots.plot2.size)
      for n = 1, widgets.plots.plot2.size do
        plot2[n] = plot2_funcs[widgets.plots.plot2.func + 1](n - 1)
      end
    end

    r.ImGui_PlotLines(ctx, 'Lines', plot2, 0, nil, -1.0, 1.0, 0, 80)
    r.ImGui_PlotHistogram(ctx, 'Histogram', plot2, 0, nil, -1.0, 1.0, 0, 80)
    r.ImGui_Separator(ctx)

    -- Animate a simple progress bar
    -- if widgets.plots.animate then
    --   widgets.plots.progress = widgets.plots.progress +
    --     (widgets.plots.progress_dir * 0.4f * r.ImGui_GetDeltaTime())
    --   if widgets.plots.progress >= +1.1 then
    --     widgets.plots.progress = +1.1
    --     widgets.plots.progress_dir = widgets.plots.progress_dir * -1
    --   elseif widgets.plots.progress <= -0.1 then
    --     widgets.plots.progress = -0.1
    --     widgets.plots.progress_dir = widgets.plots.progress_dir * -1
    --   end
    -- end
    --
    -- -- Typically we would use (-1.0f,0.0f) or (-FLT_MIN,0.0f) to use all available width,
    -- -- or (width,0.0f) for a specified width. (0.0f,0.0f) uses ItemWidth.
    -- r.ImGui_ProgressBar(ctx, progress, 0.0, 0.0)
    -- r.ImGui_SameLine(0.0, r.ImGui_GetStyle().ItemInnerSpacing.x)
    -- r.ImGui_Text(ctx, 'Progress Bar')
    --
    -- local progress_saturated = demo.clamp(progress, 0.0, 1.0);
    -- local buf = ("%d/%d"):format(progress_saturated * 1753, 1753)
    -- r.ImGui_ProgressBar(progress, 0.0, 0.0, buf);

    r.ImGui_TreePop(ctx)
  end
--
--     if (r.ImGui_TreeNode("Color/Picker Widgets"))
--     {
--         static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
--
--         static bool alpha_preview = true;
--         static bool alpha_half_preview = false;
--         static bool drag_and_drop = true;
--         static bool options_menu = true;
--         static bool hdr = false;
--         r.ImGui_Checkbox("With Alpha Preview", &alpha_preview);
--         r.ImGui_Checkbox("With Half Alpha Preview", &alpha_half_preview);
--         r.ImGui_Checkbox("With Drag and Drop", &drag_and_drop);
--         r.ImGui_Checkbox("With Options Menu", &options_menu); r.ImGui_SameLine(); HelpMarker("Right-click on the individual color widget to show options.");
--         r.ImGui_Checkbox("With HDR", &hdr); r.ImGui_SameLine(); HelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");
--         ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
--
--         r.ImGui_Text("Color widget:");
--         r.ImGui_SameLine(); HelpMarker(
--             "Click on the color square to open a color picker.\n"
--             "CTRL+click on individual component to input value.\n");
--         r.ImGui_ColorEdit3("MyColor##1", (float*)&color, misc_flags);
--
--         r.ImGui_Text("Color widget HSV with Alpha:");
--         r.ImGui_ColorEdit4("MyColor##2", (float*)&color, ImGuiColorEditFlags_DisplayHSV | misc_flags);
--
--         r.ImGui_Text("Color widget with Float Display:");
--         r.ImGui_ColorEdit4("MyColor##2f", (float*)&color, ImGuiColorEditFlags_Float | misc_flags);
--
--         r.ImGui_Text("Color button with Picker:");
--         r.ImGui_SameLine(); HelpMarker(
--             "With the ImGuiColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\n"
--             "With the ImGuiColorEditFlags_NoLabel flag you can pass a non-empty label which will only "
--             "be used for the tooltip and picker popup.");
--         r.ImGui_ColorEdit4("MyColor##3", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
--
--         r.ImGui_Text("Color button with Custom Picker Popup:");
--
--         // Generate a default palette. The palette will persist and can be edited.
--         static bool saved_palette_init = true;
--         static ImVec4 saved_palette[32] = {};
--         if (saved_palette_init)
--         {
--             for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
--             {
--                 r.ImGui_ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
--                     saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
--                 saved_palette[n].w = 1.0f; // Alpha
--             }
--             saved_palette_init = false;
--         }
--
--         static ImVec4 backup_color;
--         bool open_popup = r.ImGui_ColorButton("MyColor##3b", color, misc_flags);
--         r.ImGui_SameLine(0, r.ImGui_GetStyle().ItemInnerSpacing.x);
--         open_popup |= r.ImGui_Button("Palette");
--         if (open_popup)
--         {
--             r.ImGui_OpenPopup("mypicker");
--             backup_color = color;
--         }
--         if (r.ImGui_BeginPopup("mypicker"))
--         {
--             r.ImGui_Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
--             r.ImGui_Separator();
--             r.ImGui_ColorPicker4("##picker", (float*)&color, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
--             r.ImGui_SameLine();
--
--             r.ImGui_BeginGroup(); // Lock X position
--             r.ImGui_Text("Current");
--             r.ImGui_ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
--             r.ImGui_Text("Previous");
--             if (r.ImGui_ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
--                 color = backup_color;
--             r.ImGui_Separator();
--             r.ImGui_Text("Palette");
--             for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
--             {
--                 r.ImGui_PushID(n);
--                 if ((n % 8) != 0)
--                     r.ImGui_SameLine(0.0f, r.ImGui_GetStyle().ItemSpacing.y);
--
--                 ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
--                 if (r.ImGui_ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20)))
--                     color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!
--
--                 // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
--                 // drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
--                 if (r.ImGui_BeginDragDropTarget())
--                 {
--                     if (const ImGuiPayload* payload = r.ImGui_AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
--                         memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
--                     if (const ImGuiPayload* payload = r.ImGui_AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
--                         memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
--                     r.ImGui_EndDragDropTarget();
--                 }
--
--                 r.ImGui_PopID();
--             }
--             r.ImGui_EndGroup();
--             r.ImGui_EndPopup();
--         }
--
--         r.ImGui_Text("Color button only:");
--         static bool no_border = false;
--         r.ImGui_Checkbox("ImGuiColorEditFlags_NoBorder", &no_border);
--         r.ImGui_ColorButton("MyColor##3c", *(ImVec4*)&color, misc_flags | (no_border ? ImGuiColorEditFlags_NoBorder : 0), ImVec2(80, 80));
--
--         r.ImGui_Text("Color picker:");
--         static bool alpha = true;
--         static bool alpha_bar = true;
--         static bool side_preview = true;
--         static bool ref_color = false;
--         static ImVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
--         static int display_mode = 0;
--         static int picker_mode = 0;
--         r.ImGui_Checkbox("With Alpha", &alpha);
--         r.ImGui_Checkbox("With Alpha Bar", &alpha_bar);
--         r.ImGui_Checkbox("With Side Preview", &side_preview);
--         if (side_preview)
--         {
--             r.ImGui_SameLine();
--             r.ImGui_Checkbox("With Ref Color", &ref_color);
--             if (ref_color)
--             {
--                 r.ImGui_SameLine();
--                 r.ImGui_ColorEdit4("##RefColor", &ref_color_v.x, ImGuiColorEditFlags_NoInputs | misc_flags);
--             }
--         }
--         r.ImGui_Combo("Display Mode", &display_mode, "Auto/Current\0None\0RGB Only\0HSV Only\0Hex Only\0");
--         r.ImGui_SameLine(); HelpMarker(
--             "ColorEdit defaults to displaying RGB inputs if you don't specify a display mode, "
--             "but the user can change it with a right-click.\n\nColorPicker defaults to displaying RGB+HSV+Hex "
--             "if you don't specify a display mode.\n\nYou can change the defaults using SetColorEditOptions().");
--         r.ImGui_Combo("Picker Mode", &picker_mode, "Auto/Current\0Hue bar + SV rect\0Hue wheel + SV triangle\0");
--         r.ImGui_SameLine(); HelpMarker("User can right-click the picker to change mode.");
--         ImGuiColorEditFlags flags = misc_flags;
--         if (!alpha)            flags |= ImGuiColorEditFlags_NoAlpha;        // This is by default if you call ColorPicker3() instead of ColorPicker4()
--         if (alpha_bar)         flags |= ImGuiColorEditFlags_AlphaBar;
--         if (!side_preview)     flags |= ImGuiColorEditFlags_NoSidePreview;
--         if (picker_mode == 1)  flags |= ImGuiColorEditFlags_PickerHueBar;
--         if (picker_mode == 2)  flags |= ImGuiColorEditFlags_PickerHueWheel;
--         if (display_mode == 1) flags |= ImGuiColorEditFlags_NoInputs;       // Disable all RGB/HSV/Hex displays
--         if (display_mode == 2) flags |= ImGuiColorEditFlags_DisplayRGB;     // Override display mode
--         if (display_mode == 3) flags |= ImGuiColorEditFlags_DisplayHSV;
--         if (display_mode == 4) flags |= ImGuiColorEditFlags_DisplayHex;
--         r.ImGui_ColorPicker4("MyColor##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);
--
--         r.ImGui_Text("Set defaults in code:");
--         r.ImGui_SameLine(); HelpMarker(
--             "SetColorEditOptions() is designed to allow you to set boot-time default.\n"
--             "We don't have Push/Pop functions because you can force options on a per-widget basis if needed,"
--             "and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid"
--             "encouraging you to persistently save values that aren't forward-compatible.");
--         if (r.ImGui_Button("Default: Uint8 + HSV + Hue Bar"))
--             r.ImGui_SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_PickerHueBar);
--         if (r.ImGui_Button("Default: Float + HDR + Hue Wheel"))
--             r.ImGui_SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);
--
--         // HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
--         static ImVec4 color_hsv(0.23f, 1.0f, 1.0f, 1.0f); // Stored as HSV!
--         r.ImGui_Spacing();
--         r.ImGui_Text("HSV encoded colors");
--         r.ImGui_SameLine(); HelpMarker(
--             "By default, colors are given to ColorEdit and ColorPicker in RGB, but ImGuiColorEditFlags_InputHSV"
--             "allows you to store colors as HSV and pass them to ColorEdit and ColorPicker as HSV. This comes with the"
--             "added benefit that you can manipulate hue values with the picker even when saturation or value are zero.");
--         r.ImGui_Text("Color widget with InputHSV:");
--         r.ImGui_ColorEdit4("HSV shown as RGB##1", (float*)&color_hsv, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
--         r.ImGui_ColorEdit4("HSV shown as HSV##1", (float*)&color_hsv, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
--         r.ImGui_DragFloat4("Raw HSV values", (float*)&color_hsv, 0.01f, 0.0f, 1.0f);
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Drag/Slider Flags"))
--     {
--         // Demonstrate using advanced flags for DragXXX and SliderXXX functions. Note that the flags are the same!
--         static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
--         r.ImGui_CheckboxFlags("ImGuiSliderFlags_AlwaysClamp", &flags, ImGuiSliderFlags_AlwaysClamp);
--         r.ImGui_SameLine(); HelpMarker("Always clamp value to min/max bounds (if any) when input manually with CTRL+Click.");
--         r.ImGui_CheckboxFlags("ImGuiSliderFlags_Logarithmic", &flags, ImGuiSliderFlags_Logarithmic);
--         r.ImGui_SameLine(); HelpMarker("Enable logarithmic editing (more precision for small values).");
--         r.ImGui_CheckboxFlags("ImGuiSliderFlags_NoRoundToFormat", &flags, ImGuiSliderFlags_NoRoundToFormat);
--         r.ImGui_SameLine(); HelpMarker("Disable rounding underlying value to match precision of the format string (e.g. %.3f values are rounded to those 3 digits).");
--         r.ImGui_CheckboxFlags("ImGuiSliderFlags_NoInput", &flags, ImGuiSliderFlags_NoInput);
--         r.ImGui_SameLine(); HelpMarker("Disable CTRL+Click or Enter key allowing to input text directly into the widget.");
--
--         // Drags
--         static float drag_f = 0.5f;
--         static int drag_i = 50;
--         r.ImGui_Text("Underlying float value: %f", drag_f);
--         r.ImGui_DragFloat("DragFloat (0 -> 1)", &drag_f, 0.005f, 0.0f, 1.0f, "%.3f", flags);
--         r.ImGui_DragFloat("DragFloat (0 -> +inf)", &drag_f, 0.005f, 0.0f, FLT_MAX, "%.3f", flags);
--         r.ImGui_DragFloat("DragFloat (-inf -> 1)", &drag_f, 0.005f, -FLT_MAX, 1.0f, "%.3f", flags);
--         r.ImGui_DragFloat("DragFloat (-inf -> +inf)", &drag_f, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
--         r.ImGui_DragInt("DragInt (0 -> 100)", &drag_i, 0.5f, 0, 100, "%d", flags);
--
--         // Sliders
--         static float slider_f = 0.5f;
--         static int slider_i = 50;
--         r.ImGui_Text("Underlying float value: %f", slider_f);
--         r.ImGui_SliderFloat("SliderFloat (0 -> 1)", &slider_f, 0.0f, 1.0f, "%.3f", flags);
--         r.ImGui_SliderInt("SliderInt (0 -> 100)", &slider_i, 0, 100, "%d", flags);
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Range Widgets"))
--     {
--         static float begin = 10, end = 90;
--         static int begin_i = 100, end_i = 1000;
--         r.ImGui_DragFloatRange2("range float", &begin, &end, 0.25f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%", ImGuiSliderFlags_AlwaysClamp);
--         r.ImGui_DragIntRange2("range int", &begin_i, &end_i, 5, 0, 1000, "Min: %d units", "Max: %d units");
--         r.ImGui_DragIntRange2("range int (no bounds)", &begin_i, &end_i, 5, 0, 0, "Min: %d units", "Max: %d units");
--         r.ImGui_TreePop();
--     }
--
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
--
--     if (r.ImGui_TreeNode("Multi-component Widgets"))
--     {
--         static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
--         static int vec4i[4] = { 1, 5, 100, 255 };
--
--         r.ImGui_InputFloat2("input float2", vec4f);
--         r.ImGui_DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f);
--         r.ImGui_SliderFloat2("slider float2", vec4f, 0.0f, 1.0f);
--         r.ImGui_InputInt2("input int2", vec4i);
--         r.ImGui_DragInt2("drag int2", vec4i, 1, 0, 255);
--         r.ImGui_SliderInt2("slider int2", vec4i, 0, 255);
--         r.ImGui_Spacing();
--
--         r.ImGui_InputFloat3("input float3", vec4f);
--         r.ImGui_DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f);
--         r.ImGui_SliderFloat3("slider float3", vec4f, 0.0f, 1.0f);
--         r.ImGui_InputInt3("input int3", vec4i);
--         r.ImGui_DragInt3("drag int3", vec4i, 1, 0, 255);
--         r.ImGui_SliderInt3("slider int3", vec4i, 0, 255);
--         r.ImGui_Spacing();
--
--         r.ImGui_InputFloat4("input float4", vec4f);
--         r.ImGui_DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f);
--         r.ImGui_SliderFloat4("slider float4", vec4f, 0.0f, 1.0f);
--         r.ImGui_InputInt4("input int4", vec4i);
--         r.ImGui_DragInt4("drag int4", vec4i, 1, 0, 255);
--         r.ImGui_SliderInt4("slider int4", vec4i, 0, 255);
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Vertical Sliders"))
--     {
--         const float spacing = 4;
--         r.ImGui_PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
--
--         static int int_value = 0;
--         r.ImGui_VSliderInt("##int", ImVec2(18, 160), &int_value, 0, 5);
--         r.ImGui_SameLine();
--
--         static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
--         r.ImGui_PushID("set1");
--         for (int i = 0; i < 7; i++)
--         {
--             if (i > 0) r.ImGui_SameLine();
--             r.ImGui_PushID(i);
--             r.ImGui_PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(i / 7.0f, 0.5f, 0.5f));
--             r.ImGui_PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.5f));
--             r.ImGui_PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.5f));
--             r.ImGui_PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(i / 7.0f, 0.9f, 0.9f));
--             r.ImGui_VSliderFloat("##v", ImVec2(18, 160), &values[i], 0.0f, 1.0f, "");
--             if (r.ImGui_IsItemActive() || r.ImGui_IsItemHovered())
--                 r.ImGui_SetTooltip("%.3f", values[i]);
--             r.ImGui_PopStyleColor(4);
--             r.ImGui_PopID();
--         }
--         r.ImGui_PopID();
--
--         r.ImGui_SameLine();
--         r.ImGui_PushID("set2");
--         static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
--         const int rows = 3;
--         const ImVec2 small_slider_size(18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
--         for (int nx = 0; nx < 4; nx++)
--         {
--             if (nx > 0) r.ImGui_SameLine();
--             r.ImGui_BeginGroup();
--             for (int ny = 0; ny < rows; ny++)
--             {
--                 r.ImGui_PushID(nx * rows + ny);
--                 r.ImGui_VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
--                 if (r.ImGui_IsItemActive() || r.ImGui_IsItemHovered())
--                     r.ImGui_SetTooltip("%.3f", values2[nx]);
--                 r.ImGui_PopID();
--             }
--             r.ImGui_EndGroup();
--         }
--         r.ImGui_PopID();
--
--         r.ImGui_SameLine();
--         r.ImGui_PushID("set3");
--         for (int i = 0; i < 4; i++)
--         {
--             if (i > 0) r.ImGui_SameLine();
--             r.ImGui_PushID(i);
--             r.ImGui_PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
--             r.ImGui_VSliderFloat("##v", ImVec2(40, 160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
--             r.ImGui_PopStyleVar();
--             r.ImGui_PopID();
--         }
--         r.ImGui_PopID();
--         r.ImGui_PopStyleVar();
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Drag and Drop"))
--     {
--         if (r.ImGui_TreeNode("Drag and drop in standard widgets"))
--         {
--             // ColorEdit widgets automatically act as drag source and drag target.
--             // They are using standardized payload strings IMGUI_PAYLOAD_TYPE_COLOR_3F and IMGUI_PAYLOAD_TYPE_COLOR_4F
--             // to allow your own widgets to use colors in their drag and drop interaction.
--             // Also see 'Demo->Widgets->Color/Picker Widgets->Palette' demo.
--             HelpMarker("You can drag from the color squares.");
--             static float col1[3] = { 1.0f, 0.0f, 0.2f };
--             static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
--             r.ImGui_ColorEdit3("color 1", col1);
--             r.ImGui_ColorEdit4("color 2", col2);
--             r.ImGui_TreePop();
--         }
--
--         if (r.ImGui_TreeNode("Drag and drop to copy/swap items"))
--         {
--             enum Mode
--             {
--                 Mode_Copy,
--                 Mode_Move,
--                 Mode_Swap
--             };
--             static int mode = 0;
--             if (r.ImGui_RadioButton("Copy", mode == Mode_Copy)) { mode = Mode_Copy; } r.ImGui_SameLine();
--             if (r.ImGui_RadioButton("Move", mode == Mode_Move)) { mode = Mode_Move; } r.ImGui_SameLine();
--             if (r.ImGui_RadioButton("Swap", mode == Mode_Swap)) { mode = Mode_Swap; }
--             static const char* names[9] =
--             {
--                 "Bobby", "Beatrice", "Betty",
--                 "Brianna", "Barry", "Bernard",
--                 "Bibi", "Blaine", "Bryn"
--             };
--             for (int n = 0; n < IM_ARRAYSIZE(names); n++)
--             {
--                 r.ImGui_PushID(n);
--                 if ((n % 3) != 0)
--                     r.ImGui_SameLine();
--                 r.ImGui_Button(names[n], ImVec2(60, 60));
--
--                 // Our buttons are both drag sources and drag targets here!
--                 if (r.ImGui_BeginDragDropSource(ImGuiDragDropFlags_None))
--                 {
--                     // Set payload to carry the index of our item (could be anything)
--                     r.ImGui_SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));
--
--                     // Display preview (could be anything, e.g. when dragging an image we could decide to display
--                     // the filename and a small preview of the image, etc.)
--                     if (mode == Mode_Copy) { r.ImGui_Text("Copy %s", names[n]); }
--                     if (mode == Mode_Move) { r.ImGui_Text("Move %s", names[n]); }
--                     if (mode == Mode_Swap) { r.ImGui_Text("Swap %s", names[n]); }
--                     r.ImGui_EndDragDropSource();
--                 }
--                 if (r.ImGui_BeginDragDropTarget())
--                 {
--                     if (const ImGuiPayload* payload = r.ImGui_AcceptDragDropPayload("DND_DEMO_CELL"))
--                     {
--                         IM_ASSERT(payload->DataSize == sizeof(int));
--                         int payload_n = *(const int*)payload->Data;
--                         if (mode == Mode_Copy)
--                         {
--                             names[n] = names[payload_n];
--                         }
--                         if (mode == Mode_Move)
--                         {
--                             names[n] = names[payload_n];
--                             names[payload_n] = "";
--                         }
--                         if (mode == Mode_Swap)
--                         {
--                             const char* tmp = names[n];
--                             names[n] = names[payload_n];
--                             names[payload_n] = tmp;
--                         }
--                     }
--                     r.ImGui_EndDragDropTarget();
--                 }
--                 r.ImGui_PopID();
--             }
--             r.ImGui_TreePop();
--         }
--
--         if (r.ImGui_TreeNode("Drag to reorder items (simple)"))
--         {
--             // Simple reordering
--             HelpMarker(
--                 "We don't use the drag and drop api at all here! "
--                 "Instead we query when the item is held but not hovered, and order items accordingly.");
--             static const char* item_names[] = { "Item One", "Item Two", "Item Three", "Item Four", "Item Five" };
--             for (int n = 0; n < IM_ARRAYSIZE(item_names); n++)
--             {
--                 const char* item = item_names[n];
--                 r.ImGui_Selectable(item);
--
--                 if (r.ImGui_IsItemActive() && !r.ImGui_IsItemHovered())
--                 {
--                     int n_next = n + (r.ImGui_GetMouseDragDelta(0).y < 0.f ? -1 : 1);
--                     if (n_next >= 0 && n_next < IM_ARRAYSIZE(item_names))
--                     {
--                         item_names[n] = item_names[n_next];
--                         item_names[n_next] = item;
--                         r.ImGui_ResetMouseDragDelta();
--                     }
--                 }
--             }
--             r.ImGui_TreePop();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Querying Status (Edited/Active/Focused/Hovered etc.)"))
--     {
--         // Select an item type
--         const char* item_names[] =
--         {
--             "Text", "Button", "Button (w/ repeat)", "Checkbox", "SliderFloat", "InputText", "InputFloat",
--             "InputFloat3", "ColorEdit4", "MenuItem", "TreeNode", "TreeNode (w/ double-click)", "Combo", "ListBox"
--         };
--         static int item_type = 1;
--         r.ImGui_Combo("Item Type", &item_type, item_names, IM_ARRAYSIZE(item_names), IM_ARRAYSIZE(item_names));
--         r.ImGui_SameLine();
--         HelpMarker("Testing how various types of items are interacting with the IsItemXXX functions. Note that the bool return value of most ImGui function is generally equivalent to calling r.ImGui_IsItemHovered().");
--
--         // Submit selected item item so we can query their status in the code following it.
--         bool ret = false;
--         static bool b = false;
--         static float col4f[4] = { 1.0f, 0.5, 0.0f, 1.0f };
--         static char str[16] = {};
--         if (item_type == 0) { r.ImGui_Text("ITEM: Text"); }                                              // Testing text items with no identifier/interaction
--         if (item_type == 1) { ret = r.ImGui_Button("ITEM: Button"); }                                    // Testing button
--         if (item_type == 2) { r.ImGui_PushButtonRepeat(true); ret = r.ImGui_Button("ITEM: Button"); r.ImGui_PopButtonRepeat(); } // Testing button (with repeater)
--         if (item_type == 3) { ret = r.ImGui_Checkbox("ITEM: Checkbox", &b); }                            // Testing checkbox
--         if (item_type == 4) { ret = r.ImGui_SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f); }   // Testing basic item
--         if (item_type == 5) { ret = r.ImGui_InputText("ITEM: InputText", &str[0], IM_ARRAYSIZE(str)); }  // Testing input text (which handles tabbing)
--         if (item_type == 6) { ret = r.ImGui_InputFloat("ITEM: InputFloat", col4f, 1.0f); }               // Testing +/- buttons on scalar input
--         if (item_type == 7) { ret = r.ImGui_InputFloat3("ITEM: InputFloat3", col4f); }                   // Testing multi-component items (IsItemXXX flags are reported merged)
--         if (item_type == 8) { ret = r.ImGui_ColorEdit4("ITEM: ColorEdit4", col4f); }                     // Testing multi-component items (IsItemXXX flags are reported merged)
--         if (item_type == 9) { ret = r.ImGui_MenuItem("ITEM: MenuItem"); }                                // Testing menu item (they use ImGuiButtonFlags_PressedOnRelease button policy)
--         if (item_type == 10){ ret = r.ImGui_TreeNode("ITEM: TreeNode"); if (ret) r.ImGui_TreePop(); }     // Testing tree node
--         if (item_type == 11){ ret = r.ImGui_TreeNodeEx("ITEM: TreeNode w/ ImGuiTreeNodeFlags_OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen); } // Testing tree node with ImGuiButtonFlags_PressedOnDoubleClick button policy.
--         if (item_type == 12){ const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = r.ImGui_Combo("ITEM: Combo", &current, items, IM_ARRAYSIZE(items)); }
--         if (item_type == 13){ const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = r.ImGui_ListBox("ITEM: ListBox", &current, items, IM_ARRAYSIZE(items), IM_ARRAYSIZE(items)); }
--
--         // Display the values of IsItemHovered() and other common item state functions.
--         // Note that the ImGuiHoveredFlags_XXX flags can be combined.
--         // Because BulletText is an item itself and that would affect the output of IsItemXXX functions,
--         // we query every state in a single call to avoid storing them and to simplify the code.
--         r.ImGui_BulletText(
--             "Return value = %d\n"
--             "IsItemFocused() = %d\n"
--             "IsItemHovered() = %d\n"
--             "IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
--             "IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
--             "IsItemHovered(_AllowWhenOverlapped) = %d\n"
--             "IsItemHovered(_RectOnly) = %d\n"
--             "IsItemActive() = %d\n"
--             "IsItemEdited() = %d\n"
--             "IsItemActivated() = %d\n"
--             "IsItemDeactivated() = %d\n"
--             "IsItemDeactivatedAfterEdit() = %d\n"
--             "IsItemVisible() = %d\n"
--             "IsItemClicked() = %d\n"
--             "IsItemToggledOpen() = %d\n"
--             "GetItemRectMin() = (%.1f, %.1f)\n"
--             "GetItemRectMax() = (%.1f, %.1f)\n"
--             "GetItemRectSize() = (%.1f, %.1f)",
--             ret,
--             r.ImGui_IsItemFocused(),
--             r.ImGui_IsItemHovered(),
--             r.ImGui_IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
--             r.ImGui_IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
--             r.ImGui_IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped),
--             r.ImGui_IsItemHovered(ImGuiHoveredFlags_RectOnly),
--             r.ImGui_IsItemActive(),
--             r.ImGui_IsItemEdited(),
--             r.ImGui_IsItemActivated(),
--             r.ImGui_IsItemDeactivated(),
--             r.ImGui_IsItemDeactivatedAfterEdit(),
--             r.ImGui_IsItemVisible(),
--             r.ImGui_IsItemClicked(),
--             r.ImGui_IsItemToggledOpen(),
--             r.ImGui_GetItemRectMin().x, r.ImGui_GetItemRectMin().y,
--             r.ImGui_GetItemRectMax().x, r.ImGui_GetItemRectMax().y,
--             r.ImGui_GetItemRectSize().x, r.ImGui_GetItemRectSize().y
--         );
--
--         static bool embed_all_inside_a_child_window = false;
--         r.ImGui_Checkbox("Embed everything inside a child window (for additional testing)", &embed_all_inside_a_child_window);
--         if (embed_all_inside_a_child_window)
--             r.ImGui_BeginChild("outer_child", ImVec2(0, r.ImGui_GetFontSize() * 20.0f), true);
--
--         // Testing IsWindowFocused() function with its various flags.
--         // Note that the ImGuiFocusedFlags_XXX flags can be combined.
--         r.ImGui_BulletText(
--             "IsWindowFocused() = %d\n"
--             "IsWindowFocused(_ChildWindows) = %d\n"
--             "IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
--             "IsWindowFocused(_RootWindow) = %d\n"
--             "IsWindowFocused(_AnyWindow) = %d\n",
--             r.ImGui_IsWindowFocused(),
--             r.ImGui_IsWindowFocused(ImGuiFocusedFlags_ChildWindows),
--             r.ImGui_IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow),
--             r.ImGui_IsWindowFocused(ImGuiFocusedFlags_RootWindow),
--             r.ImGui_IsWindowFocused(ImGuiFocusedFlags_AnyWindow));
--
--         // Testing IsWindowHovered() function with its various flags.
--         // Note that the ImGuiHoveredFlags_XXX flags can be combined.
--         r.ImGui_BulletText(
--             "IsWindowHovered() = %d\n"
--             "IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
--             "IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
--             "IsWindowHovered(_ChildWindows) = %d\n"
--             "IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
--             "IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %d\n"
--             "IsWindowHovered(_RootWindow) = %d\n"
--             "IsWindowHovered(_AnyWindow) = %d\n",
--             r.ImGui_IsWindowHovered(),
--             r.ImGui_IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
--             r.ImGui_IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
--             r.ImGui_IsWindowHovered(ImGuiHoveredFlags_ChildWindows),
--             r.ImGui_IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow),
--             r.ImGui_IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup),
--             r.ImGui_IsWindowHovered(ImGuiHoveredFlags_RootWindow),
--             r.ImGui_IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
--
--         r.ImGui_BeginChild("child", ImVec2(0, 50), true);
--         r.ImGui_Text("This is another child window for testing the _ChildWindows flag.");
--         r.ImGui_EndChild();
--         if (embed_all_inside_a_child_window)
--             r.ImGui_EndChild();
--
--         static char unused_str[] = "This widget is only here to be able to tab-out of the widgets above.";
--         r.ImGui_InputText("unused", unused_str, IM_ARRAYSIZE(unused_str), ImGuiInputTextFlags_ReadOnly);
--
--         // Calling IsItemHovered() after begin returns the hovered status of the title bar.
--         // This is useful in particular if you want to create a context menu associated to the title bar of a window.
--         static bool test_window = false;
--         r.ImGui_Checkbox("Hovered/Active tests after Begin() for title bar testing", &test_window);
--         if (test_window)
--         {
--             r.ImGui_Begin("Title bar Hovered/Active tests", &test_window);
--             if (r.ImGui_BeginPopupContextItem()) // <-- This is using IsItemHovered()
--             {
--                 if (r.ImGui_MenuItem("Close")) { test_window = false; }
--                 r.ImGui_EndPopup();
--             }
--             r.ImGui_Text(
--                 "IsItemHovered() after begin = %d (== is title bar hovered)\n"
--                 "IsItemActive() after begin = %d (== is window being clicked/moved)\n",
--                 r.ImGui_IsItemHovered(), r.ImGui_IsItemActive());
--             r.ImGui_End();
--         }
--
--         r.ImGui_TreePop();
--     }
end

-- static void ShowDemoWindowLayout()
-- {
--     if (!r.ImGui_CollapsingHeader("Layout & Scrolling"))
--         return;
--
--     if (r.ImGui_TreeNode("Child windows"))
--     {
--         HelpMarker("Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window.");
--         static bool disable_mouse_wheel = false;
--         static bool disable_menu = false;
--         r.ImGui_Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);
--         r.ImGui_Checkbox("Disable Menu", &disable_menu);
--
--         // Child 1: no border, enable horizontal scrollbar
--         {
--             ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
--             if (disable_mouse_wheel)
--                 window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
--             r.ImGui_BeginChild("ChildL", ImVec2(r.ImGui_GetWindowContentRegionWidth() * 0.5f, 260), false, window_flags);
--             for (int i = 0; i < 100; i++)
--                 r.ImGui_Text("%04d: scrollable region", i);
--             r.ImGui_EndChild();
--         }
--
--         r.ImGui_SameLine();
--
--         // Child 2: rounded border
--         {
--             ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
--             if (disable_mouse_wheel)
--                 window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
--             if (!disable_menu)
--                 window_flags |= ImGuiWindowFlags_MenuBar;
--             r.ImGui_PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
--             r.ImGui_BeginChild("ChildR", ImVec2(0, 260), true, window_flags);
--             if (!disable_menu && r.ImGui_BeginMenuBar())
--             {
--                 if (r.ImGui_BeginMenu("Menu"))
--                 {
--                     ShowExampleMenuFile();
--                     r.ImGui_EndMenu();
--                 }
--                 r.ImGui_EndMenuBar();
--             }
--             if (r.ImGui_BeginTable("split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
--             {
--                 for (int i = 0; i < 100; i++)
--                 {
--                     char buf[32];
--                     sprintf(buf, "%03d", i);
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Button(buf, ImVec2(-FLT_MIN, 0.0f));
--                 }
--                 r.ImGui_EndTable();
--             }
--             r.ImGui_EndChild();
--             r.ImGui_PopStyleVar();
--         }
--
--         r.ImGui_Separator();
--
--         // Demonstrate a few extra things
--         // - Changing ImGuiCol_ChildBg (which is transparent black in default styles)
--         // - Using SetCursorPos() to position child window (the child window is an item from the POV of parent window)
--         //   You can also call SetNextWindowPos() to position the child window. The parent window will effectively
--         //   layout from this position.
--         // - Using r.ImGui_GetItemRectMin/Max() to query the "item" state (because the child window is an item from
--         //   the POV of the parent window). See 'Demo->Querying Status (Active/Focused/Hovered etc.)' for details.
--         {
--             static int offset_x = 0;
--             r.ImGui_SetNextItemWidth(100);
--             r.ImGui_DragInt("Offset X", &offset_x, 1.0f, -1000, 1000);
--
--             r.ImGui_SetCursorPosX(r.ImGui_GetCursorPosX() + (float)offset_x);
--             r.ImGui_PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
--             r.ImGui_BeginChild("Red", ImVec2(200, 100), true, ImGuiWindowFlags_None);
--             for (int n = 0; n < 50; n++)
--                 r.ImGui_Text("Some test %d", n);
--             r.ImGui_EndChild();
--             bool child_is_hovered = r.ImGui_IsItemHovered();
--             ImVec2 child_rect_min = r.ImGui_GetItemRectMin();
--             ImVec2 child_rect_max = r.ImGui_GetItemRectMax();
--             r.ImGui_PopStyleColor();
--             r.ImGui_Text("Hovered: %d", child_is_hovered);
--             r.ImGui_Text("Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)", child_rect_min.x, child_rect_min.y, child_rect_max.x, child_rect_max.y);
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Widgets Width"))
--     {
--         // Use SetNextItemWidth() to set the width of a single upcoming item.
--         // Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
--         // In real code use you'll probably want to choose width values that are proportional to your font size
--         // e.g. Using '20.0f * GetFontSize()' as width instead of '200.0f', etc.
--
--         static float f = 0.0f;
--         static bool show_indented_items = true;
--         r.ImGui_Checkbox("Show indented items", &show_indented_items);
--
--         r.ImGui_Text("SetNextItemWidth/PushItemWidth(100)");
--         r.ImGui_SameLine(); HelpMarker("Fixed width.");
--         r.ImGui_PushItemWidth(100);
--         r.ImGui_DragFloat("float##1b", &f);
--         if (show_indented_items)
--         {
--             r.ImGui_Indent();
--             r.ImGui_DragFloat("float (indented)##1b", &f);
--             r.ImGui_Unindent();
--         }
--         r.ImGui_PopItemWidth();
--
--         r.ImGui_Text("SetNextItemWidth/PushItemWidth(-100)");
--         r.ImGui_SameLine(); HelpMarker("Align to right edge minus 100");
--         r.ImGui_PushItemWidth(-100);
--         r.ImGui_DragFloat("float##2a", &f);
--         if (show_indented_items)
--         {
--             r.ImGui_Indent();
--             r.ImGui_DragFloat("float (indented)##2b", &f);
--             r.ImGui_Unindent();
--         }
--         r.ImGui_PopItemWidth();
--
--         r.ImGui_Text("SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5f)");
--         r.ImGui_SameLine(); HelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within a column set)");
--         r.ImGui_PushItemWidth(r.ImGui_GetContentRegionAvail().x * 0.5f);
--         r.ImGui_DragFloat("float##3a", &f);
--         if (show_indented_items)
--         {
--             r.ImGui_Indent();
--             r.ImGui_DragFloat("float (indented)##3b", &f);
--             r.ImGui_Unindent();
--         }
--         r.ImGui_PopItemWidth();
--
--         r.ImGui_Text("SetNextItemWidth/PushItemWidth(-GetContentRegionAvail().x * 0.5f)");
--         r.ImGui_SameLine(); HelpMarker("Align to right edge minus half");
--         r.ImGui_PushItemWidth(-r.ImGui_GetContentRegionAvail().x * 0.5f);
--         r.ImGui_DragFloat("float##4a", &f);
--         if (show_indented_items)
--         {
--             r.ImGui_Indent();
--             r.ImGui_DragFloat("float (indented)##4b", &f);
--             r.ImGui_Unindent();
--         }
--         r.ImGui_PopItemWidth();
--
--         // Demonstrate using PushItemWidth to surround three items.
--         // Calling SetNextItemWidth() before each of them would have the same effect.
--         r.ImGui_Text("SetNextItemWidth/PushItemWidth(-FLT_MIN)");
--         r.ImGui_SameLine(); HelpMarker("Align to right edge");
--         r.ImGui_PushItemWidth(-FLT_MIN);
--         r.ImGui_DragFloat("##float5a", &f);
--         if (show_indented_items)
--         {
--             r.ImGui_Indent();
--             r.ImGui_DragFloat("float (indented)##5b", &f);
--             r.ImGui_Unindent();
--         }
--         r.ImGui_PopItemWidth();
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Basic Horizontal Layout"))
--     {
--         r.ImGui_TextWrapped("(Use r.ImGui_SameLine() to keep adding items to the right of the preceding item)");
--
--         // Text
--         r.ImGui_Text("Two items: Hello"); r.ImGui_SameLine();
--         r.ImGui_TextColored(ImVec4(1,1,0,1), "Sailor");
--
--         // Adjust spacing
--         r.ImGui_Text("More spacing: Hello"); r.ImGui_SameLine(0, 20);
--         r.ImGui_TextColored(ImVec4(1,1,0,1), "Sailor");
--
--         // Button
--         r.ImGui_AlignTextToFramePadding();
--         r.ImGui_Text("Normal buttons"); r.ImGui_SameLine();
--         r.ImGui_Button("Banana"); r.ImGui_SameLine();
--         r.ImGui_Button("Apple"); r.ImGui_SameLine();
--         r.ImGui_Button("Corniflower");
--
--         // Button
--         r.ImGui_Text("Small buttons"); r.ImGui_SameLine();
--         r.ImGui_SmallButton("Like this one"); r.ImGui_SameLine();
--         r.ImGui_Text("can fit within a text block.");
--
--         // Aligned to arbitrary position. Easy/cheap column.
--         r.ImGui_Text("Aligned");
--         r.ImGui_SameLine(150); r.ImGui_Text("x=150");
--         r.ImGui_SameLine(300); r.ImGui_Text("x=300");
--         r.ImGui_Text("Aligned");
--         r.ImGui_SameLine(150); r.ImGui_SmallButton("x=150");
--         r.ImGui_SameLine(300); r.ImGui_SmallButton("x=300");
--
--         // Checkbox
--         static bool c1 = false, c2 = false, c3 = false, c4 = false;
--         r.ImGui_Checkbox("My", &c1); r.ImGui_SameLine();
--         r.ImGui_Checkbox("Tailor", &c2); r.ImGui_SameLine();
--         r.ImGui_Checkbox("Is", &c3); r.ImGui_SameLine();
--         r.ImGui_Checkbox("Rich", &c4);
--
--         // Various
--         static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
--         r.ImGui_PushItemWidth(80);
--         const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
--         static int item = -1;
--         r.ImGui_Combo("Combo", &item, items, IM_ARRAYSIZE(items)); r.ImGui_SameLine();
--         r.ImGui_SliderFloat("X", &f0, 0.0f, 5.0f); r.ImGui_SameLine();
--         r.ImGui_SliderFloat("Y", &f1, 0.0f, 5.0f); r.ImGui_SameLine();
--         r.ImGui_SliderFloat("Z", &f2, 0.0f, 5.0f);
--         r.ImGui_PopItemWidth();
--
--         r.ImGui_PushItemWidth(80);
--         r.ImGui_Text("Lists:");
--         static int selection[4] = { 0, 1, 2, 3 };
--         for (int i = 0; i < 4; i++)
--         {
--             if (i > 0) r.ImGui_SameLine();
--             r.ImGui_PushID(i);
--             r.ImGui_ListBox("", &selection[i], items, IM_ARRAYSIZE(items));
--             r.ImGui_PopID();
--             //if (r.ImGui_IsItemHovered()) r.ImGui_SetTooltip("ListBox %d hovered", i);
--         }
--         r.ImGui_PopItemWidth();
--
--         // Dummy
--         ImVec2 button_sz(40, 40);
--         r.ImGui_Button("A", button_sz); r.ImGui_SameLine();
--         r.ImGui_Dummy(button_sz); r.ImGui_SameLine();
--         r.ImGui_Button("B", button_sz);
--
--         // Manually wrapping
--         // (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
--         r.ImGui_Text("Manually wrapping:");
--         ImGuiStyle& style = r.ImGui_GetStyle();
--         int buttons_count = 20;
--         float window_visible_x2 = r.ImGui_GetWindowPos().x + r.ImGui_GetWindowContentRegionMax().x;
--         for (int n = 0; n < buttons_count; n++)
--         {
--             r.ImGui_PushID(n);
--             r.ImGui_Button("Box", button_sz);
--             float last_button_x2 = r.ImGui_GetItemRectMax().x;
--             float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
--             if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
--                 r.ImGui_SameLine();
--             r.ImGui_PopID();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Groups"))
--     {
--         HelpMarker(
--             "BeginGroup() basically locks the horizontal position for new line. "
--             "EndGroup() bundles the whole group so that you can use \"item\" functions such as "
--             "IsItemHovered()/IsItemActive() or SameLine() etc. on the whole group.");
--         r.ImGui_BeginGroup();
--         {
--             r.ImGui_BeginGroup();
--             r.ImGui_Button("AAA");
--             r.ImGui_SameLine();
--             r.ImGui_Button("BBB");
--             r.ImGui_SameLine();
--             r.ImGui_BeginGroup();
--             r.ImGui_Button("CCC");
--             r.ImGui_Button("DDD");
--             r.ImGui_EndGroup();
--             r.ImGui_SameLine();
--             r.ImGui_Button("EEE");
--             r.ImGui_EndGroup();
--             if (r.ImGui_IsItemHovered())
--                 r.ImGui_SetTooltip("First group hovered");
--         }
--         // Capture the group size and create widgets using the same size
--         ImVec2 size = r.ImGui_GetItemRectSize();
--         const float values[5] = { 0.5f, 0.20f, 0.80f, 0.60f, 0.25f };
--         r.ImGui_PlotHistogram("##values", values, IM_ARRAYSIZE(values), 0, NULL, 0.0f, 1.0f, size);
--
--         r.ImGui_Button("ACTION", ImVec2((size.x - r.ImGui_GetStyle().ItemSpacing.x) * 0.5f, size.y));
--         r.ImGui_SameLine();
--         r.ImGui_Button("REACTION", ImVec2((size.x - r.ImGui_GetStyle().ItemSpacing.x) * 0.5f, size.y));
--         r.ImGui_EndGroup();
--         r.ImGui_SameLine();
--
--         r.ImGui_Button("LEVERAGE\nBUZZWORD", size);
--         r.ImGui_SameLine();
--
--         if (r.ImGui_BeginListBox("List", size))
--         {
--             r.ImGui_Selectable("Selected", true);
--             r.ImGui_Selectable("Not Selected", false);
--             r.ImGui_EndListBox();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Text Baseline Alignment"))
--     {
--         {
--             r.ImGui_BulletText("Text baseline:");
--             r.ImGui_SameLine(); HelpMarker(
--                 "This is testing the vertical alignment that gets applied on text to keep it aligned with widgets. "
--                 "Lines only composed of text or \"small\" widgets use less vertical space than lines with framed widgets.");
--             r.ImGui_Indent();
--
--             r.ImGui_Text("KO Blahblah"); r.ImGui_SameLine();
--             r.ImGui_Button("Some framed item"); r.ImGui_SameLine();
--             HelpMarker("Baseline of button will look misaligned with text..");
--
--             // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
--             // (because we don't know what's coming after the Text() statement, we need to move the text baseline
--             // down by FramePadding.y ahead of time)
--             r.ImGui_AlignTextToFramePadding();
--             r.ImGui_Text("OK Blahblah"); r.ImGui_SameLine();
--             r.ImGui_Button("Some framed item"); r.ImGui_SameLine();
--             HelpMarker("We call AlignTextToFramePadding() to vertically align the text baseline by +FramePadding.y");
--
--             // SmallButton() uses the same vertical padding as Text
--             r.ImGui_Button("TEST##1"); r.ImGui_SameLine();
--             r.ImGui_Text("TEST"); r.ImGui_SameLine();
--             r.ImGui_SmallButton("TEST##2");
--
--             // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
--             r.ImGui_AlignTextToFramePadding();
--             r.ImGui_Text("Text aligned to framed item"); r.ImGui_SameLine();
--             r.ImGui_Button("Item##1"); r.ImGui_SameLine();
--             r.ImGui_Text("Item"); r.ImGui_SameLine();
--             r.ImGui_SmallButton("Item##2"); r.ImGui_SameLine();
--             r.ImGui_Button("Item##3");
--
--             r.ImGui_Unindent();
--         }
--
--         r.ImGui_Spacing();
--
--         {
--             r.ImGui_BulletText("Multi-line text:");
--             r.ImGui_Indent();
--             r.ImGui_Text("One\nTwo\nThree"); r.ImGui_SameLine();
--             r.ImGui_Text("Hello\nWorld"); r.ImGui_SameLine();
--             r.ImGui_Text("Banana");
--
--             r.ImGui_Text("Banana"); r.ImGui_SameLine();
--             r.ImGui_Text("Hello\nWorld"); r.ImGui_SameLine();
--             r.ImGui_Text("One\nTwo\nThree");
--
--             r.ImGui_Button("HOP##1"); r.ImGui_SameLine();
--             r.ImGui_Text("Banana"); r.ImGui_SameLine();
--             r.ImGui_Text("Hello\nWorld"); r.ImGui_SameLine();
--             r.ImGui_Text("Banana");
--
--             r.ImGui_Button("HOP##2"); r.ImGui_SameLine();
--             r.ImGui_Text("Hello\nWorld"); r.ImGui_SameLine();
--             r.ImGui_Text("Banana");
--             r.ImGui_Unindent();
--         }
--
--         r.ImGui_Spacing();
--
--         {
--             r.ImGui_BulletText("Misc items:");
--             r.ImGui_Indent();
--
--             // SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of previous Button.
--             r.ImGui_Button("80x80", ImVec2(80, 80));
--             r.ImGui_SameLine();
--             r.ImGui_Button("50x50", ImVec2(50, 50));
--             r.ImGui_SameLine();
--             r.ImGui_Button("Button()");
--             r.ImGui_SameLine();
--             r.ImGui_SmallButton("SmallButton()");
--
--             // Tree
--             const float spacing = r.ImGui_GetStyle().ItemInnerSpacing.x;
--             r.ImGui_Button("Button##1");
--             r.ImGui_SameLine(0.0f, spacing);
--             if (r.ImGui_TreeNode("Node##1"))
--             {
--                 // Placeholder tree data
--                 for (int i = 0; i < 6; i++)
--                     r.ImGui_BulletText("Item %d..", i);
--                 r.ImGui_TreePop();
--             }
--
--             // Vertically align text node a bit lower so it'll be vertically centered with upcoming widget.
--             // Otherwise you can use SmallButton() (smaller fit).
--             r.ImGui_AlignTextToFramePadding();
--
--             // Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add
--             // other contents below the node.
--             bool node_open = r.ImGui_TreeNode("Node##2");
--             r.ImGui_SameLine(0.0f, spacing); r.ImGui_Button("Button##2");
--             if (node_open)
--             {
--                 // Placeholder tree data
--                 for (int i = 0; i < 6; i++)
--                     r.ImGui_BulletText("Item %d..", i);
--                 r.ImGui_TreePop();
--             }
--
--             // Bullet
--             r.ImGui_Button("Button##3");
--             r.ImGui_SameLine(0.0f, spacing);
--             r.ImGui_BulletText("Bullet text");
--
--             r.ImGui_AlignTextToFramePadding();
--             r.ImGui_BulletText("Node");
--             r.ImGui_SameLine(0.0f, spacing); r.ImGui_Button("Button##4");
--             r.ImGui_Unindent();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Scrolling"))
--     {
--         // Vertical scroll functions
--         HelpMarker("Use SetScrollHereY() or SetScrollFromPosY() to scroll to a given vertical position.");
--
--         static int track_item = 50;
--         static bool enable_track = true;
--         static bool enable_extra_decorations = false;
--         static float scroll_to_off_px = 0.0f;
--         static float scroll_to_pos_px = 200.0f;
--
--         r.ImGui_Checkbox("Decoration", &enable_extra_decorations);
--
--         r.ImGui_Checkbox("Track", &enable_track);
--         r.ImGui_PushItemWidth(100);
--         r.ImGui_SameLine(140); enable_track |= r.ImGui_DragInt("##item", &track_item, 0.25f, 0, 99, "Item = %d");
--
--         bool scroll_to_off = r.ImGui_Button("Scroll Offset");
--         r.ImGui_SameLine(140); scroll_to_off |= r.ImGui_DragFloat("##off", &scroll_to_off_px, 1.00f, 0, FLT_MAX, "+%.0f px");
--
--         bool scroll_to_pos = r.ImGui_Button("Scroll To Pos");
--         r.ImGui_SameLine(140); scroll_to_pos |= r.ImGui_DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10, FLT_MAX, "X/Y = %.0f px");
--         r.ImGui_PopItemWidth();
--
--         if (scroll_to_off || scroll_to_pos)
--             enable_track = false;
--
--         ImGuiStyle& style = r.ImGui_GetStyle();
--         float child_w = (r.ImGui_GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 5;
--         if (child_w < 1.0f)
--             child_w = 1.0f;
--         r.ImGui_PushID("##VerticalScrolling");
--         for (int i = 0; i < 5; i++)
--         {
--             if (i > 0) r.ImGui_SameLine();
--             r.ImGui_BeginGroup();
--             const char* names[] = { "Top", "25%", "Center", "75%", "Bottom" };
--             r.ImGui_TextUnformatted(names[i]);
--
--             const ImGuiWindowFlags child_flags = enable_extra_decorations ? ImGuiWindowFlags_MenuBar : 0;
--             const ImGuiID child_id = r.ImGui_GetID((void*)(intptr_t)i);
--             const bool child_is_visible = r.ImGui_BeginChild(child_id, ImVec2(child_w, 200.0f), true, child_flags);
--             if (r.ImGui_BeginMenuBar())
--             {
--                 r.ImGui_TextUnformatted("abc");
--                 r.ImGui_EndMenuBar();
--             }
--             if (scroll_to_off)
--                 r.ImGui_SetScrollY(scroll_to_off_px);
--             if (scroll_to_pos)
--                 r.ImGui_SetScrollFromPosY(r.ImGui_GetCursorStartPos().y + scroll_to_pos_px, i * 0.25f);
--             if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
--             {
--                 for (int item = 0; item < 100; item++)
--                 {
--                     if (enable_track && item == track_item)
--                     {
--                         r.ImGui_TextColored(ImVec4(1, 1, 0, 1), "Item %d", item);
--                         r.ImGui_SetScrollHereY(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
--                     }
--                     else
--                     {
--                         r.ImGui_Text("Item %d", item);
--                     }
--                 }
--             }
--             float scroll_y = r.ImGui_GetScrollY();
--             float scroll_max_y = r.ImGui_GetScrollMaxY();
--             r.ImGui_EndChild();
--             r.ImGui_Text("%.0f/%.0f", scroll_y, scroll_max_y);
--             r.ImGui_EndGroup();
--         }
--         r.ImGui_PopID();
--
--         // Horizontal scroll functions
--         r.ImGui_Spacing();
--         HelpMarker(
--             "Use SetScrollHereX() or SetScrollFromPosX() to scroll to a given horizontal position.\n\n"
--             "Because the clipping rectangle of most window hides half worth of WindowPadding on the "
--             "left/right, using SetScrollFromPosX(+1) will usually result in clipped text whereas the "
--             "equivalent SetScrollFromPosY(+1) wouldn't.");
--         r.ImGui_PushID("##HorizontalScrolling");
--         for (int i = 0; i < 5; i++)
--         {
--             float child_height = r.ImGui_GetTextLineHeight() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
--             ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar | (enable_extra_decorations ? ImGuiWindowFlags_AlwaysVerticalScrollbar : 0);
--             ImGuiID child_id = r.ImGui_GetID((void*)(intptr_t)i);
--             bool child_is_visible = r.ImGui_BeginChild(child_id, ImVec2(-100, child_height), true, child_flags);
--             if (scroll_to_off)
--                 r.ImGui_SetScrollX(scroll_to_off_px);
--             if (scroll_to_pos)
--                 r.ImGui_SetScrollFromPosX(r.ImGui_GetCursorStartPos().x + scroll_to_pos_px, i * 0.25f);
--             if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
--             {
--                 for (int item = 0; item < 100; item++)
--                 {
--                     if (enable_track && item == track_item)
--                     {
--                         r.ImGui_TextColored(ImVec4(1, 1, 0, 1), "Item %d", item);
--                         r.ImGui_SetScrollHereX(i * 0.25f); // 0.0f:left, 0.5f:center, 1.0f:right
--                     }
--                     else
--                     {
--                         r.ImGui_Text("Item %d", item);
--                     }
--                     r.ImGui_SameLine();
--                 }
--             }
--             float scroll_x = r.ImGui_GetScrollX();
--             float scroll_max_x = r.ImGui_GetScrollMaxX();
--             r.ImGui_EndChild();
--             r.ImGui_SameLine();
--             const char* names[] = { "Left", "25%", "Center", "75%", "Right" };
--             r.ImGui_Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
--             r.ImGui_Spacing();
--         }
--         r.ImGui_PopID();
--
--         // Miscellaneous Horizontal Scrolling Demo
--         HelpMarker(
--             "Horizontal scrolling for a window is enabled via the ImGuiWindowFlags_HorizontalScrollbar flag.\n\n"
--             "You may want to also explicitly specify content width by using SetNextWindowContentWidth() before Begin().");
--         static int lines = 7;
--         r.ImGui_SliderInt("Lines", &lines, 1, 15);
--         r.ImGui_PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
--         r.ImGui_PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
--         ImVec2 scrolling_child_size = ImVec2(0, r.ImGui_GetFrameHeightWithSpacing() * 7 + 30);
--         r.ImGui_BeginChild("scrolling", scrolling_child_size, true, ImGuiWindowFlags_HorizontalScrollbar);
--         for (int line = 0; line < lines; line++)
--         {
--             // Display random stuff. For the sake of this trivial demo we are using basic Button() + SameLine()
--             // If you want to create your own time line for a real application you may be better off manipulating
--             // the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets
--             // yourself. You may also want to use the lower-level ImDrawList API.
--             int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
--             for (int n = 0; n < num_buttons; n++)
--             {
--                 if (n > 0) r.ImGui_SameLine();
--                 r.ImGui_PushID(n + line * 1000);
--                 char num_buf[16];
--                 sprintf(num_buf, "%d", n);
--                 const char* label = (!(n % 15)) ? "FizzBuzz" : (!(n % 3)) ? "Fizz" : (!(n % 5)) ? "Buzz" : num_buf;
--                 float hue = n * 0.05f;
--                 r.ImGui_PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
--                 r.ImGui_PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
--                 r.ImGui_PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
--                 r.ImGui_Button(label, ImVec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
--                 r.ImGui_PopStyleColor(3);
--                 r.ImGui_PopID();
--             }
--         }
--         float scroll_x = r.ImGui_GetScrollX();
--         float scroll_max_x = r.ImGui_GetScrollMaxX();
--         r.ImGui_EndChild();
--         r.ImGui_PopStyleVar(2);
--         float scroll_x_delta = 0.0f;
--         r.ImGui_SmallButton("<<");
--         if (r.ImGui_IsItemActive())
--             scroll_x_delta = -r.ImGui_GetIO().DeltaTime * 1000.0f;
--         r.ImGui_SameLine();
--         r.ImGui_Text("Scroll from code"); r.ImGui_SameLine();
--         r.ImGui_SmallButton(">>");
--         if (r.ImGui_IsItemActive())
--             scroll_x_delta = +r.ImGui_GetIO().DeltaTime * 1000.0f;
--         r.ImGui_SameLine();
--         r.ImGui_Text("%.0f/%.0f", scroll_x, scroll_max_x);
--         if (scroll_x_delta != 0.0f)
--         {
--             // Demonstrate a trick: you can use Begin to set yourself in the context of another window
--             // (here we are already out of your child window)
--             r.ImGui_BeginChild("scrolling");
--             r.ImGui_SetScrollX(r.ImGui_GetScrollX() + scroll_x_delta);
--             r.ImGui_EndChild();
--         }
--         r.ImGui_Spacing();
--
--         static bool show_horizontal_contents_size_demo_window = false;
--         r.ImGui_Checkbox("Show Horizontal contents size demo window", &show_horizontal_contents_size_demo_window);
--
--         if (show_horizontal_contents_size_demo_window)
--         {
--             static bool show_h_scrollbar = true;
--             static bool show_button = true;
--             static bool show_tree_nodes = true;
--             static bool show_text_wrapped = false;
--             static bool show_columns = true;
--             static bool show_tab_bar = true;
--             static bool show_child = false;
--             static bool explicit_content_size = false;
--             static float contents_size_x = 300.0f;
--             if (explicit_content_size)
--                 r.ImGui_SetNextWindowContentSize(ImVec2(contents_size_x, 0.0f));
--             r.ImGui_Begin("Horizontal contents size demo window", &show_horizontal_contents_size_demo_window, show_h_scrollbar ? ImGuiWindowFlags_HorizontalScrollbar : 0);
--             r.ImGui_PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
--             r.ImGui_PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 0));
--             HelpMarker("Test of different widgets react and impact the work rectangle growing when horizontal scrolling is enabled.\n\nUse 'Metrics->Tools->Show windows rectangles' to visualize rectangles.");
--             r.ImGui_Checkbox("H-scrollbar", &show_h_scrollbar);
--             r.ImGui_Checkbox("Button", &show_button);            // Will grow contents size (unless explicitly overwritten)
--             r.ImGui_Checkbox("Tree nodes", &show_tree_nodes);    // Will grow contents size and display highlight over full width
--             r.ImGui_Checkbox("Text wrapped", &show_text_wrapped);// Will grow and use contents size
--             r.ImGui_Checkbox("Columns", &show_columns);          // Will use contents size
--             r.ImGui_Checkbox("Tab bar", &show_tab_bar);          // Will use contents size
--             r.ImGui_Checkbox("Child", &show_child);              // Will grow and use contents size
--             r.ImGui_Checkbox("Explicit content size", &explicit_content_size);
--             r.ImGui_Text("Scroll %.1f/%.1f %.1f/%.1f", r.ImGui_GetScrollX(), r.ImGui_GetScrollMaxX(), r.ImGui_GetScrollY(), r.ImGui_GetScrollMaxY());
--             if (explicit_content_size)
--             {
--                 r.ImGui_SameLine();
--                 r.ImGui_SetNextItemWidth(100);
--                 r.ImGui_DragFloat("##csx", &contents_size_x);
--                 ImVec2 p = r.ImGui_GetCursorScreenPos();
--                 r.ImGui_GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 10, p.y + 10), IM_COL32_WHITE);
--                 r.ImGui_GetWindowDrawList()->AddRectFilled(ImVec2(p.x + contents_size_x - 10, p.y), ImVec2(p.x + contents_size_x, p.y + 10), IM_COL32_WHITE);
--                 r.ImGui_Dummy(ImVec2(0, 10));
--             }
--             r.ImGui_PopStyleVar(2);
--             r.ImGui_Separator();
--             if (show_button)
--             {
--                 r.ImGui_Button("this is a 300-wide button", ImVec2(300, 0));
--             }
--             if (show_tree_nodes)
--             {
--                 bool open = true;
--                 if (r.ImGui_TreeNode("this is a tree node"))
--                 {
--                     if (r.ImGui_TreeNode("another one of those tree node..."))
--                     {
--                         r.ImGui_Text("Some tree contents");
--                         r.ImGui_TreePop();
--                     }
--                     r.ImGui_TreePop();
--                 }
--                 r.ImGui_CollapsingHeader("CollapsingHeader", &open);
--             }
--             if (show_text_wrapped)
--             {
--                 r.ImGui_TextWrapped("This text should automatically wrap on the edge of the work rectangle.");
--             }
--             if (show_columns)
--             {
--                 r.ImGui_Text("Tables:");
--                 if (r.ImGui_BeginTable("table", 4, ImGuiTableFlags_Borders))
--                 {
--                     for (int n = 0; n < 4; n++)
--                     {
--                         r.ImGui_TableNextColumn();
--                         r.ImGui_Text("Width %.2f", r.ImGui_GetContentRegionAvail().x);
--                     }
--                     r.ImGui_EndTable();
--                 }
--                 r.ImGui_Text("Columns:");
--                 r.ImGui_Columns(4);
--                 for (int n = 0; n < 4; n++)
--                 {
--                     r.ImGui_Text("Width %.2f", r.ImGui_GetColumnWidth());
--                     r.ImGui_NextColumn();
--                 }
--                 r.ImGui_Columns(1);
--             }
--             if (show_tab_bar && r.ImGui_BeginTabBar("Hello"))
--             {
--                 if (r.ImGui_BeginTabItem("OneOneOne")) { r.ImGui_EndTabItem(); }
--                 if (r.ImGui_BeginTabItem("TwoTwoTwo")) { r.ImGui_EndTabItem(); }
--                 if (r.ImGui_BeginTabItem("ThreeThreeThree")) { r.ImGui_EndTabItem(); }
--                 if (r.ImGui_BeginTabItem("FourFourFour")) { r.ImGui_EndTabItem(); }
--                 r.ImGui_EndTabBar();
--             }
--             if (show_child)
--             {
--                 r.ImGui_BeginChild("child", ImVec2(0, 0), true);
--                 r.ImGui_EndChild();
--             }
--             r.ImGui_End();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Clipping"))
--     {
--         static ImVec2 size(100.0f, 100.0f);
--         static ImVec2 offset(30.0f, 30.0f);
--         r.ImGui_DragFloat2("size", (float*)&size, 0.5f, 1.0f, 200.0f, "%.0f");
--         r.ImGui_TextWrapped("(Click and drag to scroll)");
--
--         for (int n = 0; n < 3; n++)
--         {
--             if (n > 0)
--                 r.ImGui_SameLine();
--             r.ImGui_PushID(n);
--             r.ImGui_BeginGroup(); // Lock X position
--
--             r.ImGui_InvisibleButton("##empty", size);
--             if (r.ImGui_IsItemActive() && r.ImGui_IsMouseDragging(ImGuiMouseButton_Left))
--             {
--                 offset.x += r.ImGui_GetIO().MouseDelta.x;
--                 offset.y += r.ImGui_GetIO().MouseDelta.y;
--             }
--             const ImVec2 p0 = r.ImGui_GetItemRectMin();
--             const ImVec2 p1 = r.ImGui_GetItemRectMax();
--             const char* text_str = "Line 1 hello\nLine 2 clip me!";
--             const ImVec2 text_pos = ImVec2(p0.x + offset.x, p0.y + offset.y);
--             ImDrawList* draw_list = r.ImGui_GetWindowDrawList();
--
--             switch (n)
--             {
--             case 0:
--                 HelpMarker(
--                     "Using r.ImGui_PushClipRect():\n"
--                     "Will alter ImGui hit-testing logic + ImDrawList rendering.\n"
--                     "(use this if you want your clipping rectangle to affect interactions)");
--                 r.ImGui_PushClipRect(p0, p1, true);
--                 draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
--                 draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);
--                 r.ImGui_PopClipRect();
--                 break;
--             case 1:
--                 HelpMarker(
--                     "Using ImDrawList::PushClipRect():\n"
--                     "Will alter ImDrawList rendering only.\n"
--                     "(use this as a shortcut if you are only using ImDrawList calls)");
--                 draw_list->PushClipRect(p0, p1, true);
--                 draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
--                 draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);
--                 draw_list->PopClipRect();
--                 break;
--             case 2:
--                 HelpMarker(
--                     "Using ImDrawList::AddText() with a fine ClipRect:\n"
--                     "Will alter only this specific ImDrawList::AddText() rendering.\n"
--                     "(this is often used internally to avoid altering the clipping rectangle and minimize draw calls)");
--                 ImVec4 clip_rect(p0.x, p0.y, p1.x, p1.y); // AddText() takes a ImVec4* here so let's convert.
--                 draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
--                 draw_list->AddText(r.ImGui_GetFont(), r.ImGui_GetFontSize(), text_pos, IM_COL32_WHITE, text_str, NULL, 0.0f, &clip_rect);
--                 break;
--             }
--             r.ImGui_EndGroup();
--             r.ImGui_PopID();
--         }
--
--         r.ImGui_TreePop();
--     }
-- }
--
-- static void ShowDemoWindowPopups()
-- {
--     if (!r.ImGui_CollapsingHeader("Popups & Modal windows"))
--         return;
--
--     // The properties of popups windows are:
--     // - They block normal mouse hovering detection outside them. (*)
--     // - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
--     // - Their visibility state (~bool) is held internally by Dear ImGui instead of being held by the programmer as
--     //   we are used to with regular Begin() calls. User can manipulate the visibility state by calling OpenPopup().
--     // (*) One can use IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) to bypass it and detect hovering even
--     //     when normally blocked by a popup.
--     // Those three properties are connected. The library needs to hold their visibility state BECAUSE it can close
--     // popups at any time.
--
--     // Typical use for regular windows:
--     //   bool my_tool_is_active = false; if (r.ImGui_Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
--     // Typical use for popups:
--     //   if (r.ImGui_Button("Open")) r.ImGui_OpenPopup("MyPopup"); if (r.ImGui_BeginPopup("MyPopup") { [...] EndPopup(); }
--
--     // With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
--     // This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.
--
--     if (r.ImGui_TreeNode("Popups"))
--     {
--         r.ImGui_TextWrapped(
--             "When a popup is active, it inhibits interacting with windows that are behind the popup. "
--             "Clicking outside the popup closes it.");
--
--         static int selected_fish = -1;
--         const char* names[] = { "Bream", "Haddock", "Mackerel", "Pollock", "Tilefish" };
--         static bool toggles[] = { true, false, false, false, false };
--
--         // Simple selection popup (if you want to show the current selection inside the Button itself,
--         // you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
--         if (r.ImGui_Button("Select.."))
--             r.ImGui_OpenPopup("my_select_popup");
--         r.ImGui_SameLine();
--         r.ImGui_TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
--         if (r.ImGui_BeginPopup("my_select_popup"))
--         {
--             r.ImGui_Text("Aquarium");
--             r.ImGui_Separator();
--             for (int i = 0; i < IM_ARRAYSIZE(names); i++)
--                 if (r.ImGui_Selectable(names[i]))
--                     selected_fish = i;
--             r.ImGui_EndPopup();
--         }
--
--         // Showing a menu with toggles
--         if (r.ImGui_Button("Toggle.."))
--             r.ImGui_OpenPopup("my_toggle_popup");
--         if (r.ImGui_BeginPopup("my_toggle_popup"))
--         {
--             for (int i = 0; i < IM_ARRAYSIZE(names); i++)
--                 r.ImGui_MenuItem(names[i], "", &toggles[i]);
--             if (r.ImGui_BeginMenu("Sub-menu"))
--             {
--                 r.ImGui_MenuItem("Click me");
--                 r.ImGui_EndMenu();
--             }
--
--             r.ImGui_Separator();
--             r.ImGui_Text("Tooltip here");
--             if (r.ImGui_IsItemHovered())
--                 r.ImGui_SetTooltip("I am a tooltip over a popup");
--
--             if (r.ImGui_Button("Stacked Popup"))
--                 r.ImGui_OpenPopup("another popup");
--             if (r.ImGui_BeginPopup("another popup"))
--             {
--                 for (int i = 0; i < IM_ARRAYSIZE(names); i++)
--                     r.ImGui_MenuItem(names[i], "", &toggles[i]);
--                 if (r.ImGui_BeginMenu("Sub-menu"))
--                 {
--                     r.ImGui_MenuItem("Click me");
--                     if (r.ImGui_Button("Stacked Popup"))
--                         r.ImGui_OpenPopup("another popup");
--                     if (r.ImGui_BeginPopup("another popup"))
--                     {
--                         r.ImGui_Text("I am the last one here.");
--                         r.ImGui_EndPopup();
--                     }
--                     r.ImGui_EndMenu();
--                 }
--                 r.ImGui_EndPopup();
--             }
--             r.ImGui_EndPopup();
--         }
--
--         // Call the more complete ShowExampleMenuFile which we use in various places of this demo
--         if (r.ImGui_Button("File Menu.."))
--             r.ImGui_OpenPopup("my_file_popup");
--         if (r.ImGui_BeginPopup("my_file_popup"))
--         {
--             ShowExampleMenuFile();
--             r.ImGui_EndPopup();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Context menus"))
--     {
--         // BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
--         //    if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
--         //       OpenPopup(id);
--         //    return BeginPopup(id);
--         // For more advanced uses you may want to replicate and customize this code.
--         // See details in BeginPopupContextItem().
--         static float value = 0.5f;
--         r.ImGui_Text("Value = %.3f (<-- right-click here)", value);
--         if (r.ImGui_BeginPopupContextItem("item context menu"))
--         {
--             if (r.ImGui_Selectable("Set to zero")) value = 0.0f;
--             if (r.ImGui_Selectable("Set to PI")) value = 3.1415f;
--             r.ImGui_SetNextItemWidth(-1); // -FLT_MIN in v1.81?
--             r.ImGui_DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
--             r.ImGui_EndPopup();
--         }
--
--         // We can also use OpenPopupOnItemClick() which is the same as BeginPopupContextItem() but without the
--         // Begin() call. So here we will make it that clicking on the text field with the right mouse button (1)
--         // will toggle the visibility of the popup above.
--         r.ImGui_Text("(You can also right-click me to open the same popup as above.)");
--         r.ImGui_OpenPopupOnItemClick("item context menu", 1);
--
--         // When used after an item that has an ID (e.g.Button), we can skip providing an ID to BeginPopupContextItem().
--         // BeginPopupContextItem() will use the last item ID as the popup ID.
--         // In addition here, we want to include your editable label inside the button label.
--         // We use the ### operator to override the ID (read FAQ about ID for details)
--         static char name[32] = "Label1";
--         char buf[64];
--         sprintf(buf, "Button: %s###Button", name); // ### operator override ID ignoring the preceding label
--         r.ImGui_Button(buf);
--         if (r.ImGui_BeginPopupContextItem())
--         {
--             r.ImGui_Text("Edit name:");
--             r.ImGui_InputText("##edit", name, IM_ARRAYSIZE(name));
--             if (r.ImGui_Button("Close"))
--                 r.ImGui_CloseCurrentPopup();
--             r.ImGui_EndPopup();
--         }
--         r.ImGui_SameLine(); r.ImGui_Text("(<-- right-click here)");
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Modals"))
--     {
--         r.ImGui_TextWrapped("Modal windows are like popups but the user cannot close them by clicking outside.");
--
--         if (r.ImGui_Button("Delete.."))
--             r.ImGui_OpenPopup("Delete?");
--
--         // Always center this window when appearing
--         //v1.80 ImVec2 center(r.ImGui_GetIO().DisplaySize.x * 0.5f, r.ImGui_GetIO().DisplaySize.y * 0.5f);
--         ImVec2 center = ImGui::GetMainViewport()->GetCenter();
--         r.ImGui_SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
--
--         if (r.ImGui_BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
--         {
--             r.ImGui_Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
--             r.ImGui_Separator();
--
--             //static int unused_i = 0;
--             //r.ImGui_Combo("Combo", &unused_i, "Delete\0Delete harder\0");
--
--             static bool dont_ask_me_next_time = false;
--             r.ImGui_PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
--             r.ImGui_Checkbox("Don't ask me next time", &dont_ask_me_next_time);
--             r.ImGui_PopStyleVar();
--
--             if (r.ImGui_Button("OK", ImVec2(120, 0))) { r.ImGui_CloseCurrentPopup(); }
--             r.ImGui_SetItemDefaultFocus();
--             r.ImGui_SameLine();
--             if (r.ImGui_Button("Cancel", ImVec2(120, 0))) { r.ImGui_CloseCurrentPopup(); }
--             r.ImGui_EndPopup();
--         }
--
--         if (r.ImGui_Button("Stacked modals.."))
--             r.ImGui_OpenPopup("Stacked 1");
--         if (r.ImGui_BeginPopupModal("Stacked 1", NULL, ImGuiWindowFlags_MenuBar))
--         {
--             if (r.ImGui_BeginMenuBar())
--             {
--                 if (r.ImGui_BeginMenu("File"))
--                 {
--                     if (r.ImGui_MenuItem("Some menu item")) {}
--                     r.ImGui_EndMenu();
--                 }
--                 r.ImGui_EndMenuBar();
--             }
--             r.ImGui_Text("Hello from Stacked The First\nUsing style.Colors[ImGuiCol_ModalWindowDimBg] behind it.");
--
--             // Testing behavior of widgets stacking their own regular popups over the modal.
--             static int item = 1;
--             static float color[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
--             r.ImGui_Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
--             r.ImGui_ColorEdit4("color", color);
--
--             if (r.ImGui_Button("Add another modal.."))
--                 r.ImGui_OpenPopup("Stacked 2");
--
--             // Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close button which
--             // will close the popup. Note that the visibility state of popups is owned by imgui, so the input value
--             // of the bool actually doesn't matter here.
--             bool unused_open = true;
--             if (r.ImGui_BeginPopupModal("Stacked 2", &unused_open))
--             {
--                 r.ImGui_Text("Hello from Stacked The Second!");
--                 if (r.ImGui_Button("Close"))
--                     r.ImGui_CloseCurrentPopup();
--                 r.ImGui_EndPopup();
--             }
--
--             if (r.ImGui_Button("Close"))
--                 r.ImGui_CloseCurrentPopup();
--             r.ImGui_EndPopup();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (r.ImGui_TreeNode("Menus inside a regular window"))
--     {
--         r.ImGui_TextWrapped("Below we are testing adding menu items to a regular window. It's rather unusual but should work!");
--         r.ImGui_Separator();
--
--         // Note: As a quirk in this very specific example, we want to differentiate the parent of this menu from the
--         // parent of the various popup menus above. To do so we are encloding the items in a PushID()/PopID() block
--         // to make them two different menusets. If we don't, opening any popup above and hovering our menu here would
--         // open it. This is because once a menu is active, we allow to switch to a sibling menu by just hovering on it,
--         // which is the desired behavior for regular menus.
--         r.ImGui_PushID("foo");
--         r.ImGui_MenuItem("Menu item", "CTRL+M");
--         if (r.ImGui_BeginMenu("Menu inside a regular window"))
--         {
--             ShowExampleMenuFile();
--             r.ImGui_EndMenu();
--         }
--         r.ImGui_PopID();
--         r.ImGui_Separator();
--         r.ImGui_TreePop();
--     }
-- }
--
-- // Dummy data structure that we use for the Table demo.
-- // (pre-C++11 doesn't allow us to instantiate ImVector<MyItem> template if this structure if defined inside the demo function)
-- namespace
-- {
-- // We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
-- // This identifier will be passed down into ImGuiTableSortSpec::ColumnUserID.
-- // But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (ImGuiTableSortSpec::ColumnIndex)
-- // If you don't use sorting, you will generally never care about giving column an ID!
-- enum MyItemColumnID
-- {
--     MyItemColumnID_ID,
--     MyItemColumnID_Name,
--     MyItemColumnID_Action,
--     MyItemColumnID_Quantity,
--     MyItemColumnID_Description
-- };
--
-- struct MyItem
-- {
--     int         ID;
--     const char* Name;
--     int         Quantity;
--
--     // We have a problem which is affecting _only this demo_ and should not affect your code:
--     // As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to qsort(),
--     // however qsort doesn't allow passing user data to comparing function.
--     // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
--     // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
--     // We could technically call r.ImGui_TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
--     // very often by the sorting algorithm it would be a little wasteful.
--     static const ImGuiTableSortSpecs* s_current_sort_specs;
--
--     // Compare function to be used by qsort()
--     static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
--     {
--         const MyItem* a = (const MyItem*)lhs;
--         const MyItem* b = (const MyItem*)rhs;
--         for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
--         {
--             // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
--             // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
--             const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
--             int delta = 0;
--             switch (sort_spec->ColumnUserID)
--             {
--             case MyItemColumnID_ID:             delta = (a->ID - b->ID);                break;
--             case MyItemColumnID_Name:           delta = (strcmp(a->Name, b->Name));     break;
--             case MyItemColumnID_Quantity:       delta = (a->Quantity - b->Quantity);    break;
--             case MyItemColumnID_Description:    delta = (strcmp(a->Name, b->Name));     break;
--             default: IM_ASSERT(0); break;
--             }
--             if (delta > 0)
--                 return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
--             if (delta < 0)
--                 return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
--         }
--
--         // qsort() is instable so always return a way to differenciate items.
--         // Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
--         return (a->ID - b->ID);
--     }
-- };
-- const ImGuiTableSortSpecs* MyItem::s_current_sort_specs = NULL;
-- }
--
-- // Make the UI compact because there are so many fields
-- static void PushStyleCompact()
-- {
--     ImGuiStyle& style = r.ImGui_GetStyle();
--     r.ImGui_PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
--     r.ImGui_PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
-- }
--
-- static void PopStyleCompact()
-- {
--     r.ImGui_PopStyleVar(2);
-- }
--
-- // Show a combo box with a choice of sizing policies
-- static void EditTableSizingFlags(ImGuiTableFlags* p_flags)
-- {
--     struct EnumDesc { ImGuiTableFlags Value; const char* Name; const char* Tooltip; };
--     static const EnumDesc policies[] =
--     {
--         { ImGuiTableFlags_None,               "Default",                            "Use default sizing policy:\n- ImGuiTableFlags_SizingFixedFit if ScrollX is on or if host window has ImGuiWindowFlags_AlwaysAutoResize.\n- ImGuiTableFlags_SizingStretchSame otherwise." },
--         { ImGuiTableFlags_SizingFixedFit,     "ImGuiTableFlags_SizingFixedFit",     "Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching contents width." },
--         { ImGuiTableFlags_SizingFixedSame,    "ImGuiTableFlags_SizingFixedSame",    "Columns are all the same width, matching the maximum contents width.\nImplicitly disable ImGuiTableFlags_Resizable and enable ImGuiTableFlags_NoKeepColumnsVisible." },
--         { ImGuiTableFlags_SizingStretchProp,  "ImGuiTableFlags_SizingStretchProp",  "Columns default to _WidthStretch with weights proportional to their widths." },
--         { ImGuiTableFlags_SizingStretchSame,  "ImGuiTableFlags_SizingStretchSame",  "Columns default to _WidthStretch with same weights." }
--     };
--     int idx;
--     for (idx = 0; idx < IM_ARRAYSIZE(policies); idx++)
--         if (policies[idx].Value == (*p_flags & ImGuiTableFlags_SizingMask_))
--             break;
--     const char* preview_text = (idx < IM_ARRAYSIZE(policies)) ? policies[idx].Name + (idx > 0 ? strlen("ImGuiTableFlags") : 0) : "";
--     if (r.ImGui_BeginCombo("Sizing Policy", preview_text))
--     {
--         for (int n = 0; n < IM_ARRAYSIZE(policies); n++)
--             if (r.ImGui_Selectable(policies[n].Name, idx == n))
--                 *p_flags = (*p_flags & ~ImGuiTableFlags_SizingMask_) | policies[n].Value;
--         r.ImGui_EndCombo();
--     }
--     r.ImGui_SameLine();
--     r.ImGui_TextDisabled("(?)");
--     if (r.ImGui_IsItemHovered())
--     {
--         r.ImGui_BeginTooltip();
--         r.ImGui_PushTextWrapPos(r.ImGui_GetFontSize() * 50.0f);
--         for (int m = 0; m < IM_ARRAYSIZE(policies); m++)
--         {
--             r.ImGui_Separator();
--             r.ImGui_Text("%s:", policies[m].Name);
--             r.ImGui_Separator();
--             r.ImGui_SetCursorPosX(r.ImGui_GetCursorPosX() + r.ImGui_GetStyle().IndentSpacing * 0.5f);
--             r.ImGui_TextUnformatted(policies[m].Tooltip);
--         }
--         r.ImGui_PopTextWrapPos();
--         r.ImGui_EndTooltip();
--     }
-- }
--
-- static void EditTableColumnsFlags(ImGuiTableColumnFlags* p_flags)
-- {
--     r.ImGui_CheckboxFlags("_DefaultHide", p_flags, ImGuiTableColumnFlags_DefaultHide);
--     r.ImGui_CheckboxFlags("_DefaultSort", p_flags, ImGuiTableColumnFlags_DefaultSort);
--     if (r.ImGui_CheckboxFlags("_WidthStretch", p_flags, ImGuiTableColumnFlags_WidthStretch))
--         *p_flags &= ~(ImGuiTableColumnFlags_WidthMask_ ^ ImGuiTableColumnFlags_WidthStretch);
--     if (r.ImGui_CheckboxFlags("_WidthFixed", p_flags, ImGuiTableColumnFlags_WidthFixed))
--         *p_flags &= ~(ImGuiTableColumnFlags_WidthMask_ ^ ImGuiTableColumnFlags_WidthFixed);
--     r.ImGui_CheckboxFlags("_NoResize", p_flags, ImGuiTableColumnFlags_NoResize);
--     r.ImGui_CheckboxFlags("_NoReorder", p_flags, ImGuiTableColumnFlags_NoReorder);
--     r.ImGui_CheckboxFlags("_NoHide", p_flags, ImGuiTableColumnFlags_NoHide);
--     r.ImGui_CheckboxFlags("_NoClip", p_flags, ImGuiTableColumnFlags_NoClip);
--     r.ImGui_CheckboxFlags("_NoSort", p_flags, ImGuiTableColumnFlags_NoSort);
--     r.ImGui_CheckboxFlags("_NoSortAscending", p_flags, ImGuiTableColumnFlags_NoSortAscending);
--     r.ImGui_CheckboxFlags("_NoSortDescending", p_flags, ImGuiTableColumnFlags_NoSortDescending);
--     r.ImGui_CheckboxFlags("_NoHeaderWidth", p_flags, ImGuiTableColumnFlags_NoHeaderWidth);
--     r.ImGui_CheckboxFlags("_PreferSortAscending", p_flags, ImGuiTableColumnFlags_PreferSortAscending);
--     r.ImGui_CheckboxFlags("_PreferSortDescending", p_flags, ImGuiTableColumnFlags_PreferSortDescending);
--     r.ImGui_CheckboxFlags("_IndentEnable", p_flags, ImGuiTableColumnFlags_IndentEnable); r.ImGui_SameLine(); HelpMarker("Default for column 0");
--     r.ImGui_CheckboxFlags("_IndentDisable", p_flags, ImGuiTableColumnFlags_IndentDisable); r.ImGui_SameLine(); HelpMarker("Default for column >0");
-- }
--
-- static void ShowTableColumnsStatusFlags(ImGuiTableColumnFlags flags)
-- {
--     r.ImGui_CheckboxFlags("_IsEnabled", &flags, ImGuiTableColumnFlags_IsEnabled);
--     r.ImGui_CheckboxFlags("_IsVisible", &flags, ImGuiTableColumnFlags_IsVisible);
--     r.ImGui_CheckboxFlags("_IsSorted", &flags, ImGuiTableColumnFlags_IsSorted);
--     r.ImGui_CheckboxFlags("_IsHovered", &flags, ImGuiTableColumnFlags_IsHovered);
-- }
--
-- static void ShowDemoWindowTables()
-- {
--     //r.ImGui_SetNextItemOpen(true, ImGuiCond_Once);
--     if (!r.ImGui_CollapsingHeader("Tables & Columns"))
--         return;
--
--     // Using those as a base value to create width/height that are factor of the size of our font
--     const float TEXT_BASE_WIDTH = r.ImGui_CalcTextSize("A").x;
--     const float TEXT_BASE_HEIGHT = r.ImGui_GetTextLineHeightWithSpacing();
--
--     r.ImGui_PushID("Tables");
--
--     int open_action = -1;
--     if (r.ImGui_Button("Open all"))
--         open_action = 1;
--     r.ImGui_SameLine();
--     if (r.ImGui_Button("Close all"))
--         open_action = 0;
--     r.ImGui_SameLine();
--
--     // Options
--     static bool disable_indent = false;
--     r.ImGui_Checkbox("Disable tree indentation", &disable_indent);
--     r.ImGui_SameLine();
--     HelpMarker("Disable the indenting of tree nodes so demo tables can use the full window width.");
--     r.ImGui_Separator();
--     if (disable_indent)
--         r.ImGui_PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
--
--     // About Styling of tables
--     // Most settings are configured on a per-table basis via the flags passed to BeginTable() and TableSetupColumns APIs.
--     // There are however a few settings that a shared and part of the ImGuiStyle structure:
--     //   style.CellPadding                          // Padding within each cell
--     //   style.Colors[ImGuiCol_TableHeaderBg]       // Table header background
--     //   style.Colors[ImGuiCol_TableBorderStrong]   // Table outer and header borders
--     //   style.Colors[ImGuiCol_TableBorderLight]    // Table inner borders
--     //   style.Colors[ImGuiCol_TableRowBg]          // Table row background when ImGuiTableFlags_RowBg is enabled (even rows)
--     //   style.Colors[ImGuiCol_TableRowBgAlt]       // Table row background when ImGuiTableFlags_RowBg is enabled (odds rows)
--
--     // Demos
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Basic"))
--     {
--         // Here we will showcase three different ways to output a table.
--         // They are very simple variations of a same thing!
--
--         // [Method 1] Using TableNextRow() to create a new row, and TableSetColumnIndex() to select the column.
--         // In many situations, this is the most flexible and easy to use pattern.
--         HelpMarker("Using TableNextRow() + calling TableSetColumnIndex() _before_ each cell, in a loop.");
--         if (r.ImGui_BeginTable("table1", 3))
--         {
--             for (int row = 0; row < 4; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("Row %d Column %d", row, column);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--
--         // [Method 2] Using TableNextColumn() called multiple times, instead of using a for loop + TableSetColumnIndex().
--         // This is generally more convenient when you have code manually submitting the contents of each columns.
--         HelpMarker("Using TableNextRow() + calling TableNextColumn() _before_ each cell, manually.");
--         if (r.ImGui_BeginTable("table2", 3))
--         {
--             for (int row = 0; row < 4; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 r.ImGui_TableNextColumn();
--                 r.ImGui_Text("Row %d", row);
--                 r.ImGui_TableNextColumn();
--                 r.ImGui_Text("Some contents");
--                 r.ImGui_TableNextColumn();
--                 r.ImGui_Text("123.456");
--             }
--             r.ImGui_EndTable();
--         }
--
--         // [Method 3] We call TableNextColumn() _before_ each cell. We never call TableNextRow(),
--         // as TableNextColumn() will automatically wrap around and create new roes as needed.
--         // This is generally more convenient when your cells all contains the same type of data.
--         HelpMarker(
--             "Only using TableNextColumn(), which tends to be convenient for tables where every cells contains the same type of contents.\n"
--             "This is also more similar to the old NextColumn() function of the Columns API, and provided to facilitate the Columns->Tables API transition.");
--         if (r.ImGui_BeginTable("table3", 3))
--         {
--             for (int item = 0; item < 14; item++)
--             {
--                 r.ImGui_TableNextColumn();
--                 r.ImGui_Text("Item %d", item);
--             }
--             r.ImGui_EndTable();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Borders, background"))
--     {
--         // Expose a few Borders related flags interactively
--         enum ContentsType { CT_Text, CT_FillButton };
--         static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
--         static bool display_headers = false;
--         static int contents_type = CT_Text;
--
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Borders", &flags, ImGuiTableFlags_Borders);
--         r.ImGui_SameLine(); HelpMarker("ImGuiTableFlags_Borders\n = ImGuiTableFlags_BordersInnerV\n | ImGuiTableFlags_BordersOuterV\n | ImGuiTableFlags_BordersInnerV\n | ImGuiTableFlags_BordersOuterH");
--         r.ImGui_Indent();
--
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersH", &flags, ImGuiTableFlags_BordersH);
--         r.ImGui_Indent();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuterH", &flags, ImGuiTableFlags_BordersOuterH);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInnerH", &flags, ImGuiTableFlags_BordersInnerH);
--         r.ImGui_Unindent();
--
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
--         r.ImGui_Indent();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags, ImGuiTableFlags_BordersOuterV);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags, ImGuiTableFlags_BordersInnerV);
--         r.ImGui_Unindent();
--
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuter", &flags, ImGuiTableFlags_BordersOuter);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInner", &flags, ImGuiTableFlags_BordersInner);
--         r.ImGui_Unindent();
--
--         r.ImGui_AlignTextToFramePadding(); r.ImGui_Text("Cell contents:");
--         r.ImGui_SameLine(); r.ImGui_RadioButton("Text", &contents_type, CT_Text);
--         r.ImGui_SameLine(); r.ImGui_RadioButton("FillButton", &contents_type, CT_FillButton);
--         r.ImGui_Checkbox("Display headers", &display_headers);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody); r.ImGui_SameLine(); HelpMarker("Disable vertical borders in columns Body (borders will always appears in Headers");
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table1", 3, flags))
--         {
--             // Display headers so we can inspect their interaction with borders.
--             // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
--             if (display_headers)
--             {
--                 r.ImGui_TableSetupColumn("One");
--                 r.ImGui_TableSetupColumn("Two");
--                 r.ImGui_TableSetupColumn("Three");
--                 r.ImGui_TableHeadersRow();
--             }
--
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     char buf[32];
--                     sprintf(buf, "Hello %d,%d", column, row);
--                     if (contents_type == CT_Text)
--                         r.ImGui_TextUnformatted(buf);
--                     else if (contents_type)
--                         r.ImGui_Button(buf, ImVec2(-FLT_MIN, 0.0f));
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Resizable, stretch"))
--     {
--         // By default, if we don't enable ScrollX the sizing policy for each columns is "Stretch"
--         // Each columns maintain a sizing weight, and they will occupy all available width.
--         static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
--         r.ImGui_SameLine(); HelpMarker("Using the _Resizable flag automatically enables the _BordersInnerV flag as well, this is why the resize borders are still showing when unchecking this.");
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table1", 3, flags))
--         {
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("Hello %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Resizable, fixed"))
--     {
--         // Here we use ImGuiTableFlags_SizingFixedFit (even though _ScrollX is not set)
--         // So columns will adopt the "Fixed" policy and will maintain a fixed width regardless of the whole available width (unless table is small)
--         // If there is not enough available width to fit all columns, they will however be resized down.
--         // FIXME-TABLE: Providing a stretch-on-init would make sense especially for tables which don't have saved settings
--         HelpMarker(
--             "Using _Resizable + _SizingFixedFit flags.\n"
--             "Fixed-width columns generally makes more sense if you want to use horizontal scrolling.\n\n"
--             "Double-click a column border to auto-fit the column to its contents.");
--         PushStyleCompact();
--         static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table1", 3, flags))
--         {
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("Hello %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Resizable, mixed"))
--     {
--         HelpMarker(
--             "Using TableSetupColumn() to alter resizing policy on a per-column basis.\n\n"
--             "When combining Fixed and Stretch columns, generally you only want one, maybe two trailing columns to use _WidthStretch.");
--         static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
--
--         if (r.ImGui_BeginTable("table1", 3, flags))
--         {
--             r.ImGui_TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed);
--             r.ImGui_TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed);
--             r.ImGui_TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthStretch);
--             r.ImGui_TableHeadersRow();
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("%s %d,%d", (column == 2) ? "Stretch" : "Fixed", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         if (r.ImGui_BeginTable("table2", 6, flags))
--         {
--             r.ImGui_TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed);
--             r.ImGui_TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed);
--             r.ImGui_TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide);
--             r.ImGui_TableSetupColumn("DDD", ImGuiTableColumnFlags_WidthStretch);
--             r.ImGui_TableSetupColumn("EEE", ImGuiTableColumnFlags_WidthStretch);
--             r.ImGui_TableSetupColumn("FFF", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide);
--             r.ImGui_TableHeadersRow();
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 6; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("%s %d,%d", (column >= 3) ? "Stretch" : "Fixed", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Reorderable, hideable, with headers"))
--     {
--         HelpMarker(
--             "Click and drag column headers to reorder columns.\n\n"
--             "Right-click on a header to open a context menu.");
--         static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Reorderable", &flags, ImGuiTableFlags_Reorderable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Hideable", &flags, ImGuiTableFlags_Hideable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags, ImGuiTableFlags_NoBordersInBodyUntilResize); r.ImGui_SameLine(); HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appears in Headers)");
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table1", 3, flags))
--         {
--             // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
--             // (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
--             r.ImGui_TableSetupColumn("One");
--             r.ImGui_TableSetupColumn("Two");
--             r.ImGui_TableSetupColumn("Three");
--             r.ImGui_TableHeadersRow();
--             for (int row = 0; row < 6; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("Hello %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--
--         // Use outer_size.x == 0.0f instead of default to make the table as tight as possible (only valid when no scrolling and no stretch column)
--         if (r.ImGui_BeginTable("table2", 3, flags | ImGuiTableFlags_SizingFixedFit, ImVec2(0.0f, 0.0f)))
--         {
--             r.ImGui_TableSetupColumn("One");
--             r.ImGui_TableSetupColumn("Two");
--             r.ImGui_TableSetupColumn("Three");
--             r.ImGui_TableHeadersRow();
--             for (int row = 0; row < 6; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("Fixed %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Padding"))
--     {
--         // First example: showcase use of padding flags and effect of BorderOuterV/BorderInnerV on X padding.
--         // We don't expose BorderOuterH/BorderInnerH here because they have no effect on X padding.
--         HelpMarker(
--             "We often want outer padding activated when any using features which makes the edges of a column visible:\n"
--             "e.g.:\n"
--             "- BorderOuterV\n"
--             "- any form of row selection\n"
--             "Because of this, activating BorderOuterV sets the default to PadOuterX. Using PadOuterX or NoPadOuterX you can override the default.\n\n"
--             "Actual padding values are using style.CellPadding.\n\n"
--             "In this demo we don't show horizontal borders to emphasis how they don't affect default horizontal padding.");
--
--         static ImGuiTableFlags flags1 = ImGuiTableFlags_BordersV;
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_PadOuterX", &flags1, ImGuiTableFlags_PadOuterX);
--         r.ImGui_SameLine(); HelpMarker("Enable outer-most padding (default if ImGuiTableFlags_BordersOuterV is set)");
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoPadOuterX", &flags1, ImGuiTableFlags_NoPadOuterX);
--         r.ImGui_SameLine(); HelpMarker("Disable outer-most padding (default if ImGuiTableFlags_BordersOuterV is not set)");
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoPadInnerX", &flags1, ImGuiTableFlags_NoPadInnerX);
--         r.ImGui_SameLine(); HelpMarker("Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off)");
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags1, ImGuiTableFlags_BordersOuterV);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags1, ImGuiTableFlags_BordersInnerV);
--         static bool show_headers = false;
--         r.ImGui_Checkbox("show_headers", &show_headers);
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table_padding", 3, flags1))
--         {
--             if (show_headers)
--             {
--                 r.ImGui_TableSetupColumn("One");
--                 r.ImGui_TableSetupColumn("Two");
--                 r.ImGui_TableSetupColumn("Three");
--                 r.ImGui_TableHeadersRow();
--             }
--
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     if (row == 0)
--                     {
--                         r.ImGui_Text("Avail %.2f", r.ImGui_GetContentRegionAvail().x);
--                     }
--                     else
--                     {
--                         char buf[32];
--                         sprintf(buf, "Hello %d,%d", column, row);
--                         r.ImGui_Button(buf, ImVec2(-FLT_MIN, 0.0f));
--                     }
--                     //if (r.ImGui_TableGetColumnFlags() & ImGuiTableColumnFlags_IsHovered)
--                     //    r.ImGui_TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 100, 0, 255));
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--
--         // Second example: set style.CellPadding to (0.0) or a custom value.
--         // FIXME-TABLE: Vertical border effectively not displayed the same way as horizontal one...
--         HelpMarker("Setting style.CellPadding to (0,0) or a custom value.");
--         static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
--         static ImVec2 cell_padding(0.0f, 0.0f);
--         static bool show_widget_frame_bg = true;
--
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Borders", &flags2, ImGuiTableFlags_Borders);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersH", &flags2, ImGuiTableFlags_BordersH);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersV", &flags2, ImGuiTableFlags_BordersV);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInner", &flags2, ImGuiTableFlags_BordersInner);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuter", &flags2, ImGuiTableFlags_BordersOuter);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_RowBg", &flags2, ImGuiTableFlags_RowBg);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags2, ImGuiTableFlags_Resizable);
--         r.ImGui_Checkbox("show_widget_frame_bg", &show_widget_frame_bg);
--         r.ImGui_SliderFloat2("CellPadding", &cell_padding.x, 0.0f, 10.0f, "%.0f");
--         PopStyleCompact();
--
--         r.ImGui_PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
--         if (r.ImGui_BeginTable("table_padding_2", 3, flags2))
--         {
--             static char text_bufs[3 * 5][16]; // Mini text storage for 3x5 cells
--             static bool init = true;
--             if (!show_widget_frame_bg)
--                 r.ImGui_PushStyleColor(ImGuiCol_FrameBg, 0);
--             for (int cell = 0; cell < 3 * 5; cell++)
--             {
--                 r.ImGui_TableNextColumn();
--                 if (init)
--                     strcpy(text_bufs[cell], "edit me");
--                 r.ImGui_SetNextItemWidth(-FLT_MIN);
--                 r.ImGui_PushID(cell);
--                 r.ImGui_InputText("##cell", text_bufs[cell], IM_ARRAYSIZE(text_bufs[cell]));
--                 r.ImGui_PopID();
--             }
--             if (!show_widget_frame_bg)
--                 r.ImGui_PopStyleColor();
--             init = false;
--             r.ImGui_EndTable();
--         }
--         r.ImGui_PopStyleVar();
--
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Sizing policies"))
--     {
--         static ImGuiTableFlags flags1 = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags1, ImGuiTableFlags_Resizable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags1, ImGuiTableFlags_NoHostExtendX);
--         PopStyleCompact();
--
--         static ImGuiTableFlags sizing_policy_flags[4] = { ImGuiTableFlags_SizingFixedFit, ImGuiTableFlags_SizingFixedSame, ImGuiTableFlags_SizingStretchProp, ImGuiTableFlags_SizingStretchSame };
--         for (int table_n = 0; table_n < 4; table_n++)
--         {
--             r.ImGui_PushID(table_n);
--             r.ImGui_SetNextItemWidth(TEXT_BASE_WIDTH * 30);
--             EditTableSizingFlags(&sizing_policy_flags[table_n]);
--
--             // To make it easier to understand the different sizing policy,
--             // For each policy: we display one table where the columns have equal contents width, and one where the columns have different contents width.
--             if (r.ImGui_BeginTable("table1", 3, sizing_policy_flags[table_n] | flags1))
--             {
--                 for (int row = 0; row < 3; row++)
--                 {
--                     r.ImGui_TableNextRow();
--                     r.ImGui_TableNextColumn(); r.ImGui_Text("Oh dear");
--                     r.ImGui_TableNextColumn(); r.ImGui_Text("Oh dear");
--                     r.ImGui_TableNextColumn(); r.ImGui_Text("Oh dear");
--                 }
--                 r.ImGui_EndTable();
--             }
--             if (r.ImGui_BeginTable("table2", 3, sizing_policy_flags[table_n] | flags1))
--             {
--                 for (int row = 0; row < 3; row++)
--                 {
--                     r.ImGui_TableNextRow();
--                     r.ImGui_TableNextColumn(); r.ImGui_Text("AAAA");
--                     r.ImGui_TableNextColumn(); r.ImGui_Text("BBBBBBBB");
--                     r.ImGui_TableNextColumn(); r.ImGui_Text("CCCCCCCCCCCC");
--                 }
--                 r.ImGui_EndTable();
--             }
--             r.ImGui_PopID();
--         }
--
--         r.ImGui_Spacing();
--         r.ImGui_TextUnformatted("Advanced");
--         r.ImGui_SameLine();
--         HelpMarker("This section allows you to interact and see the effect of various sizing policies depending on whether Scroll is enabled and the contents of your columns.");
--
--         enum ContentsType { CT_ShowWidth, CT_ShortText, CT_LongText, CT_Button, CT_FillButton, CT_InputText };
--         static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
--         static int contents_type = CT_ShowWidth;
--         static int column_count = 3;
--
--         PushStyleCompact();
--         r.ImGui_PushID("Advanced");
--         r.ImGui_PushItemWidth(TEXT_BASE_WIDTH * 30);
--         EditTableSizingFlags(&flags);
--         r.ImGui_Combo("Contents", &contents_type, "Show width\0Short Text\0Long Text\0Button\0Fill Button\0InputText\0");
--         if (contents_type == CT_FillButton)
--         {
--             r.ImGui_SameLine();
--             HelpMarker("Be mindful that using right-alignment (e.g. size.x = -FLT_MIN) creates a feedback loop where contents width can feed into auto-column width can feed into contents width.");
--         }
--         r.ImGui_DragInt("Columns", &column_count, 0.1f, 1, 64, "%d", ImGuiSliderFlags_AlwaysClamp);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_PreciseWidths", &flags, ImGuiTableFlags_PreciseWidths);
--         r.ImGui_SameLine(); HelpMarker("Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoClip", &flags, ImGuiTableFlags_NoClip);
--         r.ImGui_PopItemWidth();
--         r.ImGui_PopID();
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table2", column_count, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 7)))
--         {
--             for (int cell = 0; cell < 10 * column_count; cell++)
--             {
--                 r.ImGui_TableNextColumn();
--                 int column = r.ImGui_TableGetColumnIndex();
--                 int row = r.ImGui_TableGetRowIndex();
--
--                 r.ImGui_PushID(cell);
--                 char label[32];
--                 static char text_buf[32] = "";
--                 sprintf(label, "Hello %d,%d", column, row);
--                 switch (contents_type)
--                 {
--                 case CT_ShortText:  r.ImGui_TextUnformatted(label); break;
--                 case CT_LongText:   r.ImGui_Text("Some %s text %d,%d\nOver two lines..", column == 0 ? "long" : "longeeer", column, row); break;
--                 case CT_ShowWidth:  r.ImGui_Text("W: %.1f", r.ImGui_GetContentRegionAvail().x); break;
--                 case CT_Button:     r.ImGui_Button(label); break;
--                 case CT_FillButton: r.ImGui_Button(label, ImVec2(-FLT_MIN, 0.0f)); break;
--                 case CT_InputText:  r.ImGui_SetNextItemWidth(-FLT_MIN); r.ImGui_InputText("##", text_buf, IM_ARRAYSIZE(text_buf)); break;
--                 }
--                 r.ImGui_PopID();
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Vertical scrolling, with clipping"))
--     {
--         HelpMarker("Here we activate ScrollY, which will create a child window container to allow hosting scrollable contents.\n\nWe also demonstrate using ImGuiListClipper to virtualize the submission of many items.");
--         static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
--
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
--         PopStyleCompact();
--
--         // When using ScrollX or ScrollY we need to specify a size for our table container!
--         // Otherwise by default the table will fit all available space, like a BeginChild() call.
--         ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
--         if (r.ImGui_BeginTable("table_scrolly", 3, flags, outer_size))
--         {
--             r.ImGui_TableSetupScrollFreeze(0, 1); // Make top row always visible
--             r.ImGui_TableSetupColumn("One", ImGuiTableColumnFlags_None);
--             r.ImGui_TableSetupColumn("Two", ImGuiTableColumnFlags_None);
--             r.ImGui_TableSetupColumn("Three", ImGuiTableColumnFlags_None);
--             r.ImGui_TableHeadersRow();
--
--             // Demonstrate using clipper for large vertical lists
--             ImGuiListClipper clipper;
--             clipper.Begin(1000);
--             while (clipper.Step())
--             {
--                 for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
--                 {
--                     r.ImGui_TableNextRow();
--                     for (int column = 0; column < 3; column++)
--                     {
--                         r.ImGui_TableSetColumnIndex(column);
--                         r.ImGui_Text("Hello %d,%d", column, row);
--                     }
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Horizontal scrolling"))
--     {
--         HelpMarker(
--             "When ScrollX is enabled, the default sizing policy becomes ImGuiTableFlags_SizingFixedFit, "
--             "as automatically stretching columns doesn't make much sense with horizontal scrolling.\n\n"
--             "Also note that as of the current version, you will almost always want to enable ScrollY along with ScrollX,"
--             "because the container window won't automatically extend vertically to fix contents (this may be improved in future versions).");
--         static ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
--         static int freeze_cols = 1;
--         static int freeze_rows = 1;
--
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
--         r.ImGui_SetNextItemWidth(r.ImGui_GetFrameHeight());
--         r.ImGui_DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
--         r.ImGui_SetNextItemWidth(r.ImGui_GetFrameHeight());
--         r.ImGui_DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
--         PopStyleCompact();
--
--         // When using ScrollX or ScrollY we need to specify a size for our table container!
--         // Otherwise by default the table will fit all available space, like a BeginChild() call.
--         ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
--         if (r.ImGui_BeginTable("table_scrollx", 7, flags, outer_size))
--         {
--             r.ImGui_TableSetupScrollFreeze(freeze_cols, freeze_rows);
--             r.ImGui_TableSetupColumn("Line #", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
--             r.ImGui_TableSetupColumn("One");
--             r.ImGui_TableSetupColumn("Two");
--             r.ImGui_TableSetupColumn("Three");
--             r.ImGui_TableSetupColumn("Four");
--             r.ImGui_TableSetupColumn("Five");
--             r.ImGui_TableSetupColumn("Six");
--             r.ImGui_TableHeadersRow();
--             for (int row = 0; row < 20; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 7; column++)
--                 {
--                     // Both TableNextColumn() and TableSetColumnIndex() return true when a column is visible or performing width measurement.
--                     // Because here we know that:
--                     // - A) all our columns are contributing the same to row height
--                     // - B) column 0 is always visible,
--                     // We only always submit this one column and can skip others.
--                     // More advanced per-column clipping behaviors may benefit from polling the status flags via TableGetColumnFlags().
--                     if (!r.ImGui_TableSetColumnIndex(column) && column > 0)
--                         continue;
--                     if (column == 0)
--                         r.ImGui_Text("Line %d", row);
--                     else
--                         r.ImGui_Text("Hello world %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--
--         r.ImGui_Spacing();
--         r.ImGui_TextUnformatted("Stretch + ScrollX");
--         r.ImGui_SameLine();
--         HelpMarker(
--             "Showcase using Stretch columns + ScrollX together: "
--             "this is rather unusual and only makes sense when specifying an 'inner_width' for the table!\n"
--             "Without an explicit value, inner_width is == outer_size.x and therefore using Stretch columns + ScrollX together doesn't make sense.");
--         static ImGuiTableFlags flags2 = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
--         static float inner_width = 1000.0f;
--         PushStyleCompact();
--         r.ImGui_PushID("flags3");
--         r.ImGui_PushItemWidth(TEXT_BASE_WIDTH * 30);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollX", &flags2, ImGuiTableFlags_ScrollX);
--         r.ImGui_DragFloat("inner_width", &inner_width, 1.0f, 0.0f, FLT_MAX, "%.1f");
--         r.ImGui_PopItemWidth();
--         r.ImGui_PopID();
--         PopStyleCompact();
--         if (r.ImGui_BeginTable("table2", 7, flags2, outer_size, inner_width))
--         {
--             for (int cell = 0; cell < 20 * 7; cell++)
--             {
--                 r.ImGui_TableNextColumn();
--                 r.ImGui_Text("Hello world %d,%d", r.ImGui_TableGetColumnIndex(), r.ImGui_TableGetRowIndex());
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Columns flags"))
--     {
--         // Create a first table just to show all the options/flags we want to make visible in our example!
--         const int column_count = 3;
--         const char* column_names[column_count] = { "One", "Two", "Three" };
--         static ImGuiTableColumnFlags column_flags[column_count] = { ImGuiTableColumnFlags_DefaultSort, ImGuiTableColumnFlags_None, ImGuiTableColumnFlags_DefaultHide };
--         static ImGuiTableColumnFlags column_flags_out[column_count] = { 0, 0, 0 }; // Output from TableGetColumnFlags()
--
--         if (r.ImGui_BeginTable("table_columns_flags_checkboxes", column_count, ImGuiTableFlags_None))
--         {
--             PushStyleCompact();
--             for (int column = 0; column < column_count; column++)
--             {
--                 r.ImGui_TableNextColumn();
--                 r.ImGui_PushID(column);
--                 r.ImGui_AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation
--                 r.ImGui_Text("'%s'", column_names[column]);
--                 r.ImGui_Spacing();
--                 r.ImGui_Text("Input flags:");
--                 EditTableColumnsFlags(&column_flags[column]);
--                 r.ImGui_Spacing();
--                 r.ImGui_Text("Output flags:");
--                 ShowTableColumnsStatusFlags(column_flags_out[column]);
--                 r.ImGui_PopID();
--             }
--             PopStyleCompact();
--             r.ImGui_EndTable();
--         }
--
--         // Create the real table we care about for the example!
--         // We use a scrolling table to be able to showcase the difference between the _IsEnabled and _IsVisible flags above, otherwise in
--         // a non-scrolling table columns are always visible (unless using ImGuiTableFlags_NoKeepColumnsVisible + resizing the parent window down)
--         const ImGuiTableFlags flags
--             = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
--             | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV
--             | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable;
--         ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 9);
--         if (r.ImGui_BeginTable("table_columns_flags", column_count, flags, outer_size))
--         {
--             for (int column = 0; column < column_count; column++)
--                 r.ImGui_TableSetupColumn(column_names[column], column_flags[column]);
--             r.ImGui_TableHeadersRow();
--             for (int column = 0; column < column_count; column++)
--                 column_flags_out[column] = r.ImGui_TableGetColumnFlags(column);
--             float indent_step = (float)((int)TEXT_BASE_WIDTH / 2);
--             for (int row = 0; row < 8; row++)
--             {
--                 r.ImGui_Indent(indent_step); // Add some indentation to demonstrate usage of per-column IndentEnable/IndentDisable flags.
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < column_count; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("%s %s", (column == 0) ? "Indented" : "Hello", r.ImGui_TableGetColumnName(column));
--                 }
--             }
--             r.ImGui_Unindent(indent_step * 8.0f);
--
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Columns widths"))
--     {
--         HelpMarker("Using TableSetupColumn() to setup default width.");
--
--         static ImGuiTableFlags flags1 = ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBodyUntilResize;
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags1, ImGuiTableFlags_Resizable);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags1, ImGuiTableFlags_NoBordersInBodyUntilResize);
--         PopStyleCompact();
--         if (r.ImGui_BeginTable("table1", 3, flags1))
--         {
--             // We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
--             r.ImGui_TableSetupColumn("one", ImGuiTableColumnFlags_WidthFixed, 100.0f); // Default to 100.0f
--             r.ImGui_TableSetupColumn("two", ImGuiTableColumnFlags_WidthFixed, 200.0f); // Default to 200.0f
--             r.ImGui_TableSetupColumn("three", ImGuiTableColumnFlags_WidthFixed);       // Default to auto
--             r.ImGui_TableHeadersRow();
--             for (int row = 0; row < 4; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     if (row == 0)
--                         r.ImGui_Text("(w: %5.1f)", r.ImGui_GetContentRegionAvail().x);
--                     else
--                         r.ImGui_Text("Hello %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--
--         HelpMarker("Using TableSetupColumn() to setup explicit width.\n\nUnless _NoKeepColumnsVisible is set, fixed columns with set width may still be shrunk down if there's not enough space in the host.");
--
--         static ImGuiTableFlags flags2 = ImGuiTableFlags_None;
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoKeepColumnsVisible", &flags2, ImGuiTableFlags_NoKeepColumnsVisible);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags2, ImGuiTableFlags_BordersInnerV);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags2, ImGuiTableFlags_BordersOuterV);
--         PopStyleCompact();
--         if (r.ImGui_BeginTable("table2", 4, flags2))
--         {
--             // We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
--             r.ImGui_TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
--             r.ImGui_TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
--             r.ImGui_TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 30.0f);
--             r.ImGui_TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 4; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     if (row == 0)
--                         r.ImGui_Text("(w: %5.1f)", r.ImGui_GetContentRegionAvail().x);
--                     else
--                         r.ImGui_Text("Hello %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Nested tables"))
--     {
--         HelpMarker("This demonstrate embedding a table into another table cell.");
--
--         if (r.ImGui_BeginTable("table_nested1", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
--         {
--             r.ImGui_TableSetupColumn("A0");
--             r.ImGui_TableSetupColumn("A1");
--             r.ImGui_TableHeadersRow();
--
--             r.ImGui_TableNextColumn();
--             r.ImGui_Text("A0 Row 0");
--             {
--                 float rows_height = TEXT_BASE_HEIGHT * 2;
--                 if (r.ImGui_BeginTable("table_nested2", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
--                 {
--                     r.ImGui_TableSetupColumn("B0");
--                     r.ImGui_TableSetupColumn("B1");
--                     r.ImGui_TableHeadersRow();
--
--                     r.ImGui_TableNextRow(ImGuiTableRowFlags_None, rows_height);
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("B0 Row 0");
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("B0 Row 1");
--                     r.ImGui_TableNextRow(ImGuiTableRowFlags_None, rows_height);
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("B1 Row 0");
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("B1 Row 1");
--
--                     r.ImGui_EndTable();
--                 }
--             }
--             r.ImGui_TableNextColumn(); r.ImGui_Text("A0 Row 1");
--             r.ImGui_TableNextColumn(); r.ImGui_Text("A1 Row 0");
--             r.ImGui_TableNextColumn(); r.ImGui_Text("A1 Row 1");
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Row height"))
--     {
--         HelpMarker("You can pass a 'min_row_height' to TableNextRow().\n\nRows are padded with 'style.CellPadding.y' on top and bottom, so effectively the minimum row height will always be >= 'style.CellPadding.y * 2.0f'.\n\nWe cannot honor a _maximum_ row height as that would requires a unique clipping rectangle per row.");
--         if (r.ImGui_BeginTable("table_row_height", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV))
--         {
--             for (int row = 0; row < 10; row++)
--             {
--                 float min_row_height = (float)(int)(TEXT_BASE_HEIGHT * 0.30f * row);
--                 r.ImGui_TableNextRow(ImGuiTableRowFlags_None, min_row_height);
--                 r.ImGui_TableNextColumn();
--                 r.ImGui_Text("min_row_height = %.2f", min_row_height);
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Outer size"))
--     {
--         // Showcasing use of ImGuiTableFlags_NoHostExtendX and ImGuiTableFlags_NoHostExtendY
--         // Important to that note how the two flags have slightly different behaviors!
--         r.ImGui_Text("Using NoHostExtendX and NoHostExtendY:");
--         PushStyleCompact();
--         static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
--         r.ImGui_SameLine(); HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_NoHostExtendY", &flags, ImGuiTableFlags_NoHostExtendY);
--         r.ImGui_SameLine(); HelpMarker("Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
--         PopStyleCompact();
--
--         ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 5.5f);
--         if (r.ImGui_BeginTable("table1", 3, flags, outer_size))
--         {
--             for (int row = 0; row < 10; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("Cell %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_SameLine();
--         r.ImGui_Text("Hello!");
--
--         r.ImGui_Spacing();
--
--         r.ImGui_Text("Using explicit size:");
--         if (r.ImGui_BeginTable("table2", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
--         {
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("Cell %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_SameLine();
--         if (r.ImGui_BeginTable("table3", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
--         {
--             for (int row = 0; row < 3; row++)
--             {
--                 r.ImGui_TableNextRow(0, TEXT_BASE_HEIGHT * 1.5f);
--                 for (int column = 0; column < 3; column++)
--                 {
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("Cell %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Background color"))
--     {
--         static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
--         static int row_bg_type = 1;
--         static int row_bg_target = 1;
--         static int cell_bg_type = 1;
--
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_Borders", &flags, ImGuiTableFlags_Borders);
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
--         r.ImGui_SameLine(); HelpMarker("ImGuiTableFlags_RowBg automatically sets RowBg0 to alternative colors pulled from the Style.");
--         r.ImGui_Combo("row bg type", (int*)&row_bg_type, "None\0Red\0Gradient\0");
--         r.ImGui_Combo("row bg target", (int*)&row_bg_target, "RowBg0\0RowBg1\0"); r.ImGui_SameLine(); HelpMarker("Target RowBg0 to override the alternating odd/even colors,\nTarget RowBg1 to blend with them.");
--         r.ImGui_Combo("cell bg type", (int*)&cell_bg_type, "None\0Blue\0"); r.ImGui_SameLine(); HelpMarker("We are colorizing cells to B1->C2 here.");
--         IM_ASSERT(row_bg_type >= 0 && row_bg_type <= 2);
--         IM_ASSERT(row_bg_target >= 0 && row_bg_target <= 1);
--         IM_ASSERT(cell_bg_type >= 0 && cell_bg_type <= 1);
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table1", 5, flags))
--         {
--             for (int row = 0; row < 6; row++)
--             {
--                 r.ImGui_TableNextRow();
--
--                 // Demonstrate setting a row background color with 'r.ImGui_TableSetBgColor(ImGuiTableBgTarget_RowBgX, ...)'
--                 // We use a transparent color so we can see the one behind in case our target is RowBg1 and RowBg0 was already targeted by the ImGuiTableFlags_RowBg flag.
--                 if (row_bg_type != 0)
--                 {
--                     ImU32 row_bg_color = r.ImGui_GetColorU32(row_bg_type == 1 ? ImVec4(0.7f, 0.3f, 0.3f, 0.65f) : ImVec4(0.2f + row * 0.1f, 0.2f, 0.2f, 0.65f)); // Flat or Gradient?
--                     r.ImGui_TableSetBgColor(ImGuiTableBgTarget_RowBg0 + row_bg_target, row_bg_color);
--                 }
--
--                 // Fill cells
--                 for (int column = 0; column < 5; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("%c%c", 'A' + row, '0' + column);
--
--                     // Change background of Cells B1->C2
--                     // Demonstrate setting a cell background color with 'r.ImGui_TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
--                     // (the CellBg color will be blended over the RowBg and ColumnBg colors)
--                     // We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
--                     if (row >= 1 && row <= 2 && column >= 1 && column <= 2 && cell_bg_type == 1)
--                     {
--                         ImU32 cell_bg_color = r.ImGui_GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
--                         r.ImGui_TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
--                     }
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Tree view"))
--     {
--         static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
--
--         if (r.ImGui_BeginTable("3ways", 3, flags))
--         {
--             // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
--             r.ImGui_TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
--             r.ImGui_TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
--             r.ImGui_TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
--             r.ImGui_TableHeadersRow();
--
--             // Simple storage to output a dummy file-system.
--             struct MyTreeNode
--             {
--                 const char*     Name;
--                 const char*     Type;
--                 int             Size;
--                 int             ChildIdx;
--                 int             ChildCount;
--                 static void DisplayNode(const MyTreeNode* node, const MyTreeNode* all_nodes)
--                 {
--                     r.ImGui_TableNextRow();
--                     r.ImGui_TableNextColumn();
--                     const bool is_folder = (node->ChildCount > 0);
--                     if (is_folder)
--                     {
--                         bool open = r.ImGui_TreeNodeEx(node->Name, ImGuiTreeNodeFlags_SpanFullWidth);
--                         r.ImGui_TableNextColumn();
--                         r.ImGui_TextDisabled("--");
--                         r.ImGui_TableNextColumn();
--                         r.ImGui_TextUnformatted(node->Type);
--                         if (open)
--                         {
--                             for (int child_n = 0; child_n < node->ChildCount; child_n++)
--                                 DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
--                             r.ImGui_TreePop();
--                         }
--                     }
--                     else
--                     {
--                         r.ImGui_TreeNodeEx(node->Name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
--                         r.ImGui_TableNextColumn();
--                         r.ImGui_Text("%d", node->Size);
--                         r.ImGui_TableNextColumn();
--                         r.ImGui_TextUnformatted(node->Type);
--                     }
--                 }
--             };
--             static const MyTreeNode nodes[] =
--             {
--                 { "Root",                         "Folder",       -1,       1, 3    }, // 0
--                 { "Music",                        "Folder",       -1,       4, 2    }, // 1
--                 { "Textures",                     "Folder",       -1,       6, 3    }, // 2
--                 { "desktop.ini",                  "System file",  1024,    -1,-1    }, // 3
--                 { "File1_a.wav",                  "Audio file",   123000,  -1,-1    }, // 4
--                 { "File1_b.wav",                  "Audio file",   456000,  -1,-1    }, // 5
--                 { "Image001.png",                 "Image file",   203128,  -1,-1    }, // 6
--                 { "Copy of Image001.png",         "Image file",   203256,  -1,-1    }, // 7
--                 { "Copy of Image001 (Final2).png","Image file",   203512,  -1,-1    }, // 8
--             };
--
--             MyTreeNode::DisplayNode(&nodes[0], nodes);
--
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Item width"))
--     {
--         HelpMarker(
--             "Showcase using PushItemWidth() and how it is preserved on a per-column basis.\n\n"
--             "Note that on auto-resizing non-resizable fixed columns, querying the content width for e.g. right-alignment doesn't make sense.");
--         if (r.ImGui_BeginTable("table_item_width", 3, ImGuiTableFlags_Borders))
--         {
--             r.ImGui_TableSetupColumn("small");
--             r.ImGui_TableSetupColumn("half");
--             r.ImGui_TableSetupColumn("right-align");
--             r.ImGui_TableHeadersRow();
--
--             for (int row = 0; row < 3; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 if (row == 0)
--                 {
--                     // Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
--                     r.ImGui_TableSetColumnIndex(0);
--                     r.ImGui_PushItemWidth(TEXT_BASE_WIDTH * 3.0f); // Small
--                     r.ImGui_TableSetColumnIndex(1);
--                     r.ImGui_PushItemWidth(-r.ImGui_GetContentRegionAvail().x * 0.5f);
--                     r.ImGui_TableSetColumnIndex(2);
--                     r.ImGui_PushItemWidth(-FLT_MIN); // Right-aligned
--                 }
--
--                 // Draw our contents
--                 static float dummy_f = 0.0f;
--                 r.ImGui_PushID(row);
--                 r.ImGui_TableSetColumnIndex(0);
--                 r.ImGui_SliderFloat("float0", &dummy_f, 0.0f, 1.0f);
--                 r.ImGui_TableSetColumnIndex(1);
--                 r.ImGui_SliderFloat("float1", &dummy_f, 0.0f, 1.0f);
--                 r.ImGui_TableSetColumnIndex(2);
--                 r.ImGui_SliderFloat("float2", &dummy_f, 0.0f, 1.0f);
--                 r.ImGui_PopID();
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     // Demonstrate using TableHeader() calls instead of TableHeadersRow()
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Custom headers"))
--     {
--         const int COLUMNS_COUNT = 3;
--         if (r.ImGui_BeginTable("table_custom_headers", COLUMNS_COUNT, ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
--         {
--             r.ImGui_TableSetupColumn("Apricot");
--             r.ImGui_TableSetupColumn("Banana");
--             r.ImGui_TableSetupColumn("Cherry");
--
--             // Dummy entire-column selection storage
--             // FIXME: It would be nice to actually demonstrate full-featured selection using those checkbox.
--             static bool column_selected[3] = {};
--
--             // Instead of calling TableHeadersRow() we'll submit custom headers ourselves
--             r.ImGui_TableNextRow(ImGuiTableRowFlags_Headers);
--             for (int column = 0; column < COLUMNS_COUNT; column++)
--             {
--                 r.ImGui_TableSetColumnIndex(column);
--                 const char* column_name = r.ImGui_TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
--                 r.ImGui_PushID(column);
--                 r.ImGui_PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
--                 r.ImGui_Checkbox("##checkall", &column_selected[column]);
--                 r.ImGui_PopStyleVar();
--                 r.ImGui_SameLine(0.0f, r.ImGui_GetStyle().ItemInnerSpacing.x);
--                 r.ImGui_TableHeader(column_name);
--                 r.ImGui_PopID();
--             }
--
--             for (int row = 0; row < 5; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < 3; column++)
--                 {
--                     char buf[32];
--                     sprintf(buf, "Cell %d,%d", column, row);
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Selectable(buf, column_selected[column]);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     // Demonstrate creating custom context menus inside columns, while playing it nice with context menus provided by TableHeadersRow()/TableHeader()
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Context menus"))
--     {
--         HelpMarker("By default, right-clicking over a TableHeadersRow()/TableHeader() line will open the default context-menu.\nUsing ImGuiTableFlags_ContextMenuInBody we also allow right-clicking over columns body.");
--         static ImGuiTableFlags flags1 = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders | ImGuiTableFlags_ContextMenuInBody;
--
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_ContextMenuInBody", &flags1, ImGuiTableFlags_ContextMenuInBody);
--         PopStyleCompact();
--
--         // Context Menus: first example
--         // [1.1] Right-click on the TableHeadersRow() line to open the default table context menu.
--         // [1.2] Right-click in columns also open the default table context menu (if ImGuiTableFlags_ContextMenuInBody is set)
--         const int COLUMNS_COUNT = 3;
--         if (r.ImGui_BeginTable("table_context_menu", COLUMNS_COUNT, flags1))
--         {
--             r.ImGui_TableSetupColumn("One");
--             r.ImGui_TableSetupColumn("Two");
--             r.ImGui_TableSetupColumn("Three");
--
--             // [1.1]] Right-click on the TableHeadersRow() line to open the default table context menu.
--             r.ImGui_TableHeadersRow();
--
--             // Submit dummy contents
--             for (int row = 0; row < 4; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < COLUMNS_COUNT; column++)
--                 {
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("Cell %d,%d", column, row);
--                 }
--             }
--             r.ImGui_EndTable();
--         }
--
--         // Context Menus: second example
--         // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
--         // [2.2] Right-click on the ".." to open a custom popup
--         // [2.3] Right-click in columns to open another custom popup
--         HelpMarker("Demonstrate mixing table context menu (over header), item context button (over button) and custom per-colum context menu (over column body).");
--         ImGuiTableFlags flags2 = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders;
--         if (r.ImGui_BeginTable("table_context_menu_2", COLUMNS_COUNT, flags2))
--         {
--             r.ImGui_TableSetupColumn("One");
--             r.ImGui_TableSetupColumn("Two");
--             r.ImGui_TableSetupColumn("Three");
--
--             // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
--             r.ImGui_TableHeadersRow();
--             for (int row = 0; row < 4; row++)
--             {
--                 r.ImGui_TableNextRow();
--                 for (int column = 0; column < COLUMNS_COUNT; column++)
--                 {
--                     // Submit dummy contents
--                     r.ImGui_TableSetColumnIndex(column);
--                     r.ImGui_Text("Cell %d,%d", column, row);
--                     r.ImGui_SameLine();
--
--                     // [2.2] Right-click on the ".." to open a custom popup
--                     r.ImGui_PushID(row * COLUMNS_COUNT + column);
--                     r.ImGui_SmallButton("..");
--                     if (r.ImGui_BeginPopupContextItem())
--                     {
--                         r.ImGui_Text("This is the popup for Button(\"..\") in Cell %d,%d", column, row);
--                         if (r.ImGui_Button("Close"))
--                             r.ImGui_CloseCurrentPopup();
--                         r.ImGui_EndPopup();
--                     }
--                     r.ImGui_PopID();
--                 }
--             }
--
--             // [2.3] Right-click anywhere in columns to open another custom popup
--             // (instead of testing for !IsAnyItemHovered() we could also call OpenPopup() with ImGuiPopupFlags_NoOpenOverExistingPopup
--             // to manage popup priority as the popups triggers, here "are we hovering a column" are overlapping)
--             int hovered_column = -1;
--             for (int column = 0; column < COLUMNS_COUNT + 1; column++)
--             {
--                 r.ImGui_PushID(column);
--                 if (r.ImGui_TableGetColumnFlags(column) & ImGuiTableColumnFlags_IsHovered)
--                     hovered_column = column;
--                 if (hovered_column == column && !r.ImGui_IsAnyItemHovered() && r.ImGui_IsMouseReleased(1))
--                     r.ImGui_OpenPopup("MyPopup");
--                 if (r.ImGui_BeginPopup("MyPopup"))
--                 {
--                     if (column == COLUMNS_COUNT)
--                         r.ImGui_Text("This is a custom popup for unused space after the last column.");
--                     else
--                         r.ImGui_Text("This is a custom popup for Column %d", column);
--                     if (r.ImGui_Button("Close"))
--                         r.ImGui_CloseCurrentPopup();
--                     r.ImGui_EndPopup();
--                 }
--                 r.ImGui_PopID();
--             }
--
--             r.ImGui_EndTable();
--             r.ImGui_Text("Hovered column: %d", hovered_column);
--         }
--         r.ImGui_TreePop();
--     }
--
--     // Demonstrate creating multiple tables with the same ID
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Synced instances"))
--     {
--         HelpMarker("Multiple tables with the same identifier will share their settings, width, visibility, order etc.");
--         for (int n = 0; n < 3; n++)
--         {
--             char buf[32];
--             sprintf(buf, "Synced Table %d", n);
--             bool open = r.ImGui_CollapsingHeader(buf, ImGuiTreeNodeFlags_DefaultOpen);
--             if (open && r.ImGui_BeginTable("Table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings))
--             {
--                 r.ImGui_TableSetupColumn("One");
--                 r.ImGui_TableSetupColumn("Two");
--                 r.ImGui_TableSetupColumn("Three");
--                 r.ImGui_TableHeadersRow();
--                 for (int cell = 0; cell < 9; cell++)
--                 {
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("this cell %d", cell);
--                 }
--                 r.ImGui_EndTable();
--             }
--         }
--         r.ImGui_TreePop();
--     }
--
--     // Demonstrate using Sorting facilities
--     // This is a simplified version of the "Advanced" example, where we mostly focus on the code necessary to handle sorting.
--     // Note that the "Advanced" example also showcase manually triggering a sort (e.g. if item quantities have been modified)
--     static const char* template_items_names[] =
--     {
--         "Banana", "Apple", "Cherry", "Watermelon", "Grapefruit", "Strawberry", "Mango",
--         "Kiwi", "Orange", "Pineapple", "Blueberry", "Plum", "Coconut", "Pear", "Apricot"
--     };
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Sorting"))
--     {
--         // Create item list
--         static ImVector<MyItem> items;
--         if (items.Size == 0)
--         {
--             items.resize(50, MyItem());
--             for (int n = 0; n < items.Size; n++)
--             {
--                 const int template_n = n % IM_ARRAYSIZE(template_items_names);
--                 MyItem& item = items[n];
--                 item.ID = n;
--                 item.Name = template_items_names[template_n];
--                 item.Quantity = (n * n - n) % 20; // Assign default quantities
--             }
--         }
--
--         // Options
--         static ImGuiTableFlags flags =
--             ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
--             | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
--             | ImGuiTableFlags_ScrollY;
--         PushStyleCompact();
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_SortMulti", &flags, ImGuiTableFlags_SortMulti);
--         r.ImGui_SameLine(); HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
--         r.ImGui_CheckboxFlags("ImGuiTableFlags_SortTristate", &flags, ImGuiTableFlags_SortTristate);
--         r.ImGui_SameLine(); HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");
--         PopStyleCompact();
--
--         if (r.ImGui_BeginTable("table_sorting", 4, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f))
--         {
--             // Declare columns
--             // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
--             // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
--             // Demonstrate using a mixture of flags among available sort-related flags:
--             // - ImGuiTableColumnFlags_DefaultSort
--             // - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending / ImGuiTableColumnFlags_NoSortDescending
--             // - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
--             r.ImGui_TableSetupColumn("ID",       ImGuiTableColumnFlags_DefaultSort          | ImGuiTableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_ID);
--             r.ImGui_TableSetupColumn("Name",                                                  ImGuiTableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Name);
--             r.ImGui_TableSetupColumn("Action",   ImGuiTableColumnFlags_NoSort               | ImGuiTableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Action);
--             r.ImGui_TableSetupColumn("Quantity", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthStretch, 0.0f, MyItemColumnID_Quantity);
--             r.ImGui_TableSetupScrollFreeze(0, 1); // Make row always visible
--             r.ImGui_TableHeadersRow();
--
--             // Sort our data if sort specs have been changed!
--             if (ImGuiTableSortSpecs* sorts_specs = r.ImGui_TableGetSortSpecs())
--                 if (sorts_specs->SpecsDirty)
--                 {
--                     MyItem::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
--                     if (items.Size > 1)
--                         qsort(&items[0], (size_t)items.Size, sizeof(items[0]), MyItem::CompareWithSortSpecs);
--                     MyItem::s_current_sort_specs = NULL;
--                     sorts_specs->SpecsDirty = false;
--                 }
--
--             // Demonstrate using clipper for large vertical lists
--             ImGuiListClipper clipper;
--             clipper.Begin(items.Size);
--             while (clipper.Step())
--                 for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
--                 {
--                     // Display a data item
--                     MyItem* item = &items[row_n];
--                     r.ImGui_PushID(item->ID);
--                     r.ImGui_TableNextRow();
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("%04d", item->ID);
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_TextUnformatted(item->Name);
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_SmallButton("None");
--                     r.ImGui_TableNextColumn();
--                     r.ImGui_Text("%d", item->Quantity);
--                     r.ImGui_PopID();
--                 }
--             r.ImGui_EndTable();
--         }
--         r.ImGui_TreePop();
--     }
--
--     //r.ImGui_SetNextItemOpen(true, ImGuiCond_Once); // [DEBUG]
--     if (open_action != -1)
--         r.ImGui_SetNextItemOpen(open_action != 0);
--     if (r.ImGui_TreeNode("Advanced"))
--     {
--         static ImGuiTableFlags flags =
--             ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
--             | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
--             | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
--             | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
--             | ImGuiTableFlags_SizingFixedFit;
--
--         enum ContentsType { CT_Text, CT_Button, CT_SmallButton, CT_FillButton, CT_Selectable, CT_SelectableSpanRow };
--         static int contents_type = CT_SelectableSpanRow;
--         const char* contents_type_names[] = { "Text", "Button", "SmallButton", "FillButton", "Selectable", "Selectable (span row)" };
--         static int freeze_cols = 1;
--         static int freeze_rows = 1;
--         static int items_count = IM_ARRAYSIZE(template_items_names) * 2;
--         static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);
--         static float row_min_height = 0.0f; // Auto
--         static float inner_width_with_scroll = 0.0f; // Auto-extend
--         static bool outer_size_enabled = true;
--         static bool show_headers = true;
--         static bool show_wrapped_text = false;
--         //static ImGuiTextFilter filter;
--         //r.ImGui_SetNextItemOpen(true, ImGuiCond_Once); // FIXME-TABLE: Enabling this results in initial clipped first pass on table which tend to affects column sizing
--         if (r.ImGui_TreeNode("Options"))
--         {
--             // Make the UI compact because there are so many fields
--             PushStyleCompact();
--             r.ImGui_PushItemWidth(TEXT_BASE_WIDTH * 28.0f);
--
--             if (r.ImGui_TreeNodeEx("Features:", ImGuiTreeNodeFlags_DefaultOpen))
--             {
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_Reorderable", &flags, ImGuiTableFlags_Reorderable);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_Hideable", &flags, ImGuiTableFlags_Hideable);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_Sortable", &flags, ImGuiTableFlags_Sortable);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoSavedSettings", &flags, ImGuiTableFlags_NoSavedSettings);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_ContextMenuInBody", &flags, ImGuiTableFlags_ContextMenuInBody);
--                 r.ImGui_TreePop();
--             }
--
--             if (r.ImGui_TreeNodeEx("Decorations:", ImGuiTreeNodeFlags_DefaultOpen))
--             {
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags, ImGuiTableFlags_BordersOuterV);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags, ImGuiTableFlags_BordersInnerV);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersH", &flags, ImGuiTableFlags_BordersH);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersOuterH", &flags, ImGuiTableFlags_BordersOuterH);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_BordersInnerH", &flags, ImGuiTableFlags_BordersInnerH);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody); r.ImGui_SameLine(); HelpMarker("Disable vertical borders in columns Body (borders will always appears in Headers");
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags, ImGuiTableFlags_NoBordersInBodyUntilResize); r.ImGui_SameLine(); HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appears in Headers)");
--                 r.ImGui_TreePop();
--             }
--
--             if (r.ImGui_TreeNodeEx("Sizing:", ImGuiTreeNodeFlags_DefaultOpen))
--             {
--                 EditTableSizingFlags(&flags);
--                 r.ImGui_SameLine(); HelpMarker("In the Advanced demo we override the policy of each column so those table-wide settings have less effect that typical.");
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
--                 r.ImGui_SameLine(); HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoHostExtendY", &flags, ImGuiTableFlags_NoHostExtendY);
--                 r.ImGui_SameLine(); HelpMarker("Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoKeepColumnsVisible", &flags, ImGuiTableFlags_NoKeepColumnsVisible);
--                 r.ImGui_SameLine(); HelpMarker("Only available if ScrollX is disabled.");
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_PreciseWidths", &flags, ImGuiTableFlags_PreciseWidths);
--                 r.ImGui_SameLine(); HelpMarker("Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoClip", &flags, ImGuiTableFlags_NoClip);
--                 r.ImGui_SameLine(); HelpMarker("Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with ScrollFreeze options.");
--                 r.ImGui_TreePop();
--             }
--
--             if (r.ImGui_TreeNodeEx("Padding:", ImGuiTreeNodeFlags_DefaultOpen))
--             {
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_PadOuterX", &flags, ImGuiTableFlags_PadOuterX);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoPadOuterX", &flags, ImGuiTableFlags_NoPadOuterX);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_NoPadInnerX", &flags, ImGuiTableFlags_NoPadInnerX);
--                 r.ImGui_TreePop();
--             }
--
--             if (r.ImGui_TreeNodeEx("Scrolling:", ImGuiTreeNodeFlags_DefaultOpen))
--             {
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
--                 r.ImGui_SameLine();
--                 r.ImGui_SetNextItemWidth(r.ImGui_GetFrameHeight());
--                 r.ImGui_DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
--                 r.ImGui_SameLine();
--                 r.ImGui_SetNextItemWidth(r.ImGui_GetFrameHeight());
--                 r.ImGui_DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
--                 r.ImGui_TreePop();
--             }
--
--             if (r.ImGui_TreeNodeEx("Sorting:", ImGuiTreeNodeFlags_DefaultOpen))
--             {
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_SortMulti", &flags, ImGuiTableFlags_SortMulti);
--                 r.ImGui_SameLine(); HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
--                 r.ImGui_CheckboxFlags("ImGuiTableFlags_SortTristate", &flags, ImGuiTableFlags_SortTristate);
--                 r.ImGui_SameLine(); HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");
--                 r.ImGui_TreePop();
--             }
--
--             if (r.ImGui_TreeNodeEx("Other:", ImGuiTreeNodeFlags_DefaultOpen))
--             {
--                 r.ImGui_Checkbox("show_headers", &show_headers);
--                 r.ImGui_Checkbox("show_wrapped_text", &show_wrapped_text);
--
--                 r.ImGui_DragFloat2("##OuterSize", &outer_size_value.x);
--                 r.ImGui_SameLine(0.0f, r.ImGui_GetStyle().ItemInnerSpacing.x);
--                 r.ImGui_Checkbox("outer_size", &outer_size_enabled);
--                 r.ImGui_SameLine();
--                 HelpMarker("If scrolling is disabled (ScrollX and ScrollY not set):\n"
--                     "- The table is output directly in the parent window.\n"
--                     "- OuterSize.x < 0.0f will right-align the table.\n"
--                     "- OuterSize.x = 0.0f will narrow fit the table unless there are any Stretch column.\n"
--                     "- OuterSize.y then becomes the minimum size for the table, which will extend vertically if there are more rows (unless NoHostExtendY is set).");
--
--                 // From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
--                 // To facilitate toying with this demo we will actually pass 0.0f to the BeginTable() when ScrollX is disabled.
--                 r.ImGui_DragFloat("inner_width (when ScrollX active)", &inner_width_with_scroll, 1.0f, 0.0f, FLT_MAX);
--
--                 r.ImGui_DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);
--                 r.ImGui_SameLine(); HelpMarker("Specify height of the Selectable item.");
--
--                 r.ImGui_DragInt("items_count", &items_count, 0.1f, 0, 9999);
--                 r.ImGui_Combo("items_type (first column)", &contents_type, contents_type_names, IM_ARRAYSIZE(contents_type_names));
--                 //filter.Draw("filter");
--                 r.ImGui_TreePop();
--             }
--
--             r.ImGui_PopItemWidth();
--             PopStyleCompact();
--             r.ImGui_Spacing();
--             r.ImGui_TreePop();
--         }
--
--         // Recreate/reset item list if we changed the number of items
--         static ImVector<MyItem> items;
--         static ImVector<int> selection;
--         static bool items_need_sort = false;
--         if (items.Size != items_count)
--         {
--             items.resize(items_count, MyItem());
--             for (int n = 0; n < items_count; n++)
--             {
--                 const int template_n = n % IM_ARRAYSIZE(template_items_names);
--                 MyItem& item = items[n];
--                 item.ID = n;
--                 item.Name = template_items_names[template_n];
--                 item.Quantity = (template_n == 3) ? 10 : (template_n == 4) ? 20 : 0; // Assign default quantities
--             }
--         }
--
--         const ImDrawList* parent_draw_list = r.ImGui_GetWindowDrawList();
--         const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
--         ImVec2 table_scroll_cur, table_scroll_max; // For debug display
--         const ImDrawList* table_draw_list = NULL;  // "
--
--         const float inner_width_to_use = (flags & ImGuiTableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
--         if (r.ImGui_BeginTable("table_advanced", 6, flags, outer_size_enabled ? outer_size_value : ImVec2(0, 0), inner_width_to_use))
--         {
--             // Declare columns
--             // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
--             // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
--             r.ImGui_TableSetupColumn("ID",           ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, MyItemColumnID_ID);
--             r.ImGui_TableSetupColumn("Name",         ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
--             r.ImGui_TableSetupColumn("Action",       ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Action);
--             r.ImGui_TableSetupColumn("Quantity",     ImGuiTableColumnFlags_PreferSortDescending, 0.0f, MyItemColumnID_Quantity);
--             r.ImGui_TableSetupColumn("Description",  (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch, 0.0f, MyItemColumnID_Description);
--             r.ImGui_TableSetupColumn("Hidden",       ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort);
--             r.ImGui_TableSetupScrollFreeze(freeze_cols, freeze_rows);
--
--             // Sort our data if sort specs have been changed!
--             ImGuiTableSortSpecs* sorts_specs = r.ImGui_TableGetSortSpecs();
--             if (sorts_specs && sorts_specs->SpecsDirty)
--                 items_need_sort = true;
--             if (sorts_specs && items_need_sort && items.Size > 1)
--             {
--                 MyItem::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
--                 qsort(&items[0], (size_t)items.Size, sizeof(items[0]), MyItem::CompareWithSortSpecs);
--                 MyItem::s_current_sort_specs = NULL;
--                 sorts_specs->SpecsDirty = false;
--             }
--             items_need_sort = false;
--
--             // Take note of whether we are currently sorting based on the Quantity field,
--             // we will use this to trigger sorting when we know the data of this column has been modified.
--             const bool sorts_specs_using_quantity = (r.ImGui_TableGetColumnFlags(3) & ImGuiTableColumnFlags_IsSorted) != 0;
--
--             // Show headers
--             if (show_headers)
--                 r.ImGui_TableHeadersRow();
--
--             // Show data
--             // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
--             r.ImGui_PushButtonRepeat(true);
-- #if 1
--             // Demonstrate using clipper for large vertical lists
--             ImGuiListClipper clipper;
--             clipper.Begin(items.Size);
--             while (clipper.Step())
--             {
--                 for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
-- #else
--             // Without clipper
--             {
--                 for (int row_n = 0; row_n < items.Size; row_n++)
-- #endif
--                 {
--                     MyItem* item = &items[row_n];
--                     //if (!filter.PassFilter(item->Name))
--                     //    continue;
--
--                     const bool item_is_selected = selection.contains(item->ID);
--                     r.ImGui_PushID(item->ID);
--                     r.ImGui_TableNextRow(ImGuiTableRowFlags_None, row_min_height);
--                     r.ImGui_TableNextColumn();
--
--                     // For the demo purpose we can select among different type of items submitted in the first column
--                     char label[32];
--                     sprintf(label, "%04d", item->ID);
--                     if (contents_type == CT_Text)
--                         r.ImGui_TextUnformatted(label);
--                     else if (contents_type == CT_Button)
--                         r.ImGui_Button(label);
--                     else if (contents_type == CT_SmallButton)
--                         r.ImGui_SmallButton(label);
--                     else if (contents_type == CT_FillButton)
--                         r.ImGui_Button(label, ImVec2(-FLT_MIN, 0.0f));
--                     else if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow)
--                     {
--                         ImGuiSelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ? ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap : ImGuiSelectableFlags_None;
--                         if (r.ImGui_Selectable(label, item_is_selected, selectable_flags, ImVec2(0, row_min_height)))
--                         {
--                             if (r.ImGui_GetIO().KeyCtrl)
--                             {
--                                 if (item_is_selected)
--                                     selection.find_erase_unsorted(item->ID);
--                                 else
--                                     selection.push_back(item->ID);
--                             }
--                             else
--                             {
--                                 selection.clear();
--                                 selection.push_back(item->ID);
--                             }
--                         }
--                     }
--
--                     if (r.ImGui_TableNextColumn())
--                         r.ImGui_TextUnformatted(item->Name);
--
--                     // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
--                     // and we are currently sorting on the column showing the Quantity.
--                     // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
--                     // You will probably need a more advanced system in your code if you want to automatically sort when a specific entry changes.
--                     if (r.ImGui_TableNextColumn())
--                     {
--                         if (r.ImGui_SmallButton("Chop")) { item->Quantity += 1; }
--                         if (sorts_specs_using_quantity && r.ImGui_IsItemDeactivated()) { items_need_sort = true; }
--                         r.ImGui_SameLine();
--                         if (r.ImGui_SmallButton("Eat")) { item->Quantity -= 1; }
--                         if (sorts_specs_using_quantity && r.ImGui_IsItemDeactivated()) { items_need_sort = true; }
--                     }
--
--                     if (r.ImGui_TableNextColumn())
--                         r.ImGui_Text("%d", item->Quantity);
--
--                     r.ImGui_TableNextColumn();
--                     if (show_wrapped_text)
--                         r.ImGui_TextWrapped("Lorem ipsum dolor sit amet");
--                     else
--                         r.ImGui_Text("Lorem ipsum dolor sit amet");
--
--                     if (r.ImGui_TableNextColumn())
--                         r.ImGui_Text("1234");
--
--                     r.ImGui_PopID();
--                 }
--             }
--             r.ImGui_PopButtonRepeat();
--
--             // Store some info to display debug details below
--             table_scroll_cur = ImVec2(r.ImGui_GetScrollX(), r.ImGui_GetScrollY());
--             table_scroll_max = ImVec2(r.ImGui_GetScrollMaxX(), r.ImGui_GetScrollMaxY());
--             table_draw_list = r.ImGui_GetWindowDrawList();
--             r.ImGui_EndTable();
--         }
--         static bool show_debug_details = false;
--         r.ImGui_Checkbox("Debug details", &show_debug_details);
--         if (show_debug_details && table_draw_list)
--         {
--             r.ImGui_SameLine(0.0f, 0.0f);
--             const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
--             if (table_draw_list == parent_draw_list)
--                 r.ImGui_Text(": DrawCmd: +%d (in same window)",
--                     table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
--             else
--                 r.ImGui_Text(": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
--                     table_draw_list_draw_cmd_count - 1, table_scroll_cur.x, table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
--         }
--         r.ImGui_TreePop();
--     }
--
--     r.ImGui_PopID();
--
--     ShowDemoWindowColumns();
--
--     if (disable_indent)
--         r.ImGui_PopStyleVar();
-- }
--
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
--
-- static void ShowDemoWindowMisc()
-- {
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
--
--     if (r.ImGui_CollapsingHeader("Inputs, Navigation & Focus"))
--     {
--         ImGuiIO& io = r.ImGui_GetIO();
--
--         // Display ImGuiIO output flags
--         r.ImGui_Text("WantCaptureMouse: %d", io.WantCaptureMouse);
--         r.ImGui_Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
--         r.ImGui_Text("WantTextInput: %d", io.WantTextInput);
--         r.ImGui_Text("WantSetMousePos: %d", io.WantSetMousePos);
--         r.ImGui_Text("NavActive: %d, NavVisible: %d", io.NavActive, io.NavVisible);
--
--         // Display Keyboard/Mouse state
--         if (r.ImGui_TreeNode("Keyboard, Mouse & Navigation State"))
--         {
--             if (r.ImGui_IsMousePosValid())
--                 r.ImGui_Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
--             else
--                 r.ImGui_Text("Mouse pos: <INVALID>");
--             r.ImGui_Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
--             r.ImGui_Text("Mouse down:");     for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (io.MouseDownDuration[i] >= 0.0f)   { r.ImGui_SameLine(); r.ImGui_Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
--             r.ImGui_Text("Mouse clicked:");  for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (r.ImGui_IsMouseClicked(i))          { r.ImGui_SameLine(); r.ImGui_Text("b%d", i); }
--             r.ImGui_Text("Mouse dblclick:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (r.ImGui_IsMouseDoubleClicked(i))    { r.ImGui_SameLine(); r.ImGui_Text("b%d", i); }
--             r.ImGui_Text("Mouse released:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (r.ImGui_IsMouseReleased(i))         { r.ImGui_SameLine(); r.ImGui_Text("b%d", i); }
--             r.ImGui_Text("Mouse wheel: %.1f", io.MouseWheel);
--
--             r.ImGui_Text("Keys down:");      for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (io.KeysDownDuration[i] >= 0.0f)     { r.ImGui_SameLine(); r.ImGui_Text("%d (0x%X) (%.02f secs)", i, i, io.KeysDownDuration[i]); }
--             r.ImGui_Text("Keys pressed:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (r.ImGui_IsKeyPressed(i))             { r.ImGui_SameLine(); r.ImGui_Text("%d (0x%X)", i, i); }
--             r.ImGui_Text("Keys release:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (r.ImGui_IsKeyReleased(i))            { r.ImGui_SameLine(); r.ImGui_Text("%d (0x%X)", i, i); }
--             r.ImGui_Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
--             r.ImGui_Text("Chars queue:");    for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; r.ImGui_SameLine();  r.ImGui_Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
--
--             r.ImGui_Text("NavInputs down:");     for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f)              { r.ImGui_SameLine(); r.ImGui_Text("[%d] %.2f", i, io.NavInputs[i]); }
--             r.ImGui_Text("NavInputs pressed:");  for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { r.ImGui_SameLine(); r.ImGui_Text("[%d]", i); }
--             r.ImGui_Text("NavInputs duration:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] >= 0.0f) { r.ImGui_SameLine(); r.ImGui_Text("[%d] %.2f", i, io.NavInputsDownDuration[i]); }
--
--             r.ImGui_Button("Hovering me sets the\nkeyboard capture flag");
--             if (r.ImGui_IsItemHovered())
--                 r.ImGui_CaptureKeyboardFromApp(true);
--             r.ImGui_SameLine();
--             r.ImGui_Button("Holding me clears the\nthe keyboard capture flag");
--             if (r.ImGui_IsItemActive())
--                 r.ImGui_CaptureKeyboardFromApp(false);
--
--             r.ImGui_TreePop();
--         }
--
--         if (r.ImGui_TreeNode("Tabbing"))
--         {
--             r.ImGui_Text("Use TAB/SHIFT+TAB to cycle through keyboard editable fields.");
--             static char buf[32] = "hello";
--             r.ImGui_InputText("1", buf, IM_ARRAYSIZE(buf));
--             r.ImGui_InputText("2", buf, IM_ARRAYSIZE(buf));
--             r.ImGui_InputText("3", buf, IM_ARRAYSIZE(buf));
--             r.ImGui_PushAllowKeyboardFocus(false);
--             r.ImGui_InputText("4 (tab skip)", buf, IM_ARRAYSIZE(buf));
--             //r.ImGui_SameLine(); HelpMarker("Use r.ImGui_PushAllowKeyboardFocus(bool) to disable tabbing through certain widgets.");
--             r.ImGui_PopAllowKeyboardFocus();
--             r.ImGui_InputText("5", buf, IM_ARRAYSIZE(buf));
--             r.ImGui_TreePop();
--         }
--
--         if (r.ImGui_TreeNode("Focus from code"))
--         {
--             bool focus_1 = r.ImGui_Button("Focus on 1"); r.ImGui_SameLine();
--             bool focus_2 = r.ImGui_Button("Focus on 2"); r.ImGui_SameLine();
--             bool focus_3 = r.ImGui_Button("Focus on 3");
--             int has_focus = 0;
--             static char buf[128] = "click on a button to set focus";
--
--             if (focus_1) r.ImGui_SetKeyboardFocusHere();
--             r.ImGui_InputText("1", buf, IM_ARRAYSIZE(buf));
--             if (r.ImGui_IsItemActive()) has_focus = 1;
--
--             if (focus_2) r.ImGui_SetKeyboardFocusHere();
--             r.ImGui_InputText("2", buf, IM_ARRAYSIZE(buf));
--             if (r.ImGui_IsItemActive()) has_focus = 2;
--
--             r.ImGui_PushAllowKeyboardFocus(false);
--             if (focus_3) r.ImGui_SetKeyboardFocusHere();
--             r.ImGui_InputText("3 (tab skip)", buf, IM_ARRAYSIZE(buf));
--             if (r.ImGui_IsItemActive()) has_focus = 3;
--             r.ImGui_PopAllowKeyboardFocus();
--
--             if (has_focus)
--                 r.ImGui_Text("Item with focus: %d", has_focus);
--             else
--                 r.ImGui_Text("Item with focus: <none>");
--
--             // Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
--             static float f3[3] = { 0.0f, 0.0f, 0.0f };
--             int focus_ahead = -1;
--             if (r.ImGui_Button("Focus on X")) { focus_ahead = 0; } r.ImGui_SameLine();
--             if (r.ImGui_Button("Focus on Y")) { focus_ahead = 1; } r.ImGui_SameLine();
--             if (r.ImGui_Button("Focus on Z")) { focus_ahead = 2; }
--             if (focus_ahead != -1) r.ImGui_SetKeyboardFocusHere(focus_ahead);
--             r.ImGui_SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);
--
--             r.ImGui_TextWrapped("NB: Cursor & selection are preserved when refocusing last used item in code.");
--             r.ImGui_TreePop();
--         }
--
--         if (r.ImGui_TreeNode("Dragging"))
--         {
--             r.ImGui_TextWrapped("You can use r.ImGui_GetMouseDragDelta(0) to query for the dragged amount on any widget.");
--             for (int button = 0; button < 3; button++)
--             {
--                 r.ImGui_Text("IsMouseDragging(%d):", button);
--                 r.ImGui_Text("  w/ default threshold: %d,", r.ImGui_IsMouseDragging(button));
--                 r.ImGui_Text("  w/ zero threshold: %d,", r.ImGui_IsMouseDragging(button, 0.0f));
--                 r.ImGui_Text("  w/ large threshold: %d,", r.ImGui_IsMouseDragging(button, 20.0f));
--             }
--
--             r.ImGui_Button("Drag Me");
--             if (r.ImGui_IsItemActive())
--                 r.ImGui_GetForegroundDrawList()->AddLine(io.MouseClickedPos[0], io.MousePos, r.ImGui_GetColorU32(ImGuiCol_Button), 4.0f); // Draw a line between the button and the mouse cursor
--
--             // Drag operations gets "unlocked" when the mouse has moved past a certain threshold
--             // (the default threshold is stored in io.MouseDragThreshold). You can request a lower or higher
--             // threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta().
--             ImVec2 value_raw = r.ImGui_GetMouseDragDelta(0, 0.0f);
--             ImVec2 value_with_lock_threshold = r.ImGui_GetMouseDragDelta(0);
--             ImVec2 mouse_delta = io.MouseDelta;
--             r.ImGui_Text("GetMouseDragDelta(0):");
--             r.ImGui_Text("  w/ default threshold: (%.1f, %.1f)", value_with_lock_threshold.x, value_with_lock_threshold.y);
--             r.ImGui_Text("  w/ zero threshold: (%.1f, %.1f)", value_raw.x, value_raw.y);
--             r.ImGui_Text("io.MouseDelta: (%.1f, %.1f)", mouse_delta.x, mouse_delta.y);
--             r.ImGui_TreePop();
--         }
--
--         if (r.ImGui_TreeNode("Mouse cursors"))
--         {
--             const char* mouse_cursors_names[] = { "Arrow", "TextInput", "ResizeAll", "ResizeNS", "ResizeEW", "ResizeNESW", "ResizeNWSE", "Hand", "NotAllowed" };
--             IM_ASSERT(IM_ARRAYSIZE(mouse_cursors_names) == ImGuiMouseCursor_COUNT);
--
--             ImGuiMouseCursor current = r.ImGui_GetMouseCursor();
--             r.ImGui_Text("Current mouse cursor = %d: %s", current, mouse_cursors_names[current]);
--             r.ImGui_Text("Hover to see mouse cursors:");
--             r.ImGui_SameLine(); HelpMarker(
--                 "Your application can render a different mouse cursor based on what r.ImGui_GetMouseCursor() returns. "
--                 "If software cursor rendering (io.MouseDrawCursor) is set ImGui will draw the right cursor for you, "
--                 "otherwise your backend needs to handle it.");
--             for (int i = 0; i < ImGuiMouseCursor_COUNT; i++)
--             {
--                 char label[32];
--                 sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
--                 r.ImGui_Bullet(); r.ImGui_Selectable(label, false);
--                 if (r.ImGui_IsItemHovered())
--                     r.ImGui_SetMouseCursor(i);
--             }
--             r.ImGui_TreePop();
--         }
--     }
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] About Window / ShowAboutWindow()
-- // Access from Dear ImGui Demo -> Tools -> About
-- //-----------------------------------------------------------------------------
--
-- void r.ImGui_ShowAboutWindow(bool* p_open)
-- {
--     if (!r.ImGui_Begin("About Dear ImGui", p_open, ImGuiWindowFlags_AlwaysAutoResize))
--     {
--         r.ImGui_End();
--         return;
--     }
--     r.ImGui_Text("Dear ImGui %s", r.ImGui_GetVersion());
--     r.ImGui_Separator();
--     r.ImGui_Text("By Omar Cornut and all Dear ImGui contributors.");
--     r.ImGui_Text("Dear ImGui is licensed under the MIT License, see LICENSE for more information.");
--
--     static bool show_config_info = false;
--     r.ImGui_Checkbox("Config/Build Information", &show_config_info);
--     if (show_config_info)
--     {
--         ImGuiIO& io = r.ImGui_GetIO();
--         ImGuiStyle& style = r.ImGui_GetStyle();
--
--         bool copy_to_clipboard = r.ImGui_Button("Copy to clipboard");
--         ImVec2 child_size = ImVec2(0, r.ImGui_GetTextLineHeightWithSpacing() * 18);
--         r.ImGui_BeginChildFrame(r.ImGui_GetID("cfg_infos"), child_size, ImGuiWindowFlags_NoMove);
--         if (copy_to_clipboard)
--         {
--             r.ImGui_LogToClipboard();
--             r.ImGui_LogText("```\n"); // Back quotes will make text appears without formatting when pasting on GitHub
--         }
--
--         r.ImGui_Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
--         r.ImGui_Separator();
--         r.ImGui_Text("sizeof(size_t): %d, sizeof(ImDrawIdx): %d, sizeof(ImDrawVert): %d", (int)sizeof(size_t), (int)sizeof(ImDrawIdx), (int)sizeof(ImDrawVert));
--         r.ImGui_Text("define: __cplusplus=%d", (int)__cplusplus);
-- #ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_FILE_FUNCTIONS
--         r.ImGui_Text("define: IMGUI_DISABLE_FILE_FUNCTIONS");
-- #endif
-- #ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
--         r.ImGui_Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
-- #endif
-- #ifdef IMGUI_USE_BGRA_PACKED_COLOR
--         r.ImGui_Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
-- #endif
-- #ifdef _WIN32
--         r.ImGui_Text("define: _WIN32");
-- #endif
-- #ifdef _WIN64
--         r.ImGui_Text("define: _WIN64");
-- #endif
-- #ifdef __linux__
--         r.ImGui_Text("define: __linux__");
-- #endif
-- #ifdef __APPLE__
--         r.ImGui_Text("define: __APPLE__");
-- #endif
-- #ifdef _MSC_VER
--         r.ImGui_Text("define: _MSC_VER=%d", _MSC_VER);
-- #endif
-- #ifdef _MSVC_LANG
--         r.ImGui_Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
-- #endif
-- #ifdef __MINGW32__
--         r.ImGui_Text("define: __MINGW32__");
-- #endif
-- #ifdef __MINGW64__
--         r.ImGui_Text("define: __MINGW64__");
-- #endif
-- #ifdef __GNUC__
--         r.ImGui_Text("define: __GNUC__=%d", (int)__GNUC__);
-- #endif
-- #ifdef __clang_version__
--         r.ImGui_Text("define: __clang_version__=%s", __clang_version__);
-- #endif
--         r.ImGui_Separator();
--         r.ImGui_Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
--         r.ImGui_Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
--         r.ImGui_Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
--         if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)        r.ImGui_Text(" NavEnableKeyboard");
--         if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)         r.ImGui_Text(" NavEnableGamepad");
--         if (io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)     r.ImGui_Text(" NavEnableSetMousePos");
--         if (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)     r.ImGui_Text(" NavNoCaptureKeyboard");
--         if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)                  r.ImGui_Text(" NoMouse");
--         if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)      r.ImGui_Text(" NoMouseCursorChange");
--         if (io.MouseDrawCursor)                                         r.ImGui_Text("io.MouseDrawCursor");
--         if (io.ConfigMacOSXBehaviors)                                   r.ImGui_Text("io.ConfigMacOSXBehaviors");
--         if (io.ConfigInputTextCursorBlink)                              r.ImGui_Text("io.ConfigInputTextCursorBlink");
--         if (io.ConfigWindowsResizeFromEdges)                            r.ImGui_Text("io.ConfigWindowsResizeFromEdges");
--         if (io.ConfigWindowsMoveFromTitleBarOnly)                       r.ImGui_Text("io.ConfigWindowsMoveFromTitleBarOnly");
--         if (io.ConfigMemoryCompactTimer >= 0.0f)                        r.ImGui_Text("io.ConfigMemoryCompactTimer = %.1f", io.ConfigMemoryCompactTimer);
--         r.ImGui_Text("io.BackendFlags: 0x%08X", io.BackendFlags);
--         if (io.BackendFlags & ImGuiBackendFlags_HasGamepad)             r.ImGui_Text(" HasGamepad");
--         if (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)        r.ImGui_Text(" HasMouseCursors");
--         if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos)         r.ImGui_Text(" HasSetMousePos");
--         if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)   r.ImGui_Text(" RendererHasVtxOffset");
--         r.ImGui_Separator();
--         r.ImGui_Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexWidth, io.Fonts->TexHeight);
--         r.ImGui_Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
--         r.ImGui_Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
--         r.ImGui_Separator();
--         r.ImGui_Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
--         r.ImGui_Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
--         r.ImGui_Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
--         r.ImGui_Text("style.FrameRounding: %.2f", style.FrameRounding);
--         r.ImGui_Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
--         r.ImGui_Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
--         r.ImGui_Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);
--
--         if (copy_to_clipboard)
--         {
--             r.ImGui_LogText("\n```\n");
--             r.ImGui_LogFinish();
--         }
--         r.ImGui_EndChildFrame();
--     }
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Style Editor / ShowStyleEditor()
-- //-----------------------------------------------------------------------------
-- // - ShowStyleSelector()
-- // - ShowFontSelector()
-- // - ShowStyleEditor()
-- //-----------------------------------------------------------------------------
--
-- // Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
-- // Here we use the simplified Combo() api that packs items into a single literal string.
-- // Useful for quick combo boxes where the choices are known locally.
-- bool r.ImGui_ShowStyleSelector(const char* label)
-- {
--     static int style_idx = -1;
--     if (r.ImGui_Combo(label, &style_idx, "Dark\0Light\0Classic\0"))
--     {
--         switch (style_idx)
--         {
--         case 0: r.ImGui_StyleColorsDark(); break;
--         case 1: r.ImGui_StyleColorsLight(); break;
--         case 2: r.ImGui_StyleColorsClassic(); break;
--         }
--         return true;
--     }
--     return false;
-- }
--
-- // Demo helper function to select among loaded fonts.
-- // Here we use the regular BeginCombo()/EndCombo() api which is more the more flexible one.
-- void r.ImGui_ShowFontSelector(const char* label)
-- {
--     ImGuiIO& io = r.ImGui_GetIO();
--     ImFont* font_current = r.ImGui_GetFont();
--     if (r.ImGui_BeginCombo(label, font_current->GetDebugName()))
--     {
--         for (int n = 0; n < io.Fonts->Fonts.Size; n++)
--         {
--             ImFont* font = io.Fonts->Fonts[n];
--             r.ImGui_PushID((void*)font);
--             if (r.ImGui_Selectable(font->GetDebugName(), font == font_current))
--                 io.FontDefault = font;
--             r.ImGui_PopID();
--         }
--         r.ImGui_EndCombo();
--     }
--     r.ImGui_SameLine();
--     HelpMarker(
--         "- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
--         "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
--         "- Read FAQ and docs/FONTS.md for more details.\n"
--         "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
-- }
--
-- // [Internal] Display details for a single font, called by ShowStyleEditor().
-- static void NodeFont(ImFont* font)
-- {
--     ImGuiIO& io = r.ImGui_GetIO();
--     ImGuiStyle& style = r.ImGui_GetStyle();
--     bool font_details_opened = r.ImGui_TreeNode(font, "Font: \"%s\"\n%.2f px, %d glyphs, %d file(s)",
--         font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
--     r.ImGui_SameLine(); if (r.ImGui_SmallButton("Set as default")) { io.FontDefault = font; }
--     if (!font_details_opened)
--         return;
--
--     r.ImGui_PushFont(font);
--     r.ImGui_Text("The quick brown fox jumps over the lazy dog");
--     r.ImGui_PopFont();
--     r.ImGui_DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");   // Scale only this font
--     r.ImGui_SameLine(); HelpMarker(
--         "Note than the default embedded font is NOT meant to be scaled.\n\n"
--         "Font are currently rendered into bitmaps at a given size at the time of building the atlas. "
--         "You may oversample them to get some flexibility with scaling. "
--         "You can also render at multiple sizes and select which one to use at runtime.\n\n"
--         "(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)");
--     r.ImGui_Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
--     r.ImGui_Text("Fallback character: '%c' (U+%04X)", font->FallbackChar, font->FallbackChar);
--     r.ImGui_Text("Ellipsis character: '%c' (U+%04X)", font->EllipsisChar, font->EllipsisChar);
--     const int surface_sqrt = (int)sqrtf((float)font->MetricsTotalSurface);
--     r.ImGui_Text("Texture Area: about %d px ~%dx%d px", font->MetricsTotalSurface, surface_sqrt, surface_sqrt);
--     for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
--         if (font->ConfigData)
--             if (const ImFontConfig* cfg = &font->ConfigData[config_i])
--                 r.ImGui_BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
--                     config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH, cfg->GlyphOffset.x, cfg->GlyphOffset.y);
--     if (r.ImGui_TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
--     {
--         // Display all glyphs of the fonts in separate pages of 256 characters
--         const ImU32 glyph_col = r.ImGui_GetColorU32(ImGuiCol_Text);
--         for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
--         {
--             // Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
--             // This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT
--             // is large // (if ImWchar==ImWchar32 we will do at least about 272 queries here)
--             if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
--             {
--                 base += 4096 - 256;
--                 continue;
--             }
--
--             int count = 0;
--             for (unsigned int n = 0; n < 256; n++)
--                 if (font->FindGlyphNoFallback((ImWchar)(base + n)))
--                     count++;
--             if (count <= 0)
--                 continue;
--             if (!r.ImGui_TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
--                 continue;
--             float cell_size = font->FontSize * 1;
--             float cell_spacing = style.ItemSpacing.y;
--             ImVec2 base_pos = r.ImGui_GetCursorScreenPos();
--             ImDrawList* draw_list = r.ImGui_GetWindowDrawList();
--             for (unsigned int n = 0; n < 256; n++)
--             {
--                 // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions
--                 // available here and thus cannot easily generate a zero-terminated UTF-8 encoded string.
--                 ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
--                 ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
--                 const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
--                 draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
--                 if (glyph)
--                     font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, (ImWchar)(base + n));
--                 if (glyph && r.ImGui_IsMouseHoveringRect(cell_p1, cell_p2))
--                 {
--                     r.ImGui_BeginTooltip();
--                     r.ImGui_Text("Codepoint: U+%04X", base + n);
--                     r.ImGui_Separator();
--                     r.ImGui_Text("Visible: %d", glyph->Visible);
--                     r.ImGui_Text("AdvanceX: %.1f", glyph->AdvanceX);
--                     r.ImGui_Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
--                     r.ImGui_Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
--                     r.ImGui_EndTooltip();
--                 }
--             }
--             r.ImGui_Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
--             r.ImGui_TreePop();
--         }
--         r.ImGui_TreePop();
--     }
--     r.ImGui_TreePop();
-- }
--
-- void r.ImGui_ShowStyleEditor(ImGuiStyle* ref)
-- {
--     // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
--     // (without a reference style pointer, we will use one compared locally as a reference)
--     ImGuiStyle& style = r.ImGui_GetStyle();
--     static ImGuiStyle ref_saved_style;
--
--     // Default to using internal storage as reference
--     static bool init = true;
--     if (init && ref == NULL)
--         ref_saved_style = style;
--     init = false;
--     if (ref == NULL)
--         ref = &ref_saved_style;
--
--     r.ImGui_PushItemWidth(r.ImGui_GetWindowWidth() * 0.50f);
--
--     if (r.ImGui_ShowStyleSelector("Colors##Selector"))
--         ref_saved_style = style;
--     r.ImGui_ShowFontSelector("Fonts##Selector");
--
--     // Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
--     if (r.ImGui_SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
--         style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
--     { bool border = (style.WindowBorderSize > 0.0f); if (r.ImGui_Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
--     r.ImGui_SameLine();
--     { bool border = (style.FrameBorderSize > 0.0f);  if (r.ImGui_Checkbox("FrameBorder",  &border)) { style.FrameBorderSize  = border ? 1.0f : 0.0f; } }
--     r.ImGui_SameLine();
--     { bool border = (style.PopupBorderSize > 0.0f);  if (r.ImGui_Checkbox("PopupBorder",  &border)) { style.PopupBorderSize  = border ? 1.0f : 0.0f; } }
--
--     // Save/Revert button
--     if (r.ImGui_Button("Save Ref"))
--         *ref = ref_saved_style = style;
--     r.ImGui_SameLine();
--     if (r.ImGui_Button("Revert Ref"))
--         style = *ref;
--     r.ImGui_SameLine();
--     HelpMarker(
--         "Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
--         "Use \"Export\" below to save them somewhere.");
--
--     r.ImGui_Separator();
--
--     if (r.ImGui_BeginTabBar("##tabs", ImGuiTabBarFlags_None))
--     {
--         if (r.ImGui_BeginTabItem("Sizes"))
--         {
--             r.ImGui_Text("Main");
--             r.ImGui_SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
--             r.ImGui_SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
--             r.ImGui_SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f");
--             r.ImGui_SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
--             r.ImGui_SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
--             r.ImGui_SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
--             r.ImGui_SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
--             r.ImGui_SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
--             r.ImGui_SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
--             r.ImGui_Text("Borders");
--             r.ImGui_SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
--             r.ImGui_SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
--             r.ImGui_SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
--             r.ImGui_SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
--             r.ImGui_SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
--             r.ImGui_Text("Rounding");
--             r.ImGui_SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
--             r.ImGui_SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
--             r.ImGui_SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
--             r.ImGui_SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
--             r.ImGui_SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
--             r.ImGui_SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
--             r.ImGui_SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
--             r.ImGui_SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
--             r.ImGui_Text("Alignment");
--             r.ImGui_SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
--             int window_menu_button_position = style.WindowMenuButtonPosition + 1;
--             if (r.ImGui_Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
--                 style.WindowMenuButtonPosition = window_menu_button_position - 1;
--             r.ImGui_Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
--             r.ImGui_SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
--             r.ImGui_SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
--             r.ImGui_SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
--             r.ImGui_SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
--             r.ImGui_Text("Safe Area Padding");
--             r.ImGui_SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
--             r.ImGui_SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
--             r.ImGui_EndTabItem();
--         }
--
--         if (r.ImGui_BeginTabItem("Colors"))
--         {
--             static int output_dest = 0;
--             static bool output_only_modified = true;
--             if (r.ImGui_Button("Export"))
--             {
--                 if (output_dest == 0)
--                     r.ImGui_LogToClipboard();
--                 else
--                     r.ImGui_LogToTTY();
--                 r.ImGui_LogText("ImVec4* colors = r.ImGui_GetStyle().Colors;" IM_NEWLINE);
--                 for (int i = 0; i < ImGuiCol_COUNT; i++)
--                 {
--                     const ImVec4& col = style.Colors[i];
--                     const char* name = r.ImGui_GetStyleColorName(i);
--                     if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
--                         r.ImGui_LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE,
--                             name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
--                 }
--                 r.ImGui_LogFinish();
--             }
--             r.ImGui_SameLine(); r.ImGui_SetNextItemWidth(120); r.ImGui_Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
--             r.ImGui_SameLine(); r.ImGui_Checkbox("Only Modified Colors", &output_only_modified);
--
--             static ImGuiTextFilter filter;
--             filter.Draw("Filter colors", r.ImGui_GetFontSize() * 16);
--
--             static ImGuiColorEditFlags alpha_flags = 0;
--             if (r.ImGui_RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None))             { alpha_flags = ImGuiColorEditFlags_None; } r.ImGui_SameLine();
--             if (r.ImGui_RadioButton("Alpha",  alpha_flags == ImGuiColorEditFlags_AlphaPreview))     { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } r.ImGui_SameLine();
--             if (r.ImGui_RadioButton("Both",   alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } r.ImGui_SameLine();
--             HelpMarker(
--                 "In the color list:\n"
--                 "Left-click on color square to open color picker,\n"
--                 "Right-click to open edit options menu.");
--
--             r.ImGui_BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
--             r.ImGui_PushItemWidth(-160);
--             for (int i = 0; i < ImGuiCol_COUNT; i++)
--             {
--                 const char* name = r.ImGui_GetStyleColorName(i);
--                 if (!filter.PassFilter(name))
--                     continue;
--                 r.ImGui_PushID(i);
--                 r.ImGui_ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
--                 if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
--                 {
--                     // Tips: in a real user application, you may want to merge and use an icon font into the main font,
--                     // so instead of "Save"/"Revert" you'd use icons!
--                     // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
--                     r.ImGui_SameLine(0.0f, style.ItemInnerSpacing.x); if (r.ImGui_Button("Save")) { ref->Colors[i] = style.Colors[i]; }
--                     r.ImGui_SameLine(0.0f, style.ItemInnerSpacing.x); if (r.ImGui_Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
--                 }
--                 r.ImGui_SameLine(0.0f, style.ItemInnerSpacing.x);
--                 r.ImGui_TextUnformatted(name);
--                 r.ImGui_PopID();
--             }
--             r.ImGui_PopItemWidth();
--             r.ImGui_EndChild();
--
--             r.ImGui_EndTabItem();
--         }
--
--         if (r.ImGui_BeginTabItem("Fonts"))
--         {
--             ImGuiIO& io = r.ImGui_GetIO();
--             ImFontAtlas* atlas = io.Fonts;
--             HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
--             r.ImGui_PushItemWidth(120);
--             for (int i = 0; i < atlas->Fonts.Size; i++)
--             {
--                 ImFont* font = atlas->Fonts[i];
--                 r.ImGui_PushID(font);
--                 NodeFont(font);
--                 r.ImGui_PopID();
--             }
--             if (r.ImGui_TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
--             {
--                 ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
--                 ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
--                 r.ImGui_Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
--                 r.ImGui_TreePop();
--             }
--
--             // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
--             // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
--             const float MIN_SCALE = 0.3f;
--             const float MAX_SCALE = 2.0f;
--             HelpMarker(
--                 "Those are old settings provided for convenience.\n"
--                 "However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, "
--                 "rebuild the font atlas, and call style.ScaleAllSizes() on a reference ImGuiStyle structure.\n"
--                 "Using those settings here will give you poor quality results.");
--             static float window_scale = 1.0f;
--             if (r.ImGui_DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
--                 r.ImGui_SetWindowFontScale(window_scale);
--             r.ImGui_DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
--             r.ImGui_PopItemWidth();
--
--             r.ImGui_EndTabItem();
--         }
--
--         if (r.ImGui_BeginTabItem("Rendering"))
--         {
--             r.ImGui_Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
--             r.ImGui_SameLine();
--             HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
--
--             r.ImGui_Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
--             r.ImGui_SameLine();
--             HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");
--
--             r.ImGui_Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
--             r.ImGui_PushItemWidth(100);
--             r.ImGui_DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
--             if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;
--
--             // When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
--             r.ImGui_DragFloat("Circle Segment Max Error", &style.CircleSegmentMaxError, 0.01f, 0.10f, 10.0f, "%.2f");
--             if (r.ImGui_IsItemActive())
--             {
--                 r.ImGui_SetNextWindowPos(r.ImGui_GetCursorScreenPos());
--                 r.ImGui_BeginTooltip();
--                 ImVec2 p = r.ImGui_GetCursorScreenPos();
--                 ImDrawList* draw_list = r.ImGui_GetWindowDrawList();
--                 float RAD_MIN = 10.0f, RAD_MAX = 80.0f;
--                 float off_x = 10.0f;
--                 for (int n = 0; n < 7; n++)
--                 {
--                     const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (7.0f - 1.0f);
--                     draw_list->AddCircle(ImVec2(p.x + off_x + rad, p.y + RAD_MAX), rad, r.ImGui_GetColorU32(ImGuiCol_Text), 0);
--                     off_x += 10.0f + rad * 2.0f;
--                 }
--                 r.ImGui_Dummy(ImVec2(off_x, RAD_MAX * 2.0f));
--                 r.ImGui_EndTooltip();
--             }
--             r.ImGui_SameLine();
--             HelpMarker("When drawing circle primitives with \"num_segments == 0\" tesselation will be calculated automatically.");
--
--             r.ImGui_DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
--             r.ImGui_PopItemWidth();
--
--             r.ImGui_EndTabItem();
--         }
--
--         r.ImGui_EndTabBar();
--     }
--
--     r.ImGui_PopItemWidth();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
-- //-----------------------------------------------------------------------------
-- // - ShowExampleAppMainMenuBar()
-- // - ShowExampleMenuFile()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate creating a "main" fullscreen menu bar and populating it.
-- // Note the difference between BeginMainMenuBar() and BeginMenuBar():
-- // - BeginMenuBar() = menu-bar inside current window (which needs the ImGuiWindowFlags_MenuBar flag!)
-- // - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport + call BeginMenuBar() into it.
-- static void ShowExampleAppMainMenuBar()
-- {
--     if (r.ImGui_BeginMainMenuBar())
--     {
--         if (r.ImGui_BeginMenu("File"))
--         {
--             ShowExampleMenuFile();
--             r.ImGui_EndMenu();
--         }
--         if (r.ImGui_BeginMenu("Edit"))
--         {
--             if (r.ImGui_MenuItem("Undo", "CTRL+Z")) {}
--             if (r.ImGui_MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
--             r.ImGui_Separator();
--             if (r.ImGui_MenuItem("Cut", "CTRL+X")) {}
--             if (r.ImGui_MenuItem("Copy", "CTRL+C")) {}
--             if (r.ImGui_MenuItem("Paste", "CTRL+V")) {}
--             r.ImGui_EndMenu();
--         }
--         r.ImGui_EndMainMenuBar();
--     }
-- }
--
-- // Note that shortcuts are currently provided for display only
-- // (future version will add explicit flags to BeginMenu() to request processing shortcuts)
-- static void ShowExampleMenuFile()
-- {
--     r.ImGui_MenuItem("(demo menu)", NULL, false, false);
--     if (r.ImGui_MenuItem("New")) {}
--     if (r.ImGui_MenuItem("Open", "Ctrl+O")) {}
--     if (r.ImGui_BeginMenu("Open Recent"))
--     {
--         r.ImGui_MenuItem("fish_hat.c");
--         r.ImGui_MenuItem("fish_hat.inl");
--         r.ImGui_MenuItem("fish_hat.h");
--         if (r.ImGui_BeginMenu("More.."))
--         {
--             r.ImGui_MenuItem("Hello");
--             r.ImGui_MenuItem("Sailor");
--             if (r.ImGui_BeginMenu("Recurse.."))
--             {
--                 ShowExampleMenuFile();
--                 r.ImGui_EndMenu();
--             }
--             r.ImGui_EndMenu();
--         }
--         r.ImGui_EndMenu();
--     }
--     if (r.ImGui_MenuItem("Save", "Ctrl+S")) {}
--     if (r.ImGui_MenuItem("Save As..")) {}
--
--     r.ImGui_Separator();
--     if (r.ImGui_BeginMenu("Options"))
--     {
--         static bool enabled = true;
--         r.ImGui_MenuItem("Enabled", "", &enabled);
--         r.ImGui_BeginChild("child", ImVec2(0, 60), true);
--         for (int i = 0; i < 10; i++)
--             r.ImGui_Text("Scrolling Text %d", i);
--         r.ImGui_EndChild();
--         static float f = 0.5f;
--         static int n = 0;
--         r.ImGui_SliderFloat("Value", &f, 0.0f, 1.0f);
--         r.ImGui_InputFloat("Input", &f, 0.1f);
--         r.ImGui_Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
--         r.ImGui_EndMenu();
--     }
--
--     if (r.ImGui_BeginMenu("Colors"))
--     {
--         float sz = r.ImGui_GetTextLineHeight();
--         for (int i = 0; i < ImGuiCol_COUNT; i++)
--         {
--             const char* name = r.ImGui_GetStyleColorName((ImGuiCol)i);
--             ImVec2 p = r.ImGui_GetCursorScreenPos();
--             r.ImGui_GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), r.ImGui_GetColorU32((ImGuiCol)i));
--             r.ImGui_Dummy(ImVec2(sz, sz));
--             r.ImGui_SameLine();
--             r.ImGui_MenuItem(name);
--         }
--         r.ImGui_EndMenu();
--     }
--
--     // Here we demonstrate appending again to the "Options" menu (which we already created above)
--     // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
--     // In a real code-base using it would make senses to use this feature from very different code locations.
--     if (r.ImGui_BeginMenu("Options")) // <-- Append!
--     {
--         static bool b = true;
--         r.ImGui_Checkbox("SomeOption", &b);
--         r.ImGui_EndMenu();
--     }
--
--     if (r.ImGui_BeginMenu("Disabled", false)) // Disabled
--     {
--         IM_ASSERT(0);
--     }
--     if (r.ImGui_MenuItem("Checked", NULL, true)) {}
--     if (r.ImGui_MenuItem("Quit", "Alt+F4")) {}
-- }
--
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
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Debug Log / ShowExampleAppLog()
-- //-----------------------------------------------------------------------------
--
-- // Usage:
-- //  static ExampleAppLog my_log;
-- //  my_log.AddLog("Hello %d world\n", 123);
-- //  my_log.Draw("title");
-- struct ExampleAppLog
-- {
--     ImGuiTextBuffer     Buf;
--     ImGuiTextFilter     Filter;
--     ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
--     bool                AutoScroll;  // Keep scrolling if already at the bottom.
--
--     ExampleAppLog()
--     {
--         AutoScroll = true;
--         Clear();
--     }
--
--     void    Clear()
--     {
--         Buf.clear();
--         LineOffsets.clear();
--         LineOffsets.push_back(0);
--     }
--
--     void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
--     {
--         int old_size = Buf.size();
--         va_list args;
--         va_start(args, fmt);
--         Buf.appendfv(fmt, args);
--         va_end(args);
--         for (int new_size = Buf.size(); old_size < new_size; old_size++)
--             if (Buf[old_size] == '\n')
--                 LineOffsets.push_back(old_size + 1);
--     }
--
--     void    Draw(const char* title, bool* p_open = NULL)
--     {
--         if (!r.ImGui_Begin(title, p_open))
--         {
--             r.ImGui_End();
--             return;
--         }
--
--         // Options menu
--         if (r.ImGui_BeginPopup("Options"))
--         {
--             r.ImGui_Checkbox("Auto-scroll", &AutoScroll);
--             r.ImGui_EndPopup();
--         }
--
--         // Main window
--         if (r.ImGui_Button("Options"))
--             r.ImGui_OpenPopup("Options");
--         r.ImGui_SameLine();
--         bool clear = r.ImGui_Button("Clear");
--         r.ImGui_SameLine();
--         bool copy = r.ImGui_Button("Copy");
--         r.ImGui_SameLine();
--         Filter.Draw("Filter", -100.0f);
--
--         r.ImGui_Separator();
--         r.ImGui_BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
--
--         if (clear)
--             Clear();
--         if (copy)
--             r.ImGui_LogToClipboard();
--
--         r.ImGui_PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
--         const char* buf = Buf.begin();
--         const char* buf_end = Buf.end();
--         if (Filter.IsActive())
--         {
--             // In this example we don't use the clipper when Filter is enabled.
--             // This is because we don't have a random access on the result on our filter.
--             // A real application processing logs with ten of thousands of entries may want to store the result of
--             // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
--             for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
--             {
--                 const char* line_start = buf + LineOffsets[line_no];
--                 const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
--                 if (Filter.PassFilter(line_start, line_end))
--                     r.ImGui_TextUnformatted(line_start, line_end);
--             }
--         }
--         else
--         {
--             // The simplest and easy way to display the entire buffer:
--             //   r.ImGui_TextUnformatted(buf_begin, buf_end);
--             // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
--             // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
--             // within the visible area.
--             // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
--             // on your side is recommended. Using ImGuiListClipper requires
--             // - A) random access into your data
--             // - B) items all being the  same height,
--             // both of which we can handle since we an array pointing to the beginning of each line of text.
--             // When using the filter (in the block of code above) we don't have random access into the data to display
--             // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
--             // it possible (and would be recommended if you want to search through tens of thousands of entries).
--             ImGuiListClipper clipper;
--             clipper.Begin(LineOffsets.Size);
--             while (clipper.Step())
--             {
--                 for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
--                 {
--                     const char* line_start = buf + LineOffsets[line_no];
--                     const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
--                     r.ImGui_TextUnformatted(line_start, line_end);
--                 }
--             }
--             clipper.End();
--         }
--         r.ImGui_PopStyleVar();
--
--         if (AutoScroll && r.ImGui_GetScrollY() >= r.ImGui_GetScrollMaxY())
--             r.ImGui_SetScrollHereY(1.0f);
--
--         r.ImGui_EndChild();
--         r.ImGui_End();
--     }
-- };
--
-- // Demonstrate creating a simple log window with basic filtering.
-- static void ShowExampleAppLog(bool* p_open)
-- {
--     static ExampleAppLog log;
--
--     // For the demo: add a debug button _BEFORE_ the normal log window contents
--     // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
--     // Most of the contents of the window will be added by the log.Draw() call.
--     r.ImGui_SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
--     r.ImGui_Begin("Example: Log", p_open);
--     if (r.ImGui_SmallButton("[Debug] Add 5 entries"))
--     {
--         static int counter = 0;
--         const char* categories[3] = { "info", "warn", "error" };
--         const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
--         for (int n = 0; n < 5; n++)
--         {
--             const char* category = categories[counter % IM_ARRAYSIZE(categories)];
--             const char* word = words[counter % IM_ARRAYSIZE(words)];
--             log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
--                 r.ImGui_GetFrameCount(), category, r.ImGui_GetTime(), word);
--             counter++;
--         }
--     }
--     r.ImGui_End();
--
--     // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
--     log.Draw("Example: Log", p_open);
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate create a window with multiple child windows.
-- static void ShowExampleAppLayout(bool* p_open)
-- {
--     r.ImGui_SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
--     if (r.ImGui_Begin("Example: Simple layout", p_open, ImGuiWindowFlags_MenuBar))
--     {
--         if (r.ImGui_BeginMenuBar())
--         {
--             if (r.ImGui_BeginMenu("File"))
--             {
--                 if (r.ImGui_MenuItem("Close")) *p_open = false;
--                 r.ImGui_EndMenu();
--             }
--             r.ImGui_EndMenuBar();
--         }
--
--         // Left
--         static int selected = 0;
--         {
--             r.ImGui_BeginChild("left pane", ImVec2(150, 0), true);
--             for (int i = 0; i < 100; i++)
--             {
--                 char label[128];
--                 sprintf(label, "MyObject %d", i);
--                 if (r.ImGui_Selectable(label, selected == i))
--                     selected = i;
--             }
--             r.ImGui_EndChild();
--         }
--         r.ImGui_SameLine();
--
--         // Right
--         {
--             r.ImGui_BeginGroup();
--             r.ImGui_BeginChild("item view", ImVec2(0, -r.ImGui_GetFrameHeightWithSpacing())); // Leave room for 1 line below us
--             r.ImGui_Text("MyObject: %d", selected);
--             r.ImGui_Separator();
--             if (r.ImGui_BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
--             {
--                 if (r.ImGui_BeginTabItem("Description"))
--                 {
--                     r.ImGui_TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
--                     r.ImGui_EndTabItem();
--                 }
--                 if (r.ImGui_BeginTabItem("Details"))
--                 {
--                     r.ImGui_Text("ID: 0123456789");
--                     r.ImGui_EndTabItem();
--                 }
--                 r.ImGui_EndTabBar();
--             }
--             r.ImGui_EndChild();
--             if (r.ImGui_Button("Revert")) {}
--             r.ImGui_SameLine();
--             if (r.ImGui_Button("Save")) {}
--             r.ImGui_EndGroup();
--         }
--     }
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
-- //-----------------------------------------------------------------------------
--
-- static void ShowPlaceholderObject(const char* prefix, int uid)
-- {
--     // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
--     r.ImGui_PushID(uid);
--
--     // Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add vertical spacing to make the tree lines equal high.
--     r.ImGui_TableNextRow();
--     r.ImGui_TableSetColumnIndex(0);
--     r.ImGui_AlignTextToFramePadding();
--     bool node_open = r.ImGui_TreeNode("Object", "%s_%u", prefix, uid);
--     r.ImGui_TableSetColumnIndex(1);
--     r.ImGui_Text("my sailor is rich");
--
--     if (node_open)
--     {
--         static float placeholder_members[8] = { 0.0f, 0.0f, 1.0f, 3.1416f, 100.0f, 999.0f };
--         for (int i = 0; i < 8; i++)
--         {
--             r.ImGui_PushID(i); // Use field index as identifier.
--             if (i < 2)
--             {
--                 ShowPlaceholderObject("Child", 424242);
--             }
--             else
--             {
--                 // Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
--                 r.ImGui_TableNextRow();
--                 r.ImGui_TableSetColumnIndex(0);
--                 r.ImGui_AlignTextToFramePadding();
--                 ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
--                 r.ImGui_TreeNodeEx("Field", flags, "Field_%d", i);
--
--                 r.ImGui_TableSetColumnIndex(1);
--                 r.ImGui_SetNextItemWidth(-FLT_MIN);
--                 if (i >= 5)
--                     r.ImGui_InputFloat("##value", &placeholder_members[i], 1.0f);
--                 else
--                     r.ImGui_DragFloat("##value", &placeholder_members[i], 0.01f);
--                 r.ImGui_NextColumn();
--             }
--             r.ImGui_PopID();
--         }
--         r.ImGui_TreePop();
--     }
--     r.ImGui_PopID();
-- }
--
-- // Demonstrate create a simple property editor.
-- static void ShowExampleAppPropertyEditor(bool* p_open)
-- {
--     r.ImGui_SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
--     if (!r.ImGui_Begin("Example: Property editor", p_open))
--     {
--         r.ImGui_End();
--         return;
--     }
--
--     HelpMarker(
--         "This example shows how you may implement a property editor using two columns.\n"
--         "All objects/fields data are dummies here.\n"
--         "Remember that in many simple cases, you can use r.ImGui_SameLine(xxx) to position\n"
--         "your cursor horizontally instead of using the Columns() API.");
--
--     r.ImGui_PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
--     if (r.ImGui_BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
--     {
--         // Iterate placeholder objects (all the same data)
--         for (int obj_i = 0; obj_i < 4; obj_i++)
--         {
--             ShowPlaceholderObject("Object", obj_i);
--             //r.ImGui_Separator();
--         }
--         r.ImGui_EndTable();
--     }
--     r.ImGui_PopStyleVar();
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Long Text / ShowExampleAppLongText()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate/test rendering huge amount of text, and the incidence of clipping.
-- static void ShowExampleAppLongText(bool* p_open)
-- {
--     r.ImGui_SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
--     if (!r.ImGui_Begin("Example: Long text display", p_open))
--     {
--         r.ImGui_End();
--         return;
--     }
--
--     static int test_type = 0;
--     static ImGuiTextBuffer log;
--     static int lines = 0;
--     r.ImGui_Text("Printing unusually long amount of text.");
--     r.ImGui_Combo("Test type", &test_type,
--         "Single call to TextUnformatted()\0"
--         "Multiple calls to Text(), clipped\0"
--         "Multiple calls to Text(), not clipped (slow)\0");
--     r.ImGui_Text("Buffer contents: %d lines, %d bytes", lines, log.size());
--     if (r.ImGui_Button("Clear")) { log.clear(); lines = 0; }
--     r.ImGui_SameLine();
--     if (r.ImGui_Button("Add 1000 lines"))
--     {
--         for (int i = 0; i < 1000; i++)
--             log.appendf("%i The quick brown fox jumps over the lazy dog\n", lines + i);
--         lines += 1000;
--     }
--     r.ImGui_BeginChild("Log");
--     switch (test_type)
--     {
--     case 0:
--         // Single call to TextUnformatted() with a big buffer
--         r.ImGui_TextUnformatted(log.begin(), log.end());
--         break;
--     case 1:
--         {
--             // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGuiListClipper helper.
--             r.ImGui_PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
--             ImGuiListClipper clipper;
--             clipper.Begin(lines);
--             while (clipper.Step())
--                 for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
--                     r.ImGui_Text("%i The quick brown fox jumps over the lazy dog", i);
--             r.ImGui_PopStyleVar();
--             break;
--         }
--     case 2:
--         // Multiple calls to Text(), not clipped (slow)
--         r.ImGui_PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
--         for (int i = 0; i < lines; i++)
--             r.ImGui_Text("%i The quick brown fox jumps over the lazy dog", i);
--         r.ImGui_PopStyleVar();
--         break;
--     }
--     r.ImGui_EndChild();
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate creating a window which gets auto-resized according to its content.
-- static void ShowExampleAppAutoResize(bool* p_open)
-- {
--     if (!r.ImGui_Begin("Example: Auto-resizing window", p_open, ImGuiWindowFlags_AlwaysAutoResize))
--     {
--         r.ImGui_End();
--         return;
--     }
--
--     static int lines = 10;
--     r.ImGui_TextUnformatted(
--         "Window will resize every-frame to the size of its content.\n"
--         "Note that you probably don't want to query the window size to\n"
--         "output your content because that would create a feedback loop.");
--     r.ImGui_SliderInt("Number of lines", &lines, 1, 20);
--     for (int i = 0; i < lines; i++)
--         r.ImGui_Text("%*sThis is line %d", i * 4, "", i); // Pad with space to extend size horizontally
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate creating a window with custom resize constraints.
-- static void ShowExampleAppConstrainedResize(bool* p_open)
-- {
--     struct CustomConstraints
--     {
--         // Helper functions to demonstrate programmatic constraints
--         static void Square(ImGuiSizeCallbackData* data) { data->DesiredSize.x = data->DesiredSize.y = IM_MAX(data->DesiredSize.x, data->DesiredSize.y); }
--         static void Step(ImGuiSizeCallbackData* data)   { float step = (float)(int)(intptr_t)data->UserData; data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step); }
--     };
--
--     const char* test_desc[] =
--     {
--         "Resize vertical only",
--         "Resize horizontal only",
--         "Width > 100, Height > 100",
--         "Width 400-500",
--         "Height 400-500",
--         "Custom: Always Square",
--         "Custom: Fixed Steps (100)",
--     };
--
--     static bool auto_resize = false;
--     static int type = 0;
--     static int display_lines = 10;
--     if (type == 0) r.ImGui_SetNextWindowSizeConstraints(ImVec2(-1, 0),    ImVec2(-1, FLT_MAX));      // Vertical only
--     if (type == 1) r.ImGui_SetNextWindowSizeConstraints(ImVec2(0, -1),    ImVec2(FLT_MAX, -1));      // Horizontal only
--     if (type == 2) r.ImGui_SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
--     if (type == 3) r.ImGui_SetNextWindowSizeConstraints(ImVec2(400, -1),  ImVec2(500, -1));          // Width 400-500
--     if (type == 4) r.ImGui_SetNextWindowSizeConstraints(ImVec2(-1, 400),  ImVec2(-1, 500));          // Height 400-500
--     if (type == 5) r.ImGui_SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);                     // Always Square
--     if (type == 6) r.ImGui_SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)(intptr_t)100); // Fixed Step
--
--     ImGuiWindowFlags flags = auto_resize ? ImGuiWindowFlags_AlwaysAutoResize : 0;
--     if (r.ImGui_Begin("Example: Constrained Resize", p_open, flags))
--     {
--         if (r.ImGui_Button("200x200")) { r.ImGui_SetWindowSize(ImVec2(200, 200)); } r.ImGui_SameLine();
--         if (r.ImGui_Button("500x500")) { r.ImGui_SetWindowSize(ImVec2(500, 500)); } r.ImGui_SameLine();
--         if (r.ImGui_Button("800x200")) { r.ImGui_SetWindowSize(ImVec2(800, 200)); }
--         r.ImGui_SetNextItemWidth(200);
--         r.ImGui_Combo("Constraint", &type, test_desc, IM_ARRAYSIZE(test_desc));
--         r.ImGui_SetNextItemWidth(200);
--         r.ImGui_DragInt("Lines", &display_lines, 0.2f, 1, 100);
--         r.ImGui_Checkbox("Auto-resize", &auto_resize);
--         for (int i = 0; i < display_lines; i++)
--             r.ImGui_Text("%*sHello, sailor! Making this line long enough for the example.", i * 4, "");
--     }
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate creating a simple static window with no decoration
-- // + a context-menu to choose which corner of the screen to use.
-- static void ShowExampleAppSimpleOverlay(bool* p_open)
-- {
--     const float PAD = 10.0f;
--     static int corner = 0;
--     ImGuiIO& io = r.ImGui_GetIO();
--     ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
--     if (corner != -1)
--     {
--         // v1.80 window_flags |= ImGuiWindowFlags_NoMove;
--         // v1.80 ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
--         // v1.80 ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
--         // v1.80 r.ImGui_SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
--
        -- const ImGuiViewport* viewport = ImGui::GetMainViewport();
        -- ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        -- ImVec2 work_size = viewport->WorkSize;
        -- ImVec2 window_pos, window_pos_pivot;
        -- window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        -- window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        -- window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        -- window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        -- ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        -- window_flags |= ImGuiWindowFlags_NoMove;
--     }
--     r.ImGui_SetNextWindowBgAlpha(0.35f); // Transparent background
--     if (r.ImGui_Begin("Example: Simple overlay", p_open, window_flags))
--     {
--         r.ImGui_Text("Simple overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
--         r.ImGui_Separator();
--         if (r.ImGui_IsMousePosValid())
--             r.ImGui_Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
--         else
--             r.ImGui_Text("Mouse Position: <invalid>");
--         if (r.ImGui_BeginPopupContextWindow())
--         {
--             if (r.ImGui_MenuItem("Custom",       NULL, corner == -1)) corner = -1;
--             if (r.ImGui_MenuItem("Top-left",     NULL, corner == 0)) corner = 0;
--             if (r.ImGui_MenuItem("Top-right",    NULL, corner == 1)) corner = 1;
--             if (r.ImGui_MenuItem("Bottom-left",  NULL, corner == 2)) corner = 2;
--             if (r.ImGui_MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
--             if (p_open && r.ImGui_MenuItem("Close")) *p_open = false;
--             r.ImGui_EndPopup();
--         }
--     }
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate creating a window covering the entire screen/viewport
-- static void ShowExampleAppFullscreen(bool* p_open)
-- {
--     static bool use_work_area = true;
--     static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
--
--     // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
--     // Based on your use case you may want one of the other.
--     const ImGuiViewport* viewport = ImGui::GetMainViewport();
--     ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
--     ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
--
--     if (ImGui::Begin("Example: Fullscreen window", p_open, flags))
--     {
--         ImGui::Checkbox("Use work area instead of main area", &use_work_area);
--         ImGui::SameLine();
--         HelpMarker("Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the difference.");
--
--         ImGui::CheckboxFlags("ImGuiWindowFlags_NoBackground", &flags, ImGuiWindowFlags_NoBackground);
--         ImGui::CheckboxFlags("ImGuiWindowFlags_NoDecoration", &flags, ImGuiWindowFlags_NoDecoration);
--         ImGui::Indent();
--         ImGui::CheckboxFlags("ImGuiWindowFlags_NoTitleBar", &flags, ImGuiWindowFlags_NoTitleBar);
--         ImGui::CheckboxFlags("ImGuiWindowFlags_NoCollapse", &flags, ImGuiWindowFlags_NoCollapse);
--         ImGui::CheckboxFlags("ImGuiWindowFlags_NoScrollbar", &flags, ImGuiWindowFlags_NoScrollbar);
--         ImGui::Unindent();
--
--         if (p_open && ImGui::Button("Close this window"))
--             *p_open = false;
--     }
--     ImGui::End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Manipulating window titles / ShowExampleAppWindowTitles()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate using "##" and "###" in identifiers to manipulate ID generation.
-- // This apply to all regular items as well.
-- // Read FAQ section "How can I have multiple widgets with the same label?" for details.
-- static void ShowExampleAppWindowTitles(bool*)
-- {
--     const ImGuiViewport* viewport = ImGui::GetMainViewport();
--     const ImVec2 base_pos = viewport->Pos;
--
--     // By default, Windows are uniquely identified by their title.
--     // You can use the "##" and "###" markers to manipulate the display/ID.
--
--     // Using "##" to display same title but have unique identifier.
--      ImGui::SetNextWindowPos(ImVec2(base_pos.x + 100, base_pos.y + 100), ImGuiCond_FirstUseEver);
--     r.ImGui_Begin("Same title as another window##1");
--     r.ImGui_Text("This is window 1.\nMy title is the same as window 2, but my identifier is unique.");
--     r.ImGui_End();
--
--     ImGui::SetNextWindowPos(ImVec2(base_pos.x + 100, base_pos.y + 200), ImGuiCond_FirstUseEver);
--     r.ImGui_Begin("Same title as another window##2");
--     r.ImGui_Text("This is window 2.\nMy title is the same as window 1, but my identifier is unique.");
--     r.ImGui_End();
--
--     // Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
--     char buf[128];
--     sprintf(buf, "Animated title %c %d###AnimatedTitle", "|/-\\"[(int)(r.ImGui_GetTime() / 0.25f) & 3], r.ImGui_GetFrameCount());
--     ImGui::SetNextWindowPos(ImVec2(base_pos.x + 100, base_pos.y + 300), ImGuiCond_FirstUseEver);
--     r.ImGui_Begin(buf);
--     r.ImGui_Text("This window has a changing title.");
--     r.ImGui_End();
-- }
--
-- //-----------------------------------------------------------------------------
-- // [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
-- //-----------------------------------------------------------------------------
--
-- // Demonstrate using the low-level ImDrawList to draw custom shapes.
-- static void ShowExampleAppCustomRendering(bool* p_open)
-- {
--     if (!r.ImGui_Begin("Example: Custom rendering", p_open))
--     {
--         r.ImGui_End();
--         return;
--     }
--
--     // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
--     // overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your
--     // types and ImVec2/ImVec4. Dear ImGui defines overloaded operators but they are internal to imgui.cpp and not
--     // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!
--
--     if (r.ImGui_BeginTabBar("##TabBar"))
--     {
--         if (r.ImGui_BeginTabItem("Primitives"))
--         {
--             r.ImGui_PushItemWidth(-r.ImGui_GetFontSize() * 15);
--             ImDrawList* draw_list = r.ImGui_GetWindowDrawList();
--
--             // Draw gradients
--             // (note that those are currently exacerbating our sRGB/Linear issues)
--             // Calling r.ImGui_GetColorU32() multiplies the given colors by the current Style Alpha, but you may pass the IM_COL32() directly as well..
--             r.ImGui_Text("Gradients");
--             ImVec2 gradient_size = ImVec2(r.ImGui_CalcItemWidth(), r.ImGui_GetFrameHeight());
--             {
--                 ImVec2 p0 = r.ImGui_GetCursorScreenPos();
--                 ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
--                 ImU32 col_a = r.ImGui_GetColorU32(IM_COL32(0, 0, 0, 255));
--                 ImU32 col_b = r.ImGui_GetColorU32(IM_COL32(255, 255, 255, 255));
--                 draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
--                 r.ImGui_InvisibleButton("##gradient1", gradient_size);
--             }
--             {
--                 ImVec2 p0 = r.ImGui_GetCursorScreenPos();
--                 ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
--                 ImU32 col_a = r.ImGui_GetColorU32(IM_COL32(0, 255, 0, 255));
--                 ImU32 col_b = r.ImGui_GetColorU32(IM_COL32(255, 0, 0, 255));
--                 draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
--                 r.ImGui_InvisibleButton("##gradient2", gradient_size);
--             }
--
--             // Draw a bunch of primitives
--             r.ImGui_Text("All primitives");
--             static float sz = 36.0f;
--             static float thickness = 3.0f;
--             static int ngon_sides = 6;
--             static bool circle_segments_override = false;
--             static int circle_segments_override_v = 12;
--             static bool curve_segments_override = false;
--             static int curve_segments_override_v = 8;
--             static ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
--             ImGui::DragFloat("Size", &sz, 0.2f, 2.0f, 100.0f, "%.0f");
--             r.ImGui_DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
--             r.ImGui_SliderInt("N-gon sides", &ngon_sides, 3, 12);
--             r.ImGui_Checkbox("##circlesegmentoverride", &circle_segments_override);
--             r.ImGui_SameLine(0.0f, r.ImGui_GetStyle().ItemInnerSpacing.x);
--             circle_segments_override |= r.ImGui_SliderInt("Circle segments override", &circle_segments_override_v, 3, 40);
--             r.ImGui_Checkbox("##curvessegmentoverride", &curve_segments_override);
--             r.ImGui_SameLine(0.0f, r.ImGui_GetStyle().ItemInnerSpacing.x);
--             curve_segments_override |= r.ImGui_SliderInt("Curves segments override", &curve_segments_override_v, 3, 40);
--             r.ImGui_ColorEdit4("Color", &colf.x);
--
--             const ImVec2 p = r.ImGui_GetCursorScreenPos();
--             const ImU32 col = ImColor(colf);
--             const float spacing = 10.0f;
--             const ImDrawCornerFlags corners_none = 0;
--             const ImDrawCornerFlags corners_all = ImDrawCornerFlags_All;
--             const ImDrawCornerFlags corners_tl_br = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight;
--             const float rounding = sz / 5.0f;
--             const int circle_segments = circle_segments_override ? circle_segments_override_v : 0;
--             const int curve_segments = curve_segments_override ? curve_segments_override_v : 0;
--             float x = p.x + 4.0f;
--             float y = p.y + 4.0f;
--             for (int n = 0; n < 2; n++)
--             {
--                 // First line uses a thickness of 1.0f, second line uses the configurable thickness
--                 float th = (n == 0) ? 1.0f : thickness;
--                 draw_list->AddNgon(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, ngon_sides, th);                 x += sz + spacing;  // N-gon
--                 draw_list->AddCircle(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, circle_segments, th);          x += sz + spacing;  // Circle
--                 draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 0.0f,  corners_none, th);             x += sz + spacing;  // Square
--                 draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, rounding, corners_all, th);           x += sz + spacing;  // Square with all rounded corners
--                 draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, rounding, corners_tl_br, th);         x += sz + spacing;  // Square with two rounded corners
--                 draw_list->AddTriangle(ImVec2(x+sz*0.5f,y), ImVec2(x+sz, y+sz-0.5f), ImVec2(x, y+sz-0.5f), col, th);x += sz + spacing;  // Triangle
--                 //draw_list->AddTriangle(ImVec2(x+sz*0.2f,y), ImVec2(x, y+sz-0.5f), ImVec2(x+sz*0.4f, y+sz-0.5f), col, th);x+= sz*0.4f + spacing; // Thin triangle
--                 draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y), col, th);                                       x += sz + spacing;  // Horizontal line (note: drawing a filled rectangle will be faster!)
--                 draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + sz), col, th);                                       x += spacing;       // Vertical line (note: drawing a filled rectangle will be faster!)
--                 draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y + sz), col, th);                                  x += sz + spacing;  // Diagonal line
--
--                 // Quadratic Bezier Curve (3 control points)
--                 ImVec2 cp3[3] = { ImVec2(x, y + sz * 0.6f), ImVec2(x + sz * 0.5f, y - sz * 0.4f), ImVec2(x + sz, y + sz) };
--                 draw_list->AddBezierQuadratic(cp3[0], cp3[1], cp3[2], col, th, curve_segments); x += sz + spacing;
--
--                 // Cubic Bezier Curve (4 control points)
--                 ImVec2 cp4[4] = { ImVec2(x, y), ImVec2(x + sz * 1.3f, y + sz * 0.3f), ImVec2(x + sz - sz * 1.3f, y + sz - sz * 0.3f), ImVec2(x + sz, y + sz) };
--                 draw_list->AddBezierCubic(cp4[0], cp4[1], cp4[2], cp4[3], col, th, curve_segments);
--
--                 x = p.x + 4;
--                 y += sz + spacing;
--             }
--             draw_list->AddNgonFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz*0.5f, col, ngon_sides);               x += sz + spacing;  // N-gon
--             draw_list->AddCircleFilled(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, circle_segments);            x += sz + spacing;  // Circle
--             draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col);                                    x += sz + spacing;  // Square
--             draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f);                             x += sz + spacing;  // Square with all rounded corners
--             draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_tl_br);              x += sz + spacing;  // Square with two rounded corners
--             draw_list->AddTriangleFilled(ImVec2(x+sz*0.5f,y), ImVec2(x+sz, y+sz-0.5f), ImVec2(x, y+sz-0.5f), col);  x += sz + spacing;  // Triangle
--             //draw_list->AddTriangleFilled(ImVec2(x+sz*0.2f,y), ImVec2(x, y+sz-0.5f), ImVec2(x+sz*0.4f, y+sz-0.5f), col); x += sz*0.4f + spacing; // Thin triangle
--             draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + thickness), col);                             x += sz + spacing;  // Horizontal line (faster than AddLine, but only handle integer thickness)
--             draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + thickness, y + sz), col);                             x += spacing * 2.0f;// Vertical line (faster than AddLine, but only handle integer thickness)
--             draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), col);                                      x += sz;            // Pixel (faster than AddLine)
--             draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + sz, y + sz), IM_COL32(0, 0, 0, 255), IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255), IM_COL32(0, 255, 0, 255));
--
--             r.ImGui_Dummy(ImVec2((sz + spacing) * 10.2f, (sz + spacing) * 3.0f));
--             r.ImGui_PopItemWidth();
--             r.ImGui_EndTabItem();
--         }
--
--         if (r.ImGui_BeginTabItem("Canvas"))
--         {
--             static ImVector<ImVec2> points;
--             static ImVec2 scrolling(0.0f, 0.0f);
--             static bool opt_enable_grid = true;
--             static bool opt_enable_context_menu = true;
--             static bool adding_line = false;
--
--             r.ImGui_Checkbox("Enable grid", &opt_enable_grid);
--             r.ImGui_Checkbox("Enable context menu", &opt_enable_context_menu);
--             r.ImGui_Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
--
--             // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
--             // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
--             // To use a child window instead we could use, e.g:
--             //      r.ImGui_PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
--             //      r.ImGui_PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
--             //      r.ImGui_BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
--             //      r.ImGui_PopStyleColor();
--             //      r.ImGui_PopStyleVar();
--             //      [...]
--             //      r.ImGui_EndChild();
--
--             // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
--             ImVec2 canvas_p0 = r.ImGui_GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
--             ImVec2 canvas_sz = r.ImGui_GetContentRegionAvail();   // Resize canvas to what's available
--             if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
--             if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
--             ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
--
--             // Draw border and background color
--             ImGuiIO& io = r.ImGui_GetIO();
--             ImDrawList* draw_list = r.ImGui_GetWindowDrawList();
--             draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
--             draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
--
--             // This will catch our interactions
--             r.ImGui_InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
--             const bool is_hovered = r.ImGui_IsItemHovered(); // Hovered
--             const bool is_active = r.ImGui_IsItemActive();   // Held
--             const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
--             const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
--
--             // Add first and second point
--             if (is_hovered && !adding_line && r.ImGui_IsMouseClicked(ImGuiMouseButton_Left))
--             {
--                 points.push_back(mouse_pos_in_canvas);
--                 points.push_back(mouse_pos_in_canvas);
--                 adding_line = true;
--             }
--             if (adding_line)
--             {
--                 points.back() = mouse_pos_in_canvas;
--                 if (!r.ImGui_IsMouseDown(ImGuiMouseButton_Left))
--                     adding_line = false;
--             }
--
--             // Pan (we use a zero mouse threshold when there's no context menu)
--             // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
--             const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
--             if (is_active && r.ImGui_IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
--             {
--                 scrolling.x += io.MouseDelta.x;
--                 scrolling.y += io.MouseDelta.y;
--             }
--
--             // Context menu (under default mouse threshold)
--             ImVec2 drag_delta = r.ImGui_GetMouseDragDelta(ImGuiMouseButton_Right);
--             if (opt_enable_context_menu && r.ImGui_IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
--                 r.ImGui_OpenPopupOnItemClick("context");
--             if (r.ImGui_BeginPopup("context"))
--             {
--                 if (adding_line)
--                     points.resize(points.size() - 2);
--                 adding_line = false;
--                 if (r.ImGui_MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
--                 if (r.ImGui_MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
--                 r.ImGui_EndPopup();
--             }
--
--             // Draw grid + all lines in the canvas
--             draw_list->PushClipRect(canvas_p0, canvas_p1, true);
--             if (opt_enable_grid)
--             {
--                 const float GRID_STEP = 64.0f;
--                 for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
--                     draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
--                 for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
--                     draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
--             }
--             for (int n = 0; n < points.Size; n += 2)
--                 draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
--             draw_list->PopClipRect();
--
--             r.ImGui_EndTabItem();
--         }
--
--         if (r.ImGui_BeginTabItem("BG/FG draw lists"))
--         {
--             static bool draw_bg = true;
--             static bool draw_fg = true;
--             r.ImGui_Checkbox("Draw in Background draw list", &draw_bg);
--             r.ImGui_SameLine(); HelpMarker("The Background draw list will be rendered below every Dear ImGui windows.");
--             r.ImGui_Checkbox("Draw in Foreground draw list", &draw_fg);
--             r.ImGui_SameLine(); HelpMarker("The Foreground draw list will be rendered over every Dear ImGui windows.");
--             ImVec2 window_pos = r.ImGui_GetWindowPos();
--             ImVec2 window_size = r.ImGui_GetWindowSize();
--             ImVec2 window_center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
--             if (draw_bg)
--                 r.ImGui_GetBackgroundDrawList()->AddCircle(window_center, window_size.x * 0.6f, IM_COL32(255, 0, 0, 200), 0, 10 + 4);
--             if (draw_fg)
--                 r.ImGui_GetForegroundDrawList()->AddCircle(window_center, window_size.y * 0.6f, IM_COL32(0, 255, 0, 200), 0, 10);
--             r.ImGui_EndTabItem();
--         }
--
--         r.ImGui_EndTabBar();
--     }
--
--     r.ImGui_End();
-- }
--
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

demo.loop()
