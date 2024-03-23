#include "main_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/tool.hpp"
#include "resource/asset/Vertex.hpp"

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
   0, 1, 2, 2, 3, 0
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

namespace Sego{

void MainPass::destroy(){
    RenderPass::destroy();
    vertexBuffer_.destroy();
    indexBuffer_.destroy();

    for(auto& buffer : uniformBuffers_){
        buffer.destroy();
    }

}

void MainPass::temporarilyInit(){
    //vertex    
    Vulkantool::createVertexBuffer(sizeof(vertices[0])* vertices.size(),
    (void*)vertices.data(), vertexBuffer_);

    //index
    Vulkantool::createIndexBuffer(sizeof(indices[0])* indices.size(),
    (void*)indices.data(), indexBuffer_);

    //uniform buffer
    createUniformBuffers();
}

void MainPass::createDescriptorSetLayout(){
    // Image sampler descriptor
    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.setBinding(0)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                    .setPImmutableSamplers(nullptr);
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.setBindingCount(1)
              .setPBindings(&uboLayoutBinding)
              .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);
    descriptorSetLayouts_.resize(1);
    descriptorSetLayouts_[0] = Context::Instance().device.createDescriptorSetLayout(layoutInfo);

}
void MainPass::createPipelineLayouts(){
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayoutCount(descriptorSetLayouts_.size())
                      .setPSetLayouts(descriptorSetLayouts_.data());
    pipelineLayouts_.resize(1);
    pipelineLayouts_[0] = Context::Instance().device.createPipelineLayout(pipelineLayoutInfo);

}

void MainPass::createUniformBuffers(){
    auto& Vctx = VulkanRhi::Instance();
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers_.resize(Vctx.getMaxFlightCount());

    for(int i = 0; i < Vctx.getMaxFlightCount(); ++i){
        Vulkantool::createBuffer(bufferSize,vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffers_[i]);
    }
}

void MainPass::updateUniformBuffer(uint32_t currentImage){
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    Vulkantool::updateBuffer(uniformBuffers_[currentImage], &ubo, sizeof(ubo));
}


void MainPass::CreatePiepline(){
    auto& ctx = Context::Instance();
    temporarilyInit(); //temporary init vertex buffer

     // 0. shader prepare
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/vert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/frag.spv", vk::ShaderStageFlagBits::eFragment)
    };

    //1. vertex input   BindingDescription And AttributeDescription
    vk::VertexInputBindingDescription vertex_binding_desc{};
    vertex_binding_desc.setBinding(0)
                      .setStride(sizeof(Vertex))
                      .setInputRate(vk::VertexInputRate::eVertex);
    std::array<vk::VertexInputAttributeDescription, 2> vertex_attr_descs;
    vertex_attr_descs[0].setBinding(0)
                        .setLocation(0)
                        .setFormat(vk::Format::eR32G32Sfloat)
                        .setOffset(offsetof(Vertex, pos));
    vertex_attr_descs[1].setBinding(0)
                        .setLocation(1)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(Vertex, color));

    vertex_input_ci.setVertexAttributeDescriptionCount(vertex_attr_descs.size())
                   .setPVertexAttributeDescriptions(vertex_attr_descs.data())
                   .setVertexBindingDescriptionCount(1)
                   .setPVertexBindingDescriptions(&vertex_binding_desc);

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
             .setFrontFace(vk::FrontFace::eCounterClockwise)
             .setDepthBiasEnable(VK_FALSE);
    //5. multisampling
    multisample_ci.setSampleShadingEnable(VK_FALSE)
                  .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                  .setMinSampleShading(1.0f)
                  .setPSampleMask(nullptr)
                  .setAlphaToCoverageEnable(VK_FALSE)
                  .setAlphaToOneEnable(VK_FALSE);
    //6. depth and stencil buffer
    depth_stencil_ci.setDepthTestEnable(VK_FALSE)
                    .setDepthWriteEnable(VK_TRUE)
                    .setDepthCompareOp(vk::CompareOp::eLess)
                    .setDepthBoundsTestEnable(VK_FALSE)
                    .setStencilTestEnable(VK_FALSE);
    //7. color blending
    colorblendattachment_ci.setBlendEnable(true)
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
    
    //temporary
    updateUniformBuffer(VulkanRhi.getFlightCount());

    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));

    vk::RenderPassBeginInfo renderPassBegin{};
    renderPassBegin.setRenderPass(renderPass_)
                   .setFramebuffer(framebuffers_[VulkanRhi.getImageIndex()])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, Extent));
   
    cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
    
    vk::Buffer vertexBuffers[] = { vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(indexBuffer_.buffer, 0, vk::IndexType::eUint16);

    std::vector<vk::WriteDescriptorSet> desc_writes;
	std::array<vk::DescriptorBufferInfo, 1> desc_buffer_infos{};

    desc_buffer_infos[0].setBuffer(uniformBuffers_[VulkanRhi.getFlightCount()].buffer)
                        .setOffset(0)
                        .setRange(sizeof(UniformBufferObject));
    desc_writes.resize(1);
    desc_writes[0].dstSet = nullptr;
    desc_writes[0].setDstBinding(0)
                  .setDstArrayElement(0)
                  .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                  .setDescriptorCount(1)
                  .setPBufferInfo(desc_buffer_infos.data())
                  .setPImageInfo(nullptr)
                  .setPTexelBufferView(nullptr);

    VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayouts_[0], 0, desc_writes.size(), (VkWriteDescriptorSet *)desc_writes.data());

    cmdBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    //end render pass
    cmdBuffer.endRenderPass();
}


void MainPass::addDescriptorSet(){
    vk::WriteDescriptorSet descriptorWrite{};
    
}


}