project(appconfig-test)

gengine_collect_src(SRC)

include_directories(
    .
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_INCLUDE_DIR}
    ${GTest_INCLUDE_DIRS}
    ${GENGINE_BINARY_DIR}
)

gengine_add_test(appconfig-test ${SRC})

target_link_libraries(appconfig-test
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${AdditionalOS_LIBRARIES}
    ${GTest_LIBRARIES}
)

gengine_import_binaries(appconfig-test)

