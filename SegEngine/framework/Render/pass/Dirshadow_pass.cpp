#include "pch.h"
#include "Dirshadow_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"


namespace Sego{

DirShadowPass::DirShadowPass(){
    auto& ctx = Context::Instance();
    m_format =  ctx.swapchain->GetDepthFormat();
    depthSize_ = 2048; 
    m_cascade_split_lambda = 0.95f;
}

void DirShadowPass::Init(){
    RenderPass::Init();

    shadowconstans_.resize(SHADOW_CASCADE_NUM);
    for(auto& buffer : shadowconstans_){
        Vulkantool::createBuffer(sizeof(ShadowCascadeUBO),vk::BufferUsageFlagBits::eUniformBuffer,VMA_MEMORY_USAGE_AUTO_PREFER_HOST,buffer);
    }
}

void DirShadowPass::destroy(){
    RenderPass::destroy();
    ShadowMap_.destroy();
    shadowconstans_.resize(SHADOW_CASCADE_NUM);
    for(auto& buffer : shadowconstans_){
       buffer.destroy();
    }
}

void DirShadowPass::createDescriptorSetLayout(){
    descriptorSetLayouts_.resize(1);

    //mesh DescriptorSetLayout
    std::vector<vk::DescriptorSetLayoutBinding> bindings_mesh = {
        vk::DescriptorSetLayoutBinding(0,vk::DescriptorType::eUniformBuffer,1,vk::ShaderStageFlagBits::eVertex),
        vk::DescriptorSetLayoutBinding(1,vk::DescriptorType::eUniformBuffer,1,vk::ShaderStageFlagBits::eVertex),
        vk::DescriptorSetLayoutBinding(2,vk::DescriptorType::eUniformBuffer,1,vk::ShaderStageFlagBits::eGeometry),
        vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
    };

    vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci_mesh{};
    desc_set_layout_ci_mesh.setBindingCount(static_cast<uint32_t>(bindings_mesh.size()))
                      .setBindings(bindings_mesh)
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
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis = {
        ctx.shaderManager->LoadShader("resources/shaders/DirShadow/Shadowvert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/DirShadow/shadowmapgeom.spv", vk::ShaderStageFlagBits::eGeometry),
        ctx.shaderManager->LoadShader("resources/shaders/DirShadow/shadowmapfrag.spv", vk::ShaderStageFlagBits::eFragment)
    };
     
    //1. vertex input   BindingDescription And AttributeDescription
    vk::VertexInputBindingDescription vertex_binding_desc{};
    vertex_binding_desc.setBinding(0)
                      .setStride(sizeof(MeshAndSkeletonVertex))
                      .setInputRate(vk::VertexInputRate::eVertex);
    
    std::array<vk::VertexInputAttributeDescription, 7> vertex_attr_descs_shadow;
    vertex_attr_descs_shadow[0].setBinding(0)
                        .setLocation(0)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(MeshAndSkeletonVertex, pos));
    vertex_attr_descs_shadow[1].setBinding(0)
                        .setLocation(1)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(MeshAndSkeletonVertex, normal));
    vertex_attr_descs_shadow[2].setBinding(0)
                        .setLocation(2)
                        .setFormat(vk::Format::eR32G32Sfloat)
                        .setOffset(offsetof(MeshAndSkeletonVertex, uv0));
    vertex_attr_descs_shadow[3].setBinding(0)
                        .setLocation(3)
                        .setFormat(vk::Format::eR32G32Sfloat)
                        .setOffset(offsetof(MeshAndSkeletonVertex, uv1));
    vertex_attr_descs_shadow[4].setBinding(0)
                        .setLocation(4)
                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                        .setOffset(offsetof(MeshAndSkeletonVertex, joint0));
    vertex_attr_descs_shadow[5].setBinding(0)
                        .setLocation(5)
                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                        .setOffset(offsetof(MeshAndSkeletonVertex, weight0));
    vertex_attr_descs_shadow[6].setBinding(0)
                        .setLocation(6)
                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                        .setOffset(offsetof(MeshAndSkeletonVertex, color));

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
             .setCullMode(vk::CullModeFlagBits::eBack)
             .setFrontFace(vk::FrontFace::eCounterClockwise)
             .setDepthBiasEnable(true);
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
    
    auto Result = ctx.device.createGraphicsPipeline(pipelineCache_, pipeline_ci);
    pipelines_[0] = Result.value;
}


void DirShadowPass::CreateFrameBuffer(){
    auto& ctx = Context::Instance();
    //ShadowMap_
    Vulkantool::createImageViewSampler(depthSize_,depthSize_,nullptr,1,SHADOW_CASCADE_NUM,m_format,
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
                    .setLayers(SHADOW_CASCADE_NUM);
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

    dependencies[1].setSrcSubpass(0)
                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
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
    uint32_t flightindex = Vctx.getFlightCount();

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
    static float depthBiasConstant = 1.25f;
    static float depthBiasSlope = 7.5f;
    cmdBuffer.setDepthBias(depthBiasConstant,0.0f,depthBiasSlope);


    for(const auto& Rendata : renderDatas_){
        if (Rendata->type == RenderDataType::PbrAndSkeletomMesh){
            
            vk::DeviceSize offsets[1] = { 0 };
            std::shared_ptr<PbrMeshRenderData> meshdata = std::static_pointer_cast<PbrMeshRenderData>(Rendata);
            GltfModel::Model& model = *(meshdata->model);
            cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[0]);
            cmdBuffer.bindVertexBuffers(0,1, &model.vertexBuffer_.buffer,offsets);
            if (model.indexBuffer_.buffer!= VK_NULL_HANDLE){
                cmdBuffer.bindIndexBuffer(model.indexBuffer_.buffer, 0, vk::IndexType::eUint32);
            }
            updatePushConstants(cmdBuffer,pipelineLayouts_[0],{&meshdata->model_});
            // All primitives Renderer
            for (auto node : model.nodes) {
                renderNode(cmdBuffer,node);
            }

       }

    }
    cmdBuffer.endRenderPass();
    renderDatas_.clear();
}

