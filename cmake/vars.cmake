# vars.cmake

gengine_export_var(GENGINE_BINARY_DIR ${GENGINE_BINARY_DIR})
gengine_export_var(GENGINE_CMAKE_DIR ${GENGINE_CMAKE_DIR})

# code generation
gengine_export_var(GENGINE_RDLS_DIR ${GENGINE_ROOT_DIR}/rdls)
gengine_export_var(GENGINE_PERL_DIR ${GENGINE_ROOT_DIR}/perl)

# installation
gengine_export_var(GENGINE_INSTALL_DIR ${GENGINE_ROOT_DIR}/install)
gengine_export_var(GENGINE_INCLUDE_DIR ${GENGINE_INSTALL_DIR}/include)
gengine_export_var(GENGINE_LIB_DIR ${GENGINE_INSTALL_DIR}/lib)
gengine_export_var(GENGINE_BIN_DIR ${GENGINE_INSTALL_DIR}/bin)
