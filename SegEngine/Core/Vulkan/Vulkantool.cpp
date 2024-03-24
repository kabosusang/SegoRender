#include "Vulkantool.hpp"
#include "Vulkan_rhi.hpp"
#include "stb_image.h"


namespace Sego {

void VmaBuffer::destroy(){
   if (buffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(Context::Instance().getAllocator(), buffer, allocation);
    }
}

void VmaImage::destroy(){
    if (image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(Context::Instance().getAllocator(), image, allocation);
    }
}

void VmaImageView::destroy(){
    if (image_view != VK_NULL_HANDLE)
    {
        Context::Instance().device.destroyImageView(image_view);
    }
   
    vma_image.destroy();
    
}

void VmaImageViewSampler::destroy(){
    if (sampler != VK_NULL_HANDLE)
    {
        Context::Instance().device.destroySampler(sampler);
    }
    if (image_view != VK_NULL_HANDLE)
    {
        Context::Instance().device.destroyImageView(image_view);
    }
   
    vma_image.destroy();
}


std::vector<char> ReadWholeFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary|std::ios::ate);

    if (!file.is_open()) {
        std::cout << "read " << filename << " failed" << std::endl;
        return std::vector<char>{};
    }

    auto size = file.tellg();
    std::vector<char> content(size);
    file.seekg(0);

    file.read(content.data(), content.size());

    return content;
}

vk::CommandBuffer Vulkantool::beginSingleCommands(){
    vk::CommandBufferAllocateInfo alloc_info = {};
    alloc_info.setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandPool(VulkanRhi::Instance().getSingleCommandPool())
              .setCommandBufferCount(1);
    vk::CommandBuffer CmdBuffer;
    auto result = Context::Instance().device.allocateCommandBuffers(&alloc_info, &CmdBuffer);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::beginSingleCommands Error");}


    vk::CommandBufferBeginInfo begin_info = {};
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    result = CmdBuffer.begin(&begin_info);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::beginSingleCommands Error");}
    return CmdBuffer;
}

void Vulkantool::endInstantCommands(vk::CommandBuffer command_buffer){
    auto& ctx = Context::Instance();
    
    command_buffer.end();
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1)
               .setPCommandBuffers(&command_buffer);

    vk::FenceCreateInfo fence_ci{};
    vk::Fence fence;

    auto result = ctx.device.createFence(&fence_ci, nullptr, &fence);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::endInstantCommands Error");}
    auto& queue = Context::Instance().graphicsQueue;
    result = queue.submit(1, &submit_info, fence);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::endInstantCommands Error");}
    result = ctx.device.waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::endInstantCommands Error");}
    ctx.device.destroyFence(fence);
    ctx.device.freeCommandBuffers(VulkanRhi::Instance().getSingleCommandPool(), 1, &command_buffer);
}

void Vulkantool::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags buffer_usage, 
VmaMemoryUsage memory_usage, VmaBuffer& buffer){
    auto& ctx = Context::Instance();
    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.setSize(size)
              .setUsage(buffer_usage)
              .setSharingMode(vk::SharingMode::eExclusive);
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memory_usage;
    if (memory_usage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
    {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
   vmaCreateBuffer(ctx.getAllocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&buffer), &buffer.allocation, nullptr);
}

void Vulkantool::copyBuffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size){
    vk::CommandBuffer cmd = beginSingleCommands();

    vk::BufferCopy coyeRegion = {};
    coyeRegion.size = size;
    cmd.copyBuffer(src_buffer,dst_buffer,1,&coyeRegion);
    endInstantCommands(cmd);
}

void Vulkantool::updateBuffer(VmaBuffer& buffer, void* data, size_t size){
    void* mappedData;
    vmaMapMemory(Context::Instance().getAllocator(), buffer.allocation, &mappedData);
    memcpy(mappedData, data, size);
    vmaUnmapMemory(Context::Instance().getAllocator(),buffer.allocation);
}

