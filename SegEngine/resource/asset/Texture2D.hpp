#pragma once
#include "base/Texture.hpp"

namespace Sego{

class Texture2D : public Texture{
public:
    Texture2D() = default;
    ~Texture2D();
    static std::shared_ptr<Texture2D> Create(const std::string& path,vk::Format format = vk::Format::eR8G8B8A8Unorm);
    void loadFromMemory();
    static std::shared_ptr<Texture2D> loadFormFileBiranry(std::string & filenamem,vk::Format format,uint32_t w,uint32_t h,vk::SamplerAddressMode sampleraddress);
    
    
    Texture2D(const Texture2D& other){
        width_ = other.width_;
        height_ = other.height_;
        image_data_ = other.image_data_;
        minfilter_ = other.minfilter_;
        magfilter_ = other.magfilter_;
        format_ = other.format_;
        addressmode_u = other.addressmode_u;
        addressmode_v = other.addressmode_v;
        addressmode_w = other.addressmode_w;
        texture_type_ = other.texture_type_;
        pixel_type_ = other.pixel_type_;
        mip_levels_ = other.mip_levels_;
        layes_ = other.layes_;
        loadFromMemory();
    }
};





}