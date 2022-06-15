gengine_project(interprocess-synchronization)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${X11_INCLUDE_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_library(interprocess-synchronization ${SRC})

set_target_properties (interprocess-synchronization PROPERTIES
    FOLDER gengine/libs
)



