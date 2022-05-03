cmake_minimum_required (VERSION 3.0)
project (gengine)

include(${PROJECT_SOURCE_DIR}/../cmake/main.cmake)

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

