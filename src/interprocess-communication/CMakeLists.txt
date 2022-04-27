project(interprocess-communication)

gengine_collect_src(SRC ssl)

gengine_add_struct(PipeConnection ${GENGINE_RDLS_DIR}/connection.rdl GEN_SRC)
gengine_add_struct(TcpConnection ${GENGINE_RDLS_DIR}/connection.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_library(interprocess-communication ${SRC} ${GEN_SRC})

set_target_properties (interprocess-communication PROPERTIES
    FOLDER gengine/libs
)


