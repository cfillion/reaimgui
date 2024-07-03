#!/usr/bin/env ruby

# Use to identify differences between dear imgui's API and ReaImGui's API
# This tool also performs various sanity checks on the exported API

IMGUI_H = File.join __dir__, '..', 'subprojects', 'imgui', 'imgui', 'imgui.h'
REAIMGUI_API_CPP = File.join __dir__, '..', 'api', '*.cpp'

# these functions we cannot port
NATIVE_ONLY = [
  'bool ImGui::DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t)',
  'ImGuiContext* ImGui::CreateContext(ImFontAtlas*)',
  'ImGuiContext* ImGui::GetCurrentContext()',
  'void ImGui::SetCurrentContext(ImGuiContext*)',
  'void ImGui::DestroyContext(ImGuiContext*)',

  'ImGuiIO& ImGui::GetIO()',
  'ImGuiPlatformIO& ImGui::GetPlatformIO()',
  'ImGuiStyle& ImGui::GetStyle()',

  'void ImGui::NewFrame()',
  'void ImGui::EndFrame()',
  'void ImGui::Render()',
  'void ImGui::UpdatePlatformWindows()',
  'void ImGui::RenderPlatformWindowsDefault(void*, void*)',
  'void ImGui::DestroyPlatformWindows()',

  'ImGuiViewport* ImGui::FindViewportByID(ImGuiID)',
  'ImGuiViewport* ImGui::FindViewportByPlatformHandle(void*)',

  'void ImGui::SetNextFrameWantCaptureMouse(bool)',

  'const char* ImGui::GetKeyName(ImGuiKey)',
  'const char* ImGui::GetStyleColorName(ImGuiCol)',

  'void ImGui::ShowDemoWindow(bool*)',
  'void ImGui::ShowUserGuide()',
  'void ImGui::ShowStyleEditor(ImGuiStyle*)',
  'bool ImGui::ShowStyleSelector(const char*)',
  'void ImGui::ShowFontSelector(const char*)',
  'void ImGui::StyleColorsDark(ImGuiStyle*)',
  'void ImGui::StyleColorsLight(ImGuiStyle*)',
  'void ImGui::StyleColorsClassic(ImGuiStyle*)',

  'void ImGui::GetAllocatorFunctions(ImGuiMemAllocFunc*, ImGuiMemFreeFunc*, void**)',
  'void ImGui::SetAllocatorFunctions(ImGuiMemAllocFunc, ImGuiMemFreeFunc, void*)',
  'void* ImGui::MemAlloc(size_t)',
  'void ImGui::MemFree(void*)',

  'void ImGui::TextUnformatted(const char*, const char*)',
  'bool ImGui::CollapsingHeader(const char*, ImGuiTreeNodeFlags)',
  'bool ImGui::TreeNode(const char*)',
  'bool ImGui::TreeNode(const char*, const char*, ...)',

  'ImDrawData* ImGui::GetDrawData()',
  'ImDrawListSharedData* ImGui::GetDrawListSharedData()',
  'void ImDrawList::AddCallback(ImDrawCallback, void*)',
  'void ImDrawList::AddDrawCmd()',

  'void ImDrawListSplitter::ClearFreeMemory()',

  'void ImGui::SetStateStorage(ImGuiStorage*)',
  'ImGuiStorage* ImGui::GetStateStorage()',
  'void ImGui::LoadIniSettingsFromDisk(const char*)',
  'void ImGui::LoadIniSettingsFromMemory(const char*, size_t)',
  'void ImGui::SaveIniSettingsToDisk(const char*)',
  'const char* ImGui::SaveIniSettingsToMemory(size_t*)',

  # equivalent overload implemented as GetColorEx
  'ImU32 ImGui::GetColorU32(const ImVec4&)',

  # only const char* IDs
  'void ImGui::PushID(int)',
  'void ImGui::PushID(const void*)',
  'void ImGui::PushID(const char*, const char*)',
  'bool ImGui::BeginChild(ImGuiID, const ImVec2&, ImGuiChildFlags, ImGuiWindowFlags)',
  'void ImGui::OpenPopup(ImGuiID, ImGuiPopupFlags)',

  'ImGuiID ImGui::GetID(const char*)',
  'ImGuiID ImGui::GetID(const char*, const char*)',
  'ImGuiID ImGui::GetID(const void*)',

  'void ImGui::TreePush(const void*)',
  'bool ImGui::TreeNode(const void*, const char*, ...)',
  'bool ImGui::TreeNodeEx(const void*, ImGuiTreeNodeFlags, const char*, ...)',

  # item getter callback overloads
  'bool ImGui::Combo(const char*, int*, const char* (*getter)(void* user_data, int idx), void*, int, int)',
  'bool ImGui::ListBox(const char*, int*, const char* (*getter)(void* user_data, int idx), void*, int, int)',
  'void ImGui::PlotLines(const char*, float(*values_getter)(void* data, int idx), void*, int, int, const char*, float, float, ImVec2)',
  'void ImGui::PlotHistogram(const char*, float (*values_getter)(void* data, int idx), void*, int, int, const char*, float, float, ImVec2)',

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
  'void ImGui::SetItemTooltipV(const char*, va_list)',
  'void ImGui::TextV(const char*, va_list)',
  'void ImGui::TextColoredV(const ImVec4&, const char*, va_list)',
  'void ImGui::TextDisabledV(const char*, va_list)',
  'void ImGui::TextWrappedV(const char*, va_list)',
  'void ImGui::LabelTextV(const char*, const char*, va_list)',
  'void ImGui::BulletTextV(const char*, va_list)',
  'void ImGui::LogTextV(const char*, va_list)',

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
  'void ImDrawList::PushTextureID(ImTextureID)',
  'void ImDrawList::PopTextureID()',

  # value helpers (just Text() with a "prefix: value" format string)
  'void ImGui::Value(const char*, bool)',
  'void ImGui::Value(const char*, int)',
  'void ImGui::Value(const char*, unsigned int)',
  'void ImGui::Value(const char*, float, const char*)',

  # we're providing a more useful TextFilter_Set function
  'void ImGuiTextFilter::Build()',

  # not recommended for new designs
  'void ImGui::LogButtons()',
  'void ImGui::SetWindowFontScale(float)',

  # no main viewport
  'bool ImGui::BeginMainMenuBar()',
  'void ImGui::EndMainMenuBar()',
  'bool ImGui::BeginPopupContextVoid(const char*, ImGuiPopupFlags)',

  # experimental docking API
  'ImGuiID ImGui::DockSpace(ImGuiID, const ImVec2&, ImGuiDockNodeFlags, const ImGuiWindowClass*)',
  'ImGuiID ImGui::DockSpaceOverViewport(ImGuiID, const ImGuiViewport*, ImGuiDockNodeFlags, const ImGuiWindowClass*)',
  'void ImGui::SetNextWindowClass(const ImGuiWindowClass*)',

  # broken since v0.5 migrated to new-style Begin/End before dear imgui did
  'bool ImGui::IsWindowCollapsed()',
]

