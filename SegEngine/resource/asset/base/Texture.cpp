#include "pch.h"
#include "Texture.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include <ktx.h>


namespace Sego{

void Texture::loadKtxTexture(void* p_ktx_texture,vk::Format format){
    ktxTexture* ktx_texture = (ktxTexture*)p_ktx_texture;
    ktx_uint8_t* ktx_data = ktxTexture_GetData(ktx_texture);
    ktx_size_t ktx_texture_size = ktxTexture_GetDataSize(ktx_texture);

    //create a staging buffer
    VmaBuffer staging_buffer;
    Vulkantool::createBuffer(ktx_texture_size,vk::BufferUsageFlagBits::eTransferSrc,VMA_MEMORY_USAGE_AUTO_PREFER_HOST,staging_buffer);

    // copy image pixel data to staging buffer
    Vulkantool::updateBuffer(staging_buffer,ktx_data,ktx_texture_size);

    // create buffer image copy regions
    std::vector<vk::BufferImageCopy> buffer_copy_regions;
    for (uint32_t face = 0 ;face <  layes_; face++ ){
        for (uint32_t mip ; mip < mip_levels_; mip++){
            ktx_size_t offset;
            KTX_error_code result = ktxTexture_GetImageOffset(ktx_texture, mip, 0, face, &offset);
            if (result != KTX_SUCCESS) {
                SG_CORE_ERROR("Failed to get image offset");
            }
            vk::BufferImageCopy buffer_imagecopy{};
            buffer_imagecopy.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, mip, face, 1))
                            .setImageExtent({std::max(width_ >> mip,1u),std::max(height_ >> mip, 1u),1})
                            .setBufferOffset(offset);
            buffer_copy_regions.push_back(buffer_imagecopy);
        }
    }

    // create vulkan texture
    Vulkantool::createImageViewSampler(width_,height_,nullptr,mip_levels_,layes_,format,
    minfilter_,magfilter_,addressmode_u,image_view_sampler_,vk::ImageUsageFlagBits::eTransferDst);

    // transition image layout
    vk::Image vk_image = image_view_sampler_.image();
    Vulkantool::transitionImageLayout(vk_image,vk::ImageLayout::eUndefined,vk::ImageLayout::eTransferDstOptimal,format,mip_levels_,layes_);

    // copy buffer to image
    vk::CommandBuffer cmd = Vulkantool::beginSingleCommands();
    cmd.copyBufferToImage(staging_buffer.buffer,vk_image,vk::ImageLayout::eTransferDstOptimal,buffer_copy_regions);
    Vulkantool::endInstantCommands(cmd);

    //transition image layout to shader read only optimal
    Vulkantool::transitionImageLayout(vk_image,vk::ImageLayout::eTransferDstOptimal,vk::ImageLayout::eShaderReadOnlyOptimal,format,mip_levels_,layes_);
    // clean up staging resources
    ktxTexture_Destroy(ktx_texture);
    staging_buffer.destroy();

}




}