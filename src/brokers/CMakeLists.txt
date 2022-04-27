gengine_project(brokers)

gengine_collect_src(SRC)

gengine_add_library(brokers ${SRC})

set_target_properties(brokers PROPERTIES FOLDER gengine/libs)


