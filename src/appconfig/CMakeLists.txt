gengine_project(appconfig)

gengine_collect_src(SRC)

gengine_add_struct(EntryConfig ${GENGINE_RDLS_DIR}/appconfig.rdl GEN_SRC)
gengine_add_struct(ProcessConfig ${GENGINE_RDLS_DIR}/appconfig.rdl GEN_SRC)
gengine_add_struct(ServiceConfig ${GENGINE_RDLS_DIR}/appconfig.rdl GEN_SRC)
gengine_add_struct(ThreadConfig ${GENGINE_RDLS_DIR}/appconfig.rdl GEN_SRC)
gengine_add_struct(PluginConfig ${GENGINE_RDLS_DIR}/appconfig.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_library(appconfig ${SRC} ${GEN_SRC} ${GENGINE_RDLS_DIR}/appconfig.rdl)

set_target_properties (appconfig PROPERTIES
    FOLDER gengine/libs
)


