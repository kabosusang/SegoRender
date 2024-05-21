#include "Texture2D.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include <stb_image.h>

namespace Sego{


void Texture2D::loadFromMemory(){
    Vulkantool::createImageViewSampler(
    width_,height_,image_data_.data(),mip_levels_,layes_,format_,
    minfilter_,magfilter_,addressmode_u,image_view_sampler_);
}

std::shared_ptr<Texture2D> Texture2D::loadFormFileBiranry(std::string & file_path,vk::Format format,
uint32_t w,uint32_t h,vk::SamplerAddressMode sampleraddress)
{
    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
    texture->format_ = format;
    texture->width_ = w;
    texture->height_ = h;
    texture->addressmode_u = sampleraddress;
    std::ifstream ifs(file_path, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    texture->image_data_.resize(file_size);
    ifs.read((char*)texture->image_data_.data(), file_size);
    ifs.close();

    texture->loadFromMemory();
    return texture;
}

Texture2D::~Texture2D()
{
    image_view_sampler_.destroy();
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::string &path,vk::Format format)
{
    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
    int tex_width,tex_height,tex_channels;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load(path.c_str(),&tex_width,&tex_height,&tex_channels,STBI_rgb_alpha);
    if (!pixels){
        throw std::runtime_error("failed to load texture image");
    }
    texture->width_ = tex_width;
    texture->height_ = tex_height;
    texture->image_data_.resize(tex_width*tex_height*4);
    texture->format_ = format;
    memcpy(texture->image_data_.data(),pixels,tex_width*tex_height*4);
    stbi_image_free(pixels);
    texture->loadFromMemory();
    return texture;
}


}