#include "main_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Import/gltf_import.hpp"


namespace Sego{

MainPass::MainPass(){
auto& ctx = Context::Instance();
m_formats ={
    vk::Format::eR8G8B8A8Unorm,
    ctx.swapchain->GetDepthFormat()
    };
}

void MainPass::destroy(){
    RenderPass::destroy();
    colorIVs_.destroy();
    depthIVs_.destroy();
}

void MainPass::createDescriptorSetLayout(){
    descriptorSetLayouts_.resize(3);//0 - spriter  1 - static mesh 2 - skybox(cubemap)
    
    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.setBinding(0)
                        .setDescriptorCount(1)
                        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                        .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                        .setPImmutableSamplers(nullptr);

    vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci{};
    //spritor DescriptorSetLayout
    desc_set_layout_ci.setBindingCount(1)
                      .setBindings(samplerLayoutBinding)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);

    descriptorSetLayouts_[0]= Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci);
    //mesh DescriptorSetLayout
    vk::DescriptorSetLayoutBinding samplerLayoutBinding_mesh{};
    samplerLayoutBinding_mesh.setBinding(0)
                        .setDescriptorCount(1)
                        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                        .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                        .setPImmutableSamplers(nullptr);

    vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci_mesh{};
    desc_set_layout_ci_mesh.setBindingCount(1)
                      .setBindings(samplerLayoutBinding_mesh)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);
    descriptorSetLayouts_[1]= Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci_mesh);

    //skybox(cubemap) DescriptorSetLayout
    vk::DescriptorSetLayoutBinding samplerLayoutBinding_cubemap{};
    samplerLayoutBinding_cubemap.setBinding(0)
                        .setDescriptorCount(1)
                        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                        .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                        .setPImmutableSamplers(nullptr);
    
    vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci_cubemap{};
    desc_set_layout_ci_cubemap.setBindingCount(1)
                      .setBindings(samplerLayoutBinding_cubemap)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);
    descriptorSetLayouts_[2]= Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci_cubemap);
}

void MainPass::createPipelineLayouts(){
    pipelineLayouts_.resize(3); // 0 - spriter  1 - static mesh 2 - skybox(cubemap) 
    //spriter renderer
    push_constant_ranges_ = {
        {vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)},
        {vk::ShaderStageFlagBits::eFragment,sizeof(glm::mat4),sizeof(int32_t)}
    };
    
    vk::PipelineLayoutCreateInfo pipeline_layout_ci{};
    pipeline_layout_ci.setSetLayoutCount(1)
                      .setPSetLayouts(&descriptorSetLayouts_[0])
                      .setPushConstantRangeCount(static_cast<uint32_t>(push_constant_ranges_.size()))
                      .setPPushConstantRanges(push_constant_ranges_.data());

    pipelineLayouts_[0] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci);
    //mesh renderer
    
    mesh_push_constant_ranges_ = {
        {vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)},
        {vk::ShaderStageFlagBits::eFragment,sizeof(glm::mat4),sizeof(Material)}
    };
    pipeline_layout_ci.setSetLayoutCount(1)
                      .setPSetLayouts(&descriptorSetLayouts_[1])
                      .setPushConstantRangeCount(static_cast<uint32_t>(mesh_push_constant_ranges_.size()))
                      .setPPushConstantRanges(mesh_push_constant_ranges_.data());

    pipelineLayouts_[1] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci);

    //skybox renderer
    cubmap_push_constant_ranges_ = {
        {vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)}
    };
     pipeline_layout_ci.setSetLayoutCount(1)
                      .setPSetLayouts(&descriptorSetLayouts_[2])
                      .setPushConstantRangeCount(static_cast<uint32_t>(cubmap_push_constant_ranges_.size()))
                      .setPPushConstantRanges(cubmap_push_constant_ranges_.data());

    pipelineLayouts_[2] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci);

}

