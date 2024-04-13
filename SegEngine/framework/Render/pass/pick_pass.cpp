#include "pch.h"
#include "pick_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Import/gltf_import.hpp"

namespace Sego{
PickPass::PickPass(){
auto& ctx = Context::Instance();
    m_formats = {
        vk::Format::eR8G8B8A8Unorm,
        ctx.swapchain->GetDepthFormat()
    };
}
void PickPass::destroy(){
    RenderPass::destroy();
    EntityIV_.destroy();
    DepthIV_.destroy();
}

void PickPass::createDescriptorSetLayout(){
    descriptorSetLayouts_.resize(2);
    vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci{};
   
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

    // mesh
    desc_set_layout_ci.setBindingCount(2)
                      .setBindings(bindings)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);
    
    descriptorSetLayouts_[0] = Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci);

    //spritor
    desc_set_layout_ci.setBindingCount(1)
                      .setBindings(uboLayoutBinding)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);

    descriptorSetLayouts_[1]= Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci);
}

void PickPass::createPipelineLayouts(){
    pipelineLayouts_.resize(2);
    push_constant_ranges_ = {
        {vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)}
    };
    
    vk::PipelineLayoutCreateInfo pipeline_layout_ci{};
    pipeline_layout_ci.setSetLayoutCount(1)
                      .setPSetLayouts(&descriptorSetLayouts_[0])
                      .setPushConstantRangeCount(static_cast<uint32_t>(push_constant_ranges_.size()))
                      .setPPushConstantRanges(push_constant_ranges_.data());

    //1 Normal GLTF Model Renderer
    pipelineLayouts_[0] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci);

     pipeline_layout_ci.setSetLayoutCount(1)
                      .setPSetLayouts(&descriptorSetLayouts_[1])
                      .setPushConstantRangeCount(static_cast<uint32_t>(push_constant_ranges_.size()))
                      .setPPushConstantRanges(push_constant_ranges_.data());

    pipelineLayouts_[1] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci);

}

void PickPass::CreatePiepline(){
    auto& ctx = Context::Instance();
    pipelines_.resize(2);
    
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
    
    std::array<vk::VertexInputAttributeDescription, 4> vertex_attr_descs;
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
    vertex_attr_descs[3].setBinding(0)
                        .setLocation(3)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(StaticVertex, color));

    vertex_input_ci.setVertexAttributeDescriptionCount(vertex_attr_descs.size())
                   .setPVertexAttributeDescriptions(vertex_attr_descs.data())
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
    
    auto Result = ctx.device.createGraphicsPipeline(nullptr, pipeline_ci);
    pipelines_[0] = Result.value;


    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //Sprite Renderer
    shader_stage_cis.clear();
    shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/Sprite/Spritevert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/pick/pick.spv", vk::ShaderStageFlagBits::eFragment)
    };


    //1. vertex input   BindingDescription And AttributeDescription
    vertex_binding_desc.setBinding(0)
                      .setStride(sizeof(SpriteVertex))
                      .setInputRate(vk::VertexInputRate::eVertex);
    
    std::array<vk::VertexInputAttributeDescription, 2> vertex_attr_descs_Sprite;
    vertex_attr_descs_Sprite[0].setBinding(0)
                        .setLocation(0)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(SpriteVertex, pos));
    vertex_attr_descs_Sprite[1].setBinding(0)
                        .setLocation(1)
                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                        .setOffset(offsetof(SpriteVertex, color));

    vertex_input_ci.setVertexAttributeDescriptionCount(vertex_attr_descs_Sprite.size())
                   .setPVertexAttributeDescriptions(vertex_attr_descs_Sprite.data())
                   .setVertexBindingDescriptionCount(1)
                   .setPVertexBindingDescriptions(&vertex_binding_desc);
    //color blend
    colorblendattachment_ci.setBlendEnable(true);
    blend_ci.setAttachments(colorblendattachment_ci)    
            .setLogicOpEnable(false);
    

    raster_ci.setCullMode(vk::CullModeFlagBits::eNone);
    
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

}


