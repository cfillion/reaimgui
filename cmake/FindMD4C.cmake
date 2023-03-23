if(TARGET MD4C::MD4C)
  return()
endif()

find_path(MD4C_INCLUDE_DIR
  NAMES md4c.h
  PATHS ${CMAKE_SOURCE_DIR}/vendor/md4c/src
  NO_DEFAULT_PATH
)
mark_as_advanced(MD4C_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MD4C
  REQUIRED_VARS MD4C_INCLUDE_DIR
)

add_library(md4c
  ${MD4C_INCLUDE_DIR}/entity.c
  ${MD4C_INCLUDE_DIR}/md4c-html.c
  ${MD4C_INCLUDE_DIR}/md4c.c
)

target_include_directories(md4c PUBLIC ${MD4C_INCLUDE_DIR})

add_library(MD4C::MD4C ALIAS md4c)
