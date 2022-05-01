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

gengine_add_executable(appconfig-test ${SRC})

target_link_libraries(appconfig-test
    ${Boost_LIBRARIES}
    ${GENGINE_LIBRARIES}
    ${AdditionalOS_LIBRARIES}
    ${GTest_LIBRARIES}
    gtest-common
)

gengine_import_binaries(appconfig-test)

