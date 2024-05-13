#include "pch.h"
#include "brdf_lut_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Texture2D.hpp"

namespace Sego{

BRDFLutPass::BRDFLutPass(){
    m_format =  vk::Format::eR16G16Sfloat;
    lutsize_ = 2048;  
}

void BRDFLutPass::Init(){
    RenderPass::Init();
}

void BRDFLutPass::destroy(){
    RenderPass::destroy();
    lutIv_.destroy();
}

void BRDFLutPass::createDescriptorSetLayout(){
    descriptorSetLayouts_.resize(1);

    //mesh DescriptorSetLayout
     vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci_mesh{};
    desc_set_layout_ci_mesh.setBindingCount(0)
                      .setBindings(nullptr)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);

    descriptorSetLayouts_[0] = Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci_mesh);

}

void BRDFLutPass::createPipelineLayouts(){
    pipelineLayouts_.resize(1); 

    vk::PipelineLayoutCreateInfo pipeline_layout_ci{};
    pipeline_layout_ci.setSetLayoutCount(1)
                    .setPSetLayouts(&descriptorSetLayouts_[0]);

    pipelineLayouts_[0] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci);
}

void BRDFLutPass::CreatePiepline(){
    auto& ctx = Context::Instance(); 
    pipelines_.resize(1); 

    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //Sprite Renderer
    // 0. shader prepare
     std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/Forward/Scenevert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/Pbr/Brdflutfrag.spv", vk::ShaderStageFlagBits::eFragment)
    };
     
    vertex_input_ci.setVertexAttributeDescriptionCount(0)
                   .setPVertexAttributeDescriptions(nullptr)
                   .setVertexBindingDescriptionCount(0)
                   .setPVertexBindingDescriptions(nullptr);
    vertex_input_ci.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;

    //2. input assembly 
    input_assemb_ci.setTopology(vk::PrimitiveTopology::eTriangleList) //Triangle
                  .setPrimitiveRestartEnable(VK_FALSE);

    //3. viewport and scissor
    viewport_ci.setX(0.0f).setY(0.0f).setWidth(lutsize_)
                .setHeight(lutsize_)
                .setMinDepth(0.0f).setMaxDepth(1.0f);
    scissor_ci.setOffset({0,0})
              .setExtent({lutsize_,lutsize_});
    viewport_state_ci.setViewports(viewport_ci)
                     .setScissors(scissor_ci);
    //4. rasteraizer
    raster_ci.setDepthClampEnable(VK_FALSE) //if true, fragments beyond near and far planes are clamped to them
             .setRasterizerDiscardEnable(VK_FALSE) //if true, geometry never passes through rasterization stage
             .setPolygonMode(vk::PolygonMode::eFill)
             .setLineWidth(1.0f)
             .setCullMode(vk::CullModeFlagBits::eNone)
             .setFrontFace(vk::FrontFace::eCounterClockwise)
             .setDepthBiasEnable(VK_FALSE);
    // dynamic states
    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport,
    vk::DynamicState::eScissor,vk::DynamicState::eDepthBias};
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.setDynamicStateCount(dynamic_states.size())
                .setPDynamicStates(dynamic_states.data());

    //5. multisampling
    multisample_ci.setSampleShadingEnable(VK_FALSE)
                  .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                  .setMinSampleShading(1.0f)
                  .setPSampleMask(nullptr)
                  .setAlphaToCoverageEnable(VK_FALSE)
                  .setAlphaToOneEnable(VK_FALSE);
    //6. depth and stencil buffer
    depth_stencil_ci.setDepthTestEnable(VK_FALSE)
                    .setDepthWriteEnable(VK_FALSE)
                    .setDepthCompareOp(vk::CompareOp::eLess)
                    .setDepthBoundsTestEnable(VK_FALSE)
                    .setStencilTestEnable(VK_FALSE)
                    .setMinDepthBounds(0.0f)
                    .setMaxDepthBounds(1.0f);

    //7. color blending
    colorblendattachment_ci.setBlendEnable(false)
                 .setColorWriteMask(vk::ColorComponentFlagBits::eA|
                                    vk::ColorComponentFlagBits::eB|
                                    vk::ColorComponentFlagBits::eG|
                                    vk::ColorComponentFlagBits::eR)
                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                .setAlphaBlendOp(vk::BlendOp::eAdd);

    blend_ci.setAttachments(colorblendattachment_ci)
            .setLogicOpEnable(false);
    //8. pipeline layout
    //createPipelineLayouts(); <- createPipelineLayouts() is called in RenderPass::Init()

    //9. pipeline
    pipeline_ci.setStages(shader_stage_cis)
               .setPVertexInputState(&vertex_input_ci)
               .setPInputAssemblyState(&input_assemb_ci)
               .setPViewportState(&viewport_state_ci)
               .setPRasterizationState(&raster_ci)
               .setPDynamicState(&dynamicState)
               .setPMultisampleState(&multisample_ci)
               .setPDepthStencilState(&depth_stencil_ci)
               .setPColorBlendState(&blend_ci)
               .setLayout(pipelineLayouts_[0])
               .setRenderPass(renderPass_);
    
    auto Result = ctx.device.createGraphicsPipeline(nullptr, pipeline_ci);
    pipelines_[0] = Result.value;
}


