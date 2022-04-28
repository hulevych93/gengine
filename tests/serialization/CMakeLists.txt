project(serialization-test)

gengine_collect_src(SRC)

include_directories(
    .
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_INCLUDE_DIR}
    ${GTest_INCLUDE_DIRS}
    ${GENGINE_BINARY_DIR}
)

gengine_add_executable(serialization-test ${SRC})

target_link_libraries(serialization-test
    ${Boost_LIBRARIES}
    ${GENGINE_LIBRARIES}
    ${AdditionalOS_LIBRARIES}
    ${GTest_LIBRARIES}
    gtest-common
)

gengine_import_binaries(echo)