NATIVE_ONLY_CLASSES = %w[
  ImGuiIO ImFontAtlas ImFont ImDrawData ImGuiStorage
  ImGuiStyle ImGuiInputTextCallbackData ImFontGlyphRangesBuilder
  ImGuiTextBuffer ImFontConfig
]

NATIVE_ONLY_ENUMS = [
  'InputTextFlags_CallbackResize',
  /\ADataType_/,
  'Key_None',
  /\AKey_(NamedKey|KeysData|Reserved)/,
  /\ABackendFlags_/,
  /\AFontAtlasFlags_/,
  'Cond_None',          # alias for Cond_Always
  'ColorEditFlags_HDR', # not allowed, would break float[4]<->int conversion
  /\AViewportFlags_/,
  'TreeNodeFlags_NavLeftJumpsBackHere', # marked as WIP
  /\ATableFlags_NoBordersInBody/,       # marked as alpha, to be moved to style
  /\AConfigFlags_(NavEnableGamepad)\z/, # not implemented
  /\AConfigFlags_(IsSRGB|IsTouchScreen|ViewportsEnable|DpiEnableScale(Viewports|Fonts))\z/, # backend internal flags
  'WindowFlags_NoBringToFrontOnFocus', # not supported with per-window viewports
  /\AMouseSource_/, # for backends (io.AddMouseSoruceEvent)
  'StyleVar_DockingSeparatorSize', # only applicable mid-frame to DockSpace
  /\ADragDropFlags_PayloadNoCross/,

  # only for dear imgui's internal use
  /\AWindowFlags_(ChildWindow|Tooltip|Popup|Modal|ChildMenu|DockNodeHost)\z/,
  /\ADrawListFlags_/,
  /\ADockNodeFlags_/,
]