void MainPass::CreatePiepline(){
    auto& ctx = Context::Instance();
    pipelines_.resize(3); //Sprite Renderer, Mesh Renderer,cubemap Renderer

    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //Sprite Renderer
    // 0. shader prepare
     std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/Sprite/Spritevert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/Sprite/Spritefrag.spv", vk::ShaderStageFlagBits::eFragment)
    };
    

    //1. vertex input   BindingDescription And AttributeDescription
    vk::VertexInputBindingDescription vertex_binding_desc{};
    vertex_binding_desc.setBinding(0)
                      .setStride(sizeof(SpriteVertex))
                      .setInputRate(vk::VertexInputRate::eVertex);
    
    std::array<vk::VertexInputAttributeDescription, 3> vertex_attr_descs_Sprite;
    vertex_attr_descs_Sprite[0].setBinding(0)
                                .setLocation(0)
                                .setFormat(vk::Format::eR32G32B32Sfloat)
                                .setOffset(offsetof(SpriteVertex, pos));
    vertex_attr_descs_Sprite[1].setBinding(0)
                                .setLocation(1)
                                .setFormat(vk::Format::eR32G32B32A32Sfloat)
                                .setOffset(offsetof(SpriteVertex, color));
    vertex_attr_descs_Sprite[2].setBinding(0)
                                .setLocation(2)
                                .setFormat(vk::Format::eR32G32Sfloat)
                                .setOffset(offsetof(SpriteVertex, uv));

    vertex_input_ci.setVertexAttributeDescriptionCount(vertex_attr_descs_Sprite.size())
                   .setPVertexAttributeDescriptions(vertex_attr_descs_Sprite.data())
                   .setVertexBindingDescriptionCount(1)
                   .setPVertexBindingDescriptions(&vertex_binding_desc);

    //2. input assembly 
    input_assemb_ci.setTopology(vk::PrimitiveTopology::eTriangleList) //Triangle
                  .setPrimitiveRestartEnable(VK_FALSE);

    //3. viewport and scissor
    viewport_ci.setX(0.0f).setY(0.0f).setWidth(width_)
                .setHeight(height_)
                .setMinDepth(0.0f).setMaxDepth(1.0f);
    scissor_ci.setOffset({0,0})
              .setExtent({width_,height_});
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
    
    auto Result = ctx.device.createGraphicsPipeline(nullptr, pipeline_ci);
    pipelines_[0] = Result.value;
    
    //10. destroy shader module
    for(auto& shader_stage_ci : shader_stage_cis){
        ctx.device.destroyShaderModule(shader_stage_ci.module);
    }

    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //Mesh Renderer
    shader_stage_cis.clear();
    shader_stage_cis = {
        ctx.shaderManager->LoadShader("resources/shaders/Mesh/meshvert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/Mesh/meshfrag.spv", vk::ShaderStageFlagBits::eFragment)
    };

    //1. vertex input   BindingDescription And AttributeDescription
    vk::VertexInputBindingDescription Meshvertex_binding_desc{};
    Meshvertex_binding_desc.setBinding(0)
                      .setStride(sizeof(StaticVertex))
                      .setInputRate(vk::VertexInputRate::eVertex);
    
    std::array<vk::VertexInputAttributeDescription, 4> vertex_attr_descs_Mesh;
    vertex_attr_descs_Mesh[0].setBinding(0)
                            .setLocation(0)
                            .setFormat(vk::Format::eR32G32B32Sfloat)
                            .setOffset(offsetof(StaticVertex, pos));
    vertex_attr_descs_Mesh[1].setBinding(0)
                            .setLocation(1)
                            .setFormat(vk::Format::eR32G32B32Sfloat)
                            .setOffset(offsetof(StaticVertex, normal));
    vertex_attr_descs_Mesh[2].setBinding(0)
                            .setLocation(2)
                            .setFormat(vk::Format::eR32G32Sfloat)
                            .setOffset(offsetof(StaticVertex, uv));
    vertex_attr_descs_Mesh[3].setBinding(0)
                        .setLocation(3)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(StaticVertex, color));

    vertex_input_ci.setVertexAttributeDescriptionCount(vertex_attr_descs_Mesh.size())
                   .setPVertexAttributeDescriptions(vertex_attr_descs_Mesh.data())
                   .setVertexBindingDescriptionCount(1)
                   .setPVertexBindingDescriptions(&Meshvertex_binding_desc);

    raster_ci.setCullMode(vk::CullModeFlagBits::eBack);
            
    pipeline_ci.setStages(shader_stage_cis)
               .setPVertexInputState(&vertex_input_ci)
               .setPInputAssemblyState(&input_assemb_ci)
               .setPViewportState(&viewport_state_ci)
               .setPRasterizationState(&raster_ci)
               .setPDynamicState(&dynamicState)
               .setPMultisampleState(&multisample_ci)
               .setPDepthStencilState(&depth_stencil_ci)
               .setPColorBlendState(&blend_ci)
               .setLayout(pipelineLayouts_[1])
               .setRenderPass(renderPass_);

    Result = ctx.device.createGraphicsPipeline(nullptr, pipeline_ci);
    pipelines_[1] = Result.value;

    //10. destroy shader module
    for(auto& shader_stage_ci : shader_stage_cis){
        ctx.device.destroyShaderModule(shader_stage_ci.module);
    }

    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    // Skybox Renderer
    shader_stage_cis.clear();
    shader_stage_cis = {
        ctx.shaderManager->LoadShader("resources/shaders/cubemap/Cubmapvert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/cubemap/Cubmapfrag.spv", vk::ShaderStageFlagBits::eFragment)
    };

    raster_ci.setCullMode(vk::CullModeFlagBits::eFront);

    depth_stencil_ci.setDepthTestEnable(false)
                    .setDepthWriteEnable(false);
    
    colorblendattachment_ci.setBlendEnable(false);

    pipeline_ci.setStages(shader_stage_cis)
               .setPVertexInputState(&vertex_input_ci)
               .setPInputAssemblyState(&input_assemb_ci)
               .setPViewportState(&viewport_state_ci)
               .setPRasterizationState(&raster_ci)
               .setPDynamicState(&dynamicState)
               .setPMultisampleState(&multisample_ci)
               .setPDepthStencilState(&depth_stencil_ci)
               .setPColorBlendState(&blend_ci)
               .setLayout(pipelineLayouts_[2])
               .setRenderPass(renderPass_);
    Result = ctx.device.createGraphicsPipeline(nullptr, pipeline_ci);
    pipelines_[2] = Result.value;

    //10. destroy shader module
    for(auto& shader_stage_ci : shader_stage_cis){
        ctx.device.destroyShaderModule(shader_stage_ci.module);
    }
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    // TODO: 11. create pipeline cache


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

    std::vector<vk::ImageView> attachments = { 
    colorIVs_.image_view ,
    depthIVs_.image_view
    };

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
                  .setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);

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
    
    std::array<vk::ClearValue,2> clearValues{};
    clearValues[0].setColor(vk::ClearColorValue(reinterpret_cast<vk::ClearColorValue&>(clearColor_)));
    clearValues[1].setDepthStencil({1.0f,0});

    vk::RenderPassBeginInfo renderPassBegin{};
    renderPassBegin.setRenderPass(renderPass_)
                   .setFramebuffer(framebuffer_)
                   .setClearValues(clearValues)
                   .setRenderArea(vk::Rect2D({}, {width_,height_}));
   
    cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
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

    if (skybox_){
        render_skybox(cmdBuffer);
    }

    //Render 
    for(const auto& Rendata : renderDatas_){
        if (Rendata->type == RenderDataType::Sprite){
            std::shared_ptr<SpriteRenderData> spritedata = std::static_pointer_cast<SpriteRenderData>(Rendata);
            render_sprite(cmdBuffer,spritedata);
        }

        if (Rendata->type == RenderDataType::StaticMesh){
            std::shared_ptr<StaticMeshRenderData> meshdata = std::static_pointer_cast<StaticMeshRenderData>(Rendata);
            render_mesh(cmdBuffer,meshdata);
        }
    }

    cmdBuffer.endRenderPass();
}

