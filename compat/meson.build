cpp = meson.get_compiler('cpp')

configure_file(output: 'config.hpp', configuration: configuration_data({
  'HAVE_INCLUDE_NEXT':
    cpp.compiles('#include_next <utility>', name: '#include_next'),
  'HAVE_STD_OPTIONAL': cpp.has_header('optional'),

  # std::visit (std::bad_variant_access) is not available before macOS 10.14
  'HAVE_STD_VARIANT':
    cpp.has_header('variant') and cpp.compiles('''
      #include <variant>
      int main() {
        std::visit([](auto v) {}, std::variant<int> {});
      }''', name: 'std::visit(std::variant)'),
}))

compat_dep = declare_dependency(include_directories: '.')
