# install.cmake

gengine_export_var(GENGINE_LIB_VERSION "1.0.0")

if(NOT DEFINED GENGINE_BUNDLED_MODE)
    message(FATAL_ERROR "Please, define GENGINE_BUNDLED_MODE cmake variable.")
endif()

if(NOT GENGINE_BUNDLED_MODE)
  include(GNUInstallDirs)

  file(GLOB_RECURSE INCLUDE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/" "*.h" "*.hpp")
  foreach(FILE ${INCLUDE_FILES})
      get_filename_component(SUB_DIR ${FILE} DIRECTORY)
      install(FILES ${FILE} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${SUB_DIR}")
  endforeach()
endif()
