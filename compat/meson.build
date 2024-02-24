cpp = meson.get_compiler('cpp')

compat_defines = configuration_data({
  'HAVE_INCLUDE_NEXT':
    cpp.compiles('#include_next <utility>', name: '#include_next'),
  'HAVE_STD_OPTIONAL': cpp.has_header('optional'),

  'HAVE_STD_FORMAT':   cpp.has_header('format') and cpp.compiles('''
    #include <format>
    int main() {
      std::format("hello, world!");
    }''', name: 'std::format'),

  # std::visit (std::bad_variant_access) is not available before macOS 10.14
  'HAVE_STD_VARIANT':
    cpp.has_header('variant') and cpp.compiles('''
      #include <variant>
      int main() {
        std::visit([](auto v) {}, std::variant<int> {});
      }''', name: 'std::visit(std::variant)'),
})

compat_deps = []
if not compat_defines.get('HAVE_STD_FORMAT')
  fmt_dep = dependency('fmt', allow_fallback: false, required: false)
  if not fmt_dep.found()
    fmt_dep = subproject('fmt').get_variable('fmt_header_only_dep')
  endif
  compat_deps += fmt_dep
endif

configure_file(output: 'config.hpp', configuration: compat_defines)
compat_dep = declare_dependency(dependencies: compat_deps, include_directories: '.')
