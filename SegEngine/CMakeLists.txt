cmake_minimum_required(VERSION 3.20)

project(
    SGengine
    LANGUAGES C CXX
    DESCRIPTION "Vulkan C++ wrapper"
)

include(./Core/Vulkan/CMakeLists.txt)

aux_source_directory(./Core/Vulkan/src engine_src)
message("source files: ${engine_src}")

add_library(SGengine STATIC ${engine_src})
#Vulkan
target_include_directories(SGengine PUBLIC Core/Vulkan/include)




target_compile_features(SGengine PRIVATE cxx_std_17)

target_link_libraries(SGengine PUBLIC Vulkan::Vulkan)