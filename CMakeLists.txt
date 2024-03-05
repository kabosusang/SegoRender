cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SegoRender
        LANGUAGES CXX
        DESCRIPTION "a toy 2d renderer made in vulkan")

include(cmake/FindVulkan.cmake)
include(cmake/sdl2-config.cmake)
include(cmake/CopyFiles.cmake)

find_program(GLSLC_PROGAM glslc REQUIRED)
execute_process(COMMAND ${GLSLC_PROGAM}  ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/vert.spv )
execute_process(COMMAND ${GLSLC_PROGAM}  ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/frag.spv )


# 输出目录
set(output_bin ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${output_bin})

aux_source_directory(src SRC)
add_library(SegoRender STATIC ${SRC})

target_include_directories(SegoRender PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(SegoRender PUBLIC Vulkan::Vulkan SDL2::SDL2main SDL2::SDL2)
target_compile_features(SegoRender PUBLIC cxx_std_17)

add_subdirectory(sandbox)