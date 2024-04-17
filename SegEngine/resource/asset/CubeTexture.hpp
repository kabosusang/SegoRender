#pragma once
#include "base/Texture.hpp"

namespace Sego{

    class TextureCube : public Texture{
    public:
        TextureCube();
        ~TextureCube();
        static std::shared_ptr<TextureCube> Create(const std::string& path);
        void loadFromMemory();
    };


}