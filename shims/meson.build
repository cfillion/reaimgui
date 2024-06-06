shim_sources = files([
  '0.7.cpp',
  '0.8.cpp',
  '0.8.5.cpp',
  '0.8.7.cpp',
  '0.9.cpp',
  '0.9.2.cpp',
])

shims = static_library('shims', shim_sources, dependencies: [common_dep])

install_data(sources: 'imgui.lua', install_dir: scripts_dir, install_tag: 'runtime')
