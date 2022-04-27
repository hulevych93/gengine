gengine_project(api)

gengine_export_var(GENGINE_TARGETS_COMMON_DEPENDENCIES "api")
gengine_export_var(API_FOLDER "api")

gengine_collect_rdls(RDLS)

foreach(RDL ${RDLS})
    gengine_gen_api(${RDL} ${GENGINE_BINARY_DIR} GENERATED_FILES)
endforeach()

add_library(api "dummy.cpp" ${GENERATED_FILES} ${RDLS})
