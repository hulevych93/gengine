gengine_project(service)

gengine_collect_src(SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_test(service ${SRC} ${GEN_SRC})

target_link_libraries(service
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)

gengine_import_binaries(service)
