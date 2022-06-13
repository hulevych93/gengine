# targets.cmake

set(GENGINE_TARGETS_COMMON_DEPENDENCIES)

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

macro(gengine_set_global_var)
    set(${ARGV0} ${ARGV1})
    if(${ARGV1})
        add_definitions(-D${ARGV0})
    else()
        remove_definitions(-D${ARGV0})
    endif()
endmacro()

function (prv_gengine_add_includes)
    include_directories(
        .
        ${Boost_INCLUDE_DIRS}
        ${X11_INCLUDE_DIR}
        ${GENGINE_ROOT_DIR}
        ${GENGINE_BINARY_DIR}
        ${GENGINE_INCLUDE_DIR}
        ${GTest_INCLUDE_DIRS}
    )
endfunction()

function (gengine_add_test)
    prv_gengine_add_includes()

    add_executable(${ARGV})
    add_dependencies(${ARGV0} patcher ${GENGINE_TARGET_NAME})
    gengine_patch(${ARGV0})

    add_test(NAME run-${ARGV0}
             COMMAND ${ARGV0}
             WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}/../bin)
endfunction()

function (gengine_add_executable)
    prv_gengine_add_includes()

    add_executable(${ARGV})
    add_dependencies(${ARGV0} patcher ${GENGINE_THIRDPARTY_TARGET_NAME})
    add_dependencies(${GENGINE_TARGET_NAME} ${ARGV0})
    gengine_patch(${ARGV0})
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
endfunction(gengine_add_shared_library)

macro (gengine_add_library)
    prv_gengine_add_includes()

    gengine_export_var(GENGINE_LIBRARIES ${GENGINE_LIBRARIES} ${ARGV0})

    add_library(${ARGV})
    add_dependencies(${ARGV0} ${GENGINE_THIRDPARTY_TARGET_NAME})
    add_dependencies(${GENGINE_TARGET_NAME} ${ARGV0})
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
endfunction(gengine_add_shared_entry)

function (gengine_add_plugin_entry)
    prv_gengine_add_includes()

    set(ADD_SHARED_LIBRARY_COMMAND "${ARGV0}" "SHARED")
    foreach(argmnt RANGE 1 ${ARGC})
        set(ADD_SHARED_LIBRARY_COMMAND ${ADD_SHARED_LIBRARY_COMMAND} ${ARGV${argmnt}})
    endforeach()

    add_library(${ADD_SHARED_LIBRARY_COMMAND})
    gengine_patch_plugin(${ARGV0})
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

function (prv_gengine_patch TARGET_NAME ARGUMENT)
    if(WIN32)
        set(PATCHER_UTILITY ${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/patcher.exe)
    else()
        set(PATCHER_UTILITY ${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/patcher)
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

function (gengine_import_binaries)
    file(GLOB shared_files RELATIVE "${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/" "${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/*.exe" "${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/*.dll" "${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/*.so")
    foreach(dll_file ${shared_files})
        add_custom_command(TARGET ${ARGV0}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy \"${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/${dll_file}\" \"$<TARGET_FILE_DIR:${ARGV0}>\"
        COMMENT "Importing ${GENGINE_RUNTIME_OUTPUT_DIRECTORY}/${dll_file} to ${ARGV0} location...>"
       )
    endforeach(dll_file)
endfunction(gengine_import_binaries)
