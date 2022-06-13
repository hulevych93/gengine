gengine_project(interprocess-communication-test)

gengine_collect_src(SRC)

include_directories(
    .
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_INCLUDE_DIR}
    ${GTest_INCLUDE_DIRS}
    ${GENGINE_BINARY_DIR}
)

gengine_add_test(interprocess-communication-test ${SRC})

target_link_libraries(interprocess-communication-test
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${AdditionalOS_LIBRARIES}
    ${GTest_LIBRARIES}
)

gengine_import_binaries(interprocess-communication-test)