void BRDFLutPass::CreateFrameBuffer(){
    auto& ctx = Context::Instance();
    //lutIv_
    Vulkantool::createImageAndView(lutsize_,lutsize_,1,1,vk::SampleCountFlagBits::e1,
    m_format,vk::ImageTiling::eOptimal,vk::ImageUsageFlagBits::eColorAttachment |vk::ImageUsageFlagBits::eSampled| vk::ImageUsageFlagBits::eTransferSrc,
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,vk::ImageAspectFlagBits::eColor,lutIv_);

    std::vector<vk::ImageView> attachments = { 
    lutIv_.image_view
    };

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(renderPass_)
                    .setAttachmentCount(1)
                    .setPAttachments(attachments.data())
                    .setWidth(lutsize_)
                    .setHeight(lutsize_)
                    .setLayers(1);
    framebuffer_ = Context::Instance().device.createFramebuffer(framebufferInfo);
}

void BRDFLutPass::CreateRenderPass(){
    vk::RenderPassCreateInfo createInfo;
    std::array<vk::AttachmentDescription, 1> attachments{};
    attachments[0].setFormat(m_format)
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
   
   std::array<vk::AttachmentReference,1> AttachmentRef;//color attachment reference
    //Color Ref
    AttachmentRef[0].setAttachment(0)
                     .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
  
    vk::SubpassDescription subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&AttachmentRef[0]);
    
    std::array<vk::SubpassDependency,1> dependencies;
    dependencies[0].setSrcSubpass(0)
                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);               

    createInfo.setAttachmentCount(attachments.size())
              .setPAttachments(attachments.data())
              .setSubpassCount(1)
              .setPSubpasses(&subpass)
              .setDependencyCount(static_cast<uint32_t>(dependencies.size()))
              .setPDependencies(dependencies.data());

    renderPass_ = Context::Instance().device.createRenderPass(createInfo);
}

void BRDFLutPass::Render(){
    auto& ctx = Context::Instance();
    auto& Vctx = VulkanRhi::Instance();
    auto cmdBuffer =Vulkantool::beginSingleCommands();
   
    std::array<vk::ClearValue,1> clearValues{};
    clearValues[0].setColor({0.0f,0.0f,0.0f,1.0f});

    vk::RenderPassBeginInfo renderPassBegin{};
    renderPassBegin.setRenderPass(renderPass_)
                   .setFramebuffer(framebuffer_)
                   .setClearValues(clearValues)
                   .setRenderArea(vk::Rect2D({}, {lutsize_,lutsize_}));
   
    cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    vk::Viewport viewport{};
    viewport.setWidth(static_cast<float>(lutsize_))
            .setHeight(static_cast<float>(lutsize_))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
    cmdBuffer.setViewport(0, 1, &viewport);
    vk::Rect2D scissor{};
    scissor.setOffset({0, 0})
           .setExtent({lutsize_,lutsize_});
    cmdBuffer.setScissor(0, 1, &scissor);
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
    cmdBuffer.draw(3,1,0,0);
    cmdBuffer.endRenderPass();
    Vulkantool::endInstantCommands(cmdBuffer);

    // write to texture2d
    std::future<void> lut_async = std::async(std::launch::async,[&](){
        std::vector<uint8_t> image_data;
        Vulkantool::readImagePixel(lutIv_.image(),lutsize_,lutsize_,m_format,image_data);
        
        std::ofstream ofs("resources/assets/engine/texture/brdflut.tex", std::ios::binary);
		ofs.write((const char*)image_data.data(), image_data.size());
		ofs.close();
    });

}


}