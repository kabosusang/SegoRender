#pragma once
#include "pch.h"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h> 

namespace Sego{
std::vector<char> ReadWholeFile(const std::string& filename);

//VMA Buffer
struct VmaBuffer{
    vk::Buffer buffer;
    vk::DeviceSize size;
    VmaAllocation allocation;
    void destroy();
};

//VMA Image
struct VmaImage{
    vk::Image image;
    VmaAllocation allocation;
    void destroy();
};

//VMA Image and Image View
struct VmaImageView{
    VmaImage vma_image;
    vk::ImageView image_view{};

    void destroy();
    vk::Image image() {return vma_image.image;}
};
//VMA Image and Image View and Sampler

struct VmaImageViewSampler{
    VmaImage vma_image;
    vk::ImageView image_view{};
    vk::Sampler sampler{};

    vk::ImageLayout image_layout{};
    vk::DescriptorType descriptor_type{};
    void destroy();
    vk::Image image() {return vma_image.image;}
};


class Vulkantool{
public:
        static vk::CommandBuffer beginSingleCommands();
        static void endInstantCommands(vk::CommandBuffer command_buffer);
        
        //Meory related 
        //static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
        static void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, VmaBuffer& buffer);
        static void updateBuffer(VmaBuffer& buffer, void* data, size_t size);
        static void copyBuffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size);
        static void createVertexBuffer(uint32_t buffer_size, void* data, VmaBuffer& buffer);
        static void createIndexBuffer(uint32_t buffer_size, void* data, VmaBuffer& buffer);
        

        static void transitionImageLayout(vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::Format format, uint32_t mip_levels, uint32_t layers);
public:
        //image related
        static bool hasStencilComponent(vk::Format format);
        static bool hasDepthComponent(vk::Format format);
        static vk::ImageAspectFlags calcImageAspectFlags(vk::Format format);
        static uint32_t calcFormatSize(vk::Format format);
        static vk::PipelineStageFlags pipelineStageForLayout(vk::ImageLayout layout);
        static vk::AccessFlags accessFlagsForImageLayout(vk::ImageLayout layout);
        static void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

public:
        static void createImage(uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layers, vk::SampleCountFlagBits num_samples,
		vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags image_usage, VmaMemoryUsage memory_usage, VmaImage& vma_image);
        
        static vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t layers);

        static vk::Sampler createSample(vk::Filter min_filter, vk::Filter mag_filter, uint32_t mip_levels,
	        vk::SamplerAddressMode address_mode_u, vk::SamplerAddressMode address_mode_v, vk::SamplerAddressMode address_mode_w);

        static void createImageAndView(uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layers, vk::SampleCountFlagBits num_samples,
		vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags image_usage, VmaMemoryUsage memory_usage, vk::ImageAspectFlags aspect_flags, VmaImageView& vma_image_view);
        
        static void createImageViewSampler(uint32_t width, uint32_t height, uint8_t* image_data,
        uint32_t mip_levels, uint32_t layers, vk::Format format, vk::Filter min_filter, vk::Filter mag_filter,
        vk::SamplerAddressMode address_mode, VmaImageViewSampler& vma_image_view_sampler, vk::ImageUsageFlags ext_use_flags = vk::ImageUsageFlags{});
    
        //MSAA
        static void createImageViewSampler(uint32_t width, uint32_t height, uint8_t* image_data,
        uint32_t mip_levels, uint32_t layers, vk::Format format, vk::Filter min_filter, vk::Filter mag_filter,
        vk::SamplerAddressMode address_mode, VmaImageViewSampler& vma_image_view_sampler,vk::SampleCountFlagBits sample_count, vk::ImageUsageFlags ext_use_flags = vk::ImageUsageFlags{});

        static void createImageMipmaps(vk::Image image,uint32_t width,uint32_t height,uint32_t mip_levels);

        //loda data
        static void loadimagedata(std::string_view filename,uint32_t width,uint32_t height,std::vector<uint8_t>& imagedata);
        static VmaImageViewSampler loadImageViewSampler(const std::string& filename,
        uint32_t mip_levels = 1, uint32_t layers = 1, vk::Format format = vk::Format::eR8G8B8A8Srgb, 
        vk::Filter min_filter = vk::Filter::eLinear, vk::Filter mag_filter = vk::Filter::eLinear,
        vk::SamplerAddressMode address_mode = vk::SamplerAddressMode::eRepeat, vk::ImageUsageFlags ext_use_flags =  vk::ImageUsageFlags{});

        
        static void readImagePixel(vk::Image imaghe,uint32_t width,uint32_t height,vk::Format format,
        std::vector<uint8_t>& imagedata,vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined,
        uint32_t mip_levels = 1, uint32_t layers = 1,vk::ImageLayout final_layout = vk::ImageLayout::eUndefined);



};

}