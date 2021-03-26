add_executable(genbinding EXCLUDE_FROM_ALL genbinding.cpp)
target_compile_features(genbinding PRIVATE cxx_std_17)
target_include_directories(genbinding PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor/reaper-sdk/sdk
  ${CMAKE_SOURCE_DIR}/vendor/WDL
  ${CMAKE_BINARY_DIR}/src
)
target_link_libraries(genbinding ${PROJECT_NAME})

function(add_binding lang output)
  add_custom_command(
    OUTPUT ${output}
    COMMAND $<TARGET_FILE:genbinding> ${lang} > ${output}
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    DEPENDS $<TARGET_FILE:genbinding>
    COMMAND_EXPAND_LISTS
  )

  add_custom_target(${lang}_binding DEPENDS ${output})
  add_dependencies(${lang}_binding genbinding)
  add_dependencies(bindings ${lang}_binding)
endfunction()

add_custom_target(bindings)
add_binding(cpp    "${CMAKE_CURRENT_BINARY_DIR}/reaper_imgui_functions.h")
add_binding(python "${CMAKE_CURRENT_BINARY_DIR}/imgui_python.py")
