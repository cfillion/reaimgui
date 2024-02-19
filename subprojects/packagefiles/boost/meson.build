project('boost', version: '1.84')

boost_dep = declare_dependency(
  include_directories: '.',
  compile_args: ['-DBOOST_ALL_NO_LIB=1'])
meson.override_dependency('boost', boost_dep)
if meson.is_cross_build()
  meson.override_dependency('boost', boost_dep, native: true)
endif
