# platform.cmake

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

    ADD_DEFINITIONS(-DUNICODE)
    ADD_DEFINITIONS(-D_UNICODE)
elseif(UNIX)
    FIND_PACKAGE(X11 REQUIRED)

    set(AdditionalOS_LIBRARIES
        iconv
        pthread
        ${X11_LIBRARIES}
    )
endif()

add_definitions(-DEXPORT_SYMBOLS)