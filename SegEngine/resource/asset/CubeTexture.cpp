#include "pch.h"
#include "CubeTexture.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include <ktx.h>

namespace Sego{
    
    void TextureCube::loadFromMemory(){
        
    }

    TextureCube::TextureCube(){
        texture_type_ = TextureType::Cube;
        layes_ = 6;
    }
    TextureCube::TextureCube(const std::string& path){
        texture_type_ = TextureType::Cube;
        layes_ = 6;
        Create(path);
    }

    TextureCube::~TextureCube()
    {
        image_view_sampler_.destroy();
    }

    void TextureCube::Create(const std::string &path){   
        ktxTexture* ktxTexture;
        auto result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
        if (result != KTX_SUCCESS) {
            SG_CORE_ERROR("Failed to load Cubetexture: {0}", path);
        }

        width_ = ktxTexture->baseWidth;
        height_ = ktxTexture->baseHeight;
        mip_levels_ = ktxTexture->numLevels;
        addressmode_u = addressmode_v = addressmode_w = vk::SamplerAddressMode::eClampToEdge;
        loadKtxTexture(ktxTexture,vk::Format::eR8G8B8A8Unorm);
    }

}