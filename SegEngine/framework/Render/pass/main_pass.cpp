#include "main_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

namespace Sego{



MainPass::MainPass(){
auto& ctx = Context::Instance();
m_formats ={
    vk::Format::eR8G8B8A8Srgb,
    ctx.swapchain->GetDepthFormat()
    
    };
}


void MainPass::destroy(){
    RenderPass::destroy();
 

    for(auto& buffer : uniformBuffers_){
        buffer.destroy();
    }
  
    depthIVs_.destroy();
    Rendata->destory();
}

void MainPass::temporarilyInit(){
    auto& Swctx =Context::Instance().swapchain;
    //Rendata = GlTFImporter::LoadglTFFile("resources/gltf/cartoony_rubber_ducky/scene.gltf");
    //Rendata = GlTFImporter::LoadglTFFile("resources/gltf/gg.glb"); success
     //Rendata = GlTFImporter::LoadglTFFile("resources/gltf/gun/scene.gltf");
    //Rendata = GlTFImporter::LoadglTFFile("resources/gltf/Wolf.gltf");
    Rendata = GlTFImporter::LoadglTFFile("resources/gltf/FlightHelmet/FlightHelmet.gltf");

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
    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.setBinding(1)
                        .setDescriptorCount(1)
                        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                        .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                        .setPImmutableSamplers(nullptr);
    
    vk::DescriptorSetLayoutBinding bindings[] = {uboLayoutBinding, samplerLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.setBindingCount(2)
              .setPBindings(bindings)
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
    ubo.proj = glm::perspective(glm::radians(45.0f), 1600.0f / 1000.0f, 0.1f, 10.0f);
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
                      .setStride(sizeof(StaticVertex))
                      .setInputRate(vk::VertexInputRate::eVertex);
    
    std::array<vk::VertexInputAttributeDescription, 3> vertex_attr_descs;
    vertex_attr_descs[0].setBinding(0)
                        .setLocation(0)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(StaticVertex, pos));
    vertex_attr_descs[1].setBinding(0)
                        .setLocation(1)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(StaticVertex, normal));
    vertex_attr_descs[2].setBinding(0)
                        .setLocation(2)
                        .setFormat(vk::Format::eR32G32Sfloat)
                        .setOffset(offsetof(StaticVertex, uv));
 

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
    // dynamic states
    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport,
    vk::DynamicState::eScissor};
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
    depth_stencil_ci.setDepthTestEnable(VK_TRUE)
                    .setDepthWriteEnable(VK_TRUE)
                    .setDepthCompareOp(vk::CompareOp::eLess)
                    .setDepthBoundsTestEnable(VK_FALSE)
                    .setStencilTestEnable(VK_FALSE)
                    .setMinDepthBounds(0.0f)
                    .setMaxDepthBounds(1.0f);

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
               .setPDynamicState(&dynamicState)
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
    //Color Image
    Vulkantool::createImageViewSampler(width_,height_,nullptr,1,1,m_formats[0],
    vk::Filter::eLinear, vk::Filter::eLinear,vk::SamplerAddressMode::eClampToEdge,
    colorIVs_, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);

    //Depth Image
    Vulkantool::createImageViewSampler(width_,height_,nullptr,1,1,m_formats[1],
    vk::Filter::eLinear, vk::Filter::eLinear,vk::SamplerAddressMode::eClampToEdge,
    depthIVs_, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment);

    std::vector<vk::ImageView> attachments = { colorIVs_.image_view ,depthIVs_.image_view};
    vk::FramebufferCreateInfo createInfo;
    createInfo.setAttachments(attachments)
                .setLayers(1)
                .setHeight(height_)
                .setWidth(width_)
                .setRenderPass(renderPass_);
    framebuffer_ = Context::Instance().device.createFramebuffer(createInfo);

}

