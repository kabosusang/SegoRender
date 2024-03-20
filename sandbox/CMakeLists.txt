add_executable(sandbox)
aux_source_directory(./ SANDBOX_SRC)
target_sources(sandbox PRIVATE ${SANDBOX_SRC})

target_link_libraries(sandbox PUBLIC SGengine editor)
CopyDLL(sandbox)
