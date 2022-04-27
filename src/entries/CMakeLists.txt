project(entries)

gengine_collect_src(SRC wxWidgets)

if(${wxWidgets_FOUND})
  set(SRC ${SRC}
    wxWidgets/wxArgsVisitor.h
    wxWidgets/wxArgsVisitor.cpp
  )
endif()

gengine_add_struct(PluginInfo ${GENGINE_RDLS_DIR}/plugins.rdl GEN_SRC)

gengine_add_library(entries ${SRC} ${GEN_SRC})

set_target_properties (entries PROPERTIES
    FOLDER gengine/libs
)

add_dependencies(entries core)


