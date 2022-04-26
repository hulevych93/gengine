project(boost)

set(TMP_DIR         ${GENGINE_3RD_PARTY_BUILD_DIR})
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

set(USER_CONFIG_JAM ${GENGINE_TMP_DIR}/boost-user-config.jam)
file(WRITE ${USER_CONFIG_JAM} "using ${BUILD_TOOLSET} : ${BUILD_TOOLSET_VERSION}\n")

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

set(THREADING multi)
if(EMSCRIPTEN)
    set(THREADING single)
endif()

set(ZLIB_SOURCE_FLAGS)
if(GENGINE_BUILD_CUSTOM_ZLIB)
    set(ZLIB_SOURCE_FLAGS
        -sZLIB_INCLUDE=${GENGINE_ZLIB_PREFIX}/include
        -sZLIB_LIBPATH=${GENGINE_ZLIB_LIB_DIR}
        -sZLIB_BINARY=${GENGINE_ZLIB_LIB_NAME})
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
        --user-config=${USER_CONFIG_JAM}
        --build-type=minimal
        --layout=system
        -sNO_BZIP2=1 # needed when building iostreams
        -sNO_LZMA=1 # needed when building iostreams
        --hash
        ${ZLIB_SOURCE_FLAGS}
        ${NO_EVENT_LOG}
        toolset=${BUILD_TOOLSET}-${BUILD_TOOLSET_VERSION}
        variant=${BUILD_VARIANT}
        threading=${THREADING}
        link=${LINK_TYPE}
        address-model=${ADDRESS_MODEL}
        target-os=${TARGET_OS}
        boost.locale.icu=off
)

add_custom_target(boost_tools)
set(BCP_COMMAND "${SRC_DIR}/dist/bin/bcp${GENGINE_HOST_EXECUTABLE_SUFFIX}")
set(BCP_NATIVE_BUILD_COMMAND COMMAND ${BUILD_COMMAND_BASE} tools/bcp)

ExternalProject_Add(
    boost
    DEPENDS             boost_tools
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
                        ${BCP_NATIVE_BUILD_COMMAND}
    BUILD_IN_SOURCE     1
    BUILD_BYPRODUCTS    ${Boost_LIBRARIES}

    # Logging
    LOG_CONFIGURE       1
    LOG_BUILD           1
    LOG_INSTALL         1
    LOG_TEST            1
)
