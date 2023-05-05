test_src = files([
  'color_test.cpp',
  'environment.cpp',
  'function_test.cpp',
  'resource_proxy_test.cpp',
  'resource_test.cpp',
  'texture_test.cpp',
])

eel_dep   = dependency('EEL2')
gmock_dep = dependency('gmock_main')

tests = executable('tests', test_src,
  dependencies: [common_dep, eel_dep, gmock_dep],
  link_with: [src])

test(meson.project_name(), tests,
  args: ['--gtest_color=yes'], protocol: 'gtest')
