#pragma once
#include "pch.h"
#include "context.hpp"

namespace Sego{

class RenderPass
{
public:
    virtual void Init();
    virtual void Render() = 0;
    virtual void destroy();
    //virtual void Cleanup();

    //Function 
    virtual void CreatePiepline() = 0;
    virtual void CreateFrameBuffer() = 0;
    virtual void CreateRenderPass() = 0;

protected:
    //PipeLine Struct --------------------------------------------------------------
    vk::GraphicsPipelineCreateInfo pipeline_ci{};
    //1. vertex input
    vk::PipelineVertexInputStateCreateInfo vertex_input_ci{};
    //2. input assembly
    vk::PipelineInputAssemblyStateCreateInfo input_assemb_ci{};
    //3. viewport and scissor
    vk::PipelineViewportStateCreateInfo viewport_state_ci{};
    vk::Viewport viewport_ci{};
    vk::Rect2D scissor_ci{};
    //4. rasteraizer
    vk::PipelineRasterizationStateCreateInfo raster_ci{};
    //5. multisampling
    vk::PipelineMultisampleStateCreateInfo multisample_ci{};
    // 6. depth and stencil buffer
    vk::PipelineDepthStencilStateCreateInfo depth_stencil_ci{};
    //7. color blending
    vk::PipelineColorBlendAttachmentState colorblendattachment_ci;
    vk::PipelineColorBlendStateCreateInfo blend_ci{};

    //Vulkan Objects ---------------------------------------------------------------
    std::vector<vk::PipelineLayout> pipelineLayouts_;
    vk::DescriptorPool descriptorPool_;

    //Renderer Needs---------------------------------------------------------------
    std::vector<vk::Pipeline> pipelines_;
    std::vector<vk::Framebuffer> framebuffers_;
    vk::RenderPass renderPass_;


    //Renderer tagret Extent
    uint32_t width_,height_;

};






}