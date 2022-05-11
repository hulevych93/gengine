project(tests)

include(${PROJECT_SOURCE_DIR}/../cmake/main.cmake)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(appconfig)
add_subdirectory(core)
add_subdirectory(serialization)
add_subdirectory(interprocess-communication)
