add_executable(genbinding EXCLUDE_FROM_ALL genbinding.cpp)
target_link_libraries(genbinding common ${PROJECT_NAME})

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(genbinding SWELL::swell)
endif()

find_package(cmark REQUIRED)
if(VCPKG_TOOLCHAIN)
  target_link_libraries(genbinding cmark::cmark_static)
else()
  target_link_libraries(genbinding cmark::cmark)
endif()

function(add_binding lang output)
  add_custom_command(
    OUTPUT ${output}
    COMMAND $<TARGET_FILE:genbinding> ${lang} > ${output}
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    DEPENDS $<TARGET_FILE:genbinding>
  )

  add_custom_target(${lang}_binding DEPENDS ${output})
  add_dependencies(${lang}_binding genbinding)
  add_dependencies(bindings ${lang}_binding)
endfunction()

add_executable(genshim EXCLUDE_FROM_ALL genshim.cpp)
find_package(Boost REQUIRED)
target_link_libraries(genshim Boost::headers common)
if(NOT HAVE_STD_FILESYSTEM)
  find_package(Boost REQUIRED COMPONENTS filesystem)
  target_link_libraries(genshim Boost::filesystem)
endif()

function(add_shim lang output)
  file(GLOB shims CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/shims/${lang}/*")
  add_custom_command(
    OUTPUT ${output}
    COMMAND $<TARGET_FILE:genshim> ${lang} "shims/${lang}" > ${output}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DEPENDS $<TARGET_FILE:genshim> ${shims}
  )

  add_custom_target(${lang}_shim DEPENDS ${output})
  add_dependencies(${lang}_shim genshim)
  add_dependencies(bindings ${lang}_shim)
endfunction()

add_custom_target(bindings ALL)
add_binding(cpp    "${CMAKE_CURRENT_BINARY_DIR}/reaper_imgui_functions.h")
add_binding(human  "${CMAKE_CURRENT_BINARY_DIR}/reaper_imgui_doc.html")
add_binding(python "${CMAKE_CURRENT_BINARY_DIR}/imgui_python.py")
add_shim   (lua    "${CMAKE_CURRENT_BINARY_DIR}/imgui.lua")

set(SCRIPT_FOLDER "Scripts/ReaTeam Extensions/API")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/imgui_python.py"
  DESTINATION  "${SCRIPT_FOLDER}"
  OPTIONAL
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/imgui.lua"
  DESTINATION  "${SCRIPT_FOLDER}"
  OPTIONAL
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/reaper_imgui_doc.html"
  DESTINATION "Data"
  OPTIONAL
)

if(WIN32)
  add_executable(gend3dshader EXCLUDE_FROM_ALL gend3dshader.cpp)
  target_link_libraries(gend3dshader common)
  target_link_libraries(gend3dshader D3DCompiler)
endif()
