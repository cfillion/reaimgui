project('md4c', 'c', version: 'unknown')

md4c = library('md4c',
  files(['src/entity.c', 'src/md4c.c']))
md4c_dep = declare_dependency(
  link_with: [md4c], include_directories: ['src'])
meson.override_dependency('md4c', md4c_dep)

md4c_html = library('md4c-html',
  files(['src/md4c-html.c']),
  dependencies: [md4c_dep])
md4c_html_dep = declare_dependency(
  link_with: [md4c_html], dependencies: [md4c_dep])
meson.override_dependency('md4c-html', md4c_html_dep)
