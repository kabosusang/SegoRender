#pragma once

#include <vulkan/vulkan.hpp>


class ImguiInit
{
public:
    ImguiInit();
    ~ImguiInit();

    void Init_Imgui();
 
    vk::RenderPass& GetUiRenderpass() { return uiRenderPass; }
    std::vector<vk::CommandBuffer> uiCommandBuffers;
    
    void RecreateSwapChain();
    void CleanupSwapChain();

    void RecoreImgui(int current,uint32_t imageindex);


private:
    //
    vk::DescriptorPool uiDescriptorPool;
    vk::RenderPass uiRenderPass;
    vk::CommandPool uiCommPool;

    std::vector<VkFramebuffer> uiFramebuffers;
    uint32_t MinImageCount = 0;

    uint32_t curFrame_ = 0;
};