void PickPass::CreateFrameBuffer(){
    auto& ctx = Context::Instance();
    //EntityIV_
    Vulkantool::createImageAndView(width_,height_,1,1,vk::SampleCountFlagBits::e1,
    m_formats[0],vk::ImageTiling::eOptimal,vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,vk::ImageAspectFlagBits::eColor,EntityIV_);
   //DepthIV_
    Vulkantool::createImageAndView(width_,height_,1,1,vk::SampleCountFlagBits::e1,
    m_formats[1],vk::ImageTiling::eOptimal,vk::ImageUsageFlagBits::eDepthStencilAttachment,
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,vk::ImageAspectFlagBits::eDepth,DepthIV_);
   
    std::vector<vk::ImageView> attachments = { 
    EntityIV_.image_view ,
    DepthIV_.image_view
    };

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(renderPass_)
                    .setAttachmentCount(2)
                    .setPAttachments(attachments.data())
                    .setWidth(width_)
                    .setHeight(height_)
                    .setLayers(1);
    framebuffer_ = Context::Instance().device.createFramebuffer(framebufferInfo);
}

void PickPass::CreateRenderPass(){
    vk::RenderPassCreateInfo createInfo;
    std::array<vk::AttachmentDescription, 2> attachments{};
    attachments[0].setFormat(m_formats[0])
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
    attachments[1].setFormat(m_formats[1])
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
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

    vk::SubpassDescription subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&AttachmentRef[0])
            .setPDepthStencilAttachment(&AttachmentRef[1]);
    
    createInfo.setAttachmentCount(attachments.size())
              .setPAttachments(attachments.data())
              .setSubpassCount(1)
              .setPSubpasses(&subpass)
              .setDependencyCount(0)
              .setPDependencies(nullptr);

    renderPass_ = Context::Instance().device.createRenderPass(createInfo);
}

void PickPass::Render(){
    auto& ctx = Context::Instance();
    auto& Vctx = VulkanRhi::Instance();

    auto cmdBuffer =Vctx.getCommandBuffer();
   
    std::array<vk::ClearValue,2> clearValues{};
    clearValues[0].setColor({0.0f,0.0f,0.0f,1.0f});
    clearValues[1].setDepthStencil({1.0f,0});

    vk::RenderPassBeginInfo renderPassBegin{};
    renderPassBegin.setRenderPass(renderPass_)
                   .setFramebuffer(framebuffer_)
                   .setClearValues(clearValues)
                   .setRenderArea(vk::Rect2D({}, {width_,height_}));
   
    cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    vk::Viewport viewport{};
    viewport.setWidth(static_cast<float>(width_))
            .setHeight(static_cast<float>(height_))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
    cmdBuffer.setViewport(0, 1, &viewport);
    vk::Rect2D scissor{};
    scissor.setOffset({0, 0})
           .setExtent({width_,height_});
    cmdBuffer.setScissor(0, 1, &scissor);

    //pipelines_[0] Normal GLTF Model Renderer
   uint32_t entity_index = 0;
    for(const auto& Rendata : renderDatas_){
      
       if (Rendata->type == RenderDataType::Sprite){
            std::shared_ptr<SpriteRenderData> spritedata = std::static_pointer_cast<SpriteRenderData>(Rendata);
            render_sprite(cmdBuffer,spritedata);
       }
    }
    cmdBuffer.endRenderPass();

    std::vector<uint8_t> image_data;
    Vulkantool::readImagePixel(EntityIV_.image(),width_,height_,m_formats[0],image_data);
    
    SelectEntity = decodeEntityID(&image_data[(m_mouse_y * width_ + m_mouse_x) * 4]);
 
}
 
uint32_t PickPass::ReadPixelInt(uint32_t mouse_x, uint32_t mouse_y)
{
    m_mouse_x = (uint32_t)(mouse_x * scale_ratio_);
    m_mouse_y = (uint32_t)(mouse_y * scale_ratio_);
    m_mouse_x > width_ ? width_ : m_mouse_x;
    m_mouse_y > height_ ? height_ : m_mouse_y;

    return SelectEntity;
}


uint32_t PickPass::decodeEntityID(const uint8_t *color)
{   
    uint32_t id;
    id = color[0] + (color[1] << 8) + (color[2] << 16) - 1;
    return id;
}

