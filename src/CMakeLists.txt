include(CheckCompilerFlag)
include(CheckIncludeFileCXX)
include(CheckLinkerFlag)
include(FindPkgConfig)

add_library(${PROJECT_NAME} SHARED
  api.cpp
  color.cpp
  context.cpp
  docker.cpp
  font.cpp
  main.cpp
  opengl_renderer.cpp
  resource.cpp
  viewport.cpp
  window.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor ${CMAKE_SOURCE_DIR}/vendor/reaper-sdk/sdk
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX "" # disable the "lib" prefix
  OUTPUT_NAME "reaper_imgui-${ARCH_NAME}"
)

if(WIN32)
  target_sources(${PROJECT_NAME} PRIVATE
    version.rc win32_font.cpp win32_platform.cpp win32_window.cpp)
  target_compile_options(${PROJECT_NAME} PUBLIC /W3 /WX /wd4244 /wd4267 /wd4312 /wd4996 /wd5208)
  target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX UNICODE)
  target_link_libraries(${PROJECT_NAME} Imm32)

  find_package(gl3w CONFIG REQUIRED)
  target_link_libraries(${PROJECT_NAME} unofficial::gl3w::gl3w)
else()
  target_compile_options(${PROJECT_NAME} PUBLIC
    -Wall -Wextra -Werror
    -Wno-unused-parameter -Wno-missing-field-initializers
    -Wno-unused-function -Wno-missing-braces
  )

  # Clang 11
  check_compiler_flag(CXX -Wnon-c-typedef-for-linkage HAVE_NON_C_TYPEDEF_FOR_LINKAGE)
  if(HAVE_NON_C_TYPEDEF_FOR_LINKAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC -Wno-non-c-typedef-for-linkage)
  endif()

  # GCC
  check_linker_flag(CXX -Wl,--no-undefined HAVE_NO_UNDEFINED)
  if(HAVE_NO_UNDEFINED)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--no-undefined)
  endif()

  if(APPLE)
    target_sources(${PROJECT_NAME} PRIVATE cocoa_font.mm cocoa_inject.mm
      cocoa_inputview.mm cocoa_platform.mm cocoa_window.mm)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)
    target_compile_options(${PROJECT_NAME} PRIVATE
      -fobjc-arc -Wno-error=deprecated-declarations)

    find_library(CARBON_LIB Carbon)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${CARBON_LIB} OpenGL::GL)
  else()
    target_sources(${PROJECT_NAME} PRIVATE
      fc_font.cpp gdk_platform.cpp gdk_window.cpp)

    find_package(Fontconfig REQUIRED)
    target_link_libraries(${PROJECT_NAME} Fontconfig::Fontconfig)

    pkg_search_module(Epoxy REQUIRED IMPORTED_TARGET epoxy)
    pkg_search_module(GDK REQUIRED IMPORTED_TARGET gdk-3.0)
    target_link_libraries(${PROJECT_NAME} PkgConfig::Epoxy PkgConfig::GDK)
  endif()
endif()

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} Boost::headers)

find_package(ImGui REQUIRED)
target_link_libraries(${PROJECT_NAME} ImGui::ImGui)

find_package(WDL REQUIRED)
target_link_libraries(${PROJECT_NAME} WDL::WDL)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(${PROJECT_NAME} SWELL::swell)
endif()

check_include_file_cxx(optional HAS_STD_OPTIONAL)
if(HAS_STD_OPTIONAL)
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_STD_OPTIONAL)
endif()

check_include_file_cxx(variant HAS_STD_VARIANT)
if(HAS_STD_VARIANT)
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_STD_VARIANT)
endif()

get_target_property(REAIMGUI_BASENAME ${PROJECT_NAME} OUTPUT_NAME)
set(REAIMGUI_FILENAME "${REAIMGUI_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
configure_file(version.hpp.in version.hpp)
