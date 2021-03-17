#!/usr/bin/env ruby

# Use to identify differences between dear imgui's API and ReaImGui's API
# This tool also performs various sanity checks on the exported API

# these functions we cannot port
NATIVE_ONLY = [
  'bool ImGui::DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t)',
  'ImGuiContext* ImGui::CreateContext(ImFontAtlas*)',
  'ImGuiContext* ImGui::GetCurrentContext()',
  'void ImGui::SetCurrentContext(ImGuiContext*)',
  'void ImGui::DestroyContext(ImGuiContext*)',
  'ImGuiIO& ImGui::GetIO()',
  'ImGuiStyle& ImGui::GetStyle()',
  'void ImGui::NewFrame()',
  'void ImGui::EndFrame()',
  'void ImGui::Render()',
  'ImDrawData* ImGui::GetDrawData()',
  'ImDrawListSharedData* ImGui::GetDrawListSharedData()',
  'ImGuiViewport* ImGui::GetMainViewport()',
  'void ImGui::CaptureKeyboardFromApp(bool)',
  'void ImGui::CaptureMouseFromApp(bool)',

  'ImFont* ImGui::GetFont()',
  'void ImGui::PushFont(ImFont*)',
  'void ImGui::PopFont()',
  'void ImDrawList::AddText(const ImFont*, float, const ImVec2&, ImU32, const char*, const char*, float, const ImVec4*)',

  'void ImGui::ShowDemoWindow(bool*)',
  'void ImGui::ShowAboutWindow(bool*)',
  'void ImGui::ShowUserGuide()',
  'void ImGui::ShowStyleEditor(ImGuiStyle*)',
  'bool ImGui::ShowStyleSelector(const char*)',
  'void ImGui::ShowFontSelector(const char*)',
  'void ImGui::StyleColorsDark(ImGuiStyle*)',
  'void ImGui::StyleColorsLight(ImGuiStyle*)',
  'void ImGui::StyleColorsClassic(ImGuiStyle*)',

  'void ImGui::SetAllocatorFunctions(void* (*alloc_func)(size_t sz, void* user_data), void (*free_func)(void* ptr, void* user_data), void*)',
  'void* ImGui::MemAlloc(size_t)',
  'void ImGui::MemFree(void*)',

  'void ImGui::TextUnformatted(const char*, const char*)',
  'bool ImGui::CollapsingHeader(const char*, ImGuiTreeNodeFlags)',
  'bool ImGui::TreeNode(const char*)',
  'bool ImGui::TreeNode(const char*, const char*, ...)',

  'void ImDrawList::AddCallback(ImDrawCallback, void*)',
  'void ImDrawList::AddDrawCmd()',

  'void ImGui::SetStateStorage(ImGuiStorage*)',
  'ImGuiStorage* ImGui::GetStateStorage()',
  'void ImGui::LoadIniSettingsFromDisk(const char*)',
  'void ImGui::LoadIniSettingsFromMemory(const char*, size_t)',
  'void ImGui::SaveIniSettingsToDisk(const char*)',
  'const char* ImGui::SaveIniSettingsToMemory(size_t*)',

  'ImVec4 ImGui::ColorConvertU32ToFloat4(ImU32)',
  'ImU32 ImGui::ColorConvertFloat4ToU32(const ImVec4&)',

  # equivalent overload implemented as GetColorEx
  'ImU32 ImGui::GetColorU32(const ImVec4&)',

  # only const char* IDs
  'void ImGui::PushID(int)',
  'void ImGui::PushID(const void*)',
  'void ImGui::PushID(const char*, const char*)',
  'bool ImGui::BeginChild(ImGuiID, const ImVec2&, bool, ImGuiWindowFlags)',

  'ImGuiID ImGui::GetID(const char*)',
  'ImGuiID ImGui::GetID(const char*, const char*)',
  'ImGuiID ImGui::GetID(const void*)',

  'void ImGui::TreePush(const void*)',
  'bool ImGui::TreeNode(const void*, const char*, ...)',
  'bool ImGui::TreeNodeEx(const void*, ImGuiTreeNodeFlags, const char*, ...)',

  # item getter callback overloads
  'bool ImGui::Combo(const char*, int*, bool(*items_getter)(void* data, int idx, const char** out_text), void*, int, int)',
  'bool ImGui::ListBox(const char*, int*, bool (*items_getter)(void* data, int idx, const char** out_text), void*, int, int)',
  'void ImGui::PlotLines(const char*, float(*values_getter)(void* data, int idx), void*, int, int, const char*, float, float, ImVec2)',
  'void ImGui::PlotHistogram(const char*, float(*values_getter)(void* data, int idx), void*, int, int, const char*, float, float, ImVec2)',

  # float/ImVec2 overloads
  'void ImGui::PushStyleVar(ImGuiStyleVar, float)',

  # array + explicit size overload
  'bool ImGui::Combo(const char*, int*, const char* const, int, int)',

  # va_list overloads
  'bool ImGui::TreeNodeV(const char*, const char*, va_list)',
  'bool ImGui::TreeNodeV(const void*, const char*, va_list)',
  'bool ImGui::TreeNodeExV(const char*, ImGuiTreeNodeFlags, const char*, va_list)',
  'bool ImGui::TreeNodeExV(const void*, ImGuiTreeNodeFlags, const char*, va_list)',
  'void ImGui::SetTooltipV(const char*, va_list)',
  'void ImGui::TextV(const char*, va_list)',
  'void ImGui::TextColoredV(const ImVec4&, const char*, va_list)',
  'void ImGui::TextDisabledV(const char*, va_list)',
  'void ImGui::TextWrappedV(const char*, va_list)',
  'void ImGui::LabelTextV(const char*, const char*, va_list)',
  'void ImGui::BulletTextV(const char*, va_list)',

  # not recommended (use SetNextWindow* instead)
  'void ImGui::SetWindowPos(const ImVec2&, ImGuiCond)',
  'void ImGui::SetWindowSize(const ImVec2&, ImGuiCond)',
  'void ImGui::SetWindowCollapsed(bool, ImGuiCond)',
  'void ImGui::SetWindowFocus()',
  'void ImGui::SetWindowFontScale(float)', # old API

  # use the list clipper API instead
  'void ImGui::CalcListClipping(int, float, int*, int*)',

  # single-component scalar input
  'bool ImGui::DragScalar(const char*, ImGuiDataType, void*, float, const void*, const void*, const char*, ImGuiSliderFlags)',
  'bool ImGui::SliderScalar(const char*, ImGuiDataType, void*, const void*, const void*, const char*, ImGuiSliderFlags)',
  'bool ImGui::VSliderScalar(const char*, const ImVec2&, ImGuiDataType, void*, const void*, const void*, const char*, ImGuiSliderFlags)',
  'bool ImGui::InputScalar(const char*, ImGuiDataType, void*, const void*, const void*, const char*, ImGuiInputTextFlags)',

  # legacy Columns API (2020: prefer using Tables!)
  'void ImGui::Columns(int, const char*, bool)',
  'void ImGui::NextColumn()',
  'int ImGui::GetColumnIndex()',
  'float ImGui::GetColumnWidth(int)',
  'void ImGui::SetColumnWidth(int, float)',
  'float ImGui::GetColumnOffset(int)',
  'void ImGui::SetColumnOffset(int, float)',
  'int ImGui::GetColumnsCount()',

  # primitives allocation
  'void ImDrawList::PrimReserve(int, int)',
  'void ImDrawList::PrimUnreserve(int, int)',
  'void ImDrawList::PrimRect(const ImVec2&, const ImVec2&, ImU32)',
  'void ImDrawList::PrimRectUV(const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, ImU32)',
  'void ImDrawList::PrimQuadUV(const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, ImU32)',

  # images
  'ImVec2 ImGui::GetFontTexUvWhitePixel()',
  'void ImGui::Image(ImTextureID, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec4&, const ImVec4&)',
  'bool ImGui::ImageButton(ImTextureID, const ImVec2&, const ImVec2&, const ImVec2&, int, const ImVec4&, const ImVec4&)',
  'void ImDrawList::PushTextureID(ImTextureID)',
  'void ImDrawList::PopTextureID()',
  'void ImDrawList::AddImage(ImTextureID, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, ImU32)',
  'void ImDrawList::AddImageQuad(ImTextureID, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, ImU32)',
  'void ImDrawList::AddImageRounded(ImTextureID, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, ImU32, float, ImDrawCornerFlags)',

  # value helpers
  'void ImGui::Value(const char*, bool)',
  'void ImGui::Value(const char*, int)',
  'void ImGui::Value(const char*, unsigned int)',
  'void ImGui::Value(const char*, float, const char*)',
]

