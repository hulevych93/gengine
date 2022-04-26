# thirdparties.cmake

# The `prv_def_3rd_party_lib_name` contructs a full static lib name
# with respect to platform-dependent suffix/prefix.
function(prv_def_3rd_party_lib_name COMPONENT_NAME BASE_NAME FULL_LIB_NAME_OUT)
    if(BUILD_AS_SHARED)
        set(SUFFIX "${GENGINE_SHARED_LIB_NAME_SUFFIX}")
    else()
        set(SUFFIX "${GENGINE_STATIC_LIB_NAME_SUFFIX}")
    endif()

    if(DEFINED ARGN_FORCE_PREFIX)
        if("${ARGN_FORCE_PREFIX}" STREQUAL "EMPTY")
            set(PREFIX "")
        else()
            set(PREFIX "${ARGN_FORCE_PREFIX}")
        endif()
    else()
        set(PREFIX "${GENGINE_LIB_NAME_PREFIX}")
    endif()

    set(FULL_LIB_NAME "${${COMPONENT_NAME}_LIB_DIR}/${PREFIX}${BASE_NAME}${SUFFIX}")
    set(${FULL_LIB_NAME_OUT} ${FULL_LIB_NAME} PARENT_SCOPE)
endfunction()

# Set 3rd-parties directories
gengine_export_var(GENGINE_3RD_PARTY_DOWNLOAD_DIR "${BUILD_FILES_DIR}/Download")
gengine_export_var(GENGINE_3RD_PARTY_BUILD_DIR "${BUILD_FILES_DIR}/Build")
gengine_export_var(GENGINE_3RD_PARTY_INSTALL_DIR "${BUILD_FILES_DIR}/Install")
gengine_export_var(GENGINE_3RD_PARTY_TMP_DIR "${BUILD_FILES_DIR}/Temp")

set(GENGINE_COMPONENTS_BIN_DIR ${GENGINE_BINARY_DIR}/subdirs)
set(GENGINE_GEN_DIR ${GENGINE_BINARY_DIR}/generated)
set(GENGINE_TMP_DIR ${GENGINE_BINARY_DIR}/tmp)

set(GENGINE_3RD_PARTY_CMAKE_INSTALL_COMMAND "${CMAKE_COMMAND}" --build . --config "${CMAKE_BUILD_TYPE}" --target install)

# Setting up Boost ...
set(GENGINE_BOOST_VERSION 1.70.0)
set(GENGINE_BOOST_ROOT_DIR "boost-${GENGINE_BOOST_VERSION}")
set(GENGINE_BOOST_BUILD_DIR ${GENGINE_3RD_PARTY_BUILD_DIR}/${GENGINE_BOOST_ROOT_DIR})
set(GENGINE_BOOST_INSTALL_PREFIX ${GENGINE_3RD_PARTY_INSTALL_DIR}/${GENGINE_BOOST_ROOT_DIR})
set(Boost_INCLUDE_DIRS ${GENGINE_BOOST_INSTALL_PREFIX}/include)
set(Boost_LIB_DIR ${GENGINE_BOOST_INSTALL_PREFIX}/lib)
set(Boost_COMPONENTS thread timer date_time filesystem locale regex system chrono program_options)
set(Boost_LIBRARIES)

foreach(TMP_BOOST_LIB_BASE IN LISTS Boost_COMPONENTS)
    string(TOUPPER "${TMP_BOOST_LIB_BASE}" TMP_BOOST_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_name("Boost"
                               "boost_${TMP_BOOST_LIB_BASE}"
                               GENGINE_BOOST_${TMP_BOOST_LIB_BASE_UPPER}_LIB)

    list(APPEND Boost_LIBRARIES "${GENGINE_BOOST_${TMP_BOOST_LIB_BASE_UPPER}_LIB}")
endforeach()
