# vars.cmake

gengine_export_var(GENGINE_BINARY_DIR ${GENGINE_BINARY_DIR})
gengine_export_var(GENGINE_CMAKE_DIR ${GENGINE_CMAKE_DIR})
gengine_export_var(GENGINE_INCLUDE_DIR ${GENGINE_ROOT_DIR}/src)

# code generation
gengine_export_var(GENGINE_RDLS_DIR ${GENGINE_ROOT_DIR}/rdls)
gengine_export_var(GENGINE_PERL_DIR ${GENGINE_ROOT_DIR}/perl)

gengine_export_var(GENGINE_ARCHIVE_OUTPUT_DIRECTORY ${GENGINE_BINARY_DIR}/lib)

if(WIN32 OR APPLE)
    gengine_export_var(GENGINE_LIBRARY_OUTPUT_DIRECTORY ${GENGINE_BINARY_DIR}/lib)
else()
    gengine_export_var(GENGINE_LIBRARY_OUTPUT_DIRECTORY ${GENGINE_BINARY_DIR}/bin)
endif()

gengine_export_var(GENGINE_RUNTIME_OUTPUT_DIRECTORY  ${GENGINE_BINARY_DIR}/bin)
