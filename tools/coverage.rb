#!/usr/bin/env ruby

# Use to identify differences between dear imgui's API and ReaImGui's API
# This tool also performs various sanity checks on the exported API

IMGUI_H = File.join __dir__, '..', 'vendor', 'imgui', 'imgui.h'
REAIMGUI_API_CPP = File.join __dir__, '..', 'src', 'api_*.cpp'

# these functions we cannot port
NATIVE_ONLY = [
  'ImGuiContext* ImGui::CreateContext(ImFontAtlas*)',
  'void ImGui::SetCurrentContext(ImGuiContext*)',
  'void ImGui::DestroyContext(ImGuiContext*)',

  'void ImGui::ShowDemoWindow(bool*)',
  'void ImGui::ShowStyleEditor(ImGuiStyle*)',
  'bool ImGui::ShowStyleSelector(const char*)',
  'void ImGui::ShowFontSelector(const char*)',
  'void ImGui::StyleColorsDark(ImGuiStyle*)',
  'void ImGui::StyleColorsLight(ImGuiStyle*)',
  'void ImGui::StyleColorsClassic(ImGuiStyle*)',

  'bool ImGui::DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t)',
  'void ImGui::SetAllocatorFunctions(void* (*alloc_func)(size_t sz, void* user_data), void (*free_func)(void* ptr, void* user_data), void*)',
  'void* ImGui::MemAlloc(size_t)',
  'void ImGui::MemFree(void*)',

  'void ImGui::TextUnformatted(const char*, const char*)',

  # only const char* IDs
  'void ImGui::PushID(int)',
  'void ImGui::PushID(const void*)',
  'void ImGui::PushID(const char*, const char*)',
  'bool ImGui::BeginChild(ImGuiID, const ImVec2&, bool, ImGuiWindowFlags)',

  'ImGuiID ImGui::GetID(const char*)',
  'ImGuiID ImGui::GetID(const char*, const char*)',
  'ImGuiID ImGui::GetID(const void*)',

  # no callbacks
  'bool ImGui::Combo(const char*, int*, bool(*items_getter)(void* data, int idx, const char** out_text), void*, int, int)',
  'bool ImGui::ListBox(const char*, int*, bool (*items_getter)(void* data, int idx, const char** out_text), void*, int, int)',
  'void ImGui::PlotLines(const char*, float(*values_getter)(void* data, int idx), void*, int, int, const char*, float, float, ImVec2)',
  'void ImGui::PlotHistogram(const char*, float(*values_getter)(void* data, int idx), void*, int, int, const char*, float, float, ImVec2)',

  # no float/ImVec2 overloads
  'void ImGui::PushStyleVar(ImGuiStyleVar, float)',

  # unused array + explicit size overload
  'bool ImGui::Combo(const char*, int*, const char* const, int, int)',
  'bool ImGui::ListBox(const char*, int*, const char* const, int, int)',

  # no va_list overloads
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
]

# these functions were ported using another name (eg. overloads)
RENAMES = {
  'bool ImGui::RadioButton(const char*, int*, int)' => 'RadioButtonEx',
  'ImU32 ImGui::GetColorU32(ImGuiCol, float)'       => 'GetColor',
}

