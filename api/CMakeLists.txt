add_library(api OBJECT
  button.cpp
  coloredit.cpp
  context.cpp
  dragndrop.cpp
  drawlist.cpp
  font.cpp
  image.cpp
  indev.cpp
  input.cpp
  item.cpp
  layout.cpp
  listclipper.cpp
  menu.cpp
  plot.cpp
  popup.cpp
  select.cpp
  slider.cpp
  style.cpp
  tabbar.cpp
  table.cpp
  text.cpp
  textfilter.cpp
  treenode.cpp
  utility.cpp
  viewport.cpp
  window.cpp
)

target_include_directories(api PRIVATE ${CMAKE_BINARY_DIR}/src)
target_link_libraries(api common)

get_target_property(API_FILES api SOURCES)

foreach(API_FILE IN LISTS API_FILES)
  get_filename_component(API_BASENAME "${API_FILE}" NAME_WE)
  set_property(SOURCE "${API_FILE}"
    PROPERTY COMPILE_DEFINITIONS "API_FILE=${API_BASENAME}")
endforeach()

find_package(Boost REQUIRED)
target_link_libraries(api Boost::headers)

find_package(ImGui REQUIRED)
target_link_libraries(api ImGui::ImGui)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(api SWELL::swell)
endif()
