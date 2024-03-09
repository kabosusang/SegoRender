cmake_minimum_required(VERSION 3.20)

project(
    SGengine
    LANGUAGES C CXX
    DESCRIPTION "SGengine Static Library"
)

include(./Core/Vulkan/CMakeLists.txt)

aux_source_directory(Core/Vulkan/src engine_src)
aux_source_directory(Core/Log LOG)
aux_source_directory(framework/window Window)
aux_source_directory(framework/Render/ui UI)

add_library(SGengine STATIC ${engine_src} ${LOG} ${Window} ${UI})
#Vulkan
target_include_directories(SGengine PUBLIC Core/Vulkan/include)
target_include_directories(SGengine PUBLIC ./)

target_compile_features(SGengine PRIVATE cxx_std_17)
target_link_libraries(SGengine PUBLIC Vulkan::Vulkan SDL2::SDL2main SDL2::SDL2 spdlog_header_only imgui)
