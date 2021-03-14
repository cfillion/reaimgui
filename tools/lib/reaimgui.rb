require_relative 'function'

class ReaImGui
  FUNC_R = /\ADEFINE_API \s*\(\s* (?<type>[\w\s\*]+) \s*,\s* (?<name>[\w]+) \s*,\s* (?<args>.*?) \s*(?<arg_end>,)?\s*(\/|\Z)/x
  ENUM_R = /\ADEFINE_ENUM \s*\(\s* (?<prefix>\w+) \s*,\s* (?<name>\w+) \s*,\s*/x
  ARGS_R = /\A\s* (?<args>\(.+?\)) \s*(?<arg_end>,)?\s*(\/|\Z)/x
  DEFS_R = /\A"?Default values: (?<values>.+?)(?:\.?(?:\).?)?",)?\Z/
  DEF_R  = /\A(?<name>[\w_]+) = (?<value>.+)/
  ARGN_R =  /\A(?:(?<name>[^\(\)]+)|(?<decoration>[^\(]+)\((?<name>[^\)]+)\))\z/

  attr_reader :funcs, :enums

  def initialize
    @funcs, @enums = [], []

    source_pattern = File.join __dir__, '..', '..', 'src', 'api_*.cpp'
    Dir.glob(source_pattern).each do |source_file|
      parse source_file
    end
  end

private
  def parse(source_file)
    in_function, want_args = false, false
    File.foreach source_file do |line|
      if line =~ FUNC_R
        in_function = true
        args = split_args $~[:args]
        func = Function.new $~[:type], $~[:name], args
        @funcs << func
        want_args = !$~[:arg_end]
      elsif line =~ ENUM_R
        @enums << $~[:name]
      elsif line.chomp == '});'
        in_function = false
      elsif in_function
        # parse subsequent lines inside a function definition
        if line.include? '{'
          want_args = false
        elsif want_args && line =~ ARGS_R
          @funcs.last.args += split_args $~[:args]
          want_args = !$~[:arg_end]
        elsif line =~ DEFS_R
          add_defaults @funcs.last, $~[:values]
        end
      end
    end
  end

  def split_args(args)
    args = args.split ')('
    return args if args.size < 1

    args.first[0] = '' # remove leading (
    args.last[-1] = '' # remove trailing )

    args.map do |arg|
      type, name = arg.split /\s*,\s*/
      Argument.new type, name, nil, 0
    end
  end

  def add_defaults(func, values)
    values = values.split ', ' # strict with whitespace
    values.each do |value|
      if not value =~ DEF_R
        warn "#{func.name}: invalid default value: #{value}"
        next
      end

      name, default = $~[:name], $~[:value]

      arg = func.args.find do |arg|
        arg.name == name && arg.decoration&.[](-1) == 'O'
      end

      if not arg then
        warn "#{func.name}: default for unknown optional argument: #{value}"
        next
      end

      arg.default = default
    end
  end
end

class ReaImGui::Argument < Argument
  attr_reader :raw_name, :decoration
  @@decorators = {}

  def initialize(*args)
    super

    @raw_name = name
    self.name, @decoration = $~[:name], $~[:decoration] if name =~ ReaImGui::ARGN_R
  end

  def expanded_name
    return name unless @decoration
    self.class.parse_decorators if @@decorators.empty?
    @@decorators[@decoration][name]
  end

private
  def self.parse_decorators
    api_helper = File.join __dir__, '..', '..', 'src', 'api_helper.hpp'

    File.foreach api_helper do |line|
      next unless line =~ /
        \#define\s*(?<decoration>API_.+)\((?<placeholder>[^\)]+)\)
        \s*
        (?<pattern>[^\s]+)
      /x
      pattern, placeholder = $~[:pattern], $~[:placeholder]
      @@decorators[$~[:decoration]] = proc {|arg_name|
        pattern.gsub "#{placeholder}##", arg_name
      }
    end
  end
end
