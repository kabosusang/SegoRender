#include "pch.h"
#include "Dirshadow_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Import/gltf_import.hpp"

namespace Sego{

DirShadowPass::DirShadowPass(){
auto& ctx = Context::Instance();
m_format =  ctx.swapchain->GetDepthFormat();
depthSize_ = 1024;  
}

void DirShadowPass::destroy(){
    RenderPass::destroy();
    ShadowMap_.destroy();
}

void DirShadowPass::createDescriptorSetLayout(){
    descriptorSetLayouts_.resize(1);

    //mesh DescriptorSetLayout
     vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci_mesh{};
    desc_set_layout_ci_mesh.setBindingCount(0)
                      .setBindings(nullptr)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);

    descriptorSetLayouts_[0] = Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci_mesh);

}

void DirShadowPass::createPipelineLayouts(){
    pipelineLayouts_.resize(1); //mesh
    //spriter
    push_constant_ranges_ = {
        {vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)}
    };
    
    //mesh
     vk::PipelineLayoutCreateInfo pipeline_layout_ci_mesh{};
     pipeline_layout_ci_mesh.setSetLayoutCount(1)
                      .setPSetLayouts(&descriptorSetLayouts_[0])
                      .setPushConstantRangeCount(static_cast<uint32_t>(push_constant_ranges_.size()))
                      .setPPushConstantRanges(push_constant_ranges_.data());

    pipelineLayouts_[0] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci_mesh);
}

void DirShadowPass::CreatePiepline(){
    auto& ctx = Context::Instance(); 
    pipelines_.resize(1); 

    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //Sprite Renderer
    // 0. shader prepare
     std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/DirShadow/shadowmapvert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/DirShadow/shadowmapfrag.spv", vk::ShaderStageFlagBits::eFragment)
    };
     
    //1. vertex input   BindingDescription And AttributeDescription
    vk::VertexInputBindingDescription vertex_binding_desc{};
    vertex_binding_desc.setBinding(0)
                      .setStride(sizeof(StaticVertex))
                      .setInputRate(vk::VertexInputRate::eVertex);
    
    std::array<vk::VertexInputAttributeDescription, 4> vertex_attr_descs_shadow;
    vertex_attr_descs_shadow[0].setBinding(0)
                        .setLocation(0)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(StaticVertex, pos));
    vertex_attr_descs_shadow[1].setBinding(0)
                        .setLocation(1)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(StaticVertex, normal));
    vertex_attr_descs_shadow[2].setBinding(0)
                        .setLocation(2)
                        .setFormat(vk::Format::eR32G32Sfloat)
                        .setOffset(offsetof(StaticVertex, uv));
    vertex_attr_descs_shadow[3].setBinding(0)
                        .setLocation(3)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(StaticVertex, color));

    vertex_input_ci.setVertexAttributeDescriptionCount(vertex_attr_descs_shadow.size())
                   .setPVertexAttributeDescriptions(vertex_attr_descs_shadow.data())
                   .setVertexBindingDescriptionCount(1)
                   .setPVertexBindingDescriptions(&vertex_binding_desc);

    //2. input assembly 
    input_assemb_ci.setTopology(vk::PrimitiveTopology::eTriangleList) //Triangle
                  .setPrimitiveRestartEnable(VK_FALSE);

    //3. viewport and scissor
    viewport_ci.setX(0.0f).setY(0.0f).setWidth(depthSize_)
                .setHeight(depthSize_)
                .setMinDepth(0.0f).setMaxDepth(1.0f);
    scissor_ci.setOffset({0,0})
              .setExtent({depthSize_,depthSize_});
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
    depth_stencil_ci.setDepthTestEnable(VK_TRUE)
                    .setDepthWriteEnable(VK_TRUE)
                    .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
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
    blend_ci.setLogicOpEnable(false)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(0);
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


void DirShadowPass::CreateFrameBuffer(){
    auto& ctx = Context::Instance();
    //ShadowMap_
    Vulkantool::createImageViewSampler(depthSize_,depthSize_,nullptr,1,1,m_format,
    vk::Filter::eLinear, vk::Filter::eLinear,vk::SamplerAddressMode::eClampToEdge,
    ShadowMap_,vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);


    std::vector<vk::ImageView> attachments = { 
    ShadowMap_.image_view
    };

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(renderPass_)
                    .setAttachmentCount(1)
                    .setPAttachments(attachments.data())
                    .setWidth(depthSize_)
                    .setHeight(depthSize_)
                    .setLayers(1);
    framebuffer_ = Context::Instance().device.createFramebuffer(framebufferInfo);
}

