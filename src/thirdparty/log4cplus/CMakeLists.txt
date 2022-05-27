gengine_project(log4cplus)

set(GENGINE_LOG4CPLUS_VERSION            1.2.1)
set(GENGINE_LOG4CPLUS_UNIQUE_DIR_NAME    log4cplus-${GENGINE_LOG4CPLUS_VERSION}-${GENGINE_3RD_PARTY_LIB_DIR_SUFFIX})
set(GENGINE_LOG4CPLUS_PREFIX             ${GENGINE_3RD_PARTY_INSTALL_DIR}/${GENGINE_LOG4CPLUS_UNIQUE_DIR_NAME})
set(GENGINE_LOG4CPLUS_BUILD_DIR          ${GENGINE_3RD_PARTY_BUILD_DIR}/${GENGINE_LOG4CPLUS_UNIQUE_DIR_NAME})
set(Log4cplus_INCLUDE_DIRS               ${GENGINE_LOG4CPLUS_PREFIX}/include)
set(Log4cplus_LIB_DIR                    ${GENGINE_LOG4CPLUS_PREFIX}/lib)
set(Log4cplus_COMPONENTS                 log4cplus)
set(Log4cplus_LIBRARIES)

if(WIN32)
    set(Log4cplusWin "log4cplus" "U" ${GENGINE_LIB_NAME_DEBUG_SUFFIX})
    gengine_join_list(Log4cplusWin "" Log4cplusWin)

    gengine_list_replace(Log4cplus_COMPONENTS "log4cplus" ${Log4cplusWin})
endif()

foreach(TMP_LOG4CPLUS_LIB_BASE IN LISTS Log4cplus_COMPONENTS)
    string(TOUPPER "${TMP_LOG4CPLUS_LIB_BASE}" TMP_LOG4CPLUS_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_name("Log4cplus"
                               ${TMP_LOG4CPLUS_LIB_BASE}
                               GENGINE_LOG4CPLUS_${TMP_LOG4CPLUS_LIB_BASE_UPPER}_LIB)

    list(APPEND Log4cplus_LIBRARIES "${GENGINE_LOG4CPLUS_${TMP_LOG4CPLUS_LIB_BASE_UPPER}_LIB}")
endforeach()

gengine_export_var(Log4cplus_INCLUDE_DIRS ${Log4cplus_INCLUDE_DIRS})
gengine_export_var(Log4cplus_LIBRARIES ${Log4cplus_LIBRARIES})

set(TMP_DIR         ${GENGINE_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GENGINE_LOG4CPLUS_BUILD_DIR})

gengine_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${GENGINE_LOG4CPLUS_PREFIX}
)

ExternalProject_Add(
    log4cplus
    PREFIX                  ${GENGINE_LOG4PLUS_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${GENGINE_LOG4CPLUS_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${GENGINE_3RD_PARTY_DOWNLOAD_DIR}
    URL                     https://github.com/log4cplus/log4cplus/releases/download/REL_1_2_1/log4cplus-1.2.1.tar.gz
    URL_MD5                 "c017205ce1107619fc78f3eed846c8f8"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${LOG4CPLUS_ARTIFACTS}
    # Install
    INSTALL_COMMAND         ${GENGINE_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${GENGINE_THIRDPARTY_TARGET_NAME} log4cplus)
