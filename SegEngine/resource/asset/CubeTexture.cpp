#include "pch.h"
#include "CubeTexture.hpp"
#include "Core/Vulkan/Vulkantool.hpp"

namespace Sego{
    
    void TextureCube::loadFromMemory(){
        Vulkantool::createImageViewSampler(
        width_,height_,image_data_.data(),mip_levels_,layes_,format_,
        minfilter_,magfilter_,addressmode_u,image_view_sampler_);
    }

    TextureCube::TextureCube(){
        texture_type_ = ETextureType::Cube;
        layes = 6;
    }

    TextureCube::~TextureCube()
    {
        image_view_sampler_.destroy();
    }

    std::shared_ptr<TextureCube> TextureCube::Create(const std::string &path){   
        std::shared_ptr<TextureCube> texture = std::make_shared<TextureCube>();
      
        




        return std::shared_ptr<TextureCube>();
    }

}