void Vulkantool::createVertexBuffer(uint32_t buffer_size, void* data, VmaBuffer& vertex_buffer){
    VmaBuffer staging_buffer;
    createBuffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc, 
    VMA_MEMORY_USAGE_AUTO_PREFER_HOST, staging_buffer);
    updateBuffer(staging_buffer, data, static_cast<size_t>(buffer_size));

    createBuffer(buffer_size, 
    vk::BufferUsageFlagBits::eTransferDst | 
    vk::BufferUsageFlagBits::eVertexBuffer,VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,vertex_buffer);

    copyBuffer(staging_buffer.buffer, vertex_buffer.buffer, buffer_size);
	vmaDestroyBuffer(Context::Instance().getAllocator(), staging_buffer.buffer, staging_buffer.allocation);
}

void Vulkantool::createIndexBuffer(uint32_t buffer_size, void* data,VmaBuffer& index_buffer)
{
    VmaBuffer staging_buffer;
    createBuffer(buffer_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        staging_buffer);

    // copy index data to staging buffer
    updateBuffer(staging_buffer, (void*)data, static_cast<size_t>(buffer_size));

    createBuffer(buffer_size,
        vk::BufferUsageFlagBits::eTransferDst | 
        vk::BufferUsageFlagBits::eIndexBuffer,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        index_buffer);

    copyBuffer(staging_buffer.buffer, index_buffer.buffer, buffer_size);
    vmaDestroyBuffer(Context::Instance().getAllocator(), staging_buffer.buffer, staging_buffer.allocation);
}



// //Not Use Vma
// uint32_t Vulkantool::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties){
//     auto& ctx = Context::Instance();
//     vk::PhysicalDeviceMemoryProperties memProperties;
//     ctx.phyDevice.getMemoryProperties(&memProperties);
    
//      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//             if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//                 return i;
//             }
//         }
//     throw std::runtime_error("failed to find suitable memory type!");
// }

