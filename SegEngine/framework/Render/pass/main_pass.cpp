#include "main_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"

namespace Sego{
   
void MainPass::CreatePiepline(){
    auto& ctx = Context::Instance();
    
     // 0. shader prepare
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/vert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/frag.spv", vk::ShaderStageFlagBits::eFragment)
    };

    //1. vertex input
    //vertex_input_ci

    //2. input assembly 
    input_assemb_ci.setTopology(vk::PrimitiveTopology::eTriangleList) //Triangle
                  .setPrimitiveRestartEnable(VK_FALSE);

    //3. viewport and scissor
    viewport_ci.setX(0.0f).setY(0.0f).setWidth(ctx.swapchain->GetExtent().width)
                .setHeight(ctx.swapchain->GetExtent().height)
                .setMinDepth(0.0f).setMaxDepth(1.0f);
    scissor_ci.setOffset({0,0})
              .setExtent(ctx.swapchain->GetExtent());
    viewport_state_ci.setViewports(viewport_ci)
                     .setScissors(scissor_ci);
    //4. rasteraizer
    raster_ci.setDepthClampEnable(VK_FALSE) //if true, fragments beyond near and far planes are clamped to them
             .setRasterizerDiscardEnable(VK_FALSE) //if true, geometry never passes through rasterization stage
             .setPolygonMode(vk::PolygonMode::eFill)
             .setLineWidth(1.0f)
             .setCullMode(vk::CullModeFlagBits::eBack)
             .setFrontFace(vk::FrontFace::eClockwise)
             .setDepthBiasEnable(VK_FALSE);
    //5. multisampling
    multisample_ci.setSampleShadingEnable(VK_FALSE)
                  .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                  .setMinSampleShading(1.0f)
                  .setPSampleMask(nullptr)
                  .setAlphaToCoverageEnable(VK_FALSE)
                  .setAlphaToOneEnable(VK_FALSE);
    //6. depth and stencil buffer
    depth_stencil_ci.setDepthTestEnable(VK_TRUE)
                    .setDepthWriteEnable(VK_TRUE)
                    .setDepthCompareOp(vk::CompareOp::eLess)
                    .setDepthBoundsTestEnable(VK_FALSE)
                    .setStencilTestEnable(VK_FALSE);
    //7. color blending
    colorblendattachment_ci.setBlendEnable(VK_TRUE)
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
            .setLogicOpEnable(VK_FALSE);

    //8. pipeline layout
    pipelineLayouts_.resize(1);
    pipelineLayouts_[0] = ctx.device.createPipelineLayout(vk::PipelineLayoutCreateInfo{});

    //9. pipeline
    pipeline_ci.setStages(shader_stage_cis)
               .setPVertexInputState(&vertex_input_ci)
               .setPInputAssemblyState(&input_assemb_ci)
               .setPViewportState(&viewport_state_ci)
               .setPRasterizationState(&raster_ci)
               .setPMultisampleState(&multisample_ci)
               .setPDepthStencilState(&depth_stencil_ci)
               .setPColorBlendState(&blend_ci)
               .setLayout(pipelineLayouts_[0])
               .setRenderPass(renderPass_);
    
    pipelines_.resize(1);
    auto Result = ctx.device.createGraphicsPipeline(nullptr, pipeline_ci);
    pipelines_[0] = Result.value;

}

void MainPass::CreateFrameBuffer(){
    auto& ctx = Context::Instance();
    auto& SwapchainImage = ctx.swapchain->SwapchainImagesAview_;
    auto& Extent = ctx.swapchain->GetExtent();

    framebuffers_.resize(SwapchainImage.size());
    for (int i = 0; i < SwapchainImage.size(); ++i) {
        auto& view = SwapchainImage[i].view;
        std::vector<vk::ImageView> attachments = { view };

        vk::FramebufferCreateInfo createInfo;
        createInfo.setAttachments(attachments)
                  .setLayers(1)
                  .setHeight(Extent.height)
                  .setWidth(Extent.width)
                  .setRenderPass(renderPass_);
        framebuffers_[i] = Context::Instance().device.createFramebuffer(createInfo);
    }
}

void MainPass::CreateRenderPass(){
     vk::RenderPassCreateInfo createInfo;

    vk::AttachmentDescription colorAttachment; //color attachment
    vk::AttachmentReference colorAttachmentRef;//color attachment reference

    colorAttachment.setFormat(Context::Instance().swapchain->GetFormat().format)
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eStore)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    
    colorAttachmentRef.setAttachment(0)
                     .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    //subpass
    vk::SubpassDescription subpass; //subpass
    vk::SubpassDependency dependency; //subpass dependency
    
    subpass.setColorAttachments(colorAttachmentRef)
          .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setDstSubpass(0)
              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
              .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
              .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    createInfo.setAttachments(colorAttachment)
              .setSubpasses(subpass)
              .setDependencies(dependency);
            
    renderPass_ = Context::Instance().device.createRenderPass(createInfo);
}

void MainPass::Render(){
   
    auto& ctx = Context::Instance();
    auto& VulkanRhi = VulkanRhi::Instance();
    auto& Extent = ctx.swapchain->GetExtent();
    auto cmdBuffer = VulkanRhi.getCommandBuffer();
    
    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));

    vk::RenderPassBeginInfo renderPassBegin{};
    renderPassBegin.setRenderPass(renderPass_)
                   .setFramebuffer(framebuffers_[VulkanRhi.currentImageIndex_])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, Extent));
    cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
    cmdBuffer.draw(3, 1, 0, 0);
    cmdBuffer.endRenderPass();
   
}



}