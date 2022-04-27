gengine_project(process-broker)

gengine_collect_src(SRC)

gengine_add_client(ILifetime ${GENGINE_RDLS_DIR}/lifetime.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
    ${GENGINE_BINARY_DIR}/service-broker
    ${GENGINE_BINARY_DIR}/services
    ${GENGINE_BINARY_DIR}/core
)

gengine_add_shared_library(process-broker ${SRC} ${GEN_SRC})

set_target_properties (process-broker PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(process-broker
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)


