# native compiler flags aren't set unless in a cross build
native = meson.is_cross_build()
cpp_native = meson.get_compiler('cpp', native: native)

boost_native_dep = dependency('boost', native: native, version: boost_dep.version())
md4c_html_dep    = dependency('md4c-html')
swell_native_dep = swell_dep.partial_dependency(
  compile_args: true, includes: true)

if host_machine.system() == 'windows'
  dl_dep = dependency('', required: false)
else
  dl_dep = dependency('dl')
endif

genversion_src = vcs_tag(
  command: [find_program('git'), 'describe', '--match=v*', '--dirty=+'],
  input: 'genversion.cpp.in', output: 'genversion.cpp', fallback: '0.0-nogit')
genversion = executable('genversion', genversion_src, native: native,
  dependencies: [compat_dep])
version = custom_target('version.hpp',
  command: [genversion], capture: true, output: 'version.hpp')

genbinding = executable('genbinding',
  ['genbinding.cpp', version],
  dependencies: [boost_dep, compat_dep, dl_dep, md4c_html_dep])
gendialog = executable('gendialog', ['gendialog.cpp', version], native: native,
  dependencies: [boost_native_dep, compat_dep, swell_native_dep])

if host_machine.system() == 'windows'
  d3dcompiler_dep = cpp_native.find_library('D3DCompiler')
  gend3dshader = executable('gend3dshader',
    ['gend3dshader.cpp'],
    dependencies: [d3dcompiler_dep],
    native: native)
  gend3dshader = generator(gend3dshader,
    capture: true,
    output: '@PLAINNAME@.ipp',
    arguments: ['@EXTRA_ARGS@', '@INPUT@'])
elif host_machine.system() == 'darwin'
  genmetalshader = files('genmetalshader.sh')
endif
