#pragma once
#include "EditorUI.hpp"

namespace Sego{

    class MeshMaterial{
    public:
        std::shared_ptr<ImGuiImage> colorTexture;
        std::shared_ptr<ImGuiImage> metallicRoughnessTexture;
        std::shared_ptr<ImGuiImage> normalTexture;
        std::shared_ptr<ImGuiImage> occlusionTexture;
        std::shared_ptr<ImGuiImage> emissiveTexture;

        void Reset(){
            colorTexture = nullptr;
            metallicRoughnessTexture = nullptr;
            normalTexture = nullptr;
            occlusionTexture = nullptr;
            emissiveTexture = nullptr;
        }
    };
}