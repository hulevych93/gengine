gengine_project(service-broker)

gengine_collect_src(SRC)

gengine_add_client(ILogger ${GENGINE_RDLS_DIR}/logger.rdl GEN_SRC)
gengine_add_client(ILifetime ${GENGINE_RDLS_DIR}/lifetime.rdl GEN_SRC)

gengine_add_struct(ServiceInfo ${GENGINE_RDLS_DIR}/servicerouter.rdl GEN_SRC)
gengine_add_struct(Version ${GENGINE_RDLS_DIR}/servicerouter.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
    ${GENGINE_BINARY_DIR}/service-broker
    ${GENGINE_BINARY_DIR}/service-router
    ${GENGINE_BINARY_DIR}/core
)

gengine_add_shared_library(service-broker ${SRC} ${GEN_SRC})

set_target_properties (service-broker PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(service-broker
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)


