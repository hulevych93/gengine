cmake_minimum_required (VERSION 3.0)
project (gengine-examples)

set(CMAKE_CXX_STANDARD 14)

# This tells CMake that we want "-std=c++XX" and not "-std=gnu++XX"
set(CMAKE_CXX_EXTENSIONS FALSE)
# This disables the fallback to an earlier standard if the specified one is not supported by the given compiler.
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)
