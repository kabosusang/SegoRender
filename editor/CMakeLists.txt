add_executable(Editor)
aux_source_directory(./ Editor_SRC)
aux_source_directory(Panels/ Editor_SRC)
aux_source_directory(editor/ Editor_SRC)

target_sources(Editor PRIVATE ${Editor_SRC})

target_link_libraries(Editor PUBLIC SGengine)
CopyDLL(Editor)
