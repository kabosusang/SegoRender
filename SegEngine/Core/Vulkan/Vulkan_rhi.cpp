#include "Vulkan_rhi.hpp"
#include "Core/Vulkan/VulkanContext.hpp"
#include "Core/Base/Input.hpp" 
#include "framework/Render/Render_data.hpp"
#include "framework/Render/pass/brdf_lut_pass.hpp"

namespace Sego{
VulkanRhi* VulkanRhi::instance_ = nullptr;

VulkanRhi& VulkanRhi::Instance() {
    return *instance_;
}
VulkanRhi::VulkanRhi(uint32_t windowWidth, uint32_t windowHeight){
    auto& ctx = Context::Instance();
    instance_ = this;
    ctx.initSwapchain(windowWidth, windowHeight);
    createCmdPool();
    createCmdBuffers();
    createSemaphoresAndFence();
    loadExtensionFuncs();
    createUniformBuffers();

    defaultTexture = Texture2D::Create("resources/assets/empty.jpg");
    defaultSkybox = TextureCube::Create("resources/Settings/skybox/sky.ktx");

    uiPass_ = std::make_shared<UiPass>();
    mainPass_ = std::make_shared<MainPass>();
    pickPass_ = std::make_shared<PickPass>();
    dirPass_ = std::make_shared<DirShadowPass>();
    pointPass_ = std::make_shared<PointShadowPass>();
    spotPass_ = std::make_shared<SpotShadowPass>();

    m_render_passes = {
        pointPass_,
        spotPass_,
        dirPass_,
        pickPass_,
        mainPass_,
        uiPass_
    };

    for (auto& render_pass : m_render_passes)
    {
        render_pass->Init();
    }
}

void VulkanRhi::Init(std::vector<const char*>& extensions, 
Context::GetSurfaceCallback cb, int windowWidth, int windowHeight) {
    Context::Init(extensions, cb);
    instance_ = new VulkanRhi(windowWidth,windowHeight);
}
void VulkanRhi::destory() {
    Context::Instance().device.waitIdle();
    for (auto& render_pass : m_render_passes){
        render_pass->destroy();
    }
    Context::Instance().device.destroyCommandPool(cmdPool_);
    for (auto& uniform : uniformBuffers_){
        uniform.destroy();
    }

   for (auto& buffer : SceneSettings_){
        buffer.destroy();
    }

    for (auto& semaphore : imageAvailableSemaphores_) {
        Context::Instance().device.destroySemaphore(semaphore);
    }
    for (auto& semaphore : renderFinishedSemaphores_) {
        Context::Instance().device.destroySemaphore(semaphore);
    }
    for (auto& fence : inFlightFences_) {
        Context::Instance().device.destroyFence(fence);
    }


    Context::Quit();

    delete instance_;
    instance_ = nullptr;
    defaultTexture.reset();
    defaultSkybox.reset();
}


void VulkanRhi::createCmdPool() {
    auto& ctx = Context::Instance();
    vk::CommandPoolCreateInfo info;
    info.setQueueFamilyIndex(ctx.queueInfo.graphicsIndex.value());
    info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    cmdPool_ = ctx.device.createCommandPool(info);

    info.setFlags(vk::CommandPoolCreateFlagBits::eTransient);
    singlePool_ = ctx.device.createCommandPool(info);
}

void VulkanRhi::createCmdBuffers() {
    auto& ctx = Context::Instance();
    cmdBuffers_.resize(maxFlightCount_);
    vk::CommandBufferAllocateInfo info;
    info.setCommandPool(cmdPool_);
    info.setLevel(vk::CommandBufferLevel::ePrimary);
    info.setCommandBufferCount(maxFlightCount_);
    
      // 分配命令缓冲区并将其存储到临时变量中
    auto tempCmdBuffers = ctx.device.allocateCommandBuffers(info);

    // 将临时变量中的命令缓冲区赋值给cmdBuffers_数组
    cmdBuffers_.assign(tempCmdBuffers.begin(), tempCmdBuffers.end());

}

void VulkanRhi::createSemaphoresAndFence() {
    auto& ctx = Context::Instance();
 
    //semaphores GPU-GPU
    imageAvailableSemaphores_.resize(maxFlightCount_);
    renderFinishedSemaphores_.resize(maxFlightCount_);
    vk::SemaphoreCreateInfo semaphoreInfo{};

    //fences CPU-GPU
    inFlightFences_.resize(maxFlightCount_);
    vk::FenceCreateInfo fenceInfo{};
    
    for (size_t i = 0; i < maxFlightCount_; i++) {
        imageAvailableSemaphores_[i] = ctx.device.createSemaphore(semaphoreInfo);
        renderFinishedSemaphores_[i] = ctx.device.createSemaphore(semaphoreInfo);
        inFlightFences_[i] = ctx.device.createFence(fenceInfo);
    }
   
}

void VulkanRhi::createUniformBuffers(){
    vk::DeviceSize bufferSize = sizeof(ViewProjs);
    uniformBuffers_.resize(maxFlightCount_);
    for(int i = 0; i < maxFlightCount_; ++i){
        Vulkantool::createBuffer(bufferSize,vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffers_[i]);
    }

     //SceneSetting
    vk::DeviceSize bufferSize_Scene = sizeof(SceneRenderSettings);
    SceneSettings_.resize(maxFlightCount_);
    for(int i = 0; i < maxFlightCount_; ++i){
        Vulkantool::createBuffer(bufferSize_Scene,vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU, SceneSettings_[i]);
    }

}

void VulkanRhi::loadExtensionFuncs(){
    vkCmdPushDescriptorSet_ = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(Context::Instance().device, "vkCmdPushDescriptorSetKHR");
}

//temprory
vk::ImageView VulkanRhi::getColorImageView(){
    return mainPass_->getColorTexture().image_view;
}

VmaImageViewSampler VulkanRhi::getDirShadowMap(){
    return dirPass_->getShadowImageViewSampler();
}


void VulkanRhi::render(){
    waitFrame();
    updtaUniform();
	recordFrame();
	submitFrame();
	presentFrame();
}

void VulkanRhi::waitFrame(){
    auto& ctx = Context::Instance();
    auto result = ctx.device.waitForFences(inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
    ctx.device.resetFences(inFlightFences_[currentFrame_]);

    auto resultValue = ctx.device.acquireNextImageKHR(
        ctx.swapchain->swapchain, 
        UINT64_MAX, imageAvailableSemaphores_[currentFrame_], nullptr);
    if(resultValue.result == vk::Result::eErrorOutOfDateKHR){
        framebufferResized  = true;
        recreateSwapchain();
        return;
    }
    currentImageIndex_ = resultValue.value;
}


void VulkanRhi::updtaUniform(){
    ViewProjs viewProjs;
    viewProjs.view = m_ViewMatrix;
    viewProjs.proj = m_ProjectionMatrix;
    Vulkantool::updateBuffer(uniformBuffers_[currentFrame_], &viewProjs, sizeof(ViewProjs));

    //Scene Data
    Vulkantool::updateBuffer(SceneSettings_[currentFrame_], &SceneRenderData, sizeof(SceneRenderSettings));
}


void VulkanRhi::recordFrame(){
    auto& ctx = Context::Instance();
    auto& queue = ctx.graphicsQueue;
    auto cmdBuffer = cmdBuffers_[currentFrame_];
   
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    cmdBuffer.reset();

    cmdBuffer.begin(beginInfo);
    //record all renderpass
    for (auto& render_pass : m_render_passes){
        render_pass->Render();
    }
    cmdBuffer.end();
    
}

void VulkanRhi::submitFrame(){
    auto& ctx = Context::Instance();
   
    vk::SubmitInfo submit;
    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    submit.setCommandBuffers(cmdBuffers_[currentFrame_])
          .setWaitSemaphores(imageAvailableSemaphores_[currentFrame_])
          .setWaitDstStageMask(flags)
          .setSignalSemaphores(renderFinishedSemaphores_[currentFrame_]);
    ctx.graphicsQueue.submit(submit, inFlightFences_[currentFrame_]);
}

void VulkanRhi::presentFrame(){
    auto& ctx = Context::Instance();
    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(renderFinishedSemaphores_[currentFrame_])
               .setSwapchains(ctx.swapchain->swapchain)
               .setImageIndices(currentImageIndex_);
    vk::Result result = ctx.presentQueue.presentKHR(presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR || 
    result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapchain();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("present queue execute failed");
    }
    currentFrame_ = (currentFrame_ + 1) % maxFlightCount_;
}

//Out Put Function
void VulkanRhi::recreateSwapchain(){
    auto& ctx = Context::Instance();
    ctx.device.waitIdle();
    auto [width, height] = Input::GetWindowSize(); //GetWindowSizeImpl();
    ctx.swapchain->recreateSwapChain(width,height);//Recreate Swapchain
    uiPass_->recreateframbuffer();
}

void VulkanRhi::resizeframbuffer(uint32_t w,uint32_t h){
    mainPass_->recreateframbuffer(w,h);
    pickPass_->recreateframbuffer(w,h);
}

void VulkanRhi::setClearColor(const glm::vec4& color){
    mainPass_->setClearColor(color);
}


 
 











}