project(serialization)

gengine_collect_src(SRC)

include_directories(
    ${Boost_INCLUDE_DIRS}
)

gengine_add_library(serialization ${SRC})

set_target_properties (serialization PROPERTIES
    FOLDER gengine/support
)


