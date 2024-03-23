#pragma once
#include "pch.h"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> 

namespace Sego{
std::vector<char> ReadWholeFile(const std::string& filename);

//VMA Buffer
struct VmaBuffer{
    vk::Buffer buffer;
    vk::DeviceSize size;
    VmaAllocation allocation;
    void destroy();
};




class Vulkantool{
    public:
        static vk::CommandBuffer beginSingleCommands();
        static void endInstantCommands(vk::CommandBuffer command_buffer);
        
        //Meory related 
        //static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
        static void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, VmaBuffer& buffer);
        static void updateBuffer(VmaBuffer& buffer, void* data, size_t size);
        static void copyBuffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size);
        static void createVertexBuffer(uint32_t buffer_size, void* data, VmaBuffer& buffer);
        static void createIndexBuffer(uint32_t buffer_size, void* data, VmaBuffer& buffer);

};

}