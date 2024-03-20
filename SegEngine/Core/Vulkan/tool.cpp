#include "tool.hpp"
#include "Vulkan_rhi.hpp"

namespace Sego {

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
    Context::Instance().device.allocateCommandBuffers(&alloc_info, &CmdBuffer);
    vk::CommandBufferBeginInfo begin_info = {};
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    CmdBuffer.begin(&begin_info);
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

    ctx.device.createFence(&fence_ci, nullptr, &fence);
    auto& queue = Context::Instance().graphicsQueue;

    queue.submit(1, &submit_info, fence);
    ctx.device.waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
    ctx.device.destroyFence(fence);
    ctx.device.freeCommandBuffers(VulkanRhi::Instance().getSingleCommandPool(), 1, &command_buffer);
}



}
