if(TARGET EEL::eel2)
  return()
endif()

find_package(WDL REQUIRED)

find_path(EEL_INCLUDE_DIR
  NAMES eel2/ns-eel.h
  PATHS ${WDL_DIR}
  NO_DEFAULT_PATH
)
mark_as_advanced(EEL_INCLUDE_DIR)

set(EEL_DIR "${EEL_INCLUDE_DIR}/eel2")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EEL REQUIRED_VARS EEL_DIR)

add_library(eel2
  ${EEL_DIR}/nseel-cfunc.c
  ${EEL_DIR}/nseel-compiler.c
  ${EEL_DIR}/nseel-eval.c
  ${EEL_DIR}/nseel-ram.c
  ${EEL_DIR}/y.tab.c
)
set_property(TARGET eel2 PROPERTY C_STANDARD 90)
target_compile_definitions(eel2 PRIVATE EEL_TARGET_PORTABLE)
add_library(EEL::eel2 ALIAS eel2)

add_library(eel2_import INTERFACE)
target_include_directories(eel2_import INTERFACE ${EEL_INCLUDE_DIR})
add_library(EEL::eel2_import ALIAS eel2_import)
