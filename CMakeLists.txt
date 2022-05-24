cmake_minimum_required (VERSION 3.0)
project (gengine)

set(GENGINE_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(GENGINE_BINARY_DIR ${PROJECT_BINARY_DIR}/src)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)
