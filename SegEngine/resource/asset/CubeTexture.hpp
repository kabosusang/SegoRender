#pragma once
#include "base/Texture.hpp"

namespace Sego{

    class TextureCube : public Texture{
    public:
        TextureCube();
        ~TextureCube();
        void Create(const std::string& path);
        void loadFromMemory();
    };


}