if(LICE_FOUND)
  return()
endif()

find_package(SWELL REQUIRED)

find_path(LICE_INCLUDE_DIR
  NAMES lice/lice.h
  PATHS ${WDL_DIR}
  NO_DEFAULT_PATH
)
mark_as_advanced(LICE_INCLUDE_DIR)

set(LICE_DIR "${LICE_INCLUDE_DIR}/lice")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LICE REQUIRED_VARS LICE_DIR)

add_library(lice ${LICE_DIR}/lice.cpp)

target_include_directories(lice INTERFACE ${LICE_INCLUDE_DIR})
target_link_libraries(lice PUBLIC SWELL::swell)

add_library(LICE::lice ALIAS lice)
