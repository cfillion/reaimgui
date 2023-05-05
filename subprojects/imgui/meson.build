project('imgui', 'cpp', version: 'unknown')

sources = files([
  'imgui/imgui.cpp',
  'imgui/imgui_demo.cpp',
  'imgui/imgui_draw.cpp',
  'imgui/imgui_tables.cpp',
  'imgui/imgui_widgets.cpp',
  'imgui/misc/cpp/imgui_stdlib.cpp',
  'imgui/misc/freetype/imgui_freetype.cpp',
])

dependencies = [dependency('freetype2')]

if host_machine.system() == 'darwin'
  dependencies += dependency('appleframeworks', modules: ['Carbon', 'Cocoa'])
endif

public_args = ['-DIMGUI_USER_CONFIG=<imconfig.h>']

imgui = library('imgui', sources,
  cpp_args: public_args,
  dependencies: dependencies,
  include_directories: 'imgui')

imgui_dep = declare_dependency(
  compile_args: public_args,
  include_directories: ['.', 'imgui'],
  link_with: imgui)
meson.override_dependency('imgui', imgui_dep)
