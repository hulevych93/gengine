# common.cmake file contains common utilitary cmake functions and macroses.

# The `gengine_export_var` declares a variable in the project scope
# and also in the parent project scope if one exists.
macro(gengine_export_var VARIABLE_NAME)
    set(${VARIABLE_NAME} ${ARGN})
    if(NOT PROJECT_IS_TOP_LEVEL)
        set(${VARIABLE_NAME} ${${VARIABLE_NAME}} PARENT_SCOPE)
    endif()
endmacro()

# Map list of string to list of the same length modified according to regex.
function(gengine_map_list gengine_map_list_LIST gengine_map_list_MATCH_RE gengine_map_list_REPLACE_EXPR gengine_map_list_LIST_OUT_VAR)
    set(gengine_map_list_NEW_LIST)
    foreach(gengine_map_list_ITEM IN LISTS "${gengine_map_list_LIST}")
        string(REGEX REPLACE "${gengine_map_list_MATCH_RE}" "${gengine_map_list_REPLACE_EXPR}" gengine_map_list_NEW_ITEM "${gengine_map_list_ITEM}")
        list(APPEND gengine_map_list_NEW_LIST "${gengine_map_list_NEW_ITEM}")
    endforeach()

    set("${gengine_map_list_LIST_OUT_VAR}" "${gengine_map_list_NEW_LIST}" PARENT_SCOPE)
endfunction()

# Transform a list into other list with respect to
# custom seperator value.
function(gengine_join_list LIST SEPARATOR OUTPUT)
    set(join_list_RESULT)
    set(IS_FIRST TRUE)
    foreach(VALUE ${${LIST}})
        if(IS_FIRST)
            set(IS_FIRST FALSE)
            set(join_list_RESULT "${join_list_RESULT}${VALUE}")
        else()
            set(join_list_RESULT "${join_list_RESULT}${SEPARATOR}${VALUE}")
        endif()
    endforeach()

    set("${OUTPUT}" "${join_list_RESULT}" PARENT_SCOPE)
endfunction()