bool Vulkantool::hasStencilComponent(vk::Format format){
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

bool Vulkantool::hasDepthComponent(vk::Format format){
    return format == vk::Format::eD32Sfloat ||  
    format == vk::Format::eD24UnormS8Uint|| format == vk::Format::eD32SfloatS8Uint;
}

vk::ImageAspectFlags Vulkantool::calcImageAspectFlags(vk::Format format){
    vk::ImageAspectFlags aspect_flags{};
    if(hasStencilComponent(format)){
        aspect_flags |=vk::ImageAspectFlagBits::eStencil;
    }
    if(hasDepthComponent(format)){
        aspect_flags |= vk::ImageAspectFlagBits::eDepth;
    }
    return aspect_flags ? aspect_flags : vk::ImageAspectFlagBits::eColor;
}

uint32_t Vulkantool::calcFormatSize(vk::Format format){
    switch (format) {
        case vk::Format::eR16Sfloat:
            return 2;
        case vk::Format::eR8G8B8A8Srgb:
        case vk::Format::eR8G8B8A8Unorm:
        case vk::Format::eR32Sfloat:
        case vk::Format::eD32Sfloat:
        case vk::Format::eR16G16Sfloat:
            return 4;
        case vk::Format::eR16G16B16A16Sfloat:
            return 8;
        case vk::Format::eR32G32B32A32Sfloat:
            return 16;
        default:
            // Assuming LOG_FATAL is a logging function
            SG_CORE_ERROR("unsupported format: {0}", static_cast<int>(format));
        return 0;
    }
}
//Layout


vk::AccessFlags Vulkantool::accessFlagsForImageLayout(vk::ImageLayout layout)
{
    switch (layout)
    {
    case vk::ImageLayout::ePreinitialized:
        return vk::AccessFlagBits::eHostWrite;
    case vk::ImageLayout::eTransferDstOptimal:
        return vk::AccessFlagBits::eTransferWrite;
    case vk::ImageLayout::eTransferSrcOptimal:
        return vk::AccessFlagBits::eTransferRead;
    case vk::ImageLayout::eColorAttachmentOptimal:
        return vk::AccessFlagBits::eColorAttachmentWrite;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        return vk::AccessFlagBits::eShaderRead;
    default:
        return vk::AccessFlagBits{};
    }
}

vk::PipelineStageFlags Vulkantool::pipelineStageForLayout(vk::ImageLayout layout)
{
    switch (layout)
    {
    case vk::ImageLayout::eTransferDstOptimal:
    case vk::ImageLayout::eTransferSrcOptimal:
        return vk::PipelineStageFlagBits::eTransfer;
    case vk::ImageLayout::eColorAttachmentOptimal:
        return vk::PipelineStageFlagBits::eColorAttachmentOutput;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        return vk::PipelineStageFlagBits::eEarlyFragmentTests;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        return vk::PipelineStageFlagBits::eFragmentShader;
    case vk::ImageLayout::ePreinitialized:
        return vk::PipelineStageFlagBits::eHost;
    case vk::ImageLayout::eUndefined:
        return vk::PipelineStageFlagBits::eTopOfPipe;
    default:
        return vk::PipelineStageFlagBits::eBottomOfPipe;
    }
}

void Vulkantool::transitionImageLayout(vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::Format format, uint32_t mip_levels, uint32_t layers){
    if(old_layout == new_layout){
        return;
    }
    vk::CommandBuffer command_buffer = beginSingleCommands();

    //init image memory barrier
    vk::ImageMemoryBarrier barrier = {};
    barrier.setOldLayout(old_layout)
           .setNewLayout(new_layout)
           .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setImage(image)
           .setSubresourceRange({calcImageAspectFlags(format), 0, mip_levels, 0, layers});
    vk::PipelineStageFlags src_stage = pipelineStageForLayout(old_layout);
    vk::PipelineStageFlags dst_stage = pipelineStageForLayout(new_layout);
    barrier.setSrcAccessMask(accessFlagsForImageLayout(old_layout))
           .setDstAccessMask(accessFlagsForImageLayout(new_layout));
    command_buffer.pipelineBarrier(src_stage, dst_stage, vk::DependencyFlagBits{}, 
    0, nullptr, 0, nullptr, 1, &barrier);

    endInstantCommands(command_buffer);
}
void Vulkantool::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height){
    vk::CommandBuffer command_buffer = beginSingleCommands();
    vk::BufferImageCopy region = {};
    region.setBufferOffset(0)
          .setBufferRowLength(0)
          .setBufferImageHeight(0)
          .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
          .setImageOffset({0, 0, 0})
          .setImageExtent({width, height, 1});
    command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
    endInstantCommands(command_buffer);
}

void Vulkantool::createImageMipmaps(vk::Image image,uint32_t width,uint32_t height,uint32_t mip_levels){
    vk::CommandBuffer command_buffer = beginSingleCommands();
    vk::ImageMemoryBarrier barrier = {};
    barrier.setImage(image)
           .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    int32_t mip_width = width;
    int32_t mip_height = height;
    for (uint32_t i = 1; i < mip_levels; i++) {
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
               .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
               .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
               .setDstAccessMask(vk::AccessFlagBits::eTransferRead);
        barrier.setSubresourceRange({vk::ImageAspectFlagBits::eColor, i - 1, 1, 0, 1});
        command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, 
        vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, 
        0, nullptr, 0, nullptr, 1, &barrier);
  
        vk::ImageBlit blit{};
        blit.setSrcOffsets({vk::Offset3D{0, 0, 0}, {mip_width, mip_height, 1}})
            .setDstOffsets({vk::Offset3D{0, 0, 0}, {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1}})
            .setSrcSubresource({vk::ImageAspectFlagBits::eColor, i - 1, 0, 1})
            .setDstSubresource({vk::ImageAspectFlagBits::eColor, i, 0, 1});

        command_buffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, 
        image, vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear);

        barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
               .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
               .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
               .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        barrier.setSubresourceRange({vk::ImageAspectFlagBits::eColor, i - 1, 1, 0, 1});
        command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlagBits{},
        0, nullptr, 0, nullptr, 1, &barrier);
        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }
    barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
           .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
           .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
           .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    barrier.setSubresourceRange({vk::ImageAspectFlagBits::eColor, mip_levels - 1, 1, 0, 1});
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
    vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlagBits{},
    0, nullptr, 0, nullptr, 1, &barrier);
    endInstantCommands(command_buffer);
}