void DirShadowPass::CreateRenderPass(){
    vk::RenderPassCreateInfo createInfo;
    std::array<vk::AttachmentDescription, 1> attachments{};
    attachments[0].setFormat(m_format)
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);
   
   std::array<vk::AttachmentReference,1> AttachmentRef;//color attachment reference
    //Color Ref
    AttachmentRef[0].setAttachment(0)
                     .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
  
    vk::SubpassDescription subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(0)
            .setPDepthStencilAttachment(&AttachmentRef[0]);
    
    std::array<vk::SubpassDependency,2> dependencies;
    dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader )
                    .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                    .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);               

    dependencies[1].setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests )
                    .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                    .setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);               

    createInfo.setAttachmentCount(attachments.size())
              .setPAttachments(attachments.data())
              .setSubpassCount(1)
              .setPSubpasses(&subpass)
              .setDependencyCount(static_cast<uint32_t>(dependencies.size()))
              .setPDependencies(dependencies.data());

    renderPass_ = Context::Instance().device.createRenderPass(createInfo);
}

void DirShadowPass::Render(){
    auto& ctx = Context::Instance();
    auto& Vctx = VulkanRhi::Instance();

    auto cmdBuffer =Vctx.getCommandBuffer();
   
    std::array<vk::ClearValue,1> clearValues{};
    clearValues[0].setDepthStencil({1.0f,0});

    vk::RenderPassBeginInfo renderPassBegin{};
    renderPassBegin.setRenderPass(renderPass_)
                   .setFramebuffer(framebuffer_)
                   .setClearValues(clearValues)
                   .setRenderArea(vk::Rect2D({}, {depthSize_,depthSize_}));
   
    cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    vk::Viewport viewport{};
    viewport.setWidth(static_cast<float>(depthSize_))
            .setHeight(static_cast<float>(depthSize_))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
    cmdBuffer.setViewport(0, 1, &viewport);
    vk::Rect2D scissor{};
    scissor.setOffset({0, 0})
           .setExtent({depthSize_,depthSize_});
    cmdBuffer.setScissor(0, 1, &scissor);
    float depthBiasConstant = 1.25f;
    float depthBiasSlope = 7.5f;
    cmdBuffer.setDepthBias(depthBiasConstant,0.0f,depthBiasSlope);

    for(const auto& Rendata : renderDatas_){
        if (Rendata->type == RenderDataType::StaticMesh){
            std::shared_ptr<StaticMeshRenderData> staticmeshdata = std::static_pointer_cast<StaticMeshRenderData>(Rendata);
            render_mesh(cmdBuffer,staticmeshdata);
       }

    }
    cmdBuffer.endRenderPass();

}

void DirShadowPass::render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<StaticMeshRenderData>& Rendata){
    auto& VulkanRhi = VulkanRhi::Instance();
    
    //pipelines_[0] Normal GLTF Model Renderer
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
    vk::Buffer vertexBuffers[] = { Rendata->vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(Rendata->indexBuffer_.buffer, 0, vk::IndexType::eUint32);
    desc_writes.clear();
    //Draw Notes
    for(auto& node : Rendata->nodes_){
        drawNode(cmdBuffer,pipelineLayouts_[0],node,Rendata);
    }
}

void DirShadowPass::drawNode(vk::CommandBuffer cmdBuffer , vk::PipelineLayout pipelineLayout, Node* node,std::shared_ptr<StaticMeshRenderData>& Rendata){
     auto& VulkanRhi = VulkanRhi::Instance();

    if(node->mesh.primitives.size() > 0){
        // Pass the node's matrix via push constants
        // Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
       
        glm::mat4 nodeMatrix = node->matrix;
        Node* currentParent = node->parent;
        while (currentParent) {
            nodeMatrix = currentParent->matrix * nodeMatrix;
            currentParent = currentParent->parent;
        }
        nodeMatrix = lightvp_ * Rendata->model_ * nodeMatrix;
        
         for ( auto& primitive : node->mesh.primitives) {
            updatePushConstants(cmdBuffer,pipelineLayout,{&nodeMatrix});
                // Update the push constant block
				if (primitive.indexCount > 0) {
                    // Bind the descriptor for the current primitive's texture
                    cmdBuffer.drawIndexed(primitive.indexCount,1,primitive.firstIndex,0,0);
				}
			}
		}
		for (auto& child : node->children) {
			drawNode(cmdBuffer,pipelineLayout, child,Rendata);
		}
}




}