void MainPass::render_skybox(vk::CommandBuffer cmdBuffer){
    auto& VulkanRhi = VulkanRhi::Instance();
    //pipelines_[2] Skybox Renderer
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[2]);
    vk::Buffer vertexBuffers[] = { skybox_->vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(skybox_->indexBuffer_.buffer, 0, vk::IndexType::eUint32);

     //Draw Notes
    for(auto& node : skybox_->nodes_){
        drawNode_cubemap(cmdBuffer,pipelineLayouts_[2],node);
    }

}


void MainPass::render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<StaticMeshRenderData>& Rendata){
    auto& VulkanRhi = VulkanRhi::Instance();
    
    //pipelines_[0] Normal GLTF Model Renderer
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[1]);
    vk::Buffer vertexBuffers[] = { Rendata->vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(Rendata->indexBuffer_.buffer, 0, vk::IndexType::eUint32);

    desc_writes.clear();
   // SG_CORE_INFO("Load Model Texture Size: {0}",Rendata->textures_.size());
    //Draw Notes
    for(auto& node : Rendata->nodes_){
        drawNode(cmdBuffer,pipelineLayouts_[1],node,Rendata);
    }
}

//2. Sprite Renderer
void MainPass::render_sprite(vk::CommandBuffer cmdBuffer,std::shared_ptr<SpriteRenderData> Rendata){
    auto& VulkanRhi = VulkanRhi::Instance();
    
    //pipelines_[1] Sprite Renderer
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
    vk::Buffer vertexBuffers[] = { Rendata->vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(Rendata->indexBuffer_.buffer, 0, vk::IndexType::eUint32);

    desc_writes.clear();
    //1. Uniform
  
    updatePushConstants(cmdBuffer,pipelineLayouts_[0],{&Rendata->Spritemvp_,&Rendata->UseTex});
    //2. Image Sample TODO: SpriteRendererComponent
    std::array<vk::DescriptorImageInfo,1>   desc_image_info{};   //Sample
    if (!Rendata->UseTex){
        addImageDescriptorSet(desc_writes, desc_image_info[0], 
        VulkanRhi.defaultTexture->image_view_sampler_,0); //defualt image use depth image(a kidding)
    }else{
        addImageDescriptorSet(desc_writes, desc_image_info[0], 
        Rendata->Spritetexture->image_view_sampler_,0);
    }
    
    VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayouts_[0], 0, static_cast<uint32_t>(desc_writes.size()), (VkWriteDescriptorSet *)desc_writes.data());

    //Draw
    cmdBuffer.drawIndexed(Rendata->indexCount_,1,0,0,0);
}


