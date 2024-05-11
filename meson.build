project('reaimgui', 'cpp',
  version: 'unknown',
  meson_version: '>= 1.1',
  default_options: [
    'b_lto=true', # -fno-lto added below in debug builds
    'b_ndebug=if-release',
    'b_pie=true',
    'b_vscrt=static_from_buildtype',
    'cpp_std=c++20', 'build.cpp_std=c++20',
    'cpp_winlibs=', # explicitely specified per-target
    'default_library=static',

    'freetype2:brotli=disabled',
    'freetype2:bzip2=disabled',
    'freetype2:harfbuzz=disabled',
    'freetype2:png=enabled',
    'freetype2:zlib=system',

    'warning_level=2',
    'freetype2:warning_level=0',
    'libjpeg-turbo:warning_level=0',
    'libpng:warning_level=0',
    'md4c:warning_level=0',
    'WDL:warning_level=0',
    'zlib:warning_level=0',

    'werror=true',
    'imgui:werror=false',
  ])

fs = import('fs')
windows = import('windows')

global_cpp_args = {
  'gcc': [
    '-fsigned-char', # defaults to unsigned on ARM
    '-fstack-protector-strong',
    '-fvisibility=hidden', # meson's gnu_symbol_visibility is per-target
  ],
  'msvc': [],
}
project_cpp_args = {
  'gcc': [
    '-Wno-deprecated-copy', # for eel_strings.h in GCC 12
    '-Wno-missing-braces',  # for Xcode 9
    '-Wno-missing-field-initializers',
    '-Wno-multichar',
    '-Wno-nonnull-compare',  # dynamic_cast(this)
    '-Wno-unused-parameter', # for reaper-sdk and WDL
  ],
  'msvc': [
    '/D_CRT_NONSTDC_NO_DEPRECATE',
    '/D_CRT_SECURE_NO_WARNINGS',
    '/DNOMINMAX',
    '/DUNICODE',

    '/Zc:preprocessor',

    '/wd4244', # conversion from T1 to T2, possible loss of data
    '/wd4267', # conversion from size_t to T, possible loss of data
    '/wd4624', # destructor was implicitly defined as deleted
  ],
}
project_objcpp_args = [
  '-fobjc-arc',
  '-Wno-deprecated-declarations',
]
link_args = {
  'gcc': [
    # remove unused code (-dead_strip on macOS)
    '-Wl,-dead_strip', '-Wl,--gc-sections',
    # fail at compile-time if we forgot to link against something
    '-Wl,--no-undefined',
  ],
  'msvc': [
    # store only the PDB file's basename instead of its full absolute path
    '/PDBALTPATH:%_PDB%'
  ]
}

if get_option('buildtype') == 'debug'
  global_cpp_args += {'gcc':  global_cpp_args['gcc'] + [
    '-fno-lto' # meson is lacking a b_lto=if-release option
  ]}
else
  global_cpp_args += {'msvc':  global_cpp_args['msvc'] + [
    '/GL', # whole program optimization
    '/Gy', # enable function-level linking
    '/Zc:inline', # remove unreferenced COMDAT
    '/Zi', # generate a PDB file (implies /DEBUG and /Zo)
  ]}
  link_args += {'msvc':  link_args['msvc'] + [
    '/LTCG',
    '/OPT:REF,ICF,LBR', # re-enable link-time optimizations when /DEBUG is set
  ]}
endif

native_set = [false]
if meson.is_cross_build()
  # not copying compiler flags to the native compiler unless in a cross build
  # to avoid unnecessary duplicate supported argument check status messages
  native_set += true
endif
foreach native : native_set
  cpp = meson.get_compiler('cpp', native: native)
  arg_syntax = cpp.get_argument_syntax()
  add_global_arguments(
    cpp.get_supported_arguments(global_cpp_args[arg_syntax]),
    language: ['c', 'cpp', 'objcpp'],
    native: native)
  add_project_arguments(
    cpp.get_supported_arguments(project_cpp_args[arg_syntax]),
    language: ['c', 'cpp', 'objcpp'],
    native: native)
  add_global_link_arguments(
    cpp.get_supported_link_arguments(link_args[arg_syntax]),
    language: ['c', 'cpp', 'objcpp'],
    native: native)