void DirShadowPass::renderNode(vk::CommandBuffer cmdBuffer,GltfModel::Node *node)
{
    auto& VulkanRhi = VulkanRhi::Instance();
    uint32_t flight_Index = VulkanRhi.getFlightCount();
    
    if (node->mesh){
        // Render mesh primitives
        std::vector<vk::WriteDescriptorSet> desc_write;
        std::array<vk::DescriptorBufferInfo, 3> desc_buffer_infos{};
        std::array<vk::DescriptorImageInfo,1>   desc_image_info{};
        addBufferDescriptorSet(desc_write,desc_buffer_infos[0],VulkanRhi.getCurrentUniformBuffer(),0);
        addBufferDescriptorSet(desc_write,desc_buffer_infos[1],node->mesh->uniformBuffer.buffer,1);
        addBufferDescriptorSet(desc_write,desc_buffer_infos[2],shadowconstans_[flight_Index],2);
        
        for (GltfModel::Primitive* primitive : node->mesh->primitives){
            if(primitive->material.baseColorTexture != nullptr){
                addImageDescriptorSet(desc_write, desc_image_info[0], primitive->material.baseColorTexture->image_view_sampler_,3);            
            }else{
                addImageDescriptorSet(desc_write, desc_image_info[0], VulkanRhi.defaultTexture->image_view_sampler_,3);            
            }
            VulkanRhi.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayouts_[0], 0, static_cast<uint32_t>(desc_write.size()), (VkWriteDescriptorSet *)desc_write.data());

            if (primitive->hasIndices) {
                    cmdBuffer.drawIndexed(primitive->indexCount, 1, primitive->firstIndex, 0, 0);
                } else {
                    cmdBuffer.draw(primitive->vertexCount, 1, 0, 0);
                }
        }

    }
    for (auto child : node->children) {
			renderNode(cmdBuffer,child);
	}
}

void DirShadowPass::updateCascades(ShadowCascadeCreateInfo &shadow_cascade_ci)
{
        float cascade_splits[SHADOW_CASCADE_NUM];

		float near = shadow_cascade_ci.camera_near;
		float far = shadow_cascade_ci.camera_far;
		float cascade_frustum_near = shadow_cascade_ci.light_cascade_frustum_near;
		float range = far - near;

		float min_z = near;
		float max_z = near + range;

		float range_z = max_z - min_z;
		float ratio = max_z / min_z;

		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < SHADOW_CASCADE_NUM; ++i) 
		{
			float p = (i + 1) / static_cast<float>(SHADOW_CASCADE_NUM);
			float log = min_z * std::pow(ratio, p);
			float uniform = min_z + range_z * p;
			float d = m_cascade_split_lambda * (log - uniform) + uniform;
			cascade_splits[i] = (d - near) / range;
		}

		// Calculate orthographic projection matrix for each cascade
		float last_cascade_split = 0.0f;
		for (uint32_t c = 0; c < SHADOW_CASCADE_NUM; ++c)
		{
			float cascade_split = cascade_splits[c];
			glm::vec3 frustum_corners[8] =
			{
				glm::vec3(-1.0f, 1.0f, 0.0f),
				glm::vec3(1.0f, 1.0f, 0.0f),
				glm::vec3(1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f, 1.0f, 1.0f),
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec3(1.0f, -1.0f, 1.0f),
				glm::vec3(-1.0f, -1.0f, 1.0f)
			};

			// Project frustum corners into world space
			for (uint32_t i = 0; i < 8; ++i)
			{
				glm::vec4 inv_frustum_corner = shadow_cascade_ci.inv_camera_view_proj * glm::vec4(frustum_corners[i], 1.0f);
				frustum_corners[i] = inv_frustum_corner / inv_frustum_corner.w;
			}

			for (uint32_t i = 0; i < 4; ++i)
			{
				glm::vec3 dist = frustum_corners[i + 4] - frustum_corners[i];
				frustum_corners[i + 4] = frustum_corners[i] + (dist * cascade_split);
				frustum_corners[i] = frustum_corners[i] + (dist * last_cascade_split);
			}

			// Get frustum center
			glm::vec3 frustum_center = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; ++i) 
			{
				frustum_center += frustum_corners[i];
			}
			frustum_center /= 8.0f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; ++i) 
			{
				float distance = glm::length(frustum_corners[i] - frustum_center);
				radius = std::max(radius, distance);
			}

			glm::mat4 light_view = glm::lookAtRH(frustum_center - shadow_cascade_ci.light_dir * radius, frustum_center, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 light_proj = glm::orthoRH_ZO(-radius, radius, -radius, radius, cascade_frustum_near, radius * 2.0f);
			light_proj[1][1] *= -1.0f;

			// Store split distance and matrix in cascade
			m_shadow_cascade_ubo.cascade_view_projs[c] = light_proj * light_view;
			m_cascade_splits[c] = -(near + cascade_split * range);

			last_cascade_split = cascade_split;
		}

		// update uniform buffers
		VmaBuffer uniform_buffer = shadowconstans_[VulkanRhi::Instance().getFlightCount()];
		Vulkantool::updateBuffer(uniform_buffer, (void*)&m_shadow_cascade_ubo, sizeof(ShadowCascadeUBO));
}

}