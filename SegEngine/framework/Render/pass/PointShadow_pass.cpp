#include "pch.h"
#include "PointShadow_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"

#include <glm/gtc/type_ptr.hpp>
namespace Sego{

PointShadowPass::PointShadowPass(){
    auto& ctx = Context::Instance();
    m_format =  {vk::Format::eR32Sfloat,ctx.swapchain->GetDepthFormat()};
    depthSize_ = 1024; 
}

void PointShadowPass::Init(){
    RenderPass::Init();
}

void PointShadowPass::destroy(){
     auto& ctx = Context::Instance();
    RenderPass::destroy();
   
   for (auto& shadow_cube_ubs: m_shadow_cube_ubss){
        for (VmaBuffer& uniform_buffer : shadow_cube_ubs){
            uniform_buffer.destroy();
        }
   }

    m_depth_image_view_sampler.destroy();
    for (auto& shadow_image_view_sampler : m_shadow_image_view_samplers)
    {
        shadow_image_view_sampler.destroy();
    }
    for (auto& framebuffer : m_framebuffers)
	{
		ctx.device.destroyFramebuffer(framebuffer);
	}

}
const std::vector<VmaImageViewSampler>& PointShadowPass::getShadowImageViewSamplers()
{
    return m_shadow_image_view_samplers;
}

void PointShadowPass::createDescriptorSetLayout(){
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

void PointShadowPass::createPipelineLayouts(){
    pipelineLayouts_.resize(1); //mesh
    //spriter
    push_constant_ranges_ = {
        {vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4)},
         {vk::ShaderStageFlagBits::eFragment,sizeof(glm::mat4),sizeof(glm::vec4)},
    };
    //mesh
     vk::PipelineLayoutCreateInfo pipeline_layout_ci_mesh{};
     pipeline_layout_ci_mesh.setSetLayoutCount(1)
                      .setPSetLayouts(&descriptorSetLayouts_[0])
                      .setPushConstantRangeCount(static_cast<uint32_t>(push_constant_ranges_.size()))
                      .setPPushConstantRanges(push_constant_ranges_.data());

    pipelineLayouts_[0] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci_mesh);
}

void PointShadowPass::CreatePiepline(){
    auto& ctx = Context::Instance(); 
    pipelines_.resize(1); 

    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
     std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/DirShadow/Shadowvert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/PoitShadow/PoitShadowgeom.spv", vk::ShaderStageFlagBits::eGeometry),
        ctx.shaderManager->LoadShader("resources/shaders/PoitShadow/PoitShadowfrag.spv", vk::ShaderStageFlagBits::eFragment)
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
             .setDepthBiasEnable(false);
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
            .setAttachments(colorblendattachment_ci);
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

    for (auto& shader_stage_ci : shader_stage_cis){
        ctx.device.destroyShaderModule(shader_stage_ci.module);
    }

}


void PointShadowPass::CreateFrameBuffer(){
    //ShadowMap_
    Vulkantool::createImageViewSampler(depthSize_,depthSize_,nullptr,1,SHADOW_CASCADE_NUM,m_format[1],
    vk::Filter::eLinear, vk::Filter::eLinear,vk::SamplerAddressMode::eClampToEdge,
    m_depth_image_view_sampler,vk::ImageUsageFlagBits::eDepthStencilAttachment);
}

void PointShadowPass::CreateRenderPass(){
    vk::RenderPassCreateInfo createInfo;
    std::array<vk::AttachmentDescription, 2> attachments{};
    attachments[0].setFormat(m_format[0])
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachments[1].setFormat(m_format[1])
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
    //Depth Ref
    AttachmentRef[1].setAttachment(1)
                     .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
  
    vk::SubpassDescription subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&AttachmentRef[0])
            .setPDepthStencilAttachment(&AttachmentRef[1]);
    
    std::array<vk::SubpassDependency,2> dependencies;
    dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader )
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);               

    dependencies[1].setSrcSubpass(0)
                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput )
                    .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
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

void PointShadowPass::Render(){
    auto& ctx = Context::Instance();
    auto& Vctx = VulkanRhi::Instance();
    for (size_t p = 0; p < m_framebuffers.size(); ++p){
        auto cmdBuffer =Vctx.getCommandBuffer();
        uint32_t flightindex = Vctx.getFlightCount();

        std::array<vk::ClearValue,2> clearValues{};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f } ;
        clearValues[1].setDepthStencil({1.0f,0});
        vk::RenderPassBeginInfo renderPassBegin{};
        renderPassBegin.setRenderPass(renderPass_)
                    .setFramebuffer(m_framebuffers[p])
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
                // push constants
				glm::vec4 light_pos = glm::vec4(m_light_poss[p], 1.0f);
                updatePushConstants(cmdBuffer,pipelineLayouts_[0],{&meshdata->model_,glm::value_ptr(light_pos)});
                // All primitives Renderer
                for (auto node : model.nodes) {
                    renderNode(cmdBuffer,node,p);
                }

        }

        }
        cmdBuffer.endRenderPass();
    }
    renderDatas_.clear();
}