endforeach

if host_machine.system() == 'darwin'
  add_languages('objcpp', native: false)
  objcpp = meson.get_compiler('objcpp')
  add_project_arguments(
    objcpp.get_supported_arguments(project_objcpp_args),
    language: 'objcpp')
endif

subproject('imgui')
subproject('reaper-sdk')
wdl = subproject('WDL')

boost_dep      = dependency('boost', version: '>= 1.74')
imgui_dep      = dependency('imgui')
libjpeg_dep    = dependency('libjpeg')
libpng_dep     = dependency('libpng')
reaper_sdk_dep = dependency('reaper-sdk')
wdl_dep        = dependency('WDL')
zlib_dep       = dependency('zlib')

if host_machine.system() == 'windows'
  swell_dep = dependency('', required: false)
else
  swell_dep = dependency('SWELL')
endif

arch_suffix = host_machine.cpu()
if host_machine.system() == 'windows' and arch_suffix == 'x86_64'
  arch_suffix = 'x64'
endif

resource_path = fs.expanduser(get_option('resource_path'))
if resource_path == 'auto'
  install_platform = build_machine.system()
  if install_platform == 'darwin'
    resource_path = fs.expanduser('~/Library/Application Support')
  elif install_platform == 'windows'
    appdata = run_command('cmd', '/C', 'echo %APPDATA%', check: true)
    resource_path = appdata.stdout().strip()
  else
    resource_path = fs.expanduser('~/.config')
  endif
  resource_path = resource_path / 'REAPER'
elif not fs.is_absolute(resource_path)
  error(f'resource_path=@resource_path@ must be an absolute path')
elif not fs.is_file(resource_path / 'reaper.ini')
  warning(f'resource_path=@resource_path@ does not contain a reaper.ini file')
endif
data_dir    = resource_path / 'Data'
plugins_dir = resource_path / 'UserPlugins'
scripts_dir = resource_path / 'Scripts/ReaTeam Extensions/API'

subdir('compat')
subdir('tools')

common_dep = declare_dependency(
  dependencies: [
    boost_dep, compat_dep, imgui_dep, reaper_sdk_dep, swell_dep, wdl_dep,
  ],
  sources: [version])
win32_resources = []

subdir('api')
subdir('shims')
subdir('src')

reaimgui = shared_library('reaper_imgui-' + arch_suffix, win32_resources,
  link_whole: [api, shims, src],
  name_prefix: '',
  install: true,
  install_dir: plugins_dir)

if get_option('tests').enabled()
  subdir('tests')
endif

bindings = get_option('bindings')
if bindings.length() > 0 and not meson.can_run_host_binaries()
  warning('cannot generate bindings: build system cannot run host binaries')
  bindings = []
endif
binding_targets = {
  'cpp':    { 'output': 'reaper_imgui_functions.h' },
  'zig':    { 'output': 'reaper_imgui.zig' },
  'human':  { 'output': 'reaper_imgui_doc.html', 'install_dir': data_dir    },
  'python': { 'output': 'imgui.py',              'install_dir': scripts_dir },
  'luals':  { 'output': 'imgui_defs.lua' },
}
foreach name : bindings
  binding = binding_targets[name]
  binding_file = custom_target(name + '_binding',
    build_by_default: true,
    capture: true,
    command: [genbinding, reaimgui.full_path(), name],
    depends: [reaimgui],
    install: binding.has_key('install_dir'),
    install_tag: 'runtime',
    kwargs: binding)
  set_variable(name + '_binding', binding_file)
endforeach

if get_option('examples').enabled()
  subdir('examples')
endif

summary({
  'Bindings': meson.can_run_host_binaries() ? bindings : '<unavailable>',
  'Examples': get_option('examples'),
}, section: 'Generate')

summary({
  'Resource path': resource_path,
}, section: 'Install')
