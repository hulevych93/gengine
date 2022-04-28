cmake_minimum_required (VERSION 3.0)
project (gengine)

set(GENGINE_CMAKE_DIR ${PROJECT_SOURCE_DIR}/../cmake)
include(${GENGINE_CMAKE_DIR}/common.cmake)

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# examples
gengine_export_var(GENGINE_EXAMPLES_DIR ${PROJECT_SOURCE_DIR}/../examples)

include(${GENGINE_CMAKE_DIR}/main.cmake)
include(ExternalProject)

if(UNIX)
    FIND_PACKAGE(X11 REQUIRED)

    set(AdditionalOS_LIBRARIES
        iconv
        pthread
        ${X11_LIBRARIES}
    )
endif()

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(thirdparty/boost)
add_subdirectory(thirdparty/gtest)
add_subdirectory(thirdparty/log4cplus)

include_directories(".")

# libs
add_subdirectory(api)
add_subdirectory(appconfig)
add_subdirectory(brokers)
add_subdirectory(core)
add_subdirectory(filesystem)
add_subdirectory(multithreading)
add_subdirectory(diagnostic)
add_subdirectory(interprocess-syncronization)
add_subdirectory(interprocess-communication)
add_subdirectory(entries)
add_subdirectory(json)
add_subdirectory(serialization)
add_subdirectory(services)

#binaries
add_subdirectory(service-broker)
add_subdirectory(executor-broker)
add_subdirectory(worker-broker)
add_subdirectory(process-broker)
add_subdirectory(patcher)
add_subdirectory(logger-proxy)
add_subdirectory(plugin-broker)
add_subdirectory(persistency-broker)
add_subdirectory(log4cplus-logger)

if(wxWidgets_FOUND)
    add_subdirectory(wx-logger)
endif()

add_subdirectory(console-logger)

gengine_export_includes()
gengine_export_var(GENGINE_LIBRARIES ${GENGINE_LIBRARIES} ${AdditionalOS_LIBRARIES})

