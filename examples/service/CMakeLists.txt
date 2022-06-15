gengine_project(gservice)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_test(gservice ${SRC} ${GEN_SRC})

target_link_libraries(gservice
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)

gengine_import_binaries(gservice)
