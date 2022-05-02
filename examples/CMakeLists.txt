project(examples)

include(${PROJECT_SOURCE_DIR}/../cmake/main.cmake)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(echo)
add_subdirectory(service)