# these functions were ported using another name (eg. overloads)
RENAMES = {
  'bool ImGui::RadioButton(const char*, int*, int)'         => 'RadioButtonEx',
  'ImU32 ImGui::GetColorU32(ImGuiCol, float)'               => 'GetColor',
  'bool ImGui::TreeNodeEx(const char*, ImGuiTreeNodeFlags)' => 'TreeNode',
  'ImU32 ImGui::GetColorU32(ImU32, float)'                  => 'GetColorEx',
  'const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol)'        => 'GetStyleColor',
  'bool ImGui::IsRectVisible(const ImVec2&, const ImVec2&)' => 'IsRectVisibleEx',
  'ImGuiTableSortSpecs* ImGui::TableGetSortSpecs()'         => 'TableGetColumnSortSpecs',
  'void ImDrawList::AddText(const ImFont*, float, const ImVec2&, ImU32, const char*, const char*, float, const ImVec4*)' => 'DrawList_AddTextEx',

  # variable-component input only supports double
  'bool ImGui::DragScalarN(const char*, ImGuiDataType, void*, int, float, const void*, const void*, const char*, ImGuiSliderFlags)' => 'DragDoubleN',
  'bool ImGui::SliderScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiSliderFlags)'      => 'SliderDoubleN',
  'bool ImGui::InputScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiInputTextFlags)'    => 'InputDoubleN',

  # SetWindow* by name
  'void ImGui::SetWindowPos(const char*, const ImVec2&, ImGuiCond)'  => 'SetWindowPosEx',
  'void ImGui::SetWindowSize(const char*, const ImVec2&, ImGuiCond)' => 'SetWindowSizeEx',
  'void ImGui::SetWindowCollapsed(const char*, bool, ImGuiCond)'     => 'SetWindowCollapsedEx',
  'void ImGui::SetWindowFocus(const char*)'                          => 'SetWindowFocusEx',
}

ARG_RENAMES = {
  'ListBox' => { 'items_count' => 'items_sz' },
  'ColorConvertDouble4ToU32' => { 'in_x' => 'r', 'in_y' => 'g', 'in_w' => 'b', 'in_h' => 'a' },
}