NATIVE_ONLY_CLASSES = %w[
  ImGuiIO ImFontAtlas ImFont ImDrawData ImDrawListSplitter ImGuiStoragePair
  ImGuiStyle ImGuiInputTextCallbackData ImFontGlyphRangesBuilder
  ImGuiTextBuffer ImGuiTextFilter
]

NATIVE_ONLY_ENUMS = [
  /\AInputTextFlags_Callback/,
  /\ADataType_/,
  /\ANavInput_/,
  /\ABackendFlags_/,
  /\AFontAtlasFlags_/,
  'Cond_None',          # alias for Cond_Always
  /\AKey_.+/,           # for GetKeyIndex, not implemented
  /_NoSavedSettings\z/, # saved settings are not implemented yet
  'ColorEditFlags_HDR', # not allowed, would break float[4]<->int conversion
  /\AViewportFlags_/,
  'MouseCursor_None',   # not implemented under SWELL
  'TreeNodeFlags_NavLeftJumpsBackHere', # marked as WIP
  /\ATableFlags_NoBordersInBody/,       # marked as alpha, to be moved to style
  /\AConfigFlags_(NavEnableGamepad|NavNoCaptureKeyboard)\z/, # not implemented
  /\AConfigFlags_(IsSRGB|IsTouchScreen)\z/, # backend internal flags

  # only for dear imgui's internal use
  'InputTextFlags_Multiline',
  'InputTextFlags_NoMarkEdited',
  /\AWindowFlags_(NavFlattened|ChildWindow|Tooltip|Popup|Modal|ChildMenu)\z/,
  /\AColorEditFlags__.+Mask\z/,
  /\ADrawListFlags_/,
]

