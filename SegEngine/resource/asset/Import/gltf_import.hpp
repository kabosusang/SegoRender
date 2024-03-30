#pragma once
#include "tiny_gltf.h"
#include "framework/Render/Render_data.hpp"

namespace Sego{
    class GlTFImporter{
    public:
        static void LoadNodes(const tinygltf::Node& inputNode, 
        const tinygltf::Model& input, Node* parent, 
        std::vector<uint32_t>& indexBuffer, std::vector<StaticVertex>& vertexBuffer
        ,std::shared_ptr<MeshRenderData>& meshRenderData);

        static std::shared_ptr<MeshRenderData> LoadglTFFile(const std::string& path);
    private:

    };
   


}