test_src = files([
  'api_test.cpp',
  'color_test.cpp',
  'compstr_test.cpp',
  'environment.cpp',
  'function_test.cpp',
  'resource_proxy_test.cpp',
  'resource_test.cpp',
  'texture_test.cpp',
  'types_test.cpp',
  'vernum_test.cpp',
])

eel_dep   = dependency('EEL2')
gmock_dep = dependency('gmock_main')

tests = executable('tests', test_src,
  dependencies: [common_dep, eel_dep, gmock_dep],
  link_with: [src], build_by_default: false)

test(meson.project_name(), tests,
  args: ['--gtest_color=yes'], protocol: 'gtest')
