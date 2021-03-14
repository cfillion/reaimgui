#!/usr/bin/env ruby

require_relative 'lib/reaimgui'
reaimgui = ReaImGui.new

# not using a namespace in the output to match with the ReaScript API

puts <<-HEAD
#ifndef REAPER_IMGUI_FUNCTIONS_H
#define REAPER_IMGUI_FUNCTIONS_H

#include <reaper_plugin_functions.h>

class ImGui_Context;
class ImGui_DrawList;
class ImGui_ListClipper;

struct reaper_array {
  const unsigned int size, alloc;
  double data[1];
};

template<typename T>
class ReaImGuiFunc;

template<typename R, typename... Args>
class ReaImGuiFunc<R(Args...)>
{
public:
  ReaImGuiFunc(const char *name) : m_name { name }, m_proc { nullptr } {}

  operator bool() const { return m_proc != nullptr; }

  auto operator()(Args... args)
  {
    if(!m_proc)
      m_proc = reinterpret_cast<decltype(m_proc)>(plugin_getapi(m_name));

    return m_proc(std::forward<Args>(args)...);
  }

private:
  const char *m_name;
  R(*m_proc)(Args...);
};

class ReaImGuiEnum
{
public:
  ReaImGuiEnum(const char *name) : m_name { name }, m_init { false } {}

  operator int()
  {
    if(!m_init) {
      ReaImGuiFunc<int()> func { m_name };
      m_value = func();
      m_init  = true;
    }

    return m_value;
  }

private:
  const char *m_name;
  bool m_init;
  int m_value;
};

#ifdef REAIMGUIAPI_IMPLEMENT
#  define REAIMGUIAPI_EXTERN
#  define REAIMGUIAPI_INIT(n) { n }
#else
#  define REAIMGUIAPI_EXTERN extern
#  define REAIMGUIAPI_INIT(n)
#endif

HEAD

reaimgui.funcs.sort_by(&:name).each do |func|
  name = "ImGui_#{func.name}"
  args = func.args.map do |arg|
    "#{arg.normalized_type} #{arg.expanded_name}"
  end.join ', '

  puts <<~FUNC
  REAIMGUIAPI_EXTERN ReaImGuiFunc<#{func.type}(#{args})> #{name} \
  REAIMGUIAPI_INIT("#{name}");
  FUNC
end

puts

reaimgui.enums.sort.each do |enum|
  name = "ImGui_#{enum}"
  puts <<~ENUM
  REAIMGUIAPI_EXTERN ReaImGuiEnum #{name} \
  REAIMGUIAPI_INIT("#{name}");
  ENUM
end

puts <<~FOOT

#undef REAIMGUIAPI_EXTERN
#undef REAIMGUIAPI_INIT

#endif
FOOT
