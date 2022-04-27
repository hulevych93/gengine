gengine_project(patcher)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${ADDITIONAL_OS_Headers}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

add_executable(patcher ${EXECUTABLE_TYPE} ${SRC})
gengine_export(patcher ${GENGINE_BIN_DIR})

set_target_properties (patcher PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(patcher
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Log4cplus_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)


