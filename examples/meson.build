if not bindings.contains('cpp')
  warning('cannot build the C++ example without generating the C++ binding')
  cpp_binding = disabler()
endif

shared_library('reaper_hello_world-' + arch_suffix, 'hello_world.cpp',
  sources: [cpp_binding],
  dependencies: [reaper_sdk_dep, swell_dep],
  name_prefix: '',
  install: true,
  install_dir: plugins_dir)
