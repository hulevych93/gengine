project(diagnostic)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    ${GENGINE_ROOT_DIR}
)

gengine_add_library(diagnostic ${SRC})

set_target_properties (diagnostic PROPERTIES
    FOLDER gengine/libs
)



