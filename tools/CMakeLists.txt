add_executable(genbinding EXCLUDE_FROM_ALL genbinding.cpp)
target_link_libraries(genbinding common)
target_include_directories(genbinding PRIVATE ${CMAKE_BINARY_DIR}/src)
target_link_libraries(genbinding ${PROJECT_NAME})

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(genbinding SWELL::swell)
endif()

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

add_custom_target(bindings ALL)
add_binding(cpp    "${CMAKE_CURRENT_BINARY_DIR}/reaper_imgui_functions.h")
add_binding(human  "${CMAKE_CURRENT_BINARY_DIR}/reaper_imgui_doc.html")
add_binding(python "${CMAKE_CURRENT_BINARY_DIR}/imgui_python.py")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/imgui_python.py"
  DESTINATION "Scripts/ReaTeam Extensions/API"
  OPTIONAL
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/reaper_imgui_doc.html"
  DESTINATION "Data"
  OPTIONAL
)
