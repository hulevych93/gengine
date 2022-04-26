# main.cmake

set(CMAKE_CXX_STANDARD 14)

# This tells CMake that we want "-std=c++XX" and not "-std=gnu++XX"
set(CMAKE_CXX_EXTENSIONS FALSE)
# This disables the fallback to an earlier standard if the specified one is not supported by the given compiler.
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# The `BUILD_FILES_ROOT` is the directory where all the
# project 3rd parties are supposed to be build.
if(DEFINED ENV{BUILD_FILES_ROOT})
    set(BUILD_FILES_DIR $ENV{BUILD_FILES_ROOT})
else()
     message(FATAL_ERROR "Please, define BUILD_FILES_ROOT env variable.")
endif()


include(${GENGINE_CMAKE_DIR}/thirdparty.cmake)
include(${GENGINE_CMAKE_DIR}/generate.cmake)
