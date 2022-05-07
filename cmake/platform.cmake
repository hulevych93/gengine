# platform.cmake

macro (gengine_set_runtime)
    set(RUNTIME_FLAG "/MD")
    if("${CMAKE_BUILD_TYPE}" STREQUAL DEBUG)
        set(RUNTIME_FLAG "/MDd")
    endif()

    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${RUNTIME_FLAG})
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${RUNTIME_FLAG})
    set(CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}} ${RUNTIME_FLAG})
    set(CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}} ${RUNTIME_FLAG})

    gengine_join_list(CMAKE_C_FLAGS " " CMAKE_C_FLAGS)
    gengine_join_list(CMAKE_CXX_FLAGS " " CMAKE_CXX_FLAGS)
    gengine_join_list(CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE} " " CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE})
    gengine_join_list(CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE} " " CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE})
endmacro()

if(WIN32)
    set(AdditionalOS_LIBRARIES
        Psapi.lib
        Version.lib
        Wtsapi32.lib
        wbemuuid.lib
        odbc32.lib
        Oleacc.lib
        ws2_32.lib)

    gengine_set_runtime()

    set(EXECUTABLE_TYPE WIN32)

    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)
    add_definitions(-DEXPORT_SYMBOLS)
    add_definitions(-D_WIN32_WINNT=0x0601)

elseif(UNIX)
    find_package(X11 REQUIRED)

    set(AdditionalOS_LIBRARIES
        pthread
        ${X11_LIBRARIES}
    )

    if(APPLE)
        set(AdditionalOS_LIBRARIES ${AdditionalOS_LIBRARIES} iconv)
    else()
        set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-fPIC")
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fPIC")
        gengine_join_list(CMAKE_C_FLAGS " " CMAKE_C_FLAGS)
        gengine_join_list(CMAKE_CXX_FLAGS " " CMAKE_CXX_FLAGS)

        set(AdditionalOS_LIBRARIES ${AdditionalOS_LIBRARIES} ${CMAKE_DL_LIBS} icuuc)
    endif()
endif()
