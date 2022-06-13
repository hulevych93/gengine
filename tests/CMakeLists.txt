project(tests)

set(GENGINE_TESTS)

include(${PROJECT_SOURCE_DIR}/../cmake/main.cmake)

include_directories(${PROJECT_SOURCE_DIR})

enable_testing()

add_subdirectory(appconfig)
add_subdirectory(core)
add_subdirectory(serialization)
add_subdirectory(interprocess-communication)
add_subdirectory(multithreading)

add_custom_target(run_tests
                  COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE}
                  DEPENDS ${GENGINE_TESTS}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
