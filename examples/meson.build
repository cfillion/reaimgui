if not bindings.contains('cpp')
  warning('cannot build the C++ example without generating the C++ binding')
  cpp_binding = disabler()
endif

hello_world = shared_library('reaper_hello_world-' + arch_suffix, 'hello_world.cpp',
  dependencies: [reaper_sdk_dep, swell_dep],
  install: true,
  install_dir: plugins_dir,
  name_prefix: '',
  override_options: 'cpp_std=c++17',
  sources: [cpp_binding])

zig = find_program('zig', required: false)
if not zig.found()
  warning('cannot build reaper_hello_world.zig without Zig installed on the system')
  zig = disabler()
endif
if not bindings.contains('zig')
  warning('cannot build the Zig example without generating the Zig binding')
  zig_binding = disabler()
endif
hello_zig = fs.name(hello_world.full_path()).replace('_world', '_zig')
custom_target(hello_zig,
  input: 'hello_world.zig', output: hello_zig,
  depends: zig_binding,
  command: [
    zig,
    'build-lib',
    '-dynamic',
    '-O', get_option('buildtype') == 'debug' ? 'Debug' : 'ReleaseFast',
    '-target', target_machine.cpu() + '-' + target_machine.system(),
    '-femit-bin=@OUTPUT@',
    '-fentry=ReaperPluginEntry',
    '--dep', 'reaper_imgui',
    '-Mmain=@INPUT@',
    '-Mreaper_imgui=' + zig_binding.full_path(),
  ],
  install: true, install_dir: plugins_dir)

php = find_program('php', required: false)
if not php.found()
  warning('cannot build gfx2imgui without PHP installed on the system')
  php = disabler()
endif

gfx2imgui = 'gfx2imgui.lua'
custom_target(gfx2imgui,
  input: gfx2imgui, output: gfx2imgui, capture: true,
  depend_files: files('../tools/preprocess.php'),
  command: [
    php,
    '-d', 'display_errors=stderr',
    '-d', 'short_open_tag=on',
    '@INPUT@',
  ],
  install: true, install_dir: scripts_dir)
