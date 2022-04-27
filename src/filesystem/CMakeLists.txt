project(filesystem)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${zip_INCLUDE_DIRS}
    ${GENGINE_BINARY_DIR}
)

gengine_add_library(filesystem ${SRC})

set_target_properties (filesystem PROPERTIES
    FOLDER gengine/support
)