# these functions were ported using another name (eg. overloads)
RENAMES = {
  'bool ImGui::RadioButton(const char*, int*, int)'         => 'RadioButtonEx',
  'ImU32 ImGui::GetColorU32(ImGuiCol, float)'               => 'GetColor',
  'bool ImGui::TreeNodeEx(const char*, ImGuiTreeNodeFlags)' => 'TreeNode',
  'ImU32 ImGui::GetColorU32(ImU32)'                         => 'GetColorEx',
  'const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol)'        => 'GetStyleColor',
  'bool ImGui::IsRectVisible(const ImVec2&, const ImVec2&)' => 'IsRectVisibleEx',
  'ImGuiTableSortSpecs* ImGui::TableGetSortSpecs()'         => 'TableGetColumnSortSpecs',

  # variable-component input only supports double
  'bool ImGui::DragScalarN(const char*, ImGuiDataType, void*, int, float, const void*, const void*, const char*, ImGuiSliderFlags)' => 'DragDoubleN',
  'bool ImGui::SliderScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiSliderFlags)'      => 'SliderDoubleN',
  'bool ImGui::InputScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiInputTextFlags)'    => 'InputDoubleN',
}

ARG_RENAMES = {
  'IsKeyDown'     => { 'user_key_index' => 'key_code' },
  'IsKeyPressed'  => { 'user_key_index' => 'key_code' },
  'IsKeyReleased' => { 'user_key_index' => 'key_code' },
}

