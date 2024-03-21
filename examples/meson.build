if not bindings.contains('cpp')
  warning('cannot build the C++ example without generating the C++ binding')
  cpp_binding = disabler()
endif

shared_library('reaper_hello_world-' + arch_suffix, 'hello_world.cpp',
  dependencies: [reaper_sdk_dep, swell_dep],
  install: true,
  install_dir: plugins_dir,
  name_prefix: '',
  override_options: 'cpp_std=c++17',
  sources: [cpp_binding])

php = find_program('php', required: false)

if not php.found()
  warning('cannot build gfx2imgui without PHP installed on the system')
  php = disabler()
endif

gfx2imgui = 'gfx2imgui.lua'
gfx2imgui = custom_target(gfx2imgui,
  input: gfx2imgui, output: gfx2imgui, capture: true,
  depend_files: files('../tools/preprocess.php'),
  command: [
    php,
    '-d', 'display_errors=stderr',
    '-d', 'short_open_tag=on',
    '@INPUT@',
  ],
  install: true, install_dir: scripts_dir)
