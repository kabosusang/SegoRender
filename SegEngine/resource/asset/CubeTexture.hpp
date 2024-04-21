#pragma once
#include "base/Texture.hpp"

namespace Sego{

    class TextureCube : public Texture{
    public:
        TextureCube();
        TextureCube(const std::string& path);
        
        ~TextureCube();
        void Create(const std::string& path);
        void loadFromMemory();
    };


}