void MainPass::CreateRenderPass(){
    vk::RenderPassCreateInfo createInfo;
    std::array<vk::AttachmentDescription,2> Attachments; //color attachment
    //Color Attachment
    Attachments[0].setFormat(m_formats[0])
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eStore)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    //Depth Attachment
  
    Attachments[1].setFormat(m_formats[1])
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::array<vk::AttachmentReference,2> AttachmentRef;//color attachment reference
    //Color Ref
    AttachmentRef[0].setAttachment(0)
                     .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    AttachmentRef[1].setAttachment(1)
                     .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
 
    //subpass
    vk::SubpassDescription subpass; //subpass
    vk::SubpassDependency dependency; //subpass dependency
    
    subpass.setColorAttachmentCount(1)
            .setPColorAttachments(&AttachmentRef[0])
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setPDepthStencilAttachment(&AttachmentRef[1]);

    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setDstSubpass(0)
              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests )
              .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests )
              .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    createInfo.setAttachments(Attachments)
              .setSubpasses(subpass)
              .setDependencies(dependency);
            
    renderPass_ = Context::Instance().device.createRenderPass(createInfo);
}

void MainPass::Render(){
    auto& ctx = Context::Instance();
    auto& VulkanRhi = VulkanRhi::Instance();
    auto cmdBuffer = VulkanRhi.getCommandBuffer();
    
    //temporary
    updateUniformBuffer(VulkanRhi.getFlightCount());

    std::array<vk::ClearValue,2> clearValues{};
    clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));
    clearValues[1].setDepthStencil({1.0f,0});

    vk::RenderPassBeginInfo renderPassBegin{};
    renderPassBegin.setRenderPass(renderPass_)
                   .setFramebuffer(framebuffer_)
                   .setClearValues(clearValues)
                   .setRenderArea(vk::Rect2D({}, {width_,height_}));
   
    cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
    
    vk::Viewport viewport{};
    viewport.setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(width_))
            .setHeight(static_cast<float>(height_))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
    cmdBuffer.setViewport(0, 1, &viewport);
    vk::Rect2D scissor{};
    scissor.setOffset({0, 0})
           .setExtent({width_,height_});
    cmdBuffer.setScissor(0, 1, &scissor);

    vk::Buffer vertexBuffers[] = { Rendata->vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(Rendata->indexBuffer_.buffer, 0, vk::IndexType::eUint32);

	std::array<vk::DescriptorBufferInfo, 1> desc_buffer_infos{}; //Uniform 

    addBufferDescriptorSet(desc_writes, desc_buffer_infos[0], 
    uniformBuffers_[VulkanRhi.getFlightCount()], 0);

    //Draw Notes
    for(auto& node : Rendata->nodes_){
        drawNode(pipelineLayouts_[0],node);
    }

    //cmdBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    //end render pass
    cmdBuffer.endRenderPass();
}


void MainPass::drawNode(vk::PipelineLayout pipelineLayout, Node* node){
    //Draw Node
    auto& VulkanRhi = VulkanRhi::Instance();
    auto cmdBuffer = VulkanRhi.getCommandBuffer();

    if(node->mesh.primitives.size() > 0){
        glm::mat4 nodeMatrix = node->matrix;
        Node* currentParent = node->parent;
        while (currentParent) {
            nodeMatrix = currentParent->matrix * nodeMatrix;
            currentParent = currentParent->parent;
        }
        for ( auto& primitive : node->mesh.primitives) {
				if (primitive.indexCount > 0) {
					// Get the texture index for this primitive
                    std::array<vk::DescriptorImageInfo,1>   desc_image_info{};   //Sample

                    addImageDescriptorSet(desc_writes, desc_image_info[0], 
                    Rendata->textures_[Rendata->materials_[primitive.materialIndex].baseColorTextureIndex].image_view_sampler_,1);

                    VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayouts_[0], 0, desc_writes.size(), (VkWriteDescriptorSet *)desc_writes.data());
                    
                    // Bind the descriptor for the current primitive's texture
                    cmdBuffer.drawIndexed(primitive.indexCount,1,primitive.firstIndex,0,0);
				}
			}
		}
		for (auto& child : node->children) {
			drawNode(pipelineLayout, child);
		}
}


void MainPass::recreateframbuffer(uint32_t width,uint32_t height){
    auto& ctx = Context::Instance();
    ctx.device.destroyFramebuffer(framebuffer_);
    depthIVs_.destroy();
    colorIVs_.destroy();

    width_ = width;
    height_= height;
    CreateFrameBuffer();
}
   
}



