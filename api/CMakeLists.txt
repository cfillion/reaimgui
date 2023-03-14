add_library(api OBJECT
  button.cpp
  coloredit.cpp
  context.cpp
  dragndrop.cpp
  drawlist.cpp
  font.cpp
  function.cpp
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

target_link_libraries(api common)

find_package(Boost REQUIRED)
target_link_libraries(api Boost::headers)

find_package(EEL REQUIRED)
target_link_libraries(api EEL::eel2_import)

find_package(ImGui REQUIRED)
target_link_libraries(api ImGui::ImGui)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  target_link_libraries(api SWELL::swell)
endif()
