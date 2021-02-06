include(CheckCXXCompilerFlag)

add_library(${PROJECT_NAME} SHARED
  api.cpp
  api_context.cpp
  api_enums.cpp
  api_imgui.cpp # TODO delete
  api_input.cpp
  api_item.cpp
  api_layout.cpp
  api_sliders.cpp
  api_menus.cpp
  api_params.cpp
  api_plots.cpp
  api_style.cpp
  api_tables.cpp
  api_text.cpp
  api_tooltips.cpp
  api_trees.cpp
  api_widgets.cpp
  api_windows.cpp
  main.cpp
  watchdog.cpp
  window.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor ${CMAKE_SOURCE_DIR}/vendor/reaper-sdk/sdk
  ${CMAKE_SOURCE_DIR}/vendor/WDL
  ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX "" # disable the "lib" prefix
  OUTPUT_NAME "reaper_imgui-${ARCH_NAME}"
)

if(WIN32)
  # TODO
  # target_sources(${PROJECT_NAME} PRIVATE buildinfo.rc)
  target_compile_options(${PROJECT_NAME} PUBLIC /W3 /WX /wd4996 /wd5208)
  target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX UNICODE)
  # target_link_libraries(${PROJECT_NAME} )
else()
  target_compile_options(${PROJECT_NAME} PUBLIC
    -Wall -Wextra -Werror
    -Wno-unused-parameter -Wno-missing-field-initializers
    -Wno-unused-function -Wno-missing-braces
  )

  # Clang 11
  check_cxx_compiler_flag(-Wnon-c-typedef-for-linkage HAVE_NON_C_TYPEDEF_FOR_LINKAGE)
  if(HAVE_NON_C_TYPEDEF_FOR_LINKAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC -Wno-non-c-typedef-for-linkage)
  endif()
endif()

if(APPLE)
  target_sources(${PROJECT_NAME} PRIVATE inputview.mm window_metal.mm)

  find_library(QUARTZCORE_LIB QuartzCore)
  target_link_libraries(${PROJECT_NAME} ${QUARTZCORE_LIB})
endif()

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} Boost::headers)

find_package(ImGui REQUIRED)
target_link_libraries(${PROJECT_NAME} ImGui::ImGui)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(${PROJECT_NAME} SWELL::swell)
endif()
