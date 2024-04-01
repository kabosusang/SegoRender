add_executable(Editor)
aux_source_directory(./ SANDBOX_SRC)
target_sources(Editor PRIVATE ${SANDBOX_SRC})

target_link_libraries(Editor PUBLIC SGengine)
CopyDLL(Editor)
