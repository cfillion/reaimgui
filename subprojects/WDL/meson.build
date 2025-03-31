project('WDL', ['c', 'cpp'], version: 'unknown',
  default_options: ['cpp_std=none'])

cc = meson.get_compiler('c')

wdl_dir = 'WDL/WDL'
wdl_dep = declare_dependency(
  compile_args: '-DWDL_NO_DEFINE_MINMAX',
  include_directories: ['.', 'WDL', wdl_dir])
meson.override_dependency('WDL', wdl_dep)

# EEL (used in tests ran outside of REAPER)
eel_dir = wdl_dir / 'eel2'
eel_sources = [
  eel_dir / 'nseel-cfunc.c',
  eel_dir / 'nseel-compiler.c',
  eel_dir / 'nseel-eval.c',
  eel_dir / 'nseel-ram.c',
  eel_dir / 'y.tab.c',
]
eel_args = ['-DEEL_TARGET_PORTABLE']
if cc.get_argument_syntax() == 'gcc'
  eel_args += '-Wno-implicit-function-declaration'
endif
eel = library('eel2', eel_sources, c_args: eel_args)
eel_dep = declare_dependency(dependencies: [wdl_dep], link_with: eel)
meson.override_dependency('EEL2', eel_dep)

# SWELL
if host_machine.system() != 'windows'
  swell_dependencies = [wdl_dep]
  swell_dir = wdl_dir / 'swell'
  swell_public_args = ['-DSWELL_PROVIDED_BY_APP']
  if host_machine.system() == 'darwin'
    swell_sources = swell_dir / 'swell-modstub.mm'
    add_languages('objcpp', native: false)
    swell_dependencies += dependency('appleframeworks', modules: ['AppKit'])
  else
    swell_sources = swell_dir / 'swell-modstub-generic.cpp'
  endif

  swell = library('swell', swell_sources,
    dependencies: swell_dependencies,
    cpp_args: swell_public_args, objcpp_args: swell_public_args)
  swell_dep = declare_dependency(
    compile_args: swell_public_args,
    dependencies: [wdl_dep],
    link_with: swell)
  meson.override_dependency('SWELL', swell_dep)
endif
