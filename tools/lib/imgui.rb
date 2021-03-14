require_relative 'function'

class ImGui
  FUNC_R  = /API \s+ (?<type>[\w\*\&\s]+?) \s+ (?<name>[\w]+) \( (?<args>.*?) \) (?:\s*IM_[A-Z]+\(.+\))?; /x
  ARG_R   = /\A(?<type>[\w\*&\s\<\>]+) \s+ (?<name>\w+) (?:\[ (?<size>\d*) \])? (?:\s*=\s*(?<default>.+))?\z/x
  ENUM_R  = /\A\s* Im(?:Gui)?(?<name>[\w_]+) \s* (?:,|=)/x
  CLASS_R = /(?:namespace|struct) (?<name>\w+)/

  attr_accessor :funcs, :enums

  def initialize
    @funcs, @enums = [], []
    parse File.join __dir__, '..', '..', 'vendor', 'imgui', 'imgui.h'
  end

private
  def parse(header)
    namespace = '', in_obsolete = false
    File.foreach header do |line|
      if in_obsolete
        in_obsolete = false if line.chomp == '#endif'
        next
      end

      if line =~ FUNC_R
        args = split_args $~[:args]
        @funcs << Function.new($~[:type], $~[:name], args, namespace)
      elsif line =~ ENUM_R
        next if $~[:name].end_with? '_COUNT'
        next if $~[:name].end_with? '_'
        @enums << $~[:name]
      elsif line.chomp == '#ifndef DISABLE_OBSOLETE_FUNCTIONS'
        in_obsolete = true
      elsif line =~ CLASS_R
        namespace = $~[:name]
      end
    end
  end

  def split_args(args)
    args.split(/,\s*(?=[^\)]*(?:\(|$)) /).map do |arg|
      next arg unless arg =~ ARG_R
      Argument.new $~[:type], $~[:name], $~[:default], $~[:size].to_i
    end
  end

end

class ImGui::Function < Function
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
    when /float(\*)?/
      "double#{$~[1]}"
    when /unsigned int(\*)?/, /size_t(\*)?/
      "int#{$~[1]}"
    when 'ImDrawList*'
      'ImGui_DrawList*'
    when /\AIm(?:Gui|Draw)[^\*]+(?:Flags\*)?\z/, 'ImU32'
      'int'
    when 'const char* const'
      'const char*'
    else
      type
    end
  end

  def cpp_arg_to_reascript_args(arg)
    return arg unless arg.is_a? Argument

    out = [arg]

    if arg.default == 'NULL'
      arg.default = 'nil'
    elsif arg.default == '0' && arg.type.start_with?('ImGui')
      arg.default = arg.type + '_None'
      arg.default.insert 5, '_'
      arg.default = 'ImGui_Cond_Always' if arg.default == 'ImGui_Cond_None'
      arg.default = 'ImGui_MouseButton_Left' if arg.default == 'ImGui_MouseButton_None'
    elsif arg.default =~ /\AIm(Gui)?[\w_]+\z/
      arg.default.gsub! /\AIm(Gui)?/, 'ImGui_'
    elsif arg.default == '1' && arg.type == 'ImGuiPopupFlags'
      arg.default = 'ImGui_PopupFlags_MouseButtonRight'
    elsif !arg.default.nil? && arg.type == 'float'
      arg.default = arg.default[0..-2] # 0.0f -> 0.0
    elsif arg.default =~ /\A"(.+)"\z/
      arg.default = "'#{$~[1]}'"
    end

    arg.name += '_rgba' if arg.type == 'ImU32' && %[col color].include?(arg.name)

    arg.type = cpp_type_to_reascript_type arg.type

    if arg.type.include? 'ImVec2'
      arg.type = 'double'
      arg.type += '*' if arg.default
      out << Argument.new(arg.type, arg.name, arg.default, arg.size)

      if arg.name =~ /size/
        out.first.name += '_w'
        out.last.name  += '_h'
      else
        out.first.name += '_x'
        out.last.name  += '_y'
      end

      if arg.default =~ /ImVec2\((.+?)f?,\s*(.+?)f?\)/
        out.first.default = $~[1]
        out.last.default  = $~[2]

        out.first.default = '0.0' if out.first.default == '0'
        out.last.default  = '0.0' if out.last.default  == '0'
      end
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
