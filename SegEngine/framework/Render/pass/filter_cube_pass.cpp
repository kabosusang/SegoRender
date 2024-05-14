#include "pch.h"
#include "filter_cube_pass.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/Texture2D.hpp"
#include "resource/asset/Import/gltf_import.hpp"
#include "Core/Base/Time.hpp"

#define PI 3.1415926


namespace Sego{

    struct IrradiancePCO
	{
		glm::mat4 mvp;
		float delta_phi;
		float delta_theta;
	};

	struct PrefilterPCO
	{
		glm::mat4 mvp;
		float roughness;
		uint32_t samples;
	};

FilterCubePass::FilterCubePass(std::shared_ptr<class TextureCube>& skybox_texture){
    m_format[0] =  vk::Format::eR32G32B32A32Sfloat;
    m_format[1] =  vk::Format::eR16G16B16A16Sfloat;
  
    m_sizes[0] = 64;
    m_sizes[1] = 512;
    for (int i = 0; i < 2; ++i)
    {
        m_mip_levels[i] = static_cast<uint32_t>(std::floor(std::log2(std::max(m_sizes[i], m_sizes[i])))) + 1;
    }
    
    skytexture_ = skybox_texture;
    skybox_ = std::make_shared<SkyboxRenderData>();
    skybox_ = std::static_pointer_cast<SkyboxRenderData>(GlTFImporter::LoadglTFFile("resources/Settings/skybox/cube.gltf"));
}

void FilterCubePass::Init(){
    RenderPass::Init();
}

void FilterCubePass::destroy(){
    auto& ctx = Context::Instance();
    RenderPass::destroy();
    for (auto& view : m_color_image_views){
        view.destroy();
    }
    for (auto& sampler : m_cube_image_view_samplers){
        sampler.destroy();
    }
    for (auto& pass : m_render_passes){
        ctx.device.destroyRenderPass(pass);
    }
    for (auto& frame : m_framebuffers){
        ctx.device.destroyFramebuffer(frame);
    }
}

void FilterCubePass::createDescriptorSetLayout(){
    descriptorSetLayouts_.resize(2);

    //irradianc
    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.setBinding(0)
                        .setDescriptorCount(1)
                        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                        .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                        .setPImmutableSamplers(nullptr);
    vk::DescriptorSetLayoutCreateInfo desc_set_layout_ci{};
    desc_set_layout_ci.setBindingCount(1)
                      .setBindings(samplerLayoutBinding)
                      .setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR);

    descriptorSetLayouts_[0] = Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci);
    //prefilter
    descriptorSetLayouts_[1] = Context::Instance().device.createDescriptorSetLayout(desc_set_layout_ci);
}

void FilterCubePass::createPipelineLayouts(){
    pipelineLayouts_.resize(2); 

    for (uint32_t i = 0; i < 2; i++){
        EFilterType FilterType = (EFilterType)i;

        vk::PushConstantRange push_constant_range{};
        push_constant_range.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

        switch (FilterType)
        {
        case EFilterType::Irradiance:
            push_constant_range.setSize(sizeof(IrradiancePCO));
            break;
        
        case EFilterType::Prefilter:
            push_constant_range.setSize(sizeof(PrefilterPCO));
            break;
        default:
            break;
        }

        vk::PipelineLayoutCreateInfo pipeline_layout_ci{};
        pipeline_layout_ci.setSetLayoutCount(1)
                        .setPSetLayouts(&descriptorSetLayouts_[i])
                        .setPushConstantRangeCount(1)
                        .setPPushConstantRanges(&push_constant_range);
        pipelineLayouts_[i] = Context::Instance().device.createPipelineLayout(pipeline_layout_ci);
    }
}

