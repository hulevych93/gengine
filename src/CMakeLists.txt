cmake_minimum_required (VERSION 3.0)
project (gengine)

set(GENGINE_CMAKE_DIR ${PROJECT_SOURCE_DIR}/../cmake)
include(${GENGINE_CMAKE_DIR}/common.cmake)

gengine_export_var(GENGINE_ROOT_DIR ${PROJECT_SOURCE_DIR})
gengine_export_var(GENGINE_BINARY_DIR ${PROJECT_BINARY_DIR})
gengine_export_var(GENGINE_CMAKE_DIR ${GENGINE_CMAKE_DIR})

gengine_export_var(GENGINE_RDLS_DIR ${PROJECT_SOURCE_DIR}/../rdls)
gengine_export_var(GENGINE_PERL_DIR ${PROJECT_SOURCE_DIR}/../perl)

include(${GENGINE_CMAKE_DIR}/main.cmake)
include(ExternalProject)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(thirdparty/boost)

include_directories(".")

add_subdirectory(api)
