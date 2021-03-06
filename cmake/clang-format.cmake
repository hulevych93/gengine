# clang-format.cmake

if(NOT CLANGFORMAT_EXECUTABLE)
    find_program(CLANGFORMAT_EXECUTABLE "clang-format")
    if(NOT CLANGFORMAT_EXECUTABLE)
      message("ClangFormat: ${CLANGFORMAT_EXECUTABLE} not found...")
    endif()
endif()

function(gengine_run_clang_format)
    if(CLANGFORMAT_EXECUTABLE)
        if(APPLE)
            gengine_collect_only_cpp(ALL_SRC)
        else()
            gengine_collect_only_platform_cpp(ALL_SRC)
        endif()

        foreach(SFILE ${ALL_SRC})
            execute_process(COMMAND ${CLANGFORMAT_EXECUTABLE} -style=Chromium -i ${SFILE})
        endforeach()
    endif()
endfunction()

# run clang-formatting
gengine_run_clang_format()
