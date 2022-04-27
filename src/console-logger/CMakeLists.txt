project(console-logger)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${PROJECT_BINARY_DIR}
    ${GENGINE_BINARY_DIR}/core
    ${GENGINE_BINARY_DIR}
)

gengine_add_handler(ILogger ${GENGINE_RDLS_DIR}/logger.rdl GEN_SRC)

gengine_add_executable(console-logger ${SRC} ${GEN_SRC})

set_target_properties (console-logger PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(console-logger
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)

add_dependencies(console-logger core)


