#include "pch.h"
#include "ui_pass.hpp"

#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl2.h"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/VulkanContext.hpp"
#include "framework/Render/Render_system.hpp"
#include "Core/Log/Log.h"

namespace Sego{

    void UiPass::Init(){
        //setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io.IniFilename = "resources/Settings/imgui.ini";
        //Load imgui setting 
        //ImGui::LoadIniSettingsFromDisk("resources/Settings/imgui.ini");
        //ImGui::SaveIniSettingsToDisk("resources/Settings/imgui.ini");
		// setup Dear ImGui style
        float scale = 1.50f;
        ImGuiStyle& style = ImGui::GetStyle();
        
        float fontSize = 18.0f;
        //style.ScaleAllSizes(scale);
        io.Fonts->AddFontFromFileTTF("resources/Fonts/open-sans/OpenSans-Bold.ttf",fontSize);
        io.Fonts->AddFontFromFileTTF("resources/Fonts/ARLRDBD.TTF",fontSize);
        //io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/Fonts/open-sans/OpenSans-Regular.ttf",fontSize);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/Fonts/arial.ttf",fontSize);
    
		ImGui::StyleColorsDark();
        ImVec4* colors = style.Colors; 

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        setDarkThemeColors();

        auto& ctx = Context::Instance();
        width_ = ctx.swapchain->GetExtent().width;
        height_ = ctx.swapchain->GetExtent().height;

        // create descriptor pool
		createDescriptorPool();
        CreateRenderPass();
        CreateFrameBuffer();
       
        //setup platform/renderer bindings
        ImGui_ImplSDL2_InitForVulkan(VulkanContext::Instance().GetWindowHandle());

        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.Instance = ctx.instance;
        init_info.PhysicalDevice = ctx.phyDevice;
        init_info.Device = ctx.device;
        init_info.QueueFamily = ctx.queueInfo.graphicsIndex.value();
        init_info.Queue = ctx.graphicsQueue;
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = descriptorPool_;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;
        
        bool is_success = ImGui_ImplVulkan_Init(&init_info, renderPass_);
        if(!is_success){
            SG_CORE_ERROR("UiPass Init Failed");
        }
        // add console font
        const float k_base_font_size = 14.0f;

        //UPLOAD FONTS
        {
            VkCommandBuffer command_buffer = Vulkantool::beginSingleCommands();
            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
            Vulkantool::endInstantCommands(command_buffer);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }

        SG_CORE_TRACE("UiPass Init Success");
}

void UiPass::CreatePiepline(){
        
}

void UiPass::destroy(){
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    RenderPass::destroy();
    //Save imgui setting
  
}

void UiPass::createDescriptorPool(){
    const uint32_t maxDescriptorSet = 128;
    vk::DescriptorPoolSize poolSizes[] = {
            {vk::DescriptorType::eSampler, maxDescriptorSet},
        };
            vk::DescriptorPoolCreateInfo poolInfo = {};
            poolInfo.setPoolSizeCount((uint32_t)IM_ARRAYSIZE(poolSizes))
                    .setPPoolSizes(poolSizes)
                    .setMaxSets(maxDescriptorSet)
                    .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        descriptorPool_ = Sego::Context::Instance().device.createDescriptorPool(poolInfo);
}

void UiPass::Render(){
    auto& Vctx = VulkanRhi::Instance();
    auto cmd = Vctx.getCommandBuffer();
    uint32_t image_index = Vctx.getImageIndex();
    //record render pass
    vk::RenderPassBeginInfo render_pass_info{};
    render_pass_info.setFramebuffer(framebuffers_[image_index]);
    render_pass_info.setRenderPass(renderPass_)
                    .setRenderArea({{0, 0}, {width_, height_}});
    vk::ClearValue clear_color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
    render_pass_info.setClearValueCount(1)
                    .setPClearValues(&clear_color);
    
    cmd.beginRenderPass(&render_pass_info, vk::SubpassContents::eInline);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    cmd.endRenderPass();
}

void UiPass::CreateFrameBuffer(){
    auto& ctx = Context::Instance();
    uint32_t image_count = ctx.swapchain->SwapchainImagesAview_.size();
    framebuffers_.resize(image_count);

    vk::ImageView attachments[1];
    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(renderPass_)
                .setAttachmentCount(1)
                .setPAttachments(attachments)
                .setWidth(width_)
                .setHeight(height_) 
                .setLayers(1);
    for (size_t i = 0; i < image_count; i++) {
        attachments[0] = ctx.swapchain->SwapchainImagesAview_[i].view;
        framebuffers_[i] = ctx.device.createFramebuffer(framebufferInfo);
    }
}   

void UiPass::CreateRenderPass(){
    //create render pass
    auto& ctx = Context::Instance();
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.setFormat(Sego::Context::Instance().swapchain->GetFormat().format)
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setLoadOp(vk::AttachmentLoadOp::eClear)
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(vk::ImageLayout::eUndefined)
                    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.setAttachment(0)
                    .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&colorAttachmentRef);

    vk::SubpassDependency dependency = {};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)//vk::PipelineStageFlagBits::eColorAttachmentOutput
            .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.setAttachmentCount(1)
                .setPAttachments(&colorAttachment)
                .setSubpassCount(1)
                .setPSubpasses(&subpass)
                .setDependencyCount(1)
                .setPDependencies(&dependency);
    
    renderPass_ = ctx .device.createRenderPass(renderPassInfo);
}

void UiPass::setDarkThemeColors(){
   auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

    // Headers
    colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Buttons
    colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
    colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
    colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };




}



}