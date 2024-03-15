#pragma once
#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include <vulkan/vulkan.hpp>
#include "buffer.hpp"
#include "texture.hpp"
#include "sub_mesh.hpp"

struct StaticVertex{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color;
};

namespace Sego{

struct Material{
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    uint32_t baseColorTextureIndex;
};

struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    int32_t materialIndex;
};

struct Mesh {
std::vector<Primitive> primitives;
};

struct Node{
    Node* parent;
    std::vector<Node*> children;
    Mesh mesh;
    glm::mat4 matrix;
};

struct TextureIndex{
    int32_t index;
};


struct MeshData{
public:
std::vector<Texture> images;
std::vector<TextureIndex> textures;
std::vector<Material> materials;
std::vector<Node*> nodes;
};
}