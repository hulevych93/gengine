gengine_project(worker-broker)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_shared_library(worker-broker ${SRC})

set_target_properties (worker-broker PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(worker-broker
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)


