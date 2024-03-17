# Warning: Python ReaScripts have significant CPU overhead compared to Lua/EEL2

sys.path.append(RPR_GetResourcePath() + "/Scripts/ReaTeam Extensions/API")
import imgui as ImGui

FLT_MIN, FLT_MAX = ImGui.NumericLimits_Float()

def init():
  global ctx, clipper
  ctx = ImGui.CreateContext("Track manager")
  clipper = ImGui.CreateListClipper(ctx)
  ImGui.Attach(ctx, clipper)
  loop()

def paramCheckbox(track, param):
  value = RPR_GetMediaTrackInfo_Value(track, param)
  changed, checked = ImGui.Checkbox(ctx, "##" + param, value)
  if changed:
    RPR_SetMediaTrackInfo_Value(track, param, checked)
    return True

  return False

def trackRow(ti):
  track = RPR_GetTrack(None, ti)

  if ImGui.TableSetColumnIndex(ctx, 0):
    color = ImGui.ColorConvertNative(RPR_GetTrackColor(track))
    changed, color = ImGui.ColorEdit3(ctx, "##color", color,
      ImGui.ColorEditFlags_NoInputs() | ImGui.ColorEditFlags_NoLabel())
    if changed:
      RPR_SetTrackColor(track, ImGui.ColorConvertNative(color))

  if ImGui.TableSetColumnIndex(ctx, 1):
    selected = RPR_IsTrackSelected(track)
    selectable = ImGui.Selectable(ctx, ti + 1, selected,
      ImGui.SelectableFlags_SpanAllColumns() |
      ImGui.SelectableFlags_AllowOverlap())
    if selectable[0]:
      if ImGui.IsKeyDown(ctx, ImGui.Mod_Ctrl()):
        RPR_SetTrackSelected(track, not selected)
      else:
        RPR_SetOnlyTrackSelected(track)

  if ImGui.TableSetColumnIndex(ctx, 2):
    name = RPR_GetSetMediaTrackInfo_String(track, 'P_NAME', '', False)[3]
    ImGui.SetNextItemWidth(ctx, -FLT_MIN)
    changed, name = ImGui.InputText(ctx, "##name", name, 0)
    if changed:
      RPR_GetSetMediaTrackInfo_String(track, 'P_NAME', name, True)

  if ImGui.TableSetColumnIndex(ctx, 3):
    if paramCheckbox(track, 'B_SHOWINTCP'):
      RPR_TrackList_AdjustWindows(True)

  if ImGui.TableSetColumnIndex(ctx, 4):
    if paramCheckbox(track, 'B_SHOWINMIXER'):
      RPR_TrackList_AdjustWindows(False)

  if ImGui.TableSetColumnIndex(ctx, 5):
    fxCount = RPR_TrackFX_GetCount(track)
    if ImGui.Selectable(ctx, fxCount if fxCount > 0 else '', False)[0]:
      RPR_TrackFX_Show(track, 0, 1)

  if ImGui.TableSetColumnIndex(ctx, 6):
    fxCount = RPR_TrackFX_GetRecCount(track)
    if fxCount > 0:
      ImGui.Text(ctx, fxCount)

  if ImGui.TableSetColumnIndex(ctx, 7):
    paramCheckbox(track, 'I_RECARM')
  if ImGui.TableSetColumnIndex(ctx, 8):
   paramCheckbox(track, 'B_MUTE')
  if ImGui.TableSetColumnIndex(ctx, 9):
    paramCheckbox(track, 'I_SOLO')
  if ImGui.TableSetColumnIndex(ctx, 10):
    paramCheckbox(track, 'B_HEIGHTLOCK')
  if ImGui.TableSetColumnIndex(ctx, 11):
    if paramCheckbox(track, 'B_FREEMODE'):
      RPR_UpdateArrange()

def trackTable():
  flags = ImGui.TableFlags_SizingFixedFit() | ImGui.TableFlags_RowBg()   | \
          ImGui.TableFlags_BordersOuter()   | ImGui.TableFlags_Borders() | \
          ImGui.TableFlags_Reorderable()    | ImGui.TableFlags_ScrollY() | \
          ImGui.TableFlags_Resizable()      | ImGui.TableFlags_Hideable()

  if not ImGui.BeginTable(ctx, "tracks", 12, flags):
    return

  ImGui.TableSetupScrollFreeze(ctx, 0, 1)

  ImGui.TableSetupColumn(ctx, "Color",       ImGui.TableColumnFlags_NoHeaderWidth())
  ImGui.TableSetupColumn(ctx, "#",           ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "Name",        ImGui.TableColumnFlags_WidthStretch())
  ImGui.TableSetupColumn(ctx, "TCP",         ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "MCP",         ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "FX",          ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "IN-FX",       ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "R",           ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "M",           ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "S",           ImGui.TableColumnFlags_None())
  ImGui.TableSetupColumn(ctx, "Height Lock", ImGui.TableColumnFlags_NoHeaderWidth())
  ImGui.TableSetupColumn(ctx, "FIPM",        ImGui.TableColumnFlags_NoHeaderWidth())
  ImGui.TableHeadersRow(ctx)

  trackCount = RPR_CountTracks(None)
  ImGui.ListClipper_Begin(clipper, trackCount)
  while ImGui.ListClipper_Step(clipper):
    displayStart, displayEnd = ImGui.ListClipper_GetDisplayRange(clipper)
    for ti in range(displayStart, displayEnd):
      ImGui.TableNextRow(ctx)
      ImGui.PushID(ctx, ti)
      trackRow(ti)
      ImGui.PopID(ctx)

  ImGui.EndTable(ctx)

def loop():
  ImGui.SetNextWindowSize(ctx, 700, 500, ImGui.Cond_FirstUseEver())
  visible, open = ImGui.Begin(ctx, "Track manager", True)

  if visible:
    if ImGui.Button(ctx, "Add track"):
      RPR_InsertTrackAtIndex(-1, True)

    trackTable()

    ImGui.End(ctx)

  if open:
    RPR_defer("loop()")

RPR_defer("init()")
