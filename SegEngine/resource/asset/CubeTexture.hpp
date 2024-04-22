#pragma once
#include "base/Texture.hpp"

namespace Sego{

    class TextureCube : public Texture{
    public:
        TextureCube();
        TextureCube(const std::string& path);
       
        static std::shared_ptr<TextureCube> Create(const std::string& path);
        ~TextureCube();
        void loadFromMemory();
    };

/*
 MeshComponent() = default;
        MeshComponent(const MeshComponent& other) :
        name(other.name),
        path(other.path),
        MeshData(std::make_shared<StaticMeshRenderData>(*other.MeshData)){}
*/


}