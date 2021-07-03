include(FindPkgConfig)

add_library(src OBJECT
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

target_include_directories(src PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(src common)

if(WIN32)
  target_sources(src PRIVATE
    version.rc
    win32_font.cpp
    win32_platform.cpp
    win32_window.cpp
  )

  target_link_libraries(src Imm32)

  find_package(gl3w CONFIG REQUIRED)
  target_link_libraries(src unofficial::gl3w::gl3w)
elseif(APPLE)
  target_sources(src PRIVATE
    cocoa_font.mm
    cocoa_inject.mm
    cocoa_inputview.mm
    cocoa_platform.mm
    cocoa_window.mm
  )

  target_compile_definitions(src PRIVATE GL_SILENCE_DEPRECATION)
  target_compile_options(src PRIVATE -fobjc-arc -Wno-error=deprecated-declarations)

  find_library(CARBON_LIB Carbon)
  find_package(OpenGL REQUIRED)
  target_link_libraries(src ${CARBON_LIB} OpenGL::GL)
else()
  target_sources(src PRIVATE
    fc_font.cpp
    gdk_platform.cpp
    gdk_window.cpp
  )

  find_package(Fontconfig REQUIRED)
  target_link_libraries(src Fontconfig::Fontconfig)

  pkg_search_module(Epoxy REQUIRED IMPORTED_TARGET epoxy)
  pkg_search_module(GDK REQUIRED IMPORTED_TARGET gdk-3.0)
  target_link_libraries(src PkgConfig::Epoxy PkgConfig::GDK)
endif()

find_package(Boost REQUIRED)
target_link_libraries(src Boost::headers)

find_package(ImGui REQUIRED)
target_link_libraries(src ImGui::ImGui)

find_package(WDL REQUIRED)
target_link_libraries(src WDL::WDL)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(src SWELL::swell)
endif()

get_target_property(REAIMGUI_BASENAME ${PROJECT_NAME} OUTPUT_NAME)
set(REAIMGUI_FILENAME "${REAIMGUI_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
configure_file(version.hpp.in version.hpp)
