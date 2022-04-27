project(core)

gengine_collect_src(SRC)

gengine_add_struct(SessionId ${GENGINE_RDLS_DIR}/os.rdl GEN_SRC)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    ${GENGINE_BINARY_DIR}
)

gengine_add_library(core ${SRC} ${GEN_SRC})

set_target_properties (core PROPERTIES
    FOLDER gengine/support
)

if(WIN32)
set(ADDITIONAL_OS_Headers
    ../core/Windows
)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
set(ADDITIONAL_OS_Headers
    ../core/Linux
)
endif()



