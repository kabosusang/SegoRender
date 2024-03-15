#include "imgui_init.hpp"
#include "context.hpp"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl2.h"
#include "framework/window/SGwindow.hpp"


ImguiInit::ImguiInit()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;
 
    //ImGui::StyleColorsLight();
    io.Fonts->AddFontFromFileTTF("resources/Fonts/ziti.ttf",18,NULL,io.Fonts->GetGlyphRangesChineseFull());

    //ImGui::GetIO().IniFilename = "../SGData/imgui/imgui.ini"; //ImGui Layout 

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding = 4;
    style.FrameRounding = 4;
    style.GrabRounding = 3;
    style.ScrollbarSize = 7;
    style.ScrollbarRounding = 0;

    ImVec4* colors = style.Colors; 
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.36f, 0.36f, 102.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 171.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 255.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.45f, 0.45f, 0.45f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.55f, 0.55f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.46f, 0.46f, 0.46f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.17f, 0.17f, 0.17f, 0.95f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.78f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.45f, 0.45f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.51f, 0.51f, 0.51f, 0.70f);
	colors[ImGuiCol_Tab] = ImVec4(0.21f, 0.21f, 0.21f, 0.86f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 0.97f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.40f, 0.13f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.45f, 1.00f, 0.85f, 0.35f); 

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Init_Imgui(); //Init Imgui Vulkan Data
    ImGui_ImplSDL2_InitForVulkan(Sego::SGwindow::Instance()->getWindow());

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Sego::Context::Instance().instance;
    init_info.PhysicalDevice = Sego::Context::Instance().phyDevice;
    init_info.Device = Sego::Context::Instance().device;
    init_info.QueueFamily = Sego::Context::Instance().queueInfo.graphicsIndex.value();
    init_info.Queue = Sego::Context::Instance().graphicsQueue;
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = uiDescriptorPool;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, uiRenderPass);

    // Upload the fonts for DearImgui
    {
        Sego::Context::Instance().commandManager->ExecuteCmd(Sego::Context::Instance().graphicsQueue
        , [&](vk::CommandBuffer cmd){
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        Sego::Context::Instance().device.waitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }


}
void ImguiInit::RecoreImgui(int current,uint32_t imageindex)
{
    curFrame_ = current;
    auto& cmd = uiCommandBuffers[current];
    cmd.reset();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd.begin(beginInfo);
    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0, 0, 0, 1}));
    vk::RenderPassBeginInfo renderPassBegin{};

    renderPassBegin.setRenderPass(uiRenderPass)
                   .setFramebuffer(uiFramebuffers[imageindex])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, Sego::Context::Instance().swapchain->GetExtent()));
    cmd.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    // Grab and record the draw data for Dear Imgui
    
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), 
        uiCommandBuffers[current]);
    
    cmd.endRenderPass();
    cmd.end();
}
void ImguiInit::Init_Imgui()
{
    // Create Descriptor Pool
    {
        vk::DescriptorPoolSize poolSizes[] = {
            { vk::DescriptorType::eSampler, 1000 },
            { vk::DescriptorType::eCombinedImageSampler, 1000 },
            { vk::DescriptorType::eSampledImage, 1000 },
            { vk::DescriptorType::eStorageImage, 1000 },
            { vk::DescriptorType::eUniformTexelBuffer, 1000 },
            { vk::DescriptorType::eStorageTexelBuffer, 1000 },
            { vk::DescriptorType::eUniformBuffer, 1000 },
            { vk::DescriptorType::eStorageBuffer, 1000 },
            { vk::DescriptorType::eUniformBufferDynamic, 1000 },
            { vk::DescriptorType::eStorageBufferDynamic, 1000 },
            { vk::DescriptorType::eInputAttachment, 1000 }
        };
            vk::DescriptorPoolCreateInfo poolInfo = {};
            poolInfo.setPoolSizeCount(11)
                    .setPPoolSizes(poolSizes)
                    .setMaxSets(1000 * IM_ARRAYSIZE(poolSizes))
                    .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

        uiDescriptorPool = Sego::Context::Instance().device.createDescriptorPool(poolInfo);
    }

    // Create Render Pass
    {
        vk::AttachmentDescription colorAttachment = {};
        colorAttachment.setFormat(Sego::Context::Instance().swapchain->GetFormat().format)
                        .setSamples(vk::SampleCountFlagBits::e1)
                        .setLoadOp(vk::AttachmentLoadOp::eLoad)
                        .setStoreOp(vk::AttachmentStoreOp::eStore)
                        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                        .setInitialLayout(vk::ImageLayout::ePresentSrcKHR)
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
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.setAttachmentCount(1)
                    .setPAttachments(&colorAttachment)
                    .setSubpassCount(1)
                    .setPSubpasses(&subpass)
                    .setDependencyCount(1)
                    .setPDependencies(&dependency);
        
        uiRenderPass = Sego::Context::Instance().device.createRenderPass(renderPassInfo);
    }

    //create pool
    {
        vk::CommandPoolCreateInfo poolInfo = {};
        poolInfo.setQueueFamilyIndex(Sego::Context::Instance().queueInfo.graphicsIndex.value())
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        uiCommPool = Sego::Context::Instance().device.createCommandPool(poolInfo);
    }

    //create command buffer
    {
        uiCommandBuffers.resize(Sego::Context::Instance().swapchain->images.size());
        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.setCommandPool(uiCommPool)
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount((uint32_t)uiCommandBuffers.size());

        uiCommandBuffers = Sego::Context::Instance().device.allocateCommandBuffers(allocInfo);
    }
    // create frambuffers
    {
        uiFramebuffers.resize(Sego::Context::Instance().swapchain->images.size());
        vk::ImageView attachments[1];
            
        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.setRenderPass(uiRenderPass)
                            .setAttachmentCount(1)
                            .setPAttachments(attachments)
                            .setWidth(Sego::Context::Instance().swapchain->GetExtent().width)
                            .setHeight(Sego::Context::Instance().swapchain->GetExtent().height)
                            .setLayers(1);
        for (size_t i = 0; i < uiFramebuffers.size(); i++) {
            attachments[0] = Sego::Context::Instance().swapchain->images[i].view;
            uiFramebuffers[i] = Sego::Context::Instance().device.createFramebuffer(framebufferInfo);
        }
        
    }
}

