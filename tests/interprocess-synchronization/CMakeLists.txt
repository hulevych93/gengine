gengine_project(interprocess-synchronization-test)

gengine_collect_src(SRC)

include_directories(
    .
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_INCLUDE_DIR}
    ${GTest_INCLUDE_DIRS}
    ${GENGINE_BINARY_DIR}
)

gengine_add_test(interprocess-synchronization-test ${SRC})

target_link_libraries(interprocess-synchronization-test
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${AdditionalOS_LIBRARIES}
    ${GTest_LIBRARIES}
)

gengine_import_binaries(interprocess-synchronization-test)

