project(json)

gengine_collect_src(SRC)

include_directories(
    ${Boost_INCLUDE_DIRS}
)

gengine_add_library(json ${SRC})

set_target_properties (json PROPERTIES
    FOLDER gengine/support
)