void FilterCubePass::CreatePiepline(){
    auto& ctx = Context::Instance(); 
    pipelines_.resize(2); 

    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    // 0. shader prepare
     std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis  ={
        ctx.shaderManager->LoadShader("resources/shaders/Pbr/filtercubevert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/Pbr/irradiancefrag.spv", vk::ShaderStageFlagBits::eFragment)
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

    //2. input assembly 
    input_assemb_ci.setTopology(vk::PrimitiveTopology::eTriangleList) //Triangle
                  .setPrimitiveRestartEnable(VK_FALSE);

    //3. viewport and scissor
    viewport_ci.setX(0.0f).setY(0.0f).setWidth(m_sizes[0])
                .setHeight(m_sizes[0])
                .setMinDepth(0.0f).setMaxDepth(1.0f);
    scissor_ci.setOffset({0,0})
              .setExtent({m_sizes[0],m_sizes[0]});
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
               .setRenderPass(m_render_passes[0]);
    
    auto Result = ctx.device.createGraphicsPipeline(pipelineCache_, pipeline_ci);
    pipelines_[0] = Result.value;

     //10. destroy shader module
    for(auto& shader_stage_ci : shader_stage_cis){
        ctx.device.destroyShaderModule(shader_stage_ci.module);
    }

    //Fre
    shader_stage_cis.clear();
    shader_stage_cis = {
        ctx.shaderManager->LoadShader("resources/shaders/Pbr/filtercubevert.spv", vk::ShaderStageFlagBits::eVertex),
        ctx.shaderManager->LoadShader("resources/shaders/Pbr/meshpick.spv", vk::ShaderStageFlagBits::eFragment)
    };

    viewport_ci.setX(0.0f).setY(0.0f).setWidth(m_sizes[1])
                .setHeight(m_sizes[1])
                .setMinDepth(0.0f).setMaxDepth(1.0f);
    scissor_ci.setOffset({0,0})
              .setExtent({m_sizes[1],m_sizes[1]});
    viewport_state_ci.setViewports(viewport_ci)
                     .setScissors(scissor_ci);

    //pipeline
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
               .setRenderPass(m_render_passes[1]);
    
    Result = ctx.device.createGraphicsPipeline(pipelineCache_, pipeline_ci);
    pipelines_[1] = Result.value;

     //10. destroy shader module
    for(auto& shader_stage_ci : shader_stage_cis){
        ctx.device.destroyShaderModule(shader_stage_ci.module);
    }
}


void FilterCubePass::CreateFrameBuffer(){
    auto& ctx = Context::Instance();
    for (uint32_t i = 0; i < 2 ; i++){
        EFilterType FilterType = (EFilterType)i;

        uint32_t width = m_sizes[i];
        uint32_t height = m_sizes[i];

        // 1.Color images and views
        Vulkantool::createImageAndView(width,height,1,1,vk::SampleCountFlagBits::e1,
        m_format[i],vk::ImageTiling::eOptimal,vk::ImageUsageFlagBits::eColorAttachment |vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,vk::ImageAspectFlagBits::eColor,m_color_image_views[i]);

         //2.create cubemao images and views
        Vulkantool::createImageViewSampler(width,height,nullptr,m_mip_levels[i],6
        ,m_format[i],vk::Filter::eLinear,vk::Filter::eLinear,vk::SamplerAddressMode::eClampToEdge,
        m_cube_image_view_samplers[i],vk::ImageUsageFlagBits::eTransferDst);

         //create framebuffers
        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.setRenderPass(m_render_passes[i])
                        .setAttachmentCount(1)
                        .setPAttachments(&m_color_image_views[i].image_view)
                        .setWidth(width)
                        .setHeight(height)
                        .setLayers(1);
        m_framebuffers[i] = Context::Instance().device.createFramebuffer(framebufferInfo);

    }
}

void FilterCubePass::CreateRenderPass(){

    for (uint32_t i = 0; i < 2; i++){
        EFilterType FilterType = (EFilterType)i;

        //color attachement
        vk::AttachmentDescription colorattachment{};
        colorattachment.setFormat(m_format[i])
                        .setSamples(vk::SampleCountFlagBits::e1)
                        .setLoadOp(vk::AttachmentLoadOp::eClear)
                        .setStoreOp(vk::AttachmentStoreOp::eStore)
                        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                        .setInitialLayout(vk::ImageLayout::eUndefined)
                        .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference AttachmentRef;
        AttachmentRef.setAttachment(0)
                     .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass{};
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachmentCount(1)
                .setPColorAttachments(&AttachmentRef);

        std::array<vk::SubpassDependency,1> dependencies;
        dependencies[0].setSrcSubpass(0)
                        .setDstSubpass(VK_SUBPASS_EXTERNAL)
                        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                        .setDependencyFlags(vk::DependencyFlagBits::eByRegion);

        vk::RenderPassCreateInfo createInfo;
        createInfo.setAttachmentCount(1)
                .setPAttachments(&colorattachment)
                .setSubpassCount(1)
                .setPSubpasses(&subpass)
                .setDependencyCount(static_cast<uint32_t>(dependencies.size()))
                .setPDependencies(dependencies.data());

        m_render_passes[i] = Context::Instance().device.createRenderPass(createInfo);
    }
}

void FilterCubePass::Render(){
    auto& ctx = Context::Instance();
    auto& Vctx = VulkanRhi::Instance();
    StopWatch stop_watch;
    stop_watch.start();

    for (uint32_t i = 0; i < 2;i++){ 
        EFilterType FilterType = (EFilterType)i;
        uint32_t width = m_sizes[i];
        uint32_t height = m_sizes[i];

        std::array<vk::ClearValue,1> clearValues{};
        clearValues[0].setColor({0.0f,0.0f,0.0f,1.0f});
        vk::RenderPassBeginInfo renderPassBegin{};
        renderPassBegin.setRenderPass(m_render_passes[i])
                   .setFramebuffer(m_framebuffers[i])
                   .setClearValues(clearValues)
                   .setRenderArea(vk::Rect2D({}, {width,height}));
   
        std::vector<glm::mat4> view_matrices = 
        {
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };
         vk::Viewport viewport{};
        viewport.setWidth(static_cast<float>(width))
                .setHeight(static_cast<float>(height))
                .setMinDepth(0.0f)
                .setMaxDepth(1.0f);
        vk::Rect2D scissor{};
        scissor.setOffset({0, 0})
               .setExtent({width,height});

        vk::Image cube_image = m_cube_image_view_samplers[i].image();
        Vulkantool::transitionImageLayout(cube_image,vk::ImageLayout::eUndefined,vk::ImageLayout::eTransferDstOptimal,m_format[i],m_mip_levels[i],6);
        for (uint32_t f = 0; f < 6; f++){
            
            for (uint32_t m = 0; m < m_mip_levels[i]; m++){
                auto cmdBuffer =Vulkantool::beginSingleCommands();
                cmdBuffer.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
                viewport.width = static_cast<float>(m_sizes[i] >> m);
                viewport.height = viewport.width;
                cmdBuffer.setViewport(0, 1, &viewport);
                cmdBuffer.setScissor(0, 1, &scissor);

                glm::mat4 mvp = glm::perspectiveRH_ZO((float)PI / 2.0f, 1.0f, 0.1f, 512.0f) * view_matrices[f];
                switch (FilterType)
                {
                case EFilterType::Irradiance:
                {
                    IrradiancePCO irradiance_pco;
                    irradiance_pco.mvp = mvp;
                    irradiance_pco.delta_phi = PI / 90.0f;
                    irradiance_pco.delta_theta = PI / 180.0f;
                    cmdBuffer.pushConstants(pipelineLayouts_[i],vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,0,
                    sizeof(IrradiancePCO),&irradiance_pco);
                }
                break;
                case EFilterType::Prefilter:
                {
                    PrefilterPCO prefilter_pco;
                    prefilter_pco.mvp = mvp;
                    prefilter_pco.roughness = (float)m / (float)(m_mip_levels[i] - 1);
                    prefilter_pco.samples = 32;
                    cmdBuffer.pushConstants(pipelineLayouts_[i],vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,0,
                    sizeof(PrefilterPCO),&prefilter_pco);
                }
                break;
                default:
                    break;
                }
                // 
                std::vector<vk::WriteDescriptorSet> desc_writes;
                std::array<vk::DescriptorImageInfo,1>   desc_image_info{};
                addImageDescriptorSet(desc_writes, desc_image_info[0], 
                skytexture_->image_view_sampler_,0);
                
                Vctx.getCmdPushDescriptorSet()(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayouts_[i], 0, static_cast<uint32_t>(desc_writes.size()), (VkWriteDescriptorSet *)desc_writes.data());
                //bind pipeline
                cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[i]);
                // draw indexed mesh
                //Draw Notes
                for(auto& node : skybox_->nodes_){
                    drawNode(cmdBuffer,pipelineLayouts_[i],node);
                }
                cmdBuffer.endRenderPass();
                Vulkantool::endInstantCommands(cmdBuffer);

                // transition frambuffer texture to transfer src optimal
                vk::Image color_image = m_color_image_views[i].image();
                Vulkantool::transitionImageLayout(color_image,vk::ImageLayout::eColorAttachmentOptimal,vk::ImageLayout::eTransferSrcOptimal);

                //copy frambuffer texture to cube map texture
                vk::ImageCopy image_copy{};
                image_copy.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                image_copy.srcSubresource.baseArrayLayer = 0;
                image_copy.srcSubresource.mipLevel = 0;
                image_copy.srcSubresource.layerCount = 1;
                image_copy.setSrcOffset({0,0,0});

                image_copy.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                image_copy.dstSubresource.baseArrayLayer = f;
                image_copy.dstSubresource.mipLevel = m;
                image_copy.dstSubresource.layerCount = 1;
                image_copy.setDstOffset({0,0,0}); 

                image_copy.extent.width = static_cast<uint32_t>(viewport.width);
                image_copy.extent.height = static_cast<uint32_t>(viewport.height);
                image_copy.extent.depth = 1;

                auto cmd = Vulkantool::beginSingleCommands();
                cmd.copyImage(color_image,vk::ImageLayout::eTransferSrcOptimal,
                cube_image,vk::ImageLayout::eTransferDstOptimal,1,&image_copy);
                Vulkantool::endInstantCommands(cmd);

                //transition frambuffer texture to color attachment optimal
                Vulkantool::transitionImageLayout(color_image,vk::ImageLayout::eTransferSrcOptimal,vk::ImageLayout::eColorAttachmentOptimal);
            }
        }
        //transition cube image to read only optimal
        Vulkantool::transitionImageLayout(cube_image,vk::ImageLayout::eTransferDstOptimal,vk::ImageLayout::eShaderReadOnlyOptimal,m_format[i],m_mip_levels[i],6);
    }
    SG_CORE_INFO("filter cube pass elapsed time: {}ms",stop_watch.stopMs());
}

void FilterCubePass::drawNode(vk::CommandBuffer cmdBuffer , vk::PipelineLayout pipelineLayout, Node* node){
     auto& VulkanRhi = VulkanRhi::Instance();

    if(node->mesh.primitives.size() > 0){
        // Pass the node's matrix via push constants
        // Traverse the node hierarchy to the top-most parent to get the final matrix of the current node

         for ( auto& primitive : node->mesh.primitives) {
                // Update the push constant block
				if (primitive.indexCount > 0) {
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