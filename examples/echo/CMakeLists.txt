gengine_project(echo)

gengine_collect_src(SRC)

gengine_export_var(API_FOLDER "api")

gengine_add_handler(IEcho ${GENGINE_RDLS_DIR}/echo.rdl GEN_SRC)
gengine_add_client(IEcho ${GENGINE_RDLS_DIR}/echo.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${GENGINE_ROOT_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_executable(echo ${EXECUTABLE_TYPE} ${SRC} ${GEN_SRC})

target_link_libraries(echo
    ${GENGINE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${AdditionalOS_LIBRARIES}
)

gengine_import_binaries(echo)
