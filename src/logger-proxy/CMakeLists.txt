project(logger-proxy)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
    ${GENGINE_BINARY_DIR}/core
)

gengine_add_shared_library(logger-proxy ${SRC})

set_target_properties (logger-proxy PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(logger-proxy
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)