void PointShadowPass::renderNode(vk::CommandBuffer cmdBuffer,GltfModel::Node *node,uint32_t index)
{
    auto& VulkanRhi = VulkanRhi::Instance();
    uint32_t flight_Index = VulkanRhi.getFlightCount();
    
    if (node->mesh){
        // Render mesh primitives
        std::vector<vk::WriteDescriptorSet> desc_write;
        std::array<vk::DescriptorBufferInfo, 3> desc_buffer_infos{};
        addBufferDescriptorSet(desc_write,desc_buffer_infos[0],VulkanRhi.getCurrentUniformBuffer(),0);
        addBufferDescriptorSet(desc_write,desc_buffer_infos[1],node->mesh->uniformBuffer.buffer,1);
        addBufferDescriptorSet(desc_write,desc_buffer_infos[2],m_shadow_cube_ubss[index][flight_Index],2);
        for (GltfModel::Primitive* primitive : node->mesh->primitives){
            std::array<vk::DescriptorImageInfo,1>   desc_image_info{};
            addImageDescriptorSet(desc_write, desc_image_info[0], primitive->material.GetColorImageViewSampler(),3);            
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
			renderNode(cmdBuffer,child,index);
	}
}

void PointShadowPass::createDynamicBuffers(size_t size)
{       auto& VulkanRhi = VulkanRhi::Instance();
		size_t last_size = m_framebuffers.size();
		m_shadow_image_view_samplers.resize(size);
		m_shadow_cube_ubss.resize(size);
		m_framebuffers.resize(size);
		m_light_poss.resize(size);

		for (uint32_t i = last_size; i < size; ++i)
		{
			// create shadow image view sampler
            Vulkantool::createImageViewSampler(depthSize_,depthSize_,nullptr,1,SHADOW_CASCADE_NUM,m_format[0],
            vk::Filter::eLinear, vk::Filter::eLinear,vk::SamplerAddressMode::eClampToEdge,
            m_shadow_image_view_samplers[i],vk::ImageUsageFlagBits::eColorAttachment);
           
			std::vector<vk::ImageView> attachments = {
				m_shadow_image_view_samplers[i].image_view,
				m_depth_image_view_sampler.image_view
			};

			// create framebuffers
			vk::FramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.setRenderPass(renderPass_)
                            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
                            .setPAttachments(attachments.data())
                            .setWidth(depthSize_)
                            .setHeight(depthSize_)
                            .setLayers(SHADOW_CASCADE_NUM);
            m_framebuffers[i] = Context::Instance().device.createFramebuffer(framebufferInfo);

            // create shadow face uniform buffers
            m_shadow_cube_ubss[i].resize(VulkanRhi.getMaxFlightCount());
            for (VmaBuffer& uniform_buffer : m_shadow_cube_ubss[i])
            {
                Vulkantool::createBuffer(sizeof(ShadowCubeUBO), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, uniform_buffer);
            }
        }
}

void PointShadowPass::updateCubes(const std::vector<ShadowCubeCreateInfo>& shadow_cube_cis)
{       
        auto& VulkanRhi = VulkanRhi::Instance();
		createDynamicBuffers(shadow_cube_cis.size());

		for (size_t p = 0; p < shadow_cube_cis.size(); ++p)
		{
			const ShadowCubeCreateInfo& shadow_cube_ci = shadow_cube_cis[p];
			m_light_poss[p] = shadow_cube_ci.light_pos;

			ShadowCubeUBO shadow_cube_ubo;
			glm::mat4 proj = glm::perspectiveRH_ZO(glm::radians(90.0f), 1.0f, shadow_cube_ci.light_near, shadow_cube_ci.light_far);
			for (uint32_t i = 0; i < SHADOW_FACE_NUM; ++i)
			{
				glm::mat4 view = glm::mat4(1.0f);
				switch (i)
				{
				case 0: // POSITIVE_X
					view = glm::rotate(view, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					view = glm::rotate(view, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					break;
				case 1:	// NEGATIVE_X
					view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					view = glm::rotate(view, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					break;
				case 2:	// POSITIVE_Y
					view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					break;
				case 3:	// NEGATIVE_Y
					view = glm::rotate(view, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					break;
				case 4:	// POSITIVE_Z
					view = glm::rotate(view, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					break;
				case 5:	// NEGATIVE_Z
					view = glm::rotate(view, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
					break;
				}

				shadow_cube_ubo.face_view_projs[i] = proj * view * glm::translate(glm::mat4(1.0f), -m_light_poss[p]);
			}

			// update uniform buffers
			VmaBuffer uniform_buffer = m_shadow_cube_ubss[p][VulkanRhi.getFlightCount()];
			Vulkantool::updateBuffer(uniform_buffer, (void*)&shadow_cube_ubo, sizeof(ShadowCubeUBO));
		}
	}







}