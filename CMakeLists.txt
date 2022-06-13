cmake_minimum_required (VERSION 3.0)
project (gengine)

set(GENGINE_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(GENGINE_BINARY_DIR ${PROJECT_BINARY_DIR}/src)

# If Gengine is being bundled in another project we don't want to install anything.
if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(GENGINE_BUNDLED_MODE OFF)
  else()
    set(GENGINE_BUNDLED_MODE ON)
  endif()

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)

gengine_export_var(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
gengine_export_var(Boost_LIBRARIES ${Boost_LIBRARIES})

gengine_export_var(GTest_INCLUDE_DIRS ${GTest_INCLUDE_DIRS})
gengine_export_var(GTest_LIBRARIES ${GTest_LIBRARIES})

gengine_export_var(Log4cplus_INCLUDE_DIRS ${Log4cplus_INCLUDE_DIRS})
gengine_export_var(Log4cplus_LIBRARIES ${Log4cplus_LIBRARIES})
