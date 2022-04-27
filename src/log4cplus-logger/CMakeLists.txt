project(log4cplus-logger)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${Log4cplus_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_shared_library(log4cplus-logger ${SRC})

set_target_properties (log4cplus-logger PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(log4cplus-logger
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Log4cplus_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)
