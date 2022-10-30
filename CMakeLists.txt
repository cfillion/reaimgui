cmake_minimum_required(VERSION 3.19)
project(reaimgui LANGUAGES CXX)

include(CheckCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckLinkerFlag)
include(CTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(WIN32)
    set(USER_CONFIG_DIR "$ENV{APPDATA}")
  elseif(APPLE)
    set(USER_CONFIG_DIR "~/Library/Application Support")
  else()
    set(USER_CONFIG_DIR "~/.config")
  endif()

  set(CMAKE_INSTALL_PREFIX "${USER_CONFIG_DIR}/REAPER" CACHE PATH
    "REAPER resource path where to install ReaImGui" FORCE)
endif()

if(CMAKE_OSX_ARCHITECTURES)
  list(JOIN CMAKE_OSX_ARCHITECTURES "-" ARCH_NAME)
elseif(MSVC_CXX_ARCHITECTURE_ID)
  set(ARCH_NAME ${MSVC_CXX_ARCHITECTURE_ID})
else()
  set(ARCH_NAME ${CMAKE_SYSTEM_PROCESSOR})
endif()

STRING(TOLOWER "${ARCH_NAME}" ARCH_NAME)

set(CMAKE_CXX_EXTENSIONS OFF)
if(cxx_std_20 IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(CMAKE_CXX_STANDARD 20)
else()
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL     ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE        ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)

if(VCPKG_TOOLCHAIN)
  set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL Release)
  set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
endif()

if(WIN32)
  foreach(arg
    CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_MINSIZEREL
  )
    # Embed debug information in the object files
    string(REGEX REPLACE "/Z[iI]" "/Z7" ${arg} "${${arg}}")

    # Allow expansion of function not explicitly marked for no inlining
    string(REGEX REPLACE "/Ob1" "/Ob2" ${arg} "${${arg}}")
  endforeach()

  add_compile_options(
    /utf-8
    /EHsc                                # Exception handling model
    $<$<NOT:$<CONFIG:Debug>>:/Zo>        # Enhance optimized debugging
    $<$<NOT:$<CONFIG:Debug>>:/GF>        # Eliminate duplicate strings
    $<$<NOT:$<CONFIG:Debug>>:/Gy>        # Enable function-level linking
    $<$<NOT:$<CONFIG:Debug>>:/Zc:inline> # Remove unreferenced COMDAT
  )

  add_link_options(
    $<$<NOT:$<CONFIG:Debug>>:/OPT:REF> # Remove unreferenced code
    $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF> # Remove duplicate sections

    # Use relative path to the PDB file to avoid exposing the full path
    /PDBALTPATH:%_PDB%
  )
else()
  add_compile_options(
    -fsigned-char -fstack-protector-strong -fdiagnostics-color
  )

  if(APPLE)
    add_link_options(-Wl,-dead_strip)
  endif()

  check_linker_flag(CXX -Wl,--no-undefined HAVE_NO_UNDEFINED)
  if(HAVE_NO_UNDEFINED)
    add_link_options(-Wl,--no-undefined)
  endif()
endif()

add_library(common INTERFACE)
target_include_directories(common INTERFACE
  ${CMAKE_SOURCE_DIR}/vendor ${CMAKE_SOURCE_DIR}/vendor/reaper-sdk/sdk)
if(WIN32)
  target_compile_options(common INTERFACE /W3 /WX /wd4244 /wd4267 /wd4312 /wd4996)
  target_compile_definitions(common INTERFACE NOMINMAX UNICODE)
else()
  target_compile_options(common INTERFACE
    -Wall -Wextra -Werror
    -Wno-unused-parameter -Wno-missing-field-initializers -Wno-missing-braces
  )
endif()

check_include_file_cxx(optional HAS_STD_OPTIONAL)
if(HAS_STD_OPTIONAL)
  target_compile_definitions(common INTERFACE HAS_STD_OPTIONAL)
endif()

check_include_file_cxx(variant HAS_STD_VARIANT)
if(HAS_STD_VARIANT)
  target_compile_definitions(common INTERFACE HAS_STD_VARIANT)
endif()

if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
  target_compile_definitions(common INTERFACE HAS_CPP_20)
endif()

find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --match v*
    OUTPUT_VARIABLE PROJECT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  string(REGEX REPLACE "^v" "" PROJECT_VERSION "${PROJECT_VERSION}")
else()
  set(PROJECT_VERSION "0.0-0-nogit")
  message(WARNING "git executable not found, setting PROJECT_VERSION to ${PROJECT_VERSION}")
endif()

if("${PROJECT_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)(\\.([0-9]+))?(-([0-9]+))?")
  foreach(pair "1;MAJOR" "2;MINOR" "4;PATCH" "6;TWEAK")
    list(GET pair 0 capture)
    list(GET pair 1 segment)
    if(${CMAKE_MATCH_${capture}})
      set(PROJECT_VERSION_${segment} "${CMAKE_MATCH_${capture}}")
    else()
      set(PROJECT_VERSION_${segment} 0)
    endif()
  endforeach()
endif()

add_library(${PROJECT_NAME} SHARED)
set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX "" # disable the "lib" prefix
  OUTPUT_NAME "reaper_imgui-${ARCH_NAME}"
)
set(REAPER_USER_PLUGINS "UserPlugins")
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${REAPER_USER_PLUGINS}" # Windows .dll
  LIBRARY DESTINATION "${REAPER_USER_PLUGINS}" # Linux .so/macOS .dylib
)
add_subdirectory(tools)
add_subdirectory(api)
add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE api src)
