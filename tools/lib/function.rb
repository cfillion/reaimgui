class Function < Struct.new :type, :name, :args, :namespace, :match
  def sig
    return @sig if @sig
    ns = namespace ? "#{namespace}::" : ''
    @sig = "#{type} #{ns}#{name}(#{arg_types.join ', '})".freeze
  end

  def arg_types
    args.map {|arg| arg.normalized_type if arg.respond_to? :normalized_type }
  end
end

Argument = Struct.new :type, :name, :default, :size do
  def normalized_type
    if size > 0
      "#{type}[#{size}]"
    else
      type
    end
  end
end