# these functions were not ported 1:1 (same name, otherwise add to RENAMES above too!)
OVERRIDES = {
  'const char* ImGui::GetVersion()' => 'void GetVersion(char*, int, char*, int)',
  'void ImGui::ColorConvertHSVtoRGB(float, float, float, float&, float&, float&)' => 'int ColorConvertHSVtoRGB(double, double, double, double*, double*, double*, double*)',
  'void ImGui::ColorConvertRGBtoHSV(float, float, float, float&, float&, float&)' => 'int ColorConvertRGBtoHSV(double, double, double, double*, double*, double*, double*)',
  'void ImGui::PushStyleVar(ImGuiStyleVar, const ImVec2&)'                        => 'void PushStyleVar(int, double, double*)',
  'bool ImGui::SetDragDropPayload(const char*, const void*, size_t, ImGuiCond)'   => 'bool SetDragDropPayload(const char*, const char*, int*)',
  'const ImGuiPayload* ImGui::GetDragDropPayload()'                               => 'bool GetDragDropPayload(char*, int, char*, int, bool*, bool*)',
  'bool ImGui::TreeNodeEx(const char*, ImGuiTreeNodeFlags, const char*, ...)'     => 'bool TreeNodeEx(const char*, const char*, int*)',
  'ImGuiTableSortSpecs* ImGui::TableGetSortSpecs()'                               => 'bool TableGetColumnSortSpecs(int, int*, int*, int*, int*)',

  # color array -> packed int
  'bool ImGui::ColorPicker4(const char*, float[4], ImGuiColorEditFlags, const float*)' => 'bool ColorPicker4(const char*, int*, int*, int*)',
  'const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol)' => 'int GetStyleColor(int)',

  # (array, array_size) -> reaper_array*
  'void ImGui::PlotLines(const char*, const float*, int, int, const char*, float, float, ImVec2, int)'     => 'void PlotLines(const char*, reaper_array*, int*, const char*, double*, double*, double*, double*)',
  'void ImGui::PlotHistogram(const char*, const float*, int, int, const char*, float, float, ImVec2, int)' => 'void PlotHistogram(const char*, reaper_array*, int*, const char*, double*, double*, double*, double*)',
  'void ImDrawList::AddPolyline(const ImVec2*, int, ImU32, bool, float)' => 'void DrawList_AddPolyline(reaper_array*, int, bool, double)',
  'void ImDrawList::AddConvexPolyFilled(const ImVec2*, int, ImU32)'      => 'void DrawList_AddConvexPolyFilled(reaper_array*, int, int)',

  # no callbacks
  'bool ImGui::InputText(const char*, char*, size_t, ImGuiInputTextFlags, ImGuiInputTextCallback, void*)'                         => 'bool InputText(const char*, char*, int, int*)',
  'bool ImGui::InputTextMultiline(const char*, char*, size_t, const ImVec2&, ImGuiInputTextFlags, ImGuiInputTextCallback, void*)' => 'bool InputTextMultiline(const char*, char*, int, double*, double*, int*)',
  'bool ImGui::InputTextWithHint(const char*, const char*, char*, size_t, ImGuiInputTextFlags, ImGuiInputTextCallback, void*)'    => 'bool InputTextWithHint(const char*, const char*, char*, int, int*)',
  'void ImGui::SetNextWindowSizeConstraints(const ImVec2&, const ImVec2&, ImGuiSizeCallback, void*)' => 'void SetNextWindowSizeConstraints(double, double, double, double)',

  # const char* (null-terminated) -> char* (\31-terminated)
  'bool ImGui::Combo(const char*, int*, const char*, int)' => 'bool Combo(const char*, int*, char*, int*)',

  # const char*[] + int size -> char* (\31-terminated)
  'bool ImGui::ListBox(const char*, int*, const char* const, int, int)' => 'bool ListBox(const char*, int*, char*, int*)',

  # no text_end argument
  'ImVec2 ImGui::CalcTextSize(const char*, const char*, bool, float)'        => 'void CalcTextSize(const char*, double*, double*, bool*, double*)',
  'void ImDrawList::AddText(const ImVec2&, ImU32, const char*, const char*)' => 'void DrawList_AddText(double, double, int, const char*)',

  'bool ImGui::DragScalarN(const char*, ImGuiDataType, void*, int, float, const void*, const void*, const char*, ImGuiSliderFlags)' => 'bool DragDoubleN(const char*, reaper_array*, double*, double*, double*, const char*, int*)',
  'bool ImGui::SliderScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiSliderFlags)'      => 'bool SliderDoubleN(const char*, reaper_array*, double, double, const char*, int*)',
  'bool ImGui::InputScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiInputTextFlags)'    => 'bool InputDoubleN(const char*, reaper_array*, double*, double*, const char*, int*)',

  # drag/drop payload as string
  'const ImGuiPayload* ImGui::AcceptDragDropPayload(const char*, ImGuiDragDropFlags)' => 'bool AcceptDragDropPayload(const char*, char*, int, int*)',
}

# argument position & name are checked
RESOURCES = {
  'ImGui_Context*'     => 'ctx',
  'ImGui_DrawList*'    => 'draw_list',
  'ImGui_ListClipper*' => 'clipper',
}

# types REAPER's parser knows about
TYPES = [
  'bool',   'bool*',
  'int',    'int*',    # no unsigned int :-(
  'double', 'double*', # no float :'(
  'ImGui_Context*',
  'ImGui_DrawList*',
  'ImGui_ListClipper*',
  'char*',
  'const char*',
  'reaper_array*',
  'void*',
]

# load ImGui definitions
require_relative 'lib/imgui'
imgui = ImGui.new

puts "imgui:    found %d functions, %d enums (total: %d symbols)" %
  [imgui.funcs.size, imgui.enums.size, imgui.funcs.size + imgui.enums.size]

require_relative 'lib/reaimgui'
reaimgui = ReaImGui.new
puts "reaimgui: found %d functions, %d enums (total: %d symbols)" %
  [reaimgui.funcs.size, reaimgui.enums.size, reaimgui.funcs.size + reaimgui.enums.size]

