# targets.cmake

if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(GENGINE_ARCH x86_64)
elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 4)
    set(GENGINE_ARCH x86)
endif()
			
set(GENGINE_TARGETS_COMMON_DEPENDENCIES)
if(MSVC)
    gengine_export_var(GENGINE_LIB_NAME_PREFIX)
else()
    gengine_export_var(GENGINE_LIB_NAME_PREFIX lib)
endif()

if(EMSCRIPTEN)
    gengine_export_var(GENGINE_SHARED_LIB_NAME_SUFFIX .so)
elseif(APPLE)
    gengine_export_var(GENGINE_SHARED_LIB_NAME_SUFFIX .dylib)
elseif(UNIX)
    gengine_export_var(GENGINE_SHARED_LIB_NAME_SUFFIX .so)
elseif(WIN32)
    gengine_export_var(GENGINE_SHARED_LIB_NAME_SUFFIX .dll)
endif()

if(MSVC)
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

# A stricter version of cmake_parse_arguments that ensures that there are no unrecognized arguments and that all
# one-value keywords have values.
# Note: we deliberately allow multi-value keywords to have no values, because otherwise it'd be inconvenient to use.
function(gengine_parse_arguments PREFIX OPTIONS ONE_VAL_KEYWORDS MULTI_VAL_KEYWORDS)
    cmake_parse_arguments("${PREFIX}" "${OPTIONS}" "${ONE_VAL_KEYWORDS}" "${MULTI_VAL_KEYWORDS}" ${ARGN})

    if(${PREFIX}_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unrecognized arguments: ${${PREFIX}_UNPARSED_ARGUMENTS}")
    endif()

    if(${PREFIX}_KEYWORDS_MISSING_VALUES)
        set(ONE_VALUE_KEYWORDS_MISSING_VALUES ${${PREFIX}_KEYWORDS_MISSING_VALUES})
        list(REMOVE_ITEM ONE_VALUE_KEYWORDS_MISSING_VALUES ${MULTI_VAL_KEYWORDS})

        if(ONE_VALUE_KEYWORDS_MISSING_VALUES)
            message(FATAL_ERROR "Missing values for keywords: ${ONE_VALUE_KEYWORDS_MISSING_VALUES}")
        endif()
    endif()

    # Note: if co_parse_arguments were a macro, we wouldn't need this loop. But a macro would mess arguments in
    # some cases, e.g. when ARGN contains a newline literal '\n' (e.g. it contains a piece of python code),
    # macro argument substitution would turn in into an actual newline. So, it has to be a function.
    foreach(VAR ${OPTIONS} ${ONE_VAL_KEYWORDS} ${MULTI_VAL_KEYWORDS})
        if(DEFINED ${PREFIX}_${VAR})
            set(${PREFIX}_${VAR} ${${PREFIX}_${VAR}} PARENT_SCOPE)
        else()
            unset(${PREFIX}_${VAR} PARENT_SCOPE)
        endif()
    endforeach()
endfunction()

# Causes our CMake scripts to depend on the passed files - when one of them is changed, CMake is re-run.
function(prv_add_cmake_deps)
    foreach(FILE ${ARGN})
        # Note: the destination file is not important, the whole purpose of using configure_file is its side
        # effect that causes cmake to be re-run when the source file is changed.
        get_filename_component(FILE_NAME ${FILE} NAME)
        get_filename_component(DIR_NAME ${FILE} DIRECTORY)
        string(MD5 DIR_HASH ${DIR_NAME})

        configure_file(${FILE} ${GENGINE_TMP_DIR}/${FILE_NAME}_${DIR_HASH} @ONLY)
    endforeach()
endfunction()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES .*Clang.*)
    set(GENGINE_COMPILER_IS_CLANG TRUE)
endif()

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

macro(gengine_set_global_var)
    set(${ARGV0} ${ARGV1})
    if(${ARGV1})
        add_definitions(-D${ARGV0})
    else()
        remove_definitions(-D${ARGV0})
    endif()
endmacro()

MACRO (cmp_IDE_SOURCE_PROPERTIES SOURCE_PATH HEADERS SOURCES)
    STRING(REPLACE "/" "\\\\" source_group_path ${SOURCE_PATH}  )
    source_group(${source_group_path} FILES ${HEADERS} ${SOURCES})
ENDMACRO (cmp_IDE_SOURCE_PROPERTIES NAME HEADERS SOURCES INSTALL_FILES)

#some macroses to preserve list of all binary files, whiuch should be stored on symbol server

function (prv_gengine_add_includes)
    include_directories(
        .
        ${Boost_INCLUDE_DIRS}
        ${GENGINE_ROOT_DIR}
        ${GENGINE_BINARY_DIR}
        ${GTest_INCLUDE_DIRS}
    )
endfunction()

function (gengine_add_executable)
    prv_gengine_add_includes()

    add_executable(${ARGV})
    add_dependencies(${ARGV0} patcher ${GENGINE_THIRDPARTY_TARGET_NAME})
    add_dependencies(${GENGINE_TARGET_NAME} ${ARGV0})
    gengine_patch(${ARGV0})
    gengine_export(${ARGV0} ${GENGINE_BIN_DIR})
endfunction()

function(gengine_add_shared_library)
    prv_gengine_add_includes()

    set(ADD_SHARED_LIBRARY_COMMAND "${ARGV0}" "SHARED")
    foreach(argmnt RANGE 1 ${ARGC})
        set(ADD_SHARED_LIBRARY_COMMAND ${ADD_SHARED_LIBRARY_COMMAND} ${ARGV${argmnt}})
    endforeach()

    add_library(${ADD_SHARED_LIBRARY_COMMAND})
    add_dependencies(${ARGV0} patcher ${GENGINE_THIRDPARTY_TARGET_NAME})
    add_dependencies(${GENGINE_TARGET_NAME} ${ARGV0})
    gengine_export(${ARGV0} ${GENGINE_BIN_DIR})
