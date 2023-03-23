include(FindPkgConfig)

add_library(src OBJECT
  action.cpp
  api.cpp
  color.cpp
  context.cpp
  dialog.rc
  docker.cpp
  error.cpp
  font.cpp
  function.cpp
  image.cpp
  jpeg_image.cpp
  keymap.cpp
  main.cpp
  opengl_renderer.cpp
  png_image.cpp
  renderer.cpp
  resource.cpp
  settings.cpp
  texture.cpp
  viewport.cpp
  window.cpp
)

target_include_directories(src PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(src common)

if(WIN32)
  target_sources(src PRIVATE
    version.rc
    d3d10_renderer.cpp
    win32_droptarget.cpp
    win32_font.cpp
    win32_opengl.cpp
    win32_platform.cpp
    win32_window.cpp
  )

  target_link_libraries(src Dwmapi Imm32 Opengl32)

  set(D3D_SHADERS
    "d3d10_pixel.hlsl\;ps_4_0"
    "d3d10_vertex.hlsl\;vs_4_0"
  )
  foreach(shader ${D3D_SHADERS})
    list(GET shader 0 file)
    list(GET shader 1 profile)
    target_sources(src PRIVATE ${file}.ipp)

    add_custom_command(
      OUTPUT ${file}.ipp
      COMMAND $<TARGET_FILE:gend3dshader> ${profile} ${file} > ${CMAKE_CURRENT_BINARY_DIR}/${file}.ipp
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      MAIN_DEPENDENCY ${file}
      DEPENDS $<TARGET_FILE:gend3dshader>
    )
  endforeach()
elseif(APPLE)
  target_sources(src PRIVATE
    cocoa_events.mm
    cocoa_font.mm
    cocoa_inject.mm
    cocoa_inputview.mm
    cocoa_opengl.mm
    cocoa_platform.mm
    cocoa_window.mm
    metal_renderer.mm
    metal_shader.metal.ipp
  )

  set(METAL_LIBRARIES metal_shader.metal)
  foreach(library ${METAL_LIBRARIES})
    target_sources(src PRIVATE ${library}.ipp)
    add_custom_command(
      OUTPUT ${library}.ipp
      COMMAND ../tools/genmetalshader.sh "${library}"
        "${CMAKE_CURRENT_BINARY_DIR}/${library}.ipp"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      MAIN_DEPENDENCY ${library}
    )
  endforeach()

  target_compile_options(src PRIVATE -fobjc-arc -Wno-error=deprecated-declarations)

  find_library(CARBON_LIB Carbon)
  find_library(QUARTZCORE_LIB QuartzCore)
  find_package(OpenGL REQUIRED)
  target_link_libraries(src ${CARBON_LIB} ${QUARTZCORE_LIB} OpenGL::GL)
else()
  target_sources(src PRIVATE
    fc_font.cpp
    gdk_opengl.cpp
    gdk_platform.cpp
    gdk_window.cpp
  )

  target_compile_definitions(src PRIVATE FOCUS_POLLING)

  find_package(Fontconfig REQUIRED)
  target_link_libraries(src Fontconfig::Fontconfig)

  pkg_search_module(Epoxy REQUIRED IMPORTED_TARGET epoxy)
  pkg_search_module(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
  target_link_libraries(src PkgConfig::Epoxy PkgConfig::GTK)
endif()

if(APPLE AND NOT VCPKG_TOOLCHAIN)
  # before boost otherwise IJG's libjpeg's jpeglib.h will take precedence
  # over libjpeg-turbo's jpeglib.h (-> JCS_EXT_RGBA not defined)
  find_package(libjpeg-turbo REQUIRED)
  target_link_libraries(src libjpeg-turbo::jpeg-static)
else()
  find_package(JPEG REQUIRED)
  target_link_libraries(src JPEG::JPEG)
endif()

find_package(Boost REQUIRED)
target_link_libraries(src Boost::headers)

find_package(ImGui REQUIRED)
target_link_libraries(src ImGui::ImGui)

find_package(PNG REQUIRED)
target_link_libraries(src PNG::PNG)

find_package(WDL REQUIRED)
target_link_libraries(src WDL::WDL)

find_package(EEL REQUIRED)
target_link_libraries(src EEL::eel2_import)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(src SWELL::swell)

  # Transpile the Win32 resource file
  find_package(PHP REQUIRED)
  add_custom_command(
    OUTPUT  dialog.rc_mac_dlg dialog.rc_mac_menu
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_SOURCE_DIR}/dialog.rc ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${PHP_EXECUTABLE} ${SWELL_RESGEN} --quiet dialog.rc
    MAIN_DEPENDENCY dialog.rc
    DEPENDS ${SWELL_RESGEN}
  )
endif()
