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

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    message(WARNING "CMAKE_BUILD_TYPE is empty, setting it to DEBUG")
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

include(${GENGINE_CMAKE_DIR}/generate.cmake)
include(${GENGINE_CMAKE_DIR}/sources.cmake)
include(${GENGINE_CMAKE_DIR}/targets.cmake)

if(WIN32)
    gengine_set_runtime()
elseif(UNIX)
    FIND_PACKAGE(X11 REQUIRED)

    set(AdditionalOS_LIBRARIES
        iconv
        pthread
        ${X11_LIBRARIES}
    )
endif()

include(${GENGINE_CMAKE_DIR}/thirdparty.cmake)

