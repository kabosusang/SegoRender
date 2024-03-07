#include "toy2d/renderer.hpp"

namespace toy2d {

const std::array<Vertex, 3> vertices = {
    Vertex{0.0, -0.5},
    Vertex{0.5, 0.5},
    Vertex{-0.5, 0.5},
};

const Uniform uniform{ Color{1.0, 1.0, 0.0} };

void Renderer::copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset){
    auto cmdBuf = Context::Instance().commandManager->CreateOneCommandBuffer();

    vk::CommandBufferBeginInfo begin;
    begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuf.begin(begin); {
        vk::BufferCopy region;
        region.setSize(size)
              .setSrcOffset(srcOffset)
              .setDstOffset(dstOffset);
        cmdBuf.copyBuffer(src, dst, region);
    } cmdBuf.end();

    vk::SubmitInfo submit;
    submit.setCommandBuffers(cmdBuf);
    Context::Instance().graphicsQueue.submit(submit);

    Context::Instance().device.waitIdle();

    Context::Instance().commandManager->FreeCmd(cmdBuf);
}

Renderer::Renderer(int maxFlightCount): maxFlightCount_(maxFlightCount), curFrame_(0) {
    createFences();
    createSemaphores();
    createCmdBuffers();
    createVertexBuffer();
    bufferVertexData();
    createUniformBuffers();
    bufferUniformData();
    createDescriptorPool();

    allocateSets();
    updateSets();
}

Renderer::~Renderer() {
    auto& device = Context::Instance().device;
    device.destroyDescriptorPool(descriptorPool_);


    hostuniformBuffers_.clear();
    deviceuniformBuffers_.clear();

    hostVertexBuffer_.reset();
    deviceVertexBuffer_.reset();

    for (auto& sem : imageAvaliableSems_) {
        device.destroySemaphore(sem);
    }
    for (auto& sem : renderFinishSems_) {
        device.destroySemaphore(sem);
    }
    for (auto& fence : fences_) {
        device.destroyFence(fence);
    }
}

void Renderer::DrawTriangle() {
    auto& ctx = Context::Instance();
    auto& device = ctx.device;
    if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<std::uint64_t>::max()) != vk::Result::eSuccess) {
        throw std::runtime_error("wait for fence failed");
    }
    device.resetFences(fences_[curFrame_]);

    auto& swapchain = ctx.swapchain;
    auto resultValue = device.acquireNextImageKHR(swapchain->swapchain, std::numeric_limits<std::uint64_t>::max(), imageAvaliableSems_[curFrame_], nullptr);
    if (resultValue.result != vk::Result::eSuccess) {
        throw std::runtime_error("wait for image in swapchain failed");
    }
    auto imageIndex = resultValue.value;

    auto& cmdMgr = ctx.commandManager;
    cmdBufs_[curFrame_].reset();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBufs_[curFrame_].begin(beginInfo);
    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));
    vk::RenderPassBeginInfo renderPassBegin;
    renderPassBegin.setRenderPass(ctx.renderProcess->renderPass)
                   .setFramebuffer(swapchain->framebuffers[imageIndex])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, swapchain->GetExtent()));
    cmdBufs_[curFrame_].beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipeline);
    vk::DeviceSize offset = 0;
    cmdBufs_[curFrame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->layout, 0, sets_[curFrame_], {});
    cmdBufs_[curFrame_].bindVertexBuffers(0, deviceVertexBuffer_->buffer, offset);
    cmdBufs_[curFrame_].draw(3, 1, 0, 0);
    cmdBufs_[curFrame_].endRenderPass();
    cmdBufs_[curFrame_].end();

    vk::SubmitInfo submit;
    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submit.setCommandBuffers(cmdBufs_[curFrame_])
          .setWaitSemaphores(imageAvaliableSems_[curFrame_])
          .setWaitDstStageMask(flags)
          .setSignalSemaphores(renderFinishSems_[curFrame_]);
    ctx.graphicsQueue.submit(submit, fences_[curFrame_]);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(renderFinishSems_[curFrame_])
               .setSwapchains(swapchain->swapchain)
               .setImageIndices(imageIndex);
    if (ctx.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("present queue execute failed");
    }

    curFrame_ = (curFrame_ + 1) % maxFlightCount_;
}

void Renderer::createFences() {
    fences_.resize(maxFlightCount_, nullptr);

    for (auto& fence : fences_) {
        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        fence = Context::Instance().device.createFence(fenceCreateInfo);
    }
}

void Renderer::createSemaphores() {
    auto& device = Context::Instance().device;
    vk::SemaphoreCreateInfo info;

    imageAvaliableSems_.resize(maxFlightCount_);
    renderFinishSems_.resize(maxFlightCount_);

    for (auto& sem : imageAvaliableSems_) {
        sem = device.createSemaphore(info);
    }

    for (auto& sem : renderFinishSems_) {
        sem = device.createSemaphore(info);
    }
}

void Renderer::createCmdBuffers() {
    cmdBufs_.resize(maxFlightCount_);

    for (auto& cmd : cmdBufs_) {
        cmd = Context::Instance().commandManager->CreateOneCommandBuffer();
    }
}

void Renderer::createVertexBuffer() {
    hostVertexBuffer_.reset(new Buffer(sizeof(vertices),
                                       vk::BufferUsageFlagBits::eTransferSrc, //传输源
                                       vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));//HostCoherent 共享内存
    //如果不使用eHostCoherent(代表内存GPU和CPU不同步) 需要加上下面的代码
    //Context::Instance().device.flushMappedMemoryRanges ; //映射内存 每次修改后需要刷新 从CPU传入GPU
    //Context::Instance().device.invalidateMappedMemoryRanges //读取内存 每次读取前需要刷新
    deviceVertexBuffer_.reset(new Buffer(sizeof(vertices),
                                         vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,
                                         vk::MemoryPropertyFlagBits::eDeviceLocal));
}

void Renderer::bufferVertexData() {
    void* ptr = Context::Instance().device.mapMemory(hostVertexBuffer_->memory, 0, hostVertexBuffer_->size);
        memcpy(ptr, vertices.data(), sizeof(vertices));
    Context::Instance().device.unmapMemory(hostVertexBuffer_->memory);

    copyBuffer(hostVertexBuffer_->buffer, deviceVertexBuffer_->buffer, hostVertexBuffer_->size,0,0);
}

void Renderer::createUniformBuffers() {
    hostuniformBuffers_.resize(maxFlightCount_);
    deviceuniformBuffers_.resize(maxFlightCount_);

    for (int i = 0; i < maxFlightCount_; ++i) {
        hostuniformBuffers_[i].reset(new Buffer(sizeof(Uniform),
                                               vk::BufferUsageFlagBits::eTransferSrc,
                                               vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));
        deviceuniformBuffers_[i].reset(new Buffer(sizeof(Uniform),
                                                 vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,
                                                 vk::MemoryPropertyFlagBits::eDeviceLocal));
    }
}
void Renderer::bufferUniformData(){

    for(int i = 0; i < hostuniformBuffers_.size();i++){
        auto& buffer = hostuniformBuffers_[i];
        void* ptr = Context::Instance().device.mapMemory(buffer->memory, 0, buffer->size);
        memcpy(ptr, &uniform, sizeof(uniform));
        Context::Instance().device.unmapMemory(buffer->memory);
        copyBuffer(buffer->buffer, deviceuniformBuffers_[i]->buffer, buffer->size,0,0);
    }
}

void Renderer::createDescriptorPool(){
    vk::DescriptorPoolCreateInfo createInfo;
    vk::DescriptorPoolSize poolSize;
    poolSize.setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(maxFlightCount_);

    createInfo.setMaxSets(maxFlightCount_)
              .setPoolSizes(poolSize);

    descriptorPool_ = Context::Instance().device.createDescriptorPool(createInfo);
}

void Renderer::allocateSets(){
    std::vector<vk::DescriptorSetLayout> layouts(maxFlightCount_, Context::Instance().renderProcess->setlayout);

    vk::DescriptorSetAllocateInfo allocateInfo;
    allocateInfo.setDescriptorPool(descriptorPool_)
                .setDescriptorSetCount(maxFlightCount_)
                .setSetLayouts(layouts);
    
    sets_ = Context::Instance().device.allocateDescriptorSets(allocateInfo);
}

void Renderer::updateSets(){
    for(int i = 0; i < sets_.size();i++){
        auto& set = sets_[i];

        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.setBuffer(deviceuniformBuffers_[i]->buffer)
                  .setOffset(0)
                  .setRange(deviceuniformBuffers_[i]->size);

        vk::WriteDescriptorSet write;
        write.setDescriptorType(vk::DescriptorType::eUniformBuffer)
             .setBufferInfo(bufferInfo)
             .setDstBinding(0)
             .setDstSet(set)
             .setDstArrayElement(0)
             .setDescriptorCount(1);

        Context::Instance().device.updateDescriptorSets(write, {});
    }


}




}
