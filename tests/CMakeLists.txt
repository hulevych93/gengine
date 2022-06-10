project(tests)

include(${PROJECT_SOURCE_DIR}/../cmake/main.cmake)

include_directories(${PROJECT_SOURCE_DIR})

enable_testing()
include(CTest)

add_subdirectory(appconfig)
add_subdirectory(core)
add_subdirectory(serialization)
add_subdirectory(interprocess-communication)
add_subdirectory(multithreading)

add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS appconfig-test
                          core-test
                          serialization-test
                          interprocess-communication-test)
