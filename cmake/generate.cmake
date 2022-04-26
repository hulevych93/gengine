# generate.cmake

# Generate interface files from *.rdls configuration files
function(gengine_gen_api RDL_FILE API_ROOT OUTPUT_FILES)
    set(QUERIES_GENERATED_FILES)
    execute_process(
        COMMAND perl ${GENGINE_PERL_DIR}/queryGenOut.pl -idl_file: ${RDL_FILE} -out_path: ${PROJECT_BINARY_DIR}
        OUTPUT_VARIABLE QUERIES_GENERATED_FILES
        COMMAND_ERROR_IS_FATAL ANY)

    message("vv ${QUERIES_GENERATED_FILES}")

    add_custom_command(OUTPUT ${QUERIES_GENERATED_FILES}
        COMMAND perl ${GENGINE_PERL_DIR}/interfacesGen.pl -idl_file: ${RDL_FILE} -out_path: ${PROJECT_BINARY_DIR}
        COMMAND perl ${GENGINE_PERL_DIR}/enumsGen.pl -idl_file: ${RDL_FILE} -out_path: ${PROJECT_BINARY_DIR}
        COMMAND perl ${GENGINE_PERL_DIR}/dataGen.pl -idl_file: ${RDL_FILE} -out_path: ${API_ROOT} -out_dir: ${API_FOLDER}  -header
        COMMENT "generating api files...\n"
    )

    set(${OUTPUT_FILES} ${${OUTPUT_FILES}} ${QUERIES_GENERATED_FILES} PARENT_SCOPE)
endfunction()
