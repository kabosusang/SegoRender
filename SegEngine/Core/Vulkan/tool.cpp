#include "tool.hpp"
#include "Vulkan_rhi.hpp"

// #define VMA_IMPLEMENTATION 
namespace Sego {

void VmaBuffer::destroy(){
   if (buffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(Context::Instance().getAllocator(), buffer, allocation);
    }
}


std::vector<char> ReadWholeFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary|std::ios::ate);

    if (!file.is_open()) {
        std::cout << "read " << filename << " failed" << std::endl;
        return std::vector<char>{};
    }

    auto size = file.tellg();
    std::vector<char> content(size);
    file.seekg(0);

    file.read(content.data(), content.size());

    return content;
}

vk::CommandBuffer Vulkantool::beginSingleCommands(){
    vk::CommandBufferAllocateInfo alloc_info = {};
    alloc_info.setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandPool(VulkanRhi::Instance().getSingleCommandPool())
              .setCommandBufferCount(1);
    vk::CommandBuffer CmdBuffer;
    auto result = Context::Instance().device.allocateCommandBuffers(&alloc_info, &CmdBuffer);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::beginSingleCommands Error");}


    vk::CommandBufferBeginInfo begin_info = {};
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    result = CmdBuffer.begin(&begin_info);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::beginSingleCommands Error");}
    return CmdBuffer;
}

void Vulkantool::endInstantCommands(vk::CommandBuffer command_buffer){
    auto& ctx = Context::Instance();
    
    command_buffer.end();
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1)
               .setPCommandBuffers(&command_buffer);

    vk::FenceCreateInfo fence_ci{};
    vk::Fence fence;

    auto result = ctx.device.createFence(&fence_ci, nullptr, &fence);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::endInstantCommands Error");}
    auto& queue = Context::Instance().graphicsQueue;
    result = queue.submit(1, &submit_info, fence);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::endInstantCommands Error");}
    result = ctx.device.waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::endInstantCommands Error");}
    ctx.device.destroyFence(fence);
    ctx.device.freeCommandBuffers(VulkanRhi::Instance().getSingleCommandPool(), 1, &command_buffer);
}

void Vulkantool::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags buffer_usage, 
VmaMemoryUsage memory_usage, VmaBuffer& buffer){
    auto& ctx = Context::Instance();
    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.setSize(size)
              .setUsage(buffer_usage)
              .setSharingMode(vk::SharingMode::eExclusive);
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memory_usage;
    if (memory_usage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
    {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
   vmaCreateBuffer(ctx.getAllocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&buffer), &buffer.allocation, nullptr);
}

void Vulkantool::copyBuffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size){
    vk::CommandBuffer cmd = beginSingleCommands();

    vk::BufferCopy coyeRegion = {};
    coyeRegion.size = size;
    cmd.copyBuffer(src_buffer,dst_buffer,1,&coyeRegion);
    endInstantCommands(cmd);
}

void Vulkantool::updateBuffer(VmaBuffer& buffer, void* data, size_t size){
    void* mappedData;
    vmaMapMemory(Context::Instance().getAllocator(), buffer.allocation, &mappedData);
    memcpy(mappedData, data, size);
    vmaUnmapMemory(Context::Instance().getAllocator(),buffer.allocation);
}

void Vulkantool::createVertexBuffer(uint32_t buffer_size, void* data, VmaBuffer& vertex_buffer){
    VmaBuffer staging_buffer;
    createBuffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc, 
    VMA_MEMORY_USAGE_AUTO_PREFER_HOST, staging_buffer);
    updateBuffer(staging_buffer, data, static_cast<size_t>(buffer_size));

    createBuffer(buffer_size, 
    vk::BufferUsageFlagBits::eTransferDst | 
    vk::BufferUsageFlagBits::eVertexBuffer,VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,vertex_buffer);

    copyBuffer(staging_buffer.buffer, vertex_buffer.buffer, buffer_size);
	vmaDestroyBuffer(Context::Instance().getAllocator(), staging_buffer.buffer, staging_buffer.allocation);
}

void Vulkantool::createIndexBuffer(uint32_t buffer_size, void* data,VmaBuffer& index_buffer)
{
    VmaBuffer staging_buffer;
    createBuffer(buffer_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        staging_buffer);

    // copy index data to staging buffer
    updateBuffer(staging_buffer, (void*)data, static_cast<size_t>(buffer_size));

    createBuffer(buffer_size,
        vk::BufferUsageFlagBits::eTransferDst | 
        vk::BufferUsageFlagBits::eIndexBuffer,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        index_buffer);

    copyBuffer(staging_buffer.buffer, index_buffer.buffer, buffer_size);
    vmaDestroyBuffer(Context::Instance().getAllocator(), staging_buffer.buffer, staging_buffer.allocation);
}



// //Not Use Vma
// uint32_t Vulkantool::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties){
//     auto& ctx = Context::Instance();
//     vk::PhysicalDeviceMemoryProperties memProperties;
//     ctx.phyDevice.getMemoryProperties(&memProperties);
    
//      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//             if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//                 return i;
//             }
//         }
//     throw std::runtime_error("failed to find suitable memory type!");
// }

}
