#pragma once
#include "tiny_gltf.h"
#include "resource/asset/base/Vertex.hpp"

namespace Sego{

class GltfImporter{
public:
    static void loadImages(tinygltf::Model& input);
    static void loadTextures(tinygltf::Model& input);
    static void loadMaterials(tinygltf::Model& input);
    static void loadNodes(const tinygltf::Node& inputNode, const tinygltf::Model& input,
    Node* parent, std::vector<uint32_t>& indexBuffer, 
    std::vector<StaticVertex>&  vertexBuffer);
};


static Sego::MeshData mesh_data;


}