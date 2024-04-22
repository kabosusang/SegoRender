#include "pch.h"
#include "CubeTexture.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include <ktx.h>

namespace Sego{
    
    void TextureCube::loadFromMemory(){
        
    }

    TextureCube::TextureCube(){
        texture_type_ = TextureType::Cube;
    }
    TextureCube::TextureCube(const std::string& path){
        texture_type_ = TextureType::Cube;
        Create(path);
    }

    TextureCube::~TextureCube()
    {
        image_view_sampler_.destroy();
    }

    std::shared_ptr<TextureCube> TextureCube::Create(const std::string &path){
        std::shared_ptr<TextureCube> texture = std::make_shared<TextureCube>();
        ktxTexture* ktxTexture;
        auto result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
        if (result != KTX_SUCCESS) {
            SG_CORE_ERROR("Failed to load Cubetexture: {0}", path);
        }
        texture->width_ = ktxTexture->baseWidth;
        texture->height_ = ktxTexture->baseHeight;
        texture->mip_levels_ = ktxTexture->numLevels;
        texture->layes_ = 6;
        texture->minfilter_ = vk::Filter::eLinear;
        texture->magfilter_ = vk::Filter::eLinear;
        texture->addressmode_u = vk::SamplerAddressMode::eClampToEdge;
        texture->addressmode_v = vk::SamplerAddressMode::eClampToEdge;
        texture->addressmode_w = vk::SamplerAddressMode::eClampToEdge;
        texture->loadKtxTexture(ktxTexture,vk::Format::eR8G8B8A8Unorm);
        return texture;
    }




}