# these functions were not ported 1:1
OVERRIDES = {
  'void ImGui::ColorConvertHSVtoRGB(float, float, float, float&, float&, float&)' => 'int ColorConvertHSVtoRGB(double, double, double, double*)',
  'void ImGui::PushStyleVar(ImGuiStyleVar, const ImVec2&)'                        => 'void PushStyleVar(int, double, double*)',
  'bool ImGui::SetDragDropPayload(const char*, const void*, size_t, ImGuiCond)'   => 'bool SetDragDropPayload(const char*, const char*, int*)',

  # float ref_col[] -> int* ref_col
  'bool ImGui::ColorPicker4(const char*, float[4], ImGuiColorEditFlags, const float*)' => 'bool ColorPicker4(const char*, int*, int*, int*)',

  # (float* array, int array_size) -> reaper_array*
  'void ImGui::PlotLines(const char*, const float*, int, int, const char*, float, float, ImVec2, int)' => 'void PlotLines(const char*, reaper_array*, int*, const char*, double*, double*, double*, double*)',
  'void ImGui::PlotHistogram(const char*, const float*, int, int, const char*, float, float, ImVec2, int)' => 'void PlotHistogram(const char*, reaper_array*, int*, const char*, double*, double*, double*, double*)',

  # no input text callbacks
  'bool ImGui::InputText(const char*, char*, size_t, ImGuiInputTextFlags, ImGuiInputTextCallback, void*)' => 'bool InputText(const char*, char*, int, int*)',
  'bool ImGui::InputTextMultiline(const char*, char*, size_t, const ImVec2&, ImGuiInputTextFlags, ImGuiInputTextCallback, void*)' => 'bool InputTextMultiline(const char*, char*, int, double*, double*, int*)',
  'bool ImGui::InputTextWithHint(const char*, const char*, char*, size_t, ImGuiInputTextFlags, ImGuiInputTextCallback, void*)' => 'bool InputTextWithHint(const char*, const char*, char*, int, int*)',
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

class Function < Struct.new :type, :name, :args, :namespace
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

    ns = case namespace
         when 'ImGui'
           nil
         when /\AIm(?:Gui)?/
           namespace[$~[0].size..-1]
         end

    normal = self.class.new
    normal.type = cpp_type_to_reascript_type type
    normal.namespace = nil
    normal.name = RENAMES[sig] || (ns ? "#{ns}_#{name}" : name)
    normal.name.gsub! /Float(?=\b|\d)/, 'Double'
    normal.args = args.dup
    normal.args = args.flat_map do |arg|
      cpp_arg_to_reascript_args arg
    end
    # TODO: normalize ImVec2 return to double*, double* args
    @normalized = normal
  end

private
  def cpp_type_to_reascript_type(type)
    case type
    when /float(\*)?/
      "double#{$~[1]}"
    when /unsigned int(\*)?/, /size_t(\*)?/
      "int#{$~[1]}"
    when /\AIm(?:Gui|Draw)[^\*]+(?:Flags\*)?\z/, 'ImU32'
      'int'
    when 'const char* const'
      'const char*'
    else
      type
    end
  end

  def cpp_arg_to_reascript_args(arg)
    return [] if arg == '...' # string format
    return arg unless arg.is_a? Argument

    out = [arg]

    arg.type = cpp_type_to_reascript_type arg.type

    if arg.type.include? 'ImVec2'
      # puts arg.type
      arg.type = 'double'
      arg.type += '*' if arg.default
      out << Argument.new(arg.type, arg.name + '_y', arg.default, arg.size)
      arg.name += '_x'
    elsif arg.type.include?('ImVec4') && arg.name.include?('col')
      arg.type = 'int'
      arg.name += '_rgba'
    elsif arg.type == 'double' && arg.name == 'col' && arg.size.between?(3, 4)
      arg.type = 'int*'
      arg.name += '_rgb'
      arg.name += 'a' if arg.size == 4
      arg.size = 0
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
end

Argument = Struct.new :type, :name, :default, :size

# load ImGui definitions
IMGUI_FUNC_R  = /IMGUI_API \s+ (?<type>[\w\*\&]+) \s+ (?<name>[\w]+) \( (?<args>.+?) \) (?:\s*IM_[A-Z]+\(.+\))?; /x
IMGUI_ARG_R   = /\A(?<type>[\w\*&\s\<\>]+) \s+ (?<name>\w+) (?:\[ (?<size>\d*) \])? (?:\s*=\s*(?<default>.+))?\z/x
IMGUI_ENUM_R  = /\A\s* ImGui(?<name>[\w_]+) \s* (?:,|=)/x
IMGUI_CLASS_R = /(?:namespace|struct) (?<name>\w+)/

def split_imgui_args(args)
  args.split(/,\s*(?=[^\)]*(?:\(|$)) /).map do |arg|
    next arg unless arg =~ IMGUI_ARG_R
    Argument.new $~[:type], $~[:name], $~[:default], $~[:size].to_i
  end
end

imgui_funcs, imgui_enums = [], []
namespace = ''
File.foreach IMGUI_H do |line|
  if line =~ IMGUI_CLASS_R
    namespace = $~[:name]
  elsif line =~ IMGUI_FUNC_R
    args = split_imgui_args $~[:args]
    imgui_funcs << Function.new($~[:type], $~[:name], args, namespace)
  elsif line =~ IMGUI_ENUM_R
    next if $~[:name].end_with? '_COUNT'
    next if $~[:name].end_with? '_'
    imgui_enums << $~[:name]
  end
end

puts "imgui:    found %d functions, %d enums (total: %d symbols)" %
  [imgui_funcs.size, imgui_enums.size, imgui_funcs.size + imgui_enums.size]

# load ReaImGui definitions
REAIMGUI_FUNC_R = /\ADEFINE_API \s*\(\s* (?<type>[\w\s\*]+) \s*,\s* (?<name>[\w]+) \s*,\s* (?<args>.*?) \s*(?<arg_end>,)?\s*(\/|\Z)/x
REAIMGUI_ENUM_R = /\ADEFINE_ENUM \s*\(\s* (?<name>[\w\*]+) \s*,\s*/x
REAIMGUI_ARGS_R = /\A\s* (?<args>\(.+?\)) \s*(?<arg_end>,)?\s*(\/|\Z)/x
REAIMGUI_DEFS_R = /\ADefault values: (?<values>.+)(?:\).?)",\Z/
REAIMGUI_DEF_R  = /\A(?<name>[\w_]+) = (?<value>'[^']*'|-?\d(?:\.\d)?|ImGui_[\w_]+|true|false|nil)/

def split_reaimgui_args(args)
  args = args.split ')('
  return args if args.size < 1

  args.first[0] = '' # remove leading (
  args.last[-1] = '' # remove trailing )

  args.map do |arg|
    type, name = arg.split /\s*,\s*/
    Argument.new type, name, nil, 0
  end
end

def add_reaimgui_defaults(func, values)
  values = values.split ', ' # strict with whitespace
  values.each do |value|
    if not value =~ REAIMGUI_DEF_R
      warn "#{func.name}: invalid default value: #{value}"
      next
    end

    arg = func.args.find { _1.name == "API_RO(#{$~[:name]})" }
    if not arg then
      warn "#{func.name}: default for unknown argument: #{value}"
      next
    end

    arg.default = $~[:value]
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
      elsif line =~ REAIMGUI_DEFS_R
        add_reaimgui_defaults reaimgui_funcs.last, $~[:values]
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
  unless imgui_funcs.find { _1.sig == sig }
    warn "function marked as native only not found in dear imgui: #{sig}"
  end
end

# link dear imgui functions to their corresponding ReaImGui counterparts
perfect_matches = []
partial_count = missing_overloads = missing_count = skipped_count = 0
imgui_funcs.each do |imgui_func|
  if NATIVE_ONLY.include? imgui_func.sig
    skipped_count = skipped_count + 1
    next
  end

  candidate = reaimgui_funcs.find { imgui_func.normalized.name == _1.name }
  expected_sig = OVERRIDES[imgui_func.sig] || imgui_func.normalized.sig

  if not candidate
    missing_count = missing_count + 1
    warn "not implemented: #{imgui_func.sig}"
    warn "  expected:  #{expected_sig}"
    next
  end

  perfect_match = candidate.sig == expected_sig

  if perfect_match
    if OVERRIDES.has_key? imgui_func.sig
      partial_count = partial_count + 1
    else
      perfect_matches << [imgui_func, candidate]
    end
  else
    missing_overloads = missing_overloads + 1

    warn "overload not implemented: #{imgui_func.sig}"
    warn "  expected:  #{expected_sig}"
    warn "  candidate: #{candidate.sig}"
  end
end

puts
puts "%d perfect matches, %d partial matches, %d missing overloads, %d missing functions, %d skipped" %
  [perfect_matches.size, partial_count, missing_overloads, missing_count, skipped_count]

# TODO: check argument names and compare default values
# TODO: check coverage of enums