endfunction(gengine_add_shared_library)

macro (gengine_add_library)
    prv_gengine_add_includes()

    gengine_export_var(GENGINE_LIBRARIES ${GENGINE_LIBRARIES} ${ARGV0})

    add_library(${ARGV})
    add_dependencies(${ARGV0} ${GENGINE_THIRDPARTY_TARGET_NAME})
    add_dependencies(${GENGINE_TARGET_NAME} ${ARGV0})
    gengine_export(${ARGV0} ${GENGINE_LIB_DIR})
endmacro()

function (gengine_add_shared_entry)
    prv_gengine_add_includes()

    set(ADD_SHARED_LIBRARY_COMMAND "${ARGV0}" "SHARED")
    foreach(argmnt RANGE 1 ${ARGC})
        set(ADD_SHARED_LIBRARY_COMMAND ${ADD_SHARED_LIBRARY_COMMAND} ${ARGV${argmnt}})
    endforeach()

    add_library(${ADD_SHARED_LIBRARY_COMMAND})
    add_dependencies(${ARGV0} patcher ${GENGINE_THIRDPARTY_TARGET_NAME})
    add_dependencies(${GENGINE_TARGET_NAME} ${ARGV0})
    gengine_patch(${ARGV0})
    gengine_export(${ARGV0} ${GENGINE_BIN_DIR})
endfunction(gengine_add_shared_entry)

function (gengine_add_plugin_entry)
    prv_gengine_add_includes()

    set(ADD_SHARED_LIBRARY_COMMAND "${ARGV0}" "SHARED")
    foreach(argmnt RANGE 1 ${ARGC})
        set(ADD_SHARED_LIBRARY_COMMAND ${ADD_SHARED_LIBRARY_COMMAND} ${ARGV${argmnt}})
    endforeach()

    add_library(${ADD_SHARED_LIBRARY_COMMAND})
    gengine_patch_plugin(${ARGV0})
    gengine_export(${ARGV0} ${GENGINE_BIN_DIR})
    add_dependencies("${ARGV0}" patcher ${GENGINE_THIRDPARTY_TARGET_NAME})
    add_dependencies(${GENGINE_TARGET_NAME} ${ARGV0})
endfunction(gengine_add_plugin_entry)

macro (gengine_dump_variables)
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
endmacro()

macro (gengine_service_build)
    add_custom_command(TARGET ${ARGV0}
                       POST_BUILD
                       COMMAND $<TARGET_FILE:${ARGV0}> --uninstall
                       COMMAND $<TARGET_FILE:${ARGV0}> --install
                       COMMAND $<TARGET_FILE:${ARGV0}> --start
                       COMMENT "Starting services...${ARGV0}"
                       WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
    add_custom_command(TARGET ${ARGV0}
                       PRE_BUILD
                       COMMAND if exist $<TARGET_FILE:${ARGV0}> $<TARGET_FILE:${ARGV0}> --stop
                       COMMENT "Stopping services...${ARGV0}")
endmacro()

macro (gengine_export)
    add_custom_command(TARGET ${ARGV0}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGV1}
        COMMAND ${CMAKE_COMMAND} -E copy \"$<TARGET_FILE:${ARGV0}>\" \"${ARGV1}\"
        COMMENT "Exporting $<TARGET_FILE:${ARGV0}> to ${ARGV1}/$<TARGET_FILE_NAME:${ARGV0}>"
    )
endmacro()

function (prv_gengine_patch TARGET_NAME ARGUMENT)
    if(WIN32)
        set(PATCHER_UTILITY ${GENGINE_BIN_DIR}/patcher.exe)
    else()
        set(PATCHER_UTILITY ${GENGINE_BIN_DIR}/patcher)
    endif()
    add_custom_command(TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${PATCHER_UTILITY} --entry=default ${ARGUMENT} --file=$<TARGET_FILE:${TARGET_NAME}> --config=${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}.json
        COMMENT "Patching $<TARGET_FILE:${TARGET_NAME}> with ${TARGET_NAME}.json"
    )
endfunction()

function (gengine_patch TARGET_NAME)
    prv_gengine_patch(${TARGET_NAME} "--executable")
endfunction()

function (gengine_patch_plugin TARGET_NAME)
    prv_gengine_patch(${TARGET_NAME} "--plugin")
endfunction()

function(gengine_export_includes)
    file(GLOB_RECURSE include_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/" "*.h" "*.hpp")
    foreach(include_file ${include_files})
        #message("Include ${include_file}")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${include_file}" "${GENGINE_INCLUDE_DIR}/${include_file}" COPYONLY)
    endforeach(include_file)
endfunction(gengine_export_includes)

function (gengine_import_binaries)
    file(GLOB shared_files RELATIVE "${GENGINE_BIN_DIR}/" "${GENGINE_BIN_DIR}/*.exe" "${GENGINE_BIN_DIR}/*.dll" "${GENGINE_BIN_DIR}/*.so")
    foreach(dll_file ${shared_files})
        add_custom_command(TARGET ${ARGV0}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy \"${GENGINE_BIN_DIR}/${dll_file}\" \"$<TARGET_FILE_DIR:${ARGV0}>\"
        COMMENT "Importing ${GENGINE_BIN_DIR}/${dll_file} to ${ARGV0} location...>"
       )
    endforeach(dll_file)
endfunction(gengine_import_binaries)

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