void ImguiInit::CleanupSwapChain(){
  
    auto& ctx = Sego::Context::Instance();
    for (auto framebuffer : uiFramebuffers) {
        ctx.device.destroyFramebuffer(framebuffer,nullptr);
    }
    ctx.device.freeCommandBuffers(uiCommPool, uiCommandBuffers);
    
}

void ImguiInit::RecreateSwapChain(){
    CleanupSwapChain();
    ImGui_ImplVulkan_SetMinImageCount(3);
 //create command buffer
    {
        uiCommandBuffers.resize(Sego::Context::Instance().swapchain->images.size());
        
        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.setCommandPool(uiCommPool)
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount((uint32_t)uiCommandBuffers.size());

        uiCommandBuffers = Sego::Context::Instance().device.allocateCommandBuffers(allocInfo);
    }
    // create frambuffers
    {
        uiFramebuffers.resize(Sego::Context::Instance().swapchain->images.size());
        vk::ImageView attachments[1];

        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.setRenderPass(uiRenderPass)
                            .setAttachmentCount(1)
                            .setPAttachments(attachments)
                            .setWidth(Sego::Context::Instance().swapchain->GetExtent().width)
                            .setHeight(Sego::Context::Instance().swapchain->GetExtent().height)
                            .setLayers(1);
        for (size_t i = 0; i < uiFramebuffers.size(); i++) {
            attachments[0] = Sego::Context::Instance().swapchain->images[i].view;
            uiFramebuffers[i] = Sego::Context::Instance().device.createFramebuffer(framebufferInfo);
        }
        
    }

}


ImguiInit::~ImguiInit()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    Sego::Context::Instance().device.destroyDescriptorPool(uiDescriptorPool);
    Sego::Context::Instance().device.destroyRenderPass(uiRenderPass);
    Sego::Context::Instance().device.freeCommandBuffers(uiCommPool, uiCommandBuffers);
    Sego::Context::Instance().device.destroyCommandPool(uiCommPool);
    for (auto framebuffer : uiFramebuffers) {
        Sego::Context::Instance().device.destroyFramebuffer(framebuffer);
    }
}