# these functions were not ported 1:1 (same name, otherwise add to RENAMES above too!)
OVERRIDES = {
  'const char* ImGui::GetVersion()' => 'void GetVersion(char*, int, int*, char*, int)',
  'void ImGui::PushStyleVar(ImGuiStyleVar, const ImVec2&)'                        => 'void PushStyleVar(int, double, double*)',
  'bool ImGui::SetDragDropPayload(const char*, const void*, size_t, ImGuiCond)'   => 'bool SetDragDropPayload(const char*, const char*, int*)',
  'const ImGuiPayload* ImGui::GetDragDropPayload()'                               => 'bool GetDragDropPayload(char*, int, char*, int, bool*, bool*)',
  'bool ImGui::TreeNodeEx(const char*, ImGuiTreeNodeFlags, const char*, ...)'     => 'bool TreeNodeEx(const char*, const char*, int*)',
  'ImGuiTableSortSpecs* ImGui::TableGetSortSpecs()'                               => 'bool TableGetColumnSortSpecs(int, int*, int*, int*)',
  'ImVec4 ImGui::ColorConvertU32ToFloat4(ImU32)'                                  => 'void ColorConvertU32ToDouble4(int, double*, double*, double*, double*)',

  # color array -> packed int
  'bool ImGui::ColorPicker4(const char*, float[4], ImGuiColorEditFlags, const float*)' => 'bool ColorPicker4(const char*, int*, int*, int*)',
  'const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol)' => 'int GetStyleColor(int)',

  # (array, array_size) -> reaper_array*
  'void ImGui::PlotLines(const char*, const float*, int, int, const char*, float, float, ImVec2, int)'     => 'void PlotLines(const char*, reaper_array*, int*, const char*, double*, double*, double*, double*)',
  'void ImGui::PlotHistogram(const char*, const float*, int, int, const char*, float, float, ImVec2, int)' => 'void PlotHistogram(const char*, reaper_array*, int*, const char*, double*, double*, double*, double*)',
  'void ImDrawList::AddPolyline(const ImVec2*, int, ImU32, ImDrawFlags, float)' => 'void DrawList_AddPolyline(reaper_array*, int, int, double)',
  'void ImDrawList::AddConvexPolyFilled(const ImVec2*, int, ImU32)'      => 'void DrawList_AddConvexPolyFilled(reaper_array*, int)',
  'void ImDrawList::AddConcavePolyFilled(const ImVec2*, int, ImU32)'      => 'void DrawList_AddConcavePolyFilled(reaper_array*, int)',

  # additional string buffer size argument
  'bool ImGui::Combo(const char*, int*, const char*, int)' => 'bool Combo(const char*, int*, const char*, int, int*)',

  # no text_end argument
  'ImVec2 ImGui::CalcTextSize(const char*, const char*, bool, float)'        => 'void CalcTextSize(const char*, double*, double*, bool*, double*)',
  'void ImDrawList::AddText(const ImVec2&, ImU32, const char*, const char*)' => 'void DrawList_AddText(double, double, int, const char*)',
  'void ImDrawList::AddText(const ImFont*, float, const ImVec2&, ImU32, const char*, const char*, float, const ImVec4*)' => 'void DrawList_AddTextEx(Font*, double, double, double, int, const char*, double*, double*, double*, double*, double*)',

  'bool ImGui::DragScalarN(const char*, ImGuiDataType, void*, int, float, const void*, const void*, const char*, ImGuiSliderFlags)' => 'bool DragDoubleN(const char*, reaper_array*, double*, double*, double*, const char*, int*)',
  'bool ImGui::SliderScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiSliderFlags)'      => 'bool SliderDoubleN(const char*, reaper_array*, double, double, const char*, int*)',
  'bool ImGui::InputScalarN(const char*, ImGuiDataType, void*, int, const void*, const void*, const char*, ImGuiInputTextFlags)'    => 'bool InputDoubleN(const char*, reaper_array*, double*, double*, const char*, int*)',

  # drag/drop payload as string
  'const ImGuiPayload* ImGui::AcceptDragDropPayload(const char*, ImGuiDragDropFlags)' => 'bool AcceptDragDropPayload(const char*, char*, int, int*)',

  # ImGuiID -> str_id
  'bool ImGui::BeginChildFrame(ImGuiID, const ImVec2&, ImGuiWindowFlags)' => 'bool BeginChildFrame(const char*, double, double, int*)',

  'bool ImGuiTextFilter::Draw(const char*, float)' => 'bool TextFilter_Draw(Context*, const char*, double*)',

  # preventing crashes when calling ImDrawListSplitter::Merge on a blank,
  # unused DrawList different from the one given to Split/SetCurrentChannel
  'void ImDrawListSplitter::Split(ImDrawList*, int)' => 'void DrawListSplitter_Split(int)',
  'void ImDrawListSplitter::Merge(ImDrawList*)'      => 'void DrawListSplitter_Merge()',
  'void ImDrawListSplitter::SetCurrentChannel(ImDrawList*, int)' => 'void DrawListSplitter_SetCurrentChannel(int)',
}

# argument position & name are checked
RESOURCES = {
  'Context*'          => 'ctx',
  'DrawListProxy*'    => 'draw_list',
  'DrawListSplitter*' => 'splitter',
  'Font*'             => 'font',
  'Function*'         => 'func',
  'Image*'            => 'image',
  'ImageSet*'         => 'set',
  'ListClipper*'      => 'clipper',
  'Resource*'         => 'obj',
  'TextFilter*'       => 'filter',
  'ViewportProxy*'    => 'viewport',
}

