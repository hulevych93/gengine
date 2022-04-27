project(log4cplus)

set(TMP_DIR         ${GENGINE_3RD_PARTY_BUILD_DIR})
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
