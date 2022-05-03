# main.cmake

set(CMAKE_CXX_STANDARD 14)

# This tells CMake that we want "-std=c++XX" and not "-std=gnu++XX"
set(CMAKE_CXX_EXTENSIONS FALSE)
# This disables the fallback to an earlier standard if the specified one is not supported by the given compiler.
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_BINARY_DIR}/bin)

# The `BUILD_FILES_ROOT` is the directory where all the
# project 3rd parties are supposed to be build.
if(DEFINED ENV{BUILD_FILES_ROOT})
    set(BUILD_FILES_DIR $ENV{BUILD_FILES_ROOT})
else()
     message(FATAL_ERROR "Please, define BUILD_FILES_ROOT env variable.")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    message(WARNING "CMAKE_BUILD_TYPE is empty, setting it to DEBUG")
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

set(GENGINE_CMAKE_DIR ${PROJECT_SOURCE_DIR}/../cmake)

include(${GENGINE_CMAKE_DIR}/common.cmake)

# This is the first target which we execute before building our code.
# Note: third-parties are built before this target.
if(NOT DEFINED GENGINE_THIRDPARTY_TARGET_NAME)
    gengine_export_var(GENGINE_THIRDPARTY_TARGET_NAME build_thirdparty)
    add_custom_target(${GENGINE_THIRDPARTY_TARGET_NAME})
endif()

# Full core part target
if(NOT DEFINED GENGINE_TARGET_NAME)
    gengine_export_var(GENGINE_TARGET_NAME build_gengine)
    add_custom_target(${GENGINE_TARGET_NAME})
endif()

include(${GENGINE_CMAKE_DIR}/vars.cmake)
include(${GENGINE_CMAKE_DIR}/generate.cmake)
include(${GENGINE_CMAKE_DIR}/sources.cmake)
include(${GENGINE_CMAKE_DIR}/targets.cmake)
include(${GENGINE_CMAKE_DIR}/platform.cmake)
include(${GENGINE_CMAKE_DIR}/thirdparty.cmake)
include(${GENGINE_CMAKE_DIR}/clang-format.cmake)