void MainPass::drawNode(vk::CommandBuffer cmdBuffer , vk::PipelineLayout pipelineLayout, Node* node,std::shared_ptr<StaticMeshRenderData>& Rendata){
    auto& VulkanRhi = VulkanRhi::Instance();
    if(node->mesh.primitives.size() > 0){
        
        glm::mat4 nodeMatrix = node->matrix;
        Node* currentParent = node->parent;
        while (currentParent) {
            nodeMatrix = currentParent->matrix * nodeMatrix;
            currentParent = currentParent->parent;
        }
        
        // Pass the final matrix to the vertex shader using push constants
        nodeMatrix = Rendata->Meshmvp_ * nodeMatrix ;
        
        for ( auto& primitive : node->mesh.primitives) {
            updatePushConstants(cmdBuffer,pipelineLayout,{&nodeMatrix,
            &Rendata->materials_[primitive.materialIndex]},mesh_push_constant_ranges_);
            desc_writes.clear();
				if (primitive.indexCount > 0) {

                    std::array<vk::DescriptorImageInfo,1>   desc_image_info{};  
                    if (Rendata->materials_[primitive.materialIndex].has_baseColorTexture){
                        //Sample
                        addImageDescriptorSet(desc_writes, desc_image_info[0], 
                        Rendata->textures_[Rendata->materials_[primitive.materialIndex].baseColorTextureIndex].image_view_sampler_,0);
                        VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, static_cast<uint32_t>(desc_writes.size()), (VkWriteDescriptorSet *)desc_writes.data());
                    }else{
                        //Sample
                        addImageDescriptorSet(desc_writes, desc_image_info[0], 
                        VulkanRhi.defaultTexture->image_view_sampler_,0); //defualt image use depth image(a kidding)
                        VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, static_cast<uint32_t>(desc_writes.size()), (VkWriteDescriptorSet *)desc_writes.data());
                    }
                  
                    // Bind the descriptor for the current primitive's texture
                    cmdBuffer.drawIndexed(primitive.indexCount,1,primitive.firstIndex,0,0);
				}
			}
		}
		for (auto& child : node->children) {
			drawNode(cmdBuffer,pipelineLayout, child,Rendata);
		}
}

void MainPass::drawNode_cubemap(vk::CommandBuffer cmdBuffer , vk::PipelineLayout pipelineLayout,Node* node){
    auto& VulkanRhi = VulkanRhi::Instance();
    if(node->mesh.primitives.size() > 0){
        
        glm::mat4 nodeMatrix = node->matrix;
        Node* currentParent = node->parent;
        while (currentParent) {
            nodeMatrix = currentParent->matrix * nodeMatrix;
            currentParent = currentParent->parent;
        }
        
        // Pass the final matrix to the vertex shader using push constants
        nodeMatrix = skybox_->Meshmvp_;
        for ( auto& primitive : node->mesh.primitives) {
        updatePushConstants(cmdBuffer,pipelineLayout,{&nodeMatrix},cubmap_push_constant_ranges_);
            desc_writes.clear();
				if (primitive.indexCount > 0) {
                    std::array<vk::DescriptorImageInfo,1>   desc_image_info = {};  
                    if (skybox_->materials_[primitive.materialIndex].has_baseColorTexture){
                        //Sample
                        addImageDescriptorSet(desc_writes, desc_image_info[0], 
                        VulkanRhi.defaultSkybox->image_view_sampler_,0);
                    }else{
                        //Sample
                        addImageDescriptorSet(desc_writes, desc_image_info[0], 
                        VulkanRhi.defaultSkybox->image_view_sampler_,0); //defualt image use depth image(a kidding)
                    }
                  
				    VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, static_cast<uint32_t>(desc_writes.size()), (VkWriteDescriptorSet *)desc_writes.data());

                    // Bind the descriptor for the current primitive's texture
                    cmdBuffer.drawIndexed(primitive.indexCount,1,primitive.firstIndex,0,0);
				}
			}
		}
		for (auto& child : node->children) {
			drawNode_cubemap(cmdBuffer,pipelineLayout,child);
		}
}


void MainPass::recreateframbuffer(uint32_t width,uint32_t height){
    auto& ctx = Context::Instance();
    ctx.device.waitIdle();
    ctx.device.destroyFramebuffer(framebuffer_);
    depthIVs_.destroy();
    colorIVs_.destroy();

    width_ = width;
    height_= height;
    CreateFrameBuffer();
    
}







   
}



