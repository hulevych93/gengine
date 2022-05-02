# thirdparties.cmake

include(ExternalProject)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(GENGINE_ARCH x86_64)
elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 4)
    set(GENGINE_ARCH x86)
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES .*Clang.*)
    set(GENGINE_COMPILER_IS_CLANG TRUE)
endif()

if(WIN32)
    gengine_export_var(GENGINE_LIB_NAME_PREFIX)
else()
    gengine_export_var(GENGINE_LIB_NAME_PREFIX lib)
endif()

if(APPLE)
    gengine_export_var(GENGINE_SHARED_LIB_NAME_SUFFIX .dylib)
elseif(UNIX)
    gengine_export_var(GENGINE_SHARED_LIB_NAME_SUFFIX .so)
elseif(WIN32)
    gengine_export_var(GENGINE_SHARED_LIB_NAME_SUFFIX .dll)
endif()

if(WIN32)
    gengine_export_var(GENGINE_STATIC_LIB_NAME_SUFFIX .lib)
else()
    gengine_export_var(GENGINE_STATIC_LIB_NAME_SUFFIX .a)
endif()

if(BUILD_SHARED_LIBS)
    gengine_export_var(GENGINE_LIB_NAME_SUFFIX ${GENGINE_SHARED_LIB_NAME_SUFFIX})
else()
    gengine_export_var(GENGINE_LIB_NAME_SUFFIX ${GENGINE_STATIC_LIB_NAME_SUFFIX})
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL DEBUG)
    gengine_export_var(GENGINE_LIB_NAME_DEBUG_SUFFIX d)
    if(MSVC)
        gengine_export_var(GENGINE_LIB_NAME_DEBUG_SUFFIX_MSVC ${GENGINE_LIB_NAME_DEBUG_SUFFIX})
    endif()
    gengine_export_var(GENGINE_LIB_NAME_DEBUG_SUFFIX_WITH_MINUS -d)
endif()

unset(GENGINE_3RD_PARTY_LIB_DIR_SUFFIX CACHE)

macro(prv_add_3rd_party_lib_dir_suffix SUFFIX)
    if(GENGINE_3RD_PARTY_LIB_DIR_SUFFIX)
        set(GENGINE_3RD_PARTY_LIB_DIR_SUFFIX "${GENGINE_3RD_PARTY_LIB_DIR_SUFFIX}-${SUFFIX}")
    else()
        set(GENGINE_3RD_PARTY_LIB_DIR_SUFFIX "${SUFFIX}")
    endif()
endmacro()

if(BUILD_SHARED_LIBS)
    set(TMP_LINK_TYPE "shared")
else()
    set(TMP_LINK_TYPE "static")
endif()

string(REPLACE " " "" TMP_GENERATOR_DESC "${CMAKE_GENERATOR}")

prv_add_3rd_party_lib_dir_suffix("${CMAKE_BUILD_TYPE}")
prv_add_3rd_party_lib_dir_suffix("${GENGINE_ARCH}")
prv_add_3rd_party_lib_dir_suffix("${TMP_LINK_TYPE}")
prv_add_3rd_party_lib_dir_suffix("${CMAKE_CXX_COMPILER_ID}")
prv_add_3rd_party_lib_dir_suffix("${CMAKE_CXX_COMPILER_VERSION}")
prv_add_3rd_party_lib_dir_suffix("${TMP_GENERATOR_DESC}")

# The `prv_def_3rd_party_lib_name` contructs a full static lib name
# with respect to platform-dependent suffix/prefix.
function(prv_def_3rd_party_lib_name COMPONENT_NAME BASE_NAME FULL_LIB_NAME_OUT)
    if(DEFINED ARGV4)
        set(SUFFIX "${ARGV4}")
    endif()

    if(BUILD_AS_SHARED)
        set(SUFFIX "${SUFFIX}${GENGINE_SHARED_LIB_NAME_SUFFIX}")
    else()
        set(SUFFIX "${SUFFIX}${GENGINE_STATIC_LIB_NAME_SUFFIX}")
    endif()

    if(DEFINED ARGV3)
        set(PREFIX "${ARGV3}")
    else()
        set(PREFIX "${GENGINE_LIB_NAME_PREFIX}")
    endif()

    set(FULL_LIB_NAME "${${COMPONENT_NAME}_LIB_DIR}/${PREFIX}${BASE_NAME}${SUFFIX}")
    set(${FULL_LIB_NAME_OUT} ${FULL_LIB_NAME} PARENT_SCOPE)
endfunction()

