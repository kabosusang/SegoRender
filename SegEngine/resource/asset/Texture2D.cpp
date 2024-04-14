#include "Texture2D.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include <stb_image.h>

namespace Sego{

    void Texture2D::loadFromMemory(){
        Vulkantool::createImageViewSampler(
        width_,height_,image_data_.data(),mip_levels_,layes_,format_,
        minfilter_,magfilter_,addressmode_u,image_view_sampler_);
    }

    std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path){
        std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
        int tex_width,tex_height,tex_channels;
        stbi_uc* pixels = stbi_load(path.c_str(),&tex_width,&tex_height,&tex_channels,STBI_rgb_alpha);
        if (!pixels){
            throw std::runtime_error("failed to load texture image");
        }
        texture->width_ = tex_width;
        texture->height_ = tex_height;
        texture->image_data_.resize(tex_width*tex_height*4);
        memcpy(texture->image_data_.data(),pixels,tex_width*tex_height*4);
        stbi_image_free(pixels);
        texture->loadFromMemory();
        return texture;
    }


}