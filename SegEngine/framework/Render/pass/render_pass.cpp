#include "render_pass.hpp"

namespace Sego{
    void RenderPass::Init() {
        auto& ctx = Context::Instance();
        width_ = ctx.swapchain->GetExtent().width;
        height_= ctx.swapchain->GetExtent().height;

        CreateRenderPass();
        createDescriptorSetLayout();
        createPipelineLayouts();
        CreatePiepline();
        CreateFrameBuffer();
    }
   
    void RenderPass::recreateframbuffer(){
        auto& ctx = Context::Instance();
        
        ctx.device.destroyFramebuffer(framebuffer_);

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
        ctx.device.destroyFramebuffer(framebuffer_);
        
        
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

void RenderPass::crearePipelineCache(){
    auto& ctx = Context::Instance();
    vk::PipelineCacheCreateInfo pipeline_cache_ci{};
    pipeline_cache_ci.setInitialDataSize(0);

    pipelineCache_ = ctx.device.createPipelineCache(pipeline_cache_ci);
}


void RenderPass::updatePushConstants(vk::CommandBuffer commandbuffer, 
vk::PipelineLayout pipeline_layout, const std::vector<const void*>&(pcos), std::vector<vk::PushConstantRange> push_constant_ranges){

const std::vector<vk::PushConstantRange>& pcrs = push_constant_ranges.empty() ? 
push_constant_ranges_ : push_constant_ranges;
    for (size_t c = 0; c < pcrs.size(); ++c)
    {
        const vk::PushConstantRange& push_constant_range = pcrs[c];
        commandbuffer.pushConstants(pipeline_layout,push_constant_range.stageFlags
        ,push_constant_range.offset,push_constant_range.size,pcos[c]);
    }
}

void RenderPass::addSsboBufferDescriptorSet(std::vector<vk::WriteDescriptorSet> &desc_writes, vk::DescriptorBufferInfo &desc_buffer_info, VmaBuffer buffer, uint32_t binding)
{
    desc_buffer_info.buffer = buffer.buffer;
    desc_buffer_info.offset = 0;
    desc_buffer_info.range = buffer.size;

    vk::WriteDescriptorSet desc_write{};
    desc_write.setDstSet(nullptr)
              .setDstBinding(binding)
              .setDstArrayElement(0)
              .setDescriptorType(vk::DescriptorType::eStorageBuffer)
              .setDescriptorCount(1)
              .setPBufferInfo(&desc_buffer_info);
    desc_writes.push_back(desc_write);
}


void RenderPass::addBufferDescriptorSet(std::vector<vk::WriteDescriptorSet> &desc_writes, vk::DescriptorBufferInfo &desc_buffer_info, VmaBuffer buffer, uint32_t binding){
    desc_buffer_info.buffer = buffer.buffer;
    desc_buffer_info.offset = 0;
    desc_buffer_info.range = buffer.size;

    vk::WriteDescriptorSet desc_write{};
    desc_write.setDstSet(nullptr)
              .setDstBinding(binding)
              .setDstArrayElement(0)
              .setDescriptorType(vk::DescriptorType::eUniformBuffer)
              .setDescriptorCount(1)
              .setPBufferInfo(&desc_buffer_info);
    desc_writes.push_back(desc_write);
}

void RenderPass::addImageDescriptorSet(std::vector<vk::WriteDescriptorSet> &desc_writes,
 vk::DescriptorImageInfo &desc_image_info, VmaImageViewSampler texture, uint32_t binding){
    desc_image_info.imageLayout = texture.image_layout;
    desc_image_info.imageView = texture.image_view;
    desc_image_info.sampler = texture.sampler;

    vk::WriteDescriptorSet desc_write{};
    desc_write.setDstSet(nullptr)
              .setDstBinding(binding)
              .setDstArrayElement(0)
              .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
              .setDescriptorCount(1)
              .setPImageInfo(&desc_image_info);
    desc_writes.push_back(desc_write);
}

void RenderPass::addDepthImageDescriptorSet(std::vector<vk::WriteDescriptorSet> &desc_writes,
 vk::DescriptorImageInfo &desc_image_info, VmaImageViewSampler texture, uint32_t binding){
    desc_image_info.imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    desc_image_info.imageView = texture.image_view;
    desc_image_info.sampler = texture.sampler;

    vk::WriteDescriptorSet desc_write{};
    desc_write.setDstSet(nullptr)
              .setDstBinding(binding)
              .setDstArrayElement(0)
              .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
              .setDescriptorCount(1)
              .setPImageInfo(&desc_image_info);
    desc_writes.push_back(desc_write);
}



void RenderPass::addImagesDescriptorSet(std::vector<vk::WriteDescriptorSet> &desc_writes, 
vk::DescriptorImageInfo *p_desc_image_info, 
const std::vector<VmaImageViewSampler> &textures, uint32_t binding){
    for(size_t i = 0; i < textures.size(); i++){
        p_desc_image_info[i].imageLayout = textures[i].image_layout;
        p_desc_image_info[i].imageView = textures[i].image_view;
        p_desc_image_info[i].sampler = textures[i].sampler;
    }

    vk::WriteDescriptorSet desc_write{};
    desc_write.setDstSet(nullptr)
              .setDstBinding(binding)
              .setDstArrayElement(0)
              .setDescriptorType(textures.front().descriptor_type)
              .setDescriptorCount(static_cast<uint32_t>(textures.size()))
              .setPImageInfo(p_desc_image_info);
    desc_writes.push_back(desc_write);

}

void RenderPass::setClearColor(const glm::vec4 &color){
    clearColor_ = color;
}







}