# VAR is a variable name for cmake flags which are to be passed to 3rd party build system.
# Concerning ARGN see documentation for prv_append_3rd_party_specific_compiler_flags.
function(gengine_3rd_party_common_cmake_options VAR)
    set(RESULT)
    list(APPEND RESULT "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")
    list(APPEND RESULT "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
    list(APPEND RESULT "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
    list(APPEND RESULT "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    list(APPEND RESULT "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")

    # Same as we do for our code, we must append the options to the configuration-specific variables to ensure that
    # they override the defaults and not vice versa.
    macro(prv_add_config_specific_var VAR_BASE_NAME VALUE)
        # For this to work properly in multi-generator builds the options should be appended to all of the vars
        # (_DEBUG, _RELEASE etc) simultaneously, but this may produce a command line that is too long.
        # TODO: perhaps we should do it only on iOS (which is the only platform where multi-generator builds are used).
        list(APPEND RESULT "-D${VAR_BASE_NAME}_${CMAKE_BUILD_TYPE}=${VALUE}")
    endmacro()

    prv_add_config_specific_var(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    prv_add_config_specific_var(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    prv_add_config_specific_var(CMAKE_SHARED_LINKER_FLAGS "${LD_FLAGS}")
    prv_add_config_specific_var(CMAKE_MODULE_LINKER_FLAGS "${LD_FLAGS}")
    prv_add_config_specific_var(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS}")

    # It's important to have toolset specific flags in CMAKE_<LANG>_FLAGS as cmake uses only these variables
    # when it checks for a working compiler at its very beginning.
    gengine_join_list(GENGINE_TOOLSET_ARCH_FLAGS " " TOOLSET_FLAGS_STR)
    list(APPEND RESULT "-DCMAKE_C_FLAGS=${TOOLSET_FLAGS_STR}")
    list(APPEND RESULT "-DCMAKE_CXX_FLAGS=${TOOLSET_FLAGS_STR}")

    list(APPEND RESULT "-DCMAKE_AR=${CMAKE_AR}")
    list(APPEND RESULT "-DCMAKE_RANLIB=${CMAKE_RANLIB}")
    if(NOT "${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        list(APPEND RESULT "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()
    list(APPEND RESULT "-DBOOST_ROOT=${GENGINE_BOOST_INSTALL_PREFIX}")
    list(APPEND RESULT "-DBoost_NO_BOOST_CMAKE=TRUE")
    list(APPEND RESULT "-DBoost_NO_SYSTEM_PATHS=TRUE")
    list(APPEND RESULT "-DBOOST_INCLUDEDIR=${GENGINE_BOOST_INSTALL_PREFIX}/include")
    list(APPEND RESULT "-DBOOST_LIBRARYDIR=${GENGINE_BOOST_INSTALL_PREFIX}/lib")

    if(APPLE)
        # @loader_path/../lib path instead of just @loader_path below workarounds a problem in 3rd
        # party packages when they provide some executables which has to be run. Since variable
        # CMAKE_INSTALL_RPATH is used both for libraries and for executables @loader_path doesn't
        # work for executables when searching for own libraries in lib directory while
        # @loader_path/../lib does work for both.
        list(APPEND RESULT "-DCMAKE_INSTALL_RPATH=@loader_path/../lib")
        list(APPEND RESULT "-DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE")

        list(APPEND RESULT "-DCMAKE_INSTALL_NAME_DIR=@rpath")
        list(APPEND RESULT -DCMAKE_BUILD_WITH_INSTALL_NAME_DIR=TRUE)
    elseif(UNIX AND NOT EMSCRIPTEN)
        # Same logic as above for linux.
        list(APPEND RESULT "-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib")
        list(APPEND RESULT -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE)
    endif()

    # The following options are used by the toolchain files.
    list(APPEND RESULT "-DGENGINE_ARCH=${GENGINE_ARCH}")

    set(${VAR} ${RESULT} PARENT_SCOPE)
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

# Settings up gTest
set(GENGINE_GTEST_VERSION                1.10.0)
set(GENGINE_GTEST_UNIQUE_DIR_NAME        gtest-${GENGINE_GTEST_VERSION}-${GENGINE_3RD_PARTY_LIB_DIR_SUFFIX})
set(GENGINE_GTEST_PREFIX                 ${GENGINE_3RD_PARTY_INSTALL_DIR}/${GENGINE_GTEST_UNIQUE_DIR_NAME})
set(GENGINE_GTEST_BUILD_DIR              ${GENGINE_3RD_PARTY_BUILD_DIR}/${GENGINE_GTEST_UNIQUE_DIR_NAME})
set(GTest_INCLUDE_DIRS                   ${GENGINE_GTEST_PREFIX}/include)
set(GTest_LIB_DIR                        ${GENGINE_GTEST_PREFIX}/lib)
set(GTest_COMPONENTS                     gtest${GENGINE_LIB_NAME_DEBUG_SUFFIX} gmock${GENGINE_LIB_NAME_DEBUG_SUFFIX})
set(GTest_LIBRARIES)

foreach(TMP_GTEST_LIB_BASE IN LISTS GTest_COMPONENTS)
    string(TOUPPER "${TMP_GTEST_LIB_BASE}" TMP_GTEST_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_name("GTest"
                               ${TMP_GTEST_LIB_BASE}
                               GENGINE_GTEST_${TMP_GTEST_LIB_BASE_UPPER}_LIB)

    list(APPEND GTest_LIBRARIES "${GENGINE_GTEST_${TMP_GTEST_LIB_BASE_UPPER}_LIB}")
endforeach()

gengine_export_var(GTest_INCLUDE_DIRS ${GTest_INCLUDE_DIRS})
gengine_export_var(GTest_LIBRARIES ${GTest_LIBRARIES})

# Settings up Log4cplus
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


