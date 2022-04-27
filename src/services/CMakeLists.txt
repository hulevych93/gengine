gengine_project(services)

gengine_collect_src(SRC)

gengine_add_struct(SharedConnection ${GENGINE_RDLS_DIR}/connection.rdl GEN_SRC)
gengine_add_struct(ExternalConnection ${GENGINE_RDLS_DIR}/connection.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_library(services ${SRC} ${GEN_SRC})

set_target_properties(services PROPERTIES FOLDER gengine/libs)


