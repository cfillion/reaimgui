src_sources = files([
  'action.cpp',
  'api.cpp',
  'color.cpp',
  'context.cpp',
  'docker.cpp',
  'error.cpp',
  'font.cpp',
  'function.cpp',
  'image.cpp',
  'jpeg_image.cpp',
  'keymap.cpp',
  'main.cpp',
  'menu.cpp',
  'opengl_renderer.cpp',
  'png_image.cpp',
  'renderer.cpp',
  'resource.cpp',
  'settings.cpp',
  'texture.cpp',
  'viewport.cpp',
  'window.cpp',
])

src_args = []
src_dependencies = [common_dep, libjpeg_dep, libpng_dep, zlib_dep]

dialog = custom_target('dialog.rc',
  command: [gendialog], capture: true, output: 'dialog.rc')
if host_machine.system() == 'windows'
  win32_resources += windows.compile_resources(dialog)

  version_info = configure_file(configuration: configuration_data({
    'REAIMGUI_ARCH': arch_suffix
  }), input: 'version.rc.in', output: 'version.rc')
  win32_resources += windows.compile_resources(version_info, depends: [version])
else
  src_sources += dialog
endif

if host_machine.system() == 'darwin'
  src_sources += files([
    'cocoa_events.mm',
    'cocoa_font.mm',
    'cocoa_inject.mm',
    'cocoa_inputview.mm',
    'cocoa_opengl.mm',
    'cocoa_platform.mm',
    'cocoa_window.mm',
    'metal_renderer.mm',
  ])

  src_sources += custom_target('metal_shader',
    input:  'metal_shader.metal',
    output: 'metal_shader.metal.ipp',
    command: [genmetalshader, '@INPUT@', '@OUTPUT@'])

  src_dependencies += dependency('appleframeworks',
    modules: ['AppKit', 'Carbon', 'OpenGL', 'QuartzCore'])
elif host_machine.system() == 'windows'
  src_sources += files([
    'd3d10_renderer.cpp',
    'win32_droptarget.cpp',
    'win32_font.cpp',
    'win32_opengl.cpp',
    'win32_platform.cpp',
    'win32_window.cpp',
  ])

  cpp = meson.get_compiler('cpp')
  foreach win32_lib : ['Dwmapi', 'Gdi32', 'Imm32', 'Ole32', 'Opengl32', 'Shell32']
    src_dependencies += cpp.find_library(win32_lib)
  endforeach

  d3d_shaders = {
    'd3d10_pixel.hlsl':  'ps_4_0',
    'd3d10_vertex.hlsl': 'vs_4_0',
  }
  foreach file, profile : d3d_shaders
    src_sources += gend3dshader.process(file, extra_args: [profile])
  endforeach
else
  src_args += '-DFOCUS_POLLING'

  src_sources += files([
    'fc_font.cpp',
    'gdk_opengl.cpp',
    'gdk_platform.cpp',
    'gdk_window.cpp',
  ])

  src_dependencies += [
    dependency('fontconfig'),
    dependency('epoxy'),
    dependency('gtk+-3.0'),
  ]
endif

src = static_library('src', src_sources,
  cpp_args: src_args,
  dependencies: src_dependencies)
