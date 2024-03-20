aux_source_directory(Base/ base)
add_library(editor STATIC ${base})

target_include_directories(editor PUBLIC ./)
target_link_libraries(editor PUBLIC imgui SDL2)
