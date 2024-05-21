#pragma once
#include "pch.h"
#include "context.hpp"
#include <glm/glm.hpp>
#include "framework/Render/Render_data.hpp"

namespace Sego{

class RenderPass
{
public:
    virtual void Init();
    virtual void Render() = 0;
    virtual void destroy();
   
    //virtual void Cleanup();

    //Function Virtual = 0
    virtual void CreatePiepline() = 0;
    virtual void CreateFrameBuffer() = 0;
    virtual void CreateRenderPass() = 0;
    //
    virtual void createDescriptorSetLayout();
    virtual void createPipelineLayouts();
    void crearePipelineCache();

    void updatePushConstants(vk::CommandBuffer commandbuffer,vk::PipelineLayout pipeline_layout,
    const std::vector<const void*>& (pcos),std::vector<vk::PushConstantRange> push_constant_ranges = {});
    //SSBO
    void addSsboBufferDescriptorSet(std::vector<vk::WriteDescriptorSet>& desc_writes, 
		vk::DescriptorBufferInfo& desc_buffer_info, VmaBuffer buffer, uint32_t binding);

    void addBufferDescriptorSet(std::vector<vk::WriteDescriptorSet>& desc_writes, 
		vk::DescriptorBufferInfo& desc_buffer_info, VmaBuffer buffer, uint32_t binding);
    void addImageDescriptorSet(std::vector<vk::WriteDescriptorSet>& desc_writes, 
        vk::DescriptorImageInfo& desc_image_info, VmaImageViewSampler texture, uint32_t binding);
     void addDepthImageDescriptorSet(std::vector<vk::WriteDescriptorSet>& desc_writes, 
        vk::DescriptorImageInfo& desc_image_info, VmaImageViewSampler texture, uint32_t binding);

    //Multiple Images
    void addImagesDescriptorSet(std::vector<vk::WriteDescriptorSet>& desc_writes,
		vk::DescriptorImageInfo* p_desc_image_info, const std::vector<VmaImageViewSampler>& textures, uint32_t binding);
    
    //Output Function
    virtual void recreateframbuffer();
    virtual void setClearColor(const glm::vec4& color);
    void setRenderDatas(std::vector<std::shared_ptr<RenderData>>& renderDatas){
        renderDatas_ = renderDatas;
    }

protected:
    //PipeLine Struct --------------------------------------------------------------
    vk::GraphicsPipelineCreateInfo pipeline_ci{};
    //1. vertex input
    vk::PipelineVertexInputStateCreateInfo vertex_input_ci{};
    //2. input assembly
    vk::PipelineInputAssemblyStateCreateInfo input_assemb_ci{};
    //3. viewport and scissor
    vk::PipelineViewportStateCreateInfo viewport_state_ci{};
    vk::Viewport  viewport_ci{};
    vk::Rect2D scissor_ci{};
    //4. rasteraizer
    vk::PipelineRasterizationStateCreateInfo raster_ci{};
    //5. multisampling
    vk::PipelineMultisampleStateCreateInfo multisample_ci{};
    // 6. depth and stencil buffer
    vk::PipelineDepthStencilStateCreateInfo depth_stencil_ci{};
    //7. color blending
    vk::PipelineColorBlendAttachmentState colorblendattachment_ci;
    std::vector<vk::PipelineColorBlendAttachmentState> colorblendattachments_ci;
    vk::PipelineColorBlendStateCreateInfo blend_ci{};

    //Vulkan Objects ---------------------------------------------------------------
    std::vector<vk::PushConstantRange> push_constant_ranges_;
    std::vector<vk::PipelineLayout> pipelineLayouts_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    vk::DescriptorPool descriptorPool_ = nullptr;
    std::vector<vk::WriteDescriptorSet> desc_writes;
    
    //Renderer Needs---------------------------------------------------------------
    std::vector<vk::Pipeline> pipelines_;
    vk::Framebuffer framebuffer_;
    vk::RenderPass renderPass_;

    //pipeline cache
    vk::PipelineCache pipelineCache_;
    //Renderer tagret Extent
    uint32_t width_,height_;

    //SetColor
    glm::vec4 clearColor_ = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    std::vector<std::shared_ptr<RenderData>> renderDatas_;
};






}