// Data Loading
void Vulkantool::loadimagedata(std::string_view filename,uint32_t width,uint32_t height,std::vector<uint8_t>& imagedata){
    const int k_channels = 4;
    // 预先分配足够的内存空间
    imagedata.resize(width * height * k_channels);
    uint8_t* p_image_data = stbi_load(filename.data(), (int*)&width, (int*)&height, 0, k_channels);
    if (!p_image_data) {
        SG_CORE_ERROR("failed to load texture image!");
    }else {
        // 直接操作原始数据，避免数据拷贝
        std::memcpy(imagedata.data(), p_image_data, width * height * k_channels);
        stbi_image_free(p_image_data);
    }
}



void Vulkantool::createImage(uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layers, vk::SampleCountFlagBits num_samples,
vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags image_usage, VmaMemoryUsage memory_usage, 
VmaImage& vma_image){
    vk::ImageCreateInfo image_info = {};
    image_info.setImageType(vk::ImageType::e2D)
              .setExtent({width, height, 1})
              .setMipLevels(mip_levels)
              .setArrayLayers(layers)
              .setFormat(format)
              .setTiling(tiling)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setUsage(image_usage)
              .setSamples(num_samples)
              .setSharingMode(vk::SharingMode::eExclusive);
    image_info.flags = layers == 6 ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits{};
    
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = memory_usage;
    vmaCreateImage(Context::Instance().getAllocator(), reinterpret_cast<VkImageCreateInfo*>(&image_info), &alloc_info, reinterpret_cast<VkImage*>(&vma_image.image), &vma_image.allocation, nullptr);
}

vk::ImageView Vulkantool::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t layers){
    vk::ImageViewCreateInfo view_info = {};
    view_info.setImage(image)
             .setViewType(layers == 6 ? vk::ImageViewType::eCube :(layers > 1 ? vk::ImageViewType::e2DArray: vk::ImageViewType::e2D) )
             .setFormat(format)
             .setSubresourceRange({aspect_flags, 0, mip_levels, 0, layers});
    vk::ImageView imageView;
    auto result = Context::Instance().device.createImageView(&view_info, nullptr, &imageView);
    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::createImageView Error");}
    return imageView;
}

vk::Sampler Vulkantool::createSample(vk::Filter min_filter, vk::Filter mag_filter, uint32_t mip_levels, vk::SamplerAddressMode address_mode_u, vk::SamplerAddressMode address_mode_v, vk::SamplerAddressMode address_mode_w){
    auto& ctx = Context::Instance();
    
    vk::SamplerCreateInfo sampler_info = {};
    sampler_info.setMinFilter(min_filter)
                .setMagFilter(mag_filter)
                .setAddressModeU(address_mode_u)
                .setAddressModeV(address_mode_v)
                .setAddressModeW(address_mode_w)
                .setAnisotropyEnable(VK_TRUE)
                .setMaxAnisotropy(ctx.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy)
                .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                .setUnnormalizedCoordinates(VK_FALSE)
                .setCompareEnable(VK_FALSE)
                .setCompareOp(vk::CompareOp::eAlways)
                .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                .setMipLodBias(0.0f)
                .setMinLod(0.0f)
                .setMaxLod(static_cast<float>(mip_levels));
    vk::Sampler sampler;
    auto result = Context::Instance().device.createSampler(&sampler_info, nullptr, &sampler);

    if(result != vk::Result::eSuccess ){ SG_CORE_ERROR("Vulkantool::createSample Error");}
    return sampler;
}

