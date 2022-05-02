gengine_project(executor-broker)

gengine_collect_src(SRC)

gengine_add_handler(IServiceRouter ${GENGINE_RDLS_DIR}/servicerouter.rdl GEN_SRC)
gengine_add_handler(ILogger ${GENGINE_RDLS_DIR}/logger.rdl GEN_SRC)
gengine_add_handler(ILifetime ${GENGINE_RDLS_DIR}/lifetime.rdl GEN_SRC)

gengine_add_struct(ServiceInfo ${GENGINE_RDLS_DIR}/servicerouter.rdl GEN_SRC)
gengine_add_struct(Version ${GENGINE_RDLS_DIR}/servicerouter.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_shared_library(executor-broker ${SRC} ${GEN_SRC})

set_target_properties (executor-broker PROPERTIES
    FOLDER gengine/bins
)

target_link_libraries(executor-broker
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)


