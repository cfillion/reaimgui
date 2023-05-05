lua_shims = files([
  '0.7.lua',
  '0.8.lua',
  '0.8.5.lua',
])

custom_target('lua_shims',
  build_by_default: true,
  capture: true,
  command: [genshim, 'lua', lua_shims],
  depend_files: [lua_shims],
  install: true,
  install_dir: scripts_dir,
  install_tag: 'runtime',
  output: 'imgui.lua')
