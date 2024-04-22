#pragma once
#include "Core/Vulkan/Vulkantool.hpp"

namespace Sego{

enum class TextureType
{
	BaseColor, MetallicRoughnessOcclusion, Normal, Emissive, Cube, UI, Data
};

enum class PixelType
{
	RGBA8, RGBA16, RGBA32, RG16, R16, R32
};

class Texture{
public:
    Texture() = default;
    virtual ~Texture() = default;
    uint32_t width_,height_ = 0;
    std::vector<uint8_t> image_data_ = {};
    vk::Filter minfilter_ = vk::Filter::eLinear;
    vk::Filter magfilter_ = vk::Filter::eLinear;
    vk::Format format_ = vk::Format::eR8G8B8A8Unorm;
    vk::SamplerAddressMode addressmode_u,addressmode_v,addressmode_w = vk::SamplerAddressMode::eRepeat;
    TextureType texture_type_ = TextureType::BaseColor;
	PixelType pixel_type_ = PixelType::RGBA8;
    uint32_t mip_levels_ = 1;
    uint32_t layes_ = 1;
    VmaImageViewSampler image_view_sampler_ = {};

    void loadKtxTexture(void* p_ktx_texture,vk::Format format = vk::Format::eUndefined);

    void destory(){
       image_view_sampler_.destroy();
    }



};







}