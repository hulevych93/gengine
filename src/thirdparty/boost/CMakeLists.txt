gengine_project(boost)

set(GENGINE_BOOST_VERSION 1.70.0)
set(GENGINE_BOOST_ROOT_DIR boost-${GENGINE_BOOST_VERSION}-${GENGINE_3RD_PARTY_LIB_DIR_SUFFIX})
set(GENGINE_BOOST_BUILD_DIR ${GENGINE_3RD_PARTY_BUILD_DIR}/${GENGINE_BOOST_ROOT_DIR})
set(GENGINE_BOOST_INSTALL_PREFIX ${GENGINE_3RD_PARTY_INSTALL_DIR}/${GENGINE_BOOST_ROOT_DIR})
set(Boost_INCLUDE_DIRS ${GENGINE_BOOST_INSTALL_PREFIX}/include)
set(Boost_LIB_DIR ${GENGINE_BOOST_INSTALL_PREFIX}/lib)
set(Boost_COMPONENTS thread timer date_time filesystem locale regex system chrono program_options)
set(Boost_LIBRARIES)

unset(Boost_Lib_suffix CACHE)

macro(prv_add_3rd_party_boost_lib_dir_suffix SUFFIX)
    if(Boost_Lib_suffix)
        set(Boost_Lib_suffix "${Boost_Lib_suffix}-${SUFFIX}")
    else()
        set(Boost_Lib_suffix "${SUFFIX}")
    endif()
endmacro()

if(WIN32)
    set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} "boost-1_70")
    gengine_join_list(Boost_INCLUDE_DIRS "/" Boost_INCLUDE_DIRS)

    prv_add_3rd_party_boost_lib_dir_suffix("-vc141")
    prv_add_3rd_party_boost_lib_dir_suffix("mt")
    if("${CMAKE_BUILD_TYPE}" STREQUAL DEBUG)
        prv_add_3rd_party_boost_lib_dir_suffix("gd")
    endif()
    if("${GENGINE_ARCH}" STREQUAL x86 OR "${GENGINE_ARCH}" STREQUAL arm)
        prv_add_3rd_party_boost_lib_dir_suffix("x32")
    else()
        prv_add_3rd_party_boost_lib_dir_suffix("x64")
    endif()
    prv_add_3rd_party_boost_lib_dir_suffix("1_70")
endif()

foreach(TMP_BOOST_LIB_BASE IN LISTS Boost_COMPONENTS)
    string(TOUPPER "${TMP_BOOST_LIB_BASE}" TMP_BOOST_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_name("Boost"
                               "boost_${TMP_BOOST_LIB_BASE}"
                               GENGINE_BOOST_${TMP_BOOST_LIB_BASE_UPPER}_LIB
                               "lib"
                               ${Boost_Lib_suffix})

    list(APPEND Boost_LIBRARIES "${GENGINE_BOOST_${TMP_BOOST_LIB_BASE_UPPER}_LIB}")
endforeach()

#export libs
gengine_export_var(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
gengine_export_var(Boost_LIBRARIES ${Boost_LIBRARIES})

set(TMP_DIR         ${GENGINE_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GENGINE_BOOST_BUILD_DIR})

set(BUILD_TOOLSET_VERSION ${CMAKE_CXX_COMPILER_VERSION})

if(CMAKE_COMPILER_IS_GNUCC)
    set(BUILD_TOOLSET gcc)
elseif(MSVC)
    set(BUILD_TOOLSET msvc)
    set(BUILD_TOOLSET_VERSION 14.1)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES .*Clang.*)
    set(BUILD_TOOLSET clang)
else()
    message(FATAL_ERROR "Unknown toolset for system: ${CMAKE_SYSTEM_NAME}")
endif()

gengine_map_list(Boost_COMPONENTS "(.+)" "--with-\\1" LIB_OPTS)

if(GENGINE_CUR_3RD_PARTY_BUILD_AS_SHARED)
    set(LINK_TYPE shared)
else()
    set(LINK_TYPE static)
endif()

set(BUILD_VARIANT release)
if("${CMAKE_BUILD_TYPE}" STREQUAL DEBUG)
    set(BUILD_VARIANT debug)
endif()

set(RUNTIME_LINK static)
if(WIN32)
    set(RUNTIME_LINK shared)
endif()

set(ADDRESS_MODEL 64)
if("${GENGINE_ARCH}" STREQUAL x86 OR "${GENGINE_ARCH}" STREQUAL arm)
    set(ADDRESS_MODEL 32)
endif()

set(BOOTSTRAP_COMMAND ${SRC_DIR}/bootstrap.sh)
if(CMAKE_HOST_WIN32)
    if(MSVC)
        set(BOOTSTRAP_COMMAND ${SRC_DIR}/bootstrap.bat vc141)
    else()
        set(BOOTSTRAP_COMMAND ${SRC_DIR}/bootstrap.bat gcc)
    endif()
endif()

# Turned off boost EVENT_LOG for Windows, because we don't use it.
if(WIN32)
    set(NO_EVENT_LOG -sBOOST_LOG_WITHOUT_EVENT_LOG)
endif()

set(COMPILER_FLAGS)
if(UNIX AND NOT APPLE)
    set(COMPILER_FLAGS cflags=${CMAKE_C_FLAGS} cxxflags=${CMAKE_CXX_FLAGS})
endif()

set(TARGET_OS)
if(WIN32)
    set(TARGET_OS windows)
elseif(APPLE)
    set(TARGET_OS darwin)
elseif(UNIX)
    set(TARGET_OS linux)
else()
    message(FATAL_ERROR "Unknown target OS")
endif()

set(BJAM_COMMAND "${SRC_DIR}/b2${GENGINE_HOST_EXECUTABLE_SUFFIX}")

set(BUILD_COMMAND_BASE
    ${BJAM_COMMAND}
        --build-type=minimal
        -sNO_BZIP2=1 # needed when building iostreams
        -sNO_LZMA=1 # needed when building iostreams
        --hash
        ${NO_EVENT_LOG}
        toolset=${BUILD_TOOLSET}-${BUILD_TOOLSET_VERSION}
        variant=${BUILD_VARIANT}
        threading=multi
        link=${LINK_TYPE}
        address-model=${ADDRESS_MODEL}
        ${COMPILER_FLAGS}
)

ExternalProject_Add(
    boost
    PREFIX              ${GENGINE_BOOST_INSTALL_PREFIX}
    TMP_DIR             ${TMP_DIR}
    STAMP_DIR           ${GENGINE_BOOST_INSTALL_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR        ${GENGINE_3RD_PARTY_DOWNLOAD_DIR}
    URL                 https://sourceforge.net/projects/boost/files/boost/1.70.0/boost_1_70_0.tar.bz2/download
    URL_MD5            "242ecc63507711d6706b9b0c0d0c7d4f"
    # Configure
    SOURCE_DIR          ${SRC_DIR}
    CONFIGURE_COMMAND   ${BOOTSTRAP_COMMAND}
    # Build
    BUILD_COMMAND       ${BUILD_COMMAND_BASE} --prefix=${GENGINE_BOOST_INSTALL_PREFIX} -j8 -d2 ${LIB_OPTS} install
        # Install
    INSTALL_COMMAND     ""
    BUILD_IN_SOURCE     1

    # Logging
    LOG_CONFIGURE       1
    LOG_BUILD           1
    LOG_INSTALL         1
    LOG_TEST            1
)

add_dependencies(${GENGINE_THIRDPARTY_TARGET_NAME} boost)
