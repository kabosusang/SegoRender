cmake_minimum_required(VERSION 3.20)

project(
    SGengine
    LANGUAGES C CXX
    DESCRIPTION "SGengine Static Library"
)

include(./Core/Vulkan/CMakeLists.txt)
include_directories(${PROJECT_SOURCE_DIR})

set(PCH_HEADER "pch.h")  # 设置预编译头文件的路径
# 添加预编译头
#target_precompile_headers(SegEngine PRIVATE ${PCH_HEADER})
aux_source_directory(. ENTRY)

aux_source_directory(Core/Base engine_src)
aux_source_directory(Core/Log engine_src)
aux_source_directory(Core/Window engine_src)
aux_source_directory(Core/Layer engine_src)
aux_source_directory(Core/Scene engine_src)

aux_source_directory(framework/Render/pass frame_work)
aux_source_directory(imgui frame_work)

aux_source_directory(resource/shader renderer_code)
aux_source_directory(framework/Render renderer_code)
aux_source_directory(Core/Vulkan renderer_code)
aux_source_directory(Core/Vulkan/src renderer_code)
aux_source_directory(Renderer renderer_code)


aux_source_directory(resource/asset/ Asset)
aux_source_directory(resource/asset/Import Asset)
aux_source_directory(resource/asset/base Asset)


add_library(SGengine STATIC ${ENTRY} ${engine_src} ${frame_work} ${Asset} ${renderer_code})
#Vulkan
target_include_directories(SGengine PUBLIC Core/Vulkan/include)
target_include_directories(SGengine PUBLIC ./)


target_compile_features(SGengine PRIVATE cxx_std_17)
target_link_libraries(SGengine PUBLIC Vulkan::Vulkan SDL2::SDL2main SDL2::SDL2 spdlog imgui 
glm::glm stb_image tinygltf VulkanMemoryAllocator EnTT)
