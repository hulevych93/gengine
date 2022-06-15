# vars.cmake

if(NOT DEFINED GENGINE_VARS_EXPORTED)
gengine_export_var(GENGINE_VARS_EXPORTED 1)

gengine_export_var(GENGINE_BINARY_DIR ${GENGINE_BINARY_DIR})
gengine_export_var(GENGINE_CMAKE_DIR ${GENGINE_CMAKE_DIR})
gengine_export_var(GENGINE_INCLUDE_DIR ${GENGINE_ROOT_DIR}/src)

# code generation
gengine_export_var(GENGINE_RDLS_DIR ${GENGINE_ROOT_DIR}/rdls)
gengine_export_var(GENGINE_PERL_DIR ${GENGINE_ROOT_DIR}/perl)

#output
gengine_export_var(GENGINE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
gengine_export_var(GENGINE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
gengine_export_var(GENGINE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
