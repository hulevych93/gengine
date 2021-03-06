# generate.cmake
# Generate interface files from *.rdls configuration files

find_package(Perl)
if(NOT PERL_FOUND)
    message(FATAL_ERROR "Please, install perl.")
endif()

set(PERL_SCRIPTS)
file(GLOB_RECURSE PERL_SCRIPTS
        "${GENGINE_PERL_DIR}/*.pl"
        "${GENGINE_PERL_DIR}/*.pm")

function(gengine_gen_api RDL_FILE API_ROOT OUTPUT_FILES)
    set(QUERIES_GENERATED_FILES)
    execute_process(
        COMMAND ${PERL_EXECUTABLE} ${GENGINE_PERL_DIR}/queryGenOut.pl -idl_file: ${RDL_FILE} -out_path: ${PROJECT_BINARY_DIR}
        OUTPUT_VARIABLE QUERIES_GENERATED_FILES)

    add_custom_command(OUTPUT ${QUERIES_GENERATED_FILES}
        COMMAND ${PERL_EXECUTABLE} ${GENGINE_PERL_DIR}/interfacesGen.pl -idl_file: ${RDL_FILE} -out_path: ${PROJECT_BINARY_DIR}
        COMMAND ${PERL_EXECUTABLE} ${GENGINE_PERL_DIR}/enumsGen.pl -idl_file: ${RDL_FILE} -out_path: ${PROJECT_BINARY_DIR}
        COMMAND ${PERL_EXECUTABLE} ${GENGINE_PERL_DIR}/dataGen.pl -idl_file: ${RDL_FILE} -out_path: ${API_ROOT} -out_dir: ${API_FOLDER}  -header
        DEPENDS ${RDL_FILE} ${PERL_SCRIPTS}
        COMMENT "generating api files...\n"
    )

    set(${OUTPUT_FILES} ${${OUTPUT_FILES}} ${QUERIES_GENERATED_FILES} PARENT_SCOPE)
endfunction()

function(gengine_add_client CLASS_NAME IDL_FILE GENERATED_OUT)
    string(REGEX REPLACE "^_?I(.*)$" "\\1" BASE_NAME "${CLASS_NAME}")
    set(MY_SOURCES
        ${BASE_NAME}Client.cpp
        ${BASE_NAME}DummyClient.cpp
    )
    add_custom_command(
        COMMAND ${PERL_EXECUTABLE} ${GENGINE_PERL_DIR}/interprocessGen.pl -idl_file: ${IDL_FILE} -out_path: ${API_FOLDER} -iface: ${CLASS_NAME} -client
        DEPENDS ${IDL_FILE} ${PERL_SCRIPTS}
        OUTPUT ${MY_SOURCES}
    )
    set_source_files_properties(${MY_SOURCES} PROPERTIES GENERATED TRUE)
    set(${GENERATED_OUT} ${${GENERATED_OUT}} ${MY_SOURCES} PARENT_SCOPE)
endfunction()

function(gengine_add_handler CLASS_NAME IDL_FILE GENERATED_OUT)
    string(REGEX REPLACE "^_?I(.*)$" "\\1" BASE_NAME "${CLASS_NAME}")
    set(MY_SOURCES ${BASE_NAME}RequestHandler.cpp)
    add_custom_command(
        COMMAND ${PERL_EXECUTABLE} ${GENGINE_PERL_DIR}/interprocessGen.pl -idl_file: ${IDL_FILE} -out_path: ${API_FOLDER} -iface: ${CLASS_NAME} -server
        DEPENDS ${IDL_FILE} ${PERL_SCRIPTS}
        OUTPUT ${MY_SOURCES}
    )
    set_source_files_properties(${MY_SOURCES} PROPERTIES GENERATED TRUE)
    set(${GENERATED_OUT} ${${GENERATED_OUT}} ${MY_SOURCES} PARENT_SCOPE)
endfunction()

function(gengine_add_struct CLASS_NAME IDL_FILE GENERATED_OUT)
    string(REGEX REPLACE "^_?I(.*)$" "\\1" BASE_NAME "${CLASS_NAME}")
    set(MY_SOURCES ${BASE_NAME}.cpp)
    add_custom_command(
        COMMAND ${PERL_EXECUTABLE} ${GENGINE_PERL_DIR}/dataGen.pl -idl_file: ${IDL_FILE} -out_path: ${API_FOLDER} -data: ${CLASS_NAME} -source
        DEPENDS ${IDL_FILE} ${PERL_SCRIPTS}
        OUTPUT ${MY_SOURCES}
    )
    set_source_files_properties(${MY_SOURCES} PROPERTIES GENERATED TRUE)
    set(${GENERATED_OUT} ${${GENERATED_OUT}} ${MY_SOURCES} PARENT_SCOPE)
endfunction()
