# vars.cmake

if(NOT DEFINED GENGINE_ROOT_DIR)
gengine_export_var(GENGINE_ROOT_DIR ${PROJECT_SOURCE_DIR})
gengine_export_var(GENGINE_BINARY_DIR ${PROJECT_BINARY_DIR})
gengine_export_var(GENGINE_CMAKE_DIR ${GENGINE_CMAKE_DIR})

# code generation
gengine_export_var(GENGINE_RDLS_DIR ${PROJECT_SOURCE_DIR}/../rdls)
gengine_export_var(GENGINE_PERL_DIR ${PROJECT_SOURCE_DIR}/../perl)

# installation
gengine_export_var(GENGINE_INSTALL_DIR ${GENGINE_ROOT_DIR}/../install)
gengine_export_var(GENGINE_INCLUDE_DIR ${GENGINE_INSTALL_DIR}/include)
gengine_export_var(GENGINE_LIB_DIR ${GENGINE_INSTALL_DIR}/lib)
gengine_export_var(GENGINE_BIN_DIR ${GENGINE_INSTALL_DIR}/bin)
endif()