# types REAPER's parser knows about
TYPES = [
  'bool',   'bool*',
  'int',    'int*',    # no unsigned int :-(
  'double', 'double*', # no float :'(
  'char*',
  'const char*',
  'reaper_array*',
  'LICE_IBitmap*',
  'void*',
] + RESOURCES.keys

class Function < Struct.new :type, :name, :args, :namespace, :match
  def sig
    return @sig if @sig
    arg_types = args.map {|arg|
      next arg unless arg.is_a? Argument
      if arg.size > 0
        "#{arg.type}[#{arg.size}]"
      else
        arg.type
      end
    }
    ns = namespace ? "#{namespace}::" : ''
    @sig = "#{type} #{ns}#{name}(#{arg_types.join ', '})".freeze
  end

  def normalized
    return @normalized if @normalized

    normal = self.class.new
    normal.type = if constructor?
                    cpp_type_to_reascript_type namespace + '*'
                  else
                    cpp_type_to_reascript_type type
                  end
    normal.name = RENAMES[sig] || normalized_name
    normal.name.gsub! /Float(?=\b|\d)/, 'Double'
    normal.args = args.dup
    normal.args = args.flat_map do |arg|
      cpp_arg_to_reascript_args arg
    end

    if normal.args.last == '...'
      fmt = normal.args.find { _1.name == 'fmt' }
      fmt.name = 'text'
      normal.args.pop
    end

    if normal.type == 'ImVec2'
      x = Argument.new 'double*', nil, nil, 0
      y = x.dup
      if normal.name =~ /Size/
        x.name = 'w'
        y.name = 'h'
      else
        x.name = 'x'
        y.name = 'y'
      end

      normal.type = 'void'
      pos = normal.args.index { not _1.default.nil? }
      pos ||= normal.args.size
      normal.args.insert pos, x
      normal.args.insert pos+1, y
    end

    @normalized = normal
  end

