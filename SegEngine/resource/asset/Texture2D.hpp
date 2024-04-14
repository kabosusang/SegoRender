#pragma once
#include "base/Texture.hpp"

namespace Sego{

class Texture2D : public Texture{
public:
    static std::shared_ptr<Texture2D> Create(const std::string& path);
    void loadFromMemory();
    
};





}