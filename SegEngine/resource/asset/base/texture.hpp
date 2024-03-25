#pragma once
#include "Core/Vulkan/Vulkantool.hpp"

namespace Sego{

enum class ETextureType
{
	BaseColor, MetallicRoughnessOcclusion, Normal, Emissive, Cube, UI, Data
};

enum class EPixelType
{
	RGBA8, RGBA16, RGBA32, RG16, R16, R32
};


class Texture{
public:
    Texture();
    virtual ~Texture();

    uint32_t width_,height_;
    vk::Filter minfilter_,magfilter_;
    vk::SamplerAddressMode addressmode_u,addressmode_v,addressmode_w;
    ETextureType texture_type_;
	EPixelType pixel_type_;

    uint32_t mip_levels_;
    uint32_t layes_;

    VmaImageViewSampler image_view_sampler_;
    std::vector<uint8_t> image_data_;

    
};





}