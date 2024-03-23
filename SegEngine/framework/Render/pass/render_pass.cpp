#include "render_pass.hpp"

namespace Sego{
    void RenderPass::Init() {
        CreateRenderPass();
        createDescriptorSetLayout();
        createPipelineLayouts();
        CreatePiepline();
        CreateFrameBuffer();
    }
   
    void RenderPass::recreateframbuffer(){
        auto& ctx = Context::Instance();
        for (auto& framebuffer : framebuffers_) {
            ctx.device.destroyFramebuffer(framebuffer);
        }
        framebuffers_.clear();

        width_ = ctx.swapchain->GetExtent().width;
        height_= ctx.swapchain->GetExtent().height;
        CreateFrameBuffer();
    }

    void RenderPass::destroy(){
        auto& ctx = Context::Instance();
        //descriptorSetLayout
        for (auto& descriptorSetLayout : descriptorSetLayouts_) {
            ctx.device.destroyDescriptorSetLayout(descriptorSetLayout);
        }

        //DescriptorPool
        ctx.device.destroyDescriptorPool(descriptorPool_);

        for (auto& framebuffer : framebuffers_) {
            ctx.device.destroyFramebuffer(framebuffer);
        }
        for (auto& pipeline : pipelines_) {
            ctx.device.destroyPipeline(pipeline);
        }
        for (auto& pipelineLayout : pipelineLayouts_) {
            ctx.device.destroyPipelineLayout(pipelineLayout);
        }
        //Renderpass
        ctx.device.destroyRenderPass(renderPass_);
    }

void RenderPass::createDescriptorSetLayout(){

}
void RenderPass::createPipelineLayouts(){

}





}