#define MAX_SIZE 124u
void PickPass::recreateframbuffer(uint32_t width, uint32_t height){
   
   if (width > height)
		{
			width_ = std::min(width, MAX_SIZE);
			height_ = width_ * height / width;
			scale_ratio_ = (float)width_ / width;
		}
		else
		{
			height_ = std::min(height, MAX_SIZE);
			width_ = height_ * width / height;
			scale_ratio_ = (float)height_ / height;
		}
	CreateFrameBuffer();
}



void PickPass::render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<MeshRenderData> Rendata){
    auto& VulkanRhi = VulkanRhi::Instance();
    
    //pipelines_[0] Normal GLTF Model Renderer
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
    vk::Buffer vertexBuffers[] = { Rendata->vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(Rendata->indexBuffer_.buffer, 0, vk::IndexType::eUint32);

    desc_writes.clear();
    //1. Uniform
    std::array<vk::DescriptorBufferInfo, 1> desc_buffer_infos{}; //Uniform 
    addBufferDescriptorSet(desc_writes, desc_buffer_infos[0], 
    VulkanRhi.getCurrentUniformBuffer(), 0);

    //2. Image Sample
    std::array<vk::DescriptorImageInfo,1>   desc_image_info{};   //Sample
    
    addImageDescriptorSet(desc_writes, desc_image_info[0], 
    Rendata->textures_[Rendata->materials_[0].baseColorTextureIndex].image_view_sampler_,1);
    
    //Draw Notes
    for(auto& node : Rendata->nodes_){
        drawNode(cmdBuffer,pipelineLayouts_[0],node);
    }
}

//2. Sprite Renderer
void PickPass::render_sprite(vk::CommandBuffer cmdBuffer,std::shared_ptr<SpriteRenderData> Rendata){
    auto& VulkanRhi = VulkanRhi::Instance();
    
    //pipelines_[1] Sprite Renderer
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[1]);
    vk::Buffer vertexBuffers[] = { Rendata->vertexBuffer_.buffer };
    vk::DeviceSize offsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    cmdBuffer.bindIndexBuffer(Rendata->indexBuffer_.buffer, 0, vk::IndexType::eUint32);

    desc_writes.clear();
    //1. Uniform
    std::array<vk::DescriptorBufferInfo, 1> desc_buffer_infos{}; //Uniform 
    addBufferDescriptorSet(desc_writes, desc_buffer_infos[0], 
    VulkanRhi.getCurrentUniformBuffer(), 0);

    updatePushConstants(cmdBuffer,pipelineLayouts_[1],{&Rendata->Spritemodel});
    //2. Image Sample TODO: SpriteRendererComponent
    
    VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayouts_[1], 0, static_cast<uint32_t>(desc_writes.size()), (VkWriteDescriptorSet *)desc_writes.data());

    //Draw
    cmdBuffer.drawIndexed(Rendata->indexCount_,1,0,0,0);
}


void PickPass::drawNode(vk::CommandBuffer cmdBuffer , vk::PipelineLayout pipelineLayout, Node* node){
     auto& VulkanRhi = VulkanRhi::Instance();

    if(node->mesh.primitives.size() > 0){
        for ( auto& primitive : node->mesh.primitives) {
                // Pass the node's matrix via push constants
                // Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
                glm::mat4 nodeMatrix = node->matrix;
                Node* currentParent = node->parent;
                while (currentParent) {
                    nodeMatrix = currentParent->matrix * nodeMatrix;
                    currentParent = currentParent->parent;
                }
                
                cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &nodeMatrix);
                // Update the push constant block
				if (primitive.indexCount > 0) {
                
					VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayouts_[0], 0, static_cast<uint32_t>(desc_writes.size()), (VkWriteDescriptorSet *)desc_writes.data());
    
                    // Bind the descriptor for the current primitive's texture
                    cmdBuffer.drawIndexed(primitive.indexCount,1,primitive.firstIndex,0,0);
				}
			}
		}
		for (auto& child : node->children) {
			drawNode(cmdBuffer,pipelineLayout, child);
		}
}




}