set(API_FILES
  button.cpp
  coloredit.cpp
  context.cpp
  dragndrop.cpp
  drawlist.cpp
  font.cpp
  indev.cpp
  input.cpp
  item.cpp
  layout.cpp
  listclipper.cpp
  menu.cpp
  plot.cpp
  popup.cpp
  secret.cpp
  select.cpp
  slider.cpp
  style.cpp
  tabbar.cpp
  table.cpp
  text.cpp
  treenode.cpp
  utility.cpp
  viewport.cpp
  window.cpp
)

foreach(API_FILE IN LISTS API_FILES)
  string(REGEX REPLACE "^(.+)\.cpp$" "\\1" API_BASENAME "${API_FILE}")
  target_sources(${PROJECT_NAME} PRIVATE "${API_FILE}")
  set_source_files_properties(
    "${API_FILE}" TARGET_DIRECTORY ${PROJECT_NAME}
    PROPERTIES
      COMPILE_DEFINITIONS API_FILE=\"${API_BASENAME}\"
  )
endforeach()