void Vulkantool::createImageAndView(uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layers, vk::SampleCountFlagBits num_samples,
vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags image_usage, VmaMemoryUsage memory_usage, vk::ImageAspectFlags aspect_flags, VmaImageView& vma_image_view){

    createImage(width, height, mip_levels, layers, num_samples, format, tiling, image_usage, memory_usage, vma_image_view.vma_image);
    vma_image_view.image_view = createImageView(vma_image_view.vma_image.image, format, aspect_flags, mip_levels, layers);

}

void Vulkantool::createImageViewSampler(uint32_t width, uint32_t height, uint8_t* image_data,
uint32_t mip_levels, uint32_t layers, vk::Format format, vk::Filter min_filter, vk::Filter mag_filter,
vk::SamplerAddressMode address_mode, VmaImageViewSampler& vma_image_view_sampler, vk::ImageUsageFlags ext_use_flags){
    //create image
        vk::ImageUsageFlags image_usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
        if(image_data){
            image_usage |= vk::ImageUsageFlagBits::eTransferDst;
        }
        if(ext_use_flags){
            image_usage |= ext_use_flags;
        }

        createImage(width, height, mip_levels, layers, 
        vk::SampleCountFlagBits::e1, format, vk::ImageTiling::eOptimal, 
        image_usage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, vma_image_view_sampler.vma_image);

        vk::Image image = vma_image_view_sampler.image();
        vma_image_view_sampler.image_view = createImageView(
        image, format, calcImageAspectFlags(format), mip_levels, layers);

        //create sampler
        vma_image_view_sampler.sampler = createSample(min_filter, mag_filter, mip_levels, address_mode, address_mode, address_mode);
        //set image layout add descriptor type
        vma_image_view_sampler.image_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
        vma_image_view_sampler.descriptor_type = vk::DescriptorType::eCombinedImageSampler;

        if(ext_use_flags & vk::ImageUsageFlagBits::eDepthStencilAttachment){
            vma_image_view_sampler.image_layout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
        }else if(ext_use_flags & vk::ImageUsageFlagBits::eInputAttachment){
            vma_image_view_sampler.descriptor_type = vk::DescriptorType::eInputAttachment;
        }
        if(image_data){
            size_t image_size = width * height * calcFormatSize(format);
            VmaBuffer stagin_buffer;
            createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, stagin_buffer);

            //Copy image pixel data to staging buffer
            updateBuffer(stagin_buffer, image_data, image_size);

            //transition image to DST_OP for copy into
            transitionImageLayout(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, format, mip_levels, layers);
            //copy staging buffer to image
            copyBufferToImage(stagin_buffer.buffer, image, width,height);
        
            //clear staging buffer
            vmaDestroyBuffer(Context::Instance().getAllocator(), stagin_buffer.buffer, stagin_buffer.allocation);

            // generate image mipmaps, and transition image to READ_ONLY_OPT state for shader reading
            createImageMipmaps(image, width, height, mip_levels);
        }

}

VmaImageViewSampler Vulkantool::loadImageViewSampler(const std::string &filename, 
uint32_t mip_levels, uint32_t layers, vk::Format format, vk::Filter min_filter, 
vk::Filter mag_filter, vk::SamplerAddressMode address_mode, vk::ImageUsageFlags ext_use_flags)
{
    int wdith,height,k_channels;
    
    uint8_t* image_data = stbi_load(filename.c_str(), &wdith, &height, &k_channels, STBI_rgb_alpha);
    if (!image_data) {
        SG_CORE_ERROR("failed to load texture image!");
    }

    VmaImageViewSampler vma_image_view_sampler;
    createImageViewSampler(wdith, height, image_data, 
    mip_levels, layers, format, min_filter, mag_filter, address_mode, 
    vma_image_view_sampler, ext_use_flags);

    stbi_image_free(image_data);

    return vma_image_view_sampler;
}







}
