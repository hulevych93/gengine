project(multithreading)

gengine_collect_src(SRC)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_BINARY_DIR}
)

gengine_add_library(multithreading ${SRC})

set_target_properties (multithreading PROPERTIES
    FOLDER gengine/support
)


