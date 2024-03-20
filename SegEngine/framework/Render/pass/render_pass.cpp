#include "render_pass.hpp"

namespace Sego{
    void RenderPass::Init() {
      
        CreateRenderPass();
        CreatePiepline();
        CreateFrameBuffer();
    }
   

  

    void RenderPass::destroy(){
        auto& ctx = Context::Instance();

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




}