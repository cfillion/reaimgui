add_library(api OBJECT
  button.cpp
  # coloredit.cpp
  # context.cpp
  # dragndrop.cpp
  # drawlist.cpp
  # font.cpp
  # indev.cpp
  # input.cpp
  # item.cpp
  # layout.cpp
  # listclipper.cpp
  # menu.cpp
  # plot.cpp
  # popup.cpp
  # secret.cpp
  # select.cpp
  # slider.cpp
  # style.cpp
  # tabbar.cpp
  # table.cpp
  # text.cpp
  # treenode.cpp
  # utility.cpp
  # viewport.cpp
  # window.cpp
)

target_include_directories(api PRIVATE
  ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src)
target_link_libraries(api common)

get_target_property(API_FILES api SOURCES)

foreach(API_FILE IN LISTS API_FILES)
  get_filename_component(API_BASENAME "${API_FILE}" NAME_WE)
  set(API_FILE_DEF "${CMAKE_CURRENT_BINARY_DIR}/${API_BASENAME}.apidef.cpp")

  add_custom_command(
    OUTPUT "${API_FILE_DEF}"
    COMMAND $<TARGET_FILE:genapidef> "${CMAKE_CURRENT_SOURCE_DIR}/${API_FILE}"
      -Isrc -Ivendor -Ivendor/WDL -Ivendor/WDL/WDL > "${API_FILE_DEF}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DEPENDS $<TARGET_FILE:genapidef> "${API_FILE}"
    COMMAND_EXPAND_LISTS
  )

  target_sources(api PRIVATE "${API_FILE_DEF}")
endforeach()

find_package(Boost REQUIRED)
target_link_libraries(api Boost::headers)

find_package(ImGui REQUIRED)
target_link_libraries(api ImGui::ImGui)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(api SWELL::swell)
endif()
