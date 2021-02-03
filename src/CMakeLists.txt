include(CheckCXXCompilerFlag)

add_library(${PROJECT_NAME} SHARED
  api.cpp
  api_window.cpp
  main.cpp
  window.cpp
  heartbeat.cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor ${CMAKE_SOURCE_DIR}/vendor/reaper-sdk/sdk
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

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} Boost::headers)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(${PROJECT_NAME} SWELL::swell)
endif()