private
  def cpp_type_to_reascript_type(type)
    case type
    when /float([&\*])?/
      "double#{$~[1] && '*'}"
    when /unsigned int(\*)?/, /size_t(\*)?/
      "int#{$~[1]}"
    when 'ImTextureID'
      'Image*'
    when /Callback\z/
      "Function*"
    when /\AIm(?:Gui|Draw)[^\*]+(?:Flags\*)?\z/, 'ImU32'
      'int'
    when 'const char* const'
      'const char*'
    when /\AIm(?:Gui)?(DrawList|Viewport)\*\z/
      "#{$1}Proxy*"
    when /\A(?:const )?Im(?:Gui)?(?!Vec)(\w+)\*\z/
      "#{$1}*"
    else
      type
    end
  end

  def cpp_arg_to_reascript_args(arg)
    return arg unless arg.is_a? Argument

    out = [arg]

    if arg.default == 'NULL'
      arg.default = nil # no default value on reaimgui's side
    elsif arg.default == '0' && arg.type.start_with?(/Im(Gui)?/) && arg.type != 'ImGuiID'
      arg.default = arg.type[$~[0].size..-1] + '_None'
      arg.default = 'Cond_Always' if arg.default == 'Cond_None'
      arg.default = 'MouseButton_Left' if arg.default == 'MouseButton_None'
    elsif arg.default =~ /\AIm(Gui)?[\w_]+\z/
      arg.default.gsub! /\AIm(Gui)?/, '' # remove ImGui_ enum prefix (hidden in documentation)
    elsif arg.default == '1' && arg.type == 'ImGuiPopupFlags'
      arg.default = 'PopupFlags_MouseButtonRight'
    elsif !arg.default.nil? && arg.type == 'float'
      arg.default = arg.default[0..-2] # 0.0f -> 0.0
    elsif (matches = arg.default.to_s.scan(/(?:ImVec4\(|\G(?!\A))\s*([\d\.]+)f?\s*,?/)) && !matches.empty?
      arg.default = '0x' + matches.map { |m| '%02X' % (m[0].to_f * 255) }.join
    elsif arg.default == 'IM_COL32_WHITE'
      arg.default = '0xFFFFFFFF'
    end

    arg.name += '_rgba' if arg.type == 'ImU32' && %[col color].include?(arg.name)
    arg.type = cpp_type_to_reascript_type arg.type
    arg.name = arg.name[4..-1] if arg.name =~ /\Aout_.+/ && arg.type.end_with?('*')
    arg.name = RESOURCES[arg.type] if arg.type == 'Image*'

    if arg.type.include? 'ImVec2'
      null_optional = arg.type.end_with? '*'
      arg.type = 'double'
      arg.type += '*' if arg.default or null_optional
      out << Argument.new(arg.type, arg.name, arg.default, arg.size)

      if arg.name =~ /size/
        out.first.name += '_w'
        out.last.name  += '_h'
      else
        out.first.name += '_x'
        out.last.name  += '_y'
      end
    elsif arg.type.include?('ImVec4') && arg.name.include?('col')
      arg.type = 'int'
      arg.name += '_rgba'
    elsif arg.type.include? 'ImVec4'
      arg.type = 'double'
      arg.type += '*' if arg.default
      3.times do
        out << Argument.new(arg.type, arg.name, arg.default, arg.size)
      end

      out[0].name += '_x'
      out[1].name += '_y'
      out[2].name += '_w'
      out[3].name += '_h'
    elsif arg.type == 'double' && arg.name == 'col' && arg.size.between?(3, 4)
      arg.type = 'int*'
      arg.name += '_rgb'
      arg.name += 'a' if arg.size == 4
      arg.size = 0
    elsif arg.type == 'void*' && ['user_data', 'custom_callback_data'].include?(arg.name) # callbacks
      return []
    elsif arg.name == 'buf_size'
      arg.name = 'buf_sz'
    end

    if (matches = arg.default.to_s.scan(/(?:ImVec2\(|\G(?!\A))\s*([^f,\)]+)f?\s*,?/)) && !matches.empty?
      matches.flatten!
      out.each_with_index do |out_arg, index|
        out_arg.default = matches[index]
        out_arg.default += '.0' if out_arg.default =~ /\A[0-9]+\z/
      end
    end

    arg.type += '*' if arg.default && arg.type[-1] != '*'

    if arg.size > 1
      out = []
      arg.size.times do |n|
        array_el = arg.dup
        array_el.type += '*' # C arrays are pointers
        array_el.name = array_el.name + (n + 1).to_s
        array_el.size = 0
        out << array_el
      end
      return out
    end

    out
  end

  def constructor?
    namespace == name
  end

  def normalized_name
    ns = case namespace
         when 'ImGui'
           nil
         when /\AIm(?:Gui)?/
           namespace[$~[0].size..-1]
         end

    return name unless ns
    return "Create#{ns}" if constructor?
    return "#{ns}_#{name}"
  end
end

Argument = Struct.new :type, :name, :default, :size, :decoration

# load ImGui definitions
IMGUI_FUNC_R  = /IMGUI_API \s+ (?:(?<type>[\w\*\&\s]+?) \s*)? (?<=\b) (?<name>[\w]+) \( (?<args>.*?) \) (?:\s*IM_[A-Z]+\(.+\))?; /x
IMGUI_ARG_R   = /\A(?<type>[\w\*&\s\<\>]+) \s+ (?<name>\w+) (?:\[ (?<size>\d*) \])? (?:\s*=\s*(?<default>.+))?\z/x
IMGUI_ENUM_R  = /(?:\A|,) \s* Im(?:Gui)?(?<name>[\w_]+) \s* (?=,|=)/x
IMGUI_CLASS_R = /(?:namespace|struct) (?<name>\w+)/

def split_imgui_args(args)
  args.split(/,\s*(?=[^\)]*(?:\(|$)) /).map do |arg|
    next arg unless arg =~ IMGUI_ARG_R
    Argument.new $~[:type], $~[:name], $~[:default], $~[:size].to_i
  end
end

imgui_funcs, imgui_enums = [], []
namespace = '', in_obsolete = false, in_enum = false
File.foreach IMGUI_H do |line|
  line.strip!
  if line.start_with? '//'
  elsif in_obsolete
    in_obsolete = false if line == '#endif'
  elsif line.start_with? '#ifndef IMGUI_DISABLE_OBSOLETE_' # FUNCTIONS, KEYIO
    in_obsolete = true
  elsif in_enum
    if line.include? '}'
      in_enum = false
    elsif enums = line.scan(IMGUI_ENUM_R)
      enums.flatten.each {|enum|
        next if enum.end_with? '_COUNT'
        next if enum.end_with? '_' # internal flags and masks
        imgui_enums << enum
      }
    end
  elsif line =~ IMGUI_FUNC_R
    args = split_imgui_args $~[:args]
    imgui_funcs << Function.new($~[:type], $~[:name], args, namespace)
  elsif line.start_with? 'enum '
    in_enum = true
  elsif line =~ IMGUI_CLASS_R
    namespace = $~[:name]
  end
end

puts "imgui:    found %d functions, %d enums (total: %d symbols)" %
  [imgui_funcs.size, imgui_enums.size, imgui_funcs.size + imgui_enums.size]

# load ReaImGui definitions
REAIMGUI_FUNC_R = /\AAPI_FUNC \s*\(\s* (?<version>[0-9_]+) \s*,\s* (?<type>[\w\s\*]+) \s*,\s* (?<name>[\w]+) \s*,\s* (?<args>.*?) \s*(?<arg_end>,)?\s*(\/|\Z)/x
REAIMGUI_ENUM_R = /\AAPI_ENUM \s*\(\s* (?<version>[0-9_]+) \s*,\s* (?<prefix>\w+) \s*,\s* (?<name>\w+) \s*,\s*/x
REAIMGUI_ARGS_R = /\A\s* (?<args>\(.+?\)) \s*(?<arg_end>,)?\s*(\/|\Z)/x
REAIMGUI_ARGT_R = /\A(?<decoration>[A-Z]+)\s*<\s*(?<type>.+)\s*>\z/

def split_reaimgui_args(args)
  return [] if args == 'API_NO_ARGS'
  args = args.split /\)\s*\(/
  return args if args.size < 1

  args.first[0] = '' # remove leading (
  args.last[-1] = '' # remove trailing )

  args.map do |arg|
    type, name, default = arg.split /\s*,\s*/
    if type =~ REAIMGUI_ARGT_R
      type, decoration = $~[:type], $~[:decoration]
    end
    type.delete_prefix! 'class '
    default.gsub! /\AIm(Gui)?/, '' if default
    Argument.new type, name, default, 0, decoration
  end
end

reaimgui_funcs, reaimgui_enums = [], []
Dir.glob(REAIMGUI_API_CPP).each do |source_file|
  in_function, want_args = false, false
  File.foreach source_file do |line|
    if line =~ REAIMGUI_FUNC_R
      in_function = true
      args = split_reaimgui_args $~[:args]
      func = Function.new $~[:type], $~[:name], args
      reaimgui_funcs << func
      want_args = !$~[:arg_end]
    elsif line =~ REAIMGUI_ENUM_R
      reaimgui_enums << $~[:name]
    elsif line.chomp == '});'
      in_function = false
    elsif in_function
      # parse subsequent lines inside a function definition
      if line.include? '{'
        want_args = false
      elsif want_args && line =~ REAIMGUI_ARGS_R
        reaimgui_funcs.last.args += split_reaimgui_args $~[:args]
        want_args = !$~[:arg_end]
      end
    end
  end
end

puts "reaimgui: found %d functions, %d enums (total: %d symbols)" %
  [reaimgui_funcs.size, reaimgui_enums.size, reaimgui_funcs.size + reaimgui_enums.size]

# basic argument type validation
reaimgui_funcs.each do |func|
  unless func.type == 'void' || TYPES.include?(func.type)
    warn "#{func.name}: invalid return type: #{func.type}"
  end

  first_arg_is_resource = false
  func.args.each_with_index do |arg, i|
    unless TYPES.include? arg.type
      warn "#{func.name}: invalid argument type for '#{arg.name}': #{arg.type}"
    end

    if RESOURCES.has_key?(arg.type)
      if i == 0
        first_arg_is_resource = true

        unless arg.name == RESOURCES[arg.type]
          warn "#{func.name}: argument of type '#{arg.type}' should be named '#{RESOURCES[arg.type]}', got '#{arg.name}'"
        end
      elsif !first_arg_is_resource
        warn "#{func.name}: argument of type '#{arg.type}' should come first"
      end
    end
  end

  # ignore leading resource type for easier matching
  func.args.shift if !func.args.empty? && RESOURCES.has_key?(func.args.first.type)
end

NATIVE_ONLY.each do |sig|
  unless imgui_funcs.any? { _1.sig == sig }
    warn "function marked as native only not found in dear imgui: #{sig}"
  end
end

NATIVE_ONLY_ENUMS.each do |rule|
  if imgui_enums.none? { _1.match? rule }
    warn "enum marked as native only not found in dear imgui: #{rule}"
  end

  reaimgui_enums.select { _1.match? rule }.each do |enum|
    warn "enum marked as native only but exported anyway: #{enum}"
  end
end

skipped_enums = 0
(imgui_enums - reaimgui_enums).each do |im_enum|
  if NATIVE_ONLY_ENUMS.any? { im_enum.match? _1 }
    skipped_enums += 1
    next
  end

  warn "missing enum: #{im_enum}"
end

# (reaimgui_enums - imgui_enums).each do |extra_enum|
#   warn "unknown exported enum: #{extra_enum}"
# end

# link dear imgui functions to their corresponding ReaImGui counterparts
perfect_count = manual_count = missing_overloads = missing_count = skipped_count = 0
imgui_funcs.each do |imgui_func|
  if imgui_func.name[0] == '_' ||
      NATIVE_ONLY.include?(imgui_func.sig) ||
      NATIVE_ONLY_CLASSES.include?(imgui_func.namespace)
    skipped_count += 1
    next
  end

  candidate = reaimgui_funcs.find { imgui_func.normalized.name == _1.name }
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
reaimgui_funcs.each do |func|
  found_optional = false
  func.args.each_with_index do |rea_arg, i|
    unless rea_arg.name =~ /\A[a-z0-9_]+\z/
      warn "#{func.name}: invalid argument ##{i+1} name '#{rea_arg.name}' (not snake case?)"
      next
    end

    if rea_arg.decoration == 'RO'
      found_optional = true
    elsif found_optional && rea_arg.decoration.start_with?('W')
      warn "#{func.name}: output argument '#{rea_arg.name}' is after input values"
    end

    if rea_arg.decoration&.include?('S') && !rea_arg.name.end_with?('_sz')
      warn "#{func.name}: argument ##{i+1} of type '*S<#{rea_arg.type}>' is named '#{rea_arg.name}', expected to end with '_sz'"
    end

    unless func.match
      # no default value = nil
      # if decoration && decoration[-1] == 'O' && rea_arg.default.nil?
      #   warn "#{func.name}: argument ##{i+1} '#{rea_arg.name}' has no documented default value"
      # end
      next
    end

    imgui_arg = func.match.normalized.args[i]

    rename = ARG_RENAMES[func.name]&.[](imgui_arg.name)
    unless rea_arg.name == imgui_arg.name || rea_arg.name == rename
      warn "#{func.name}: argument ##{i+1} of type '#{rea_arg.type}' is named '#{rea_arg.name}', expected '#{rename || imgui_arg.name}'"
    end

    unless rea_arg.default == imgui_arg.default
      warn "#{func.name}: argument ##{i+1} '#{rea_arg.name}' has documented default value '#{rea_arg.default}', expected '#{imgui_arg.default}'"
    end
  end
end

puts
puts "functions: %d perfect matches, %d manual matches, %d missing overloads, %d not implemented, %d skipped" %
  [perfect_count, manual_count, missing_overloads, missing_count, skipped_count]

puts "functions: %.2f%% complete (%.2f%% total)" %
  [(perfect_count + manual_count).to_f / (imgui_funcs.size - skipped_count) * 100,
   (perfect_count + manual_count).to_f / imgui_funcs.size * 100]

extra_enums = reaimgui_enums - imgui_enums
enums_count = reaimgui_enums.size - extra_enums.size
puts "enums:     %d skipped, %d exclusive" % [skipped_enums, extra_enums.size]
puts "enums:     %.2f%% complete (%.2f%% total)" %
  [enums_count.to_f / (imgui_enums.size - skipped_enums) * 100,
   enums_count.to_f / imgui_enums.size * 100]