# basic argument type validation
reaimgui.funcs.each do |func|
  unless func.type == 'void' || TYPES.include?(func.type)
    warn "#{func.name}: invalid return type: #{func.type}"
  end

  func.args.each_with_index do |arg, i|
    unless TYPES.include? arg.type
      warn "#{func.name}: invalid argument type for '#{arg.name}': #{arg.type}"
    end

    if RESOURCES.has_key?(arg.type)
      unless i == 0
        warn "#{func.name}: argument of type '#{arg.type}' should come first"
      end

      unless arg.name == RESOURCES[arg.type]
        warn "#{func.name}: argument of type '#{arg.type}' should be named '#{RESOURCES[arg.type]}', got '#{arg.name}'"
      end
    end
  end

  # ignore leading resource type for easier matching
  func.args.shift if RESOURCES.has_key? func.args.first.type
end

NATIVE_ONLY.each do |sig|
  unless imgui.funcs.any? { _1.sig == sig }
    warn "function marked as native only not found in dear imgui: #{sig}"
  end
end

NATIVE_ONLY_ENUMS.each do |rule|
  reaimgui.enums.select { _1.match? rule }.each do |enum|
    warn "enum marked as native only but exported anyway: #{enum}"
  end
end

# link dear imgui functions to their corresponding ReaImGui counterparts
perfect_count = manual_count = missing_overloads = missing_count = skipped_count = 0
imgui.funcs.each do |imgui_func|
  if imgui_func.name[0] == '_' ||
      NATIVE_ONLY.include?(imgui_func.sig) ||
      NATIVE_ONLY_CLASSES.include?(imgui_func.namespace)
    skipped_count += 1
    next
  end

  candidate = reaimgui.funcs.find { imgui_func.normalized.name == _1.name }
  expected_sig = OVERRIDES[imgui_func.sig] || imgui_func.normalized.sig

  if not candidate
    missing_count += 1
    warn "not implemented: #{imgui_func.sig}"
    warn "  expected:  #{expected_sig}"
    next
  end

  perfect_match = candidate.sig == expected_sig

  if perfect_match
    if OVERRIDES.has_key? imgui_func.sig
      manual_count += 1
    else
      perfect_count += 1
      candidate.match = imgui_func
    end
  else
    missing_overloads += 1

    warn "not implemented: #{imgui_func.sig}"
    warn "  expected:  #{expected_sig}"
    warn "  candidate: #{candidate.sig}"
  end
end

# check argument names and default values
reaimgui.funcs.each do |func|
  func.args.each_with_index do |rea_arg, i|
    unless rea_arg.name
      warn "#{func.name}: invalid argument ##{i+1} '#{rea_arg.name}'"
      next
    end

    unless rea_arg.name =~ /\A[a-z0-9_]+\z/
      warn "#{func.name}: invalid argument ##{i+1} name '#{rea_arg.name}' (not snake case?)"
      next
    end

    unless func.match
      if rea_arg.decoration && rea_arg.decoration[-1] == 'O' && rea_arg.default.nil?
        warn "#{func.name}: argument ##{i+1} '#{rea_arg.name}' has no documented default value"
      end
      next
    end

    imgui_arg = func.match.normalized.args[i]

    unless rea_arg.name == imgui_arg.name || rea_arg.name == ARG_RENAMES[func.name]&.[](imgui_arg.name)
      warn "#{func.name}: argument ##{i+1} of type '#{rea_arg.type}' (#{rea_arg.decoration}) is named '#{rea_arg.name}', expected '#{imgui_arg.name}'"
    end

    unless rea_arg.default == imgui_arg.default
      if rea_arg.default.nil?
        warn "#{func.name}: argument ##{i+1} '#{rea_arg.name}' has no documented default value, expected #{imgui_arg.default}"
      else
        warn "#{func.name}: argument ##{i+1} '#{rea_arg.name}' has documented default value #{rea_arg.default}, expected #{imgui_arg.default}"
      end
    end
  end
end

skipped_enums = 0
imgui.enums.each do |im_enum|
  unless reaimgui.enums.include? im_enum
    if NATIVE_ONLY_ENUMS.any? { im_enum.match? _1 }
      skipped_enums += 1
      next
    end

    warn "missing enum: #{im_enum}"
  end
end

puts
puts "functions: %d perfect matches, %d manual matches, %d missing overloads, %d not implemented, %d skipped" %
  [perfect_count, manual_count, missing_overloads, missing_count, skipped_count]

puts "functions: %.2f%% complete (%.2f%% total)" %
  [(perfect_count + manual_count).to_f / (imgui.funcs.size - skipped_count) * 100,
   (perfect_count + manual_count).to_f / imgui.funcs.size * 100]

puts "enums:     %d skipped" % skipped_enums
puts "enums:     %.2f%% complete (%.2f%% total)" %
  [reaimgui.enums.size.to_f / (imgui.enums.size - skipped_enums) * 100,
   reaimgui.enums.size.to_f / imgui.enums.size * 100]
