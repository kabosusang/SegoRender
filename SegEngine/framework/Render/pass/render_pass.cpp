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
    desc_image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
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
              .setDescriptorCount(textures.size())
              .setPImageInfo(p_desc_image_info);
    desc_writes.push_back(desc_write);

}



}