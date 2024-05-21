#pragma once
#include "pch.h"
#include "tiny_gltf.h"
#include "core/Vulkan/Vulkantool.hpp"
#include "base/Vertex.hpp"

#define MAX_NUM_JOINTS 128u


namespace GltfModel{
struct Node;
struct BoundingBox{
    glm::vec3 min;
    glm::vec3 max;
    BoundingBox();
    bool valid = false;
    BoundingBox(const glm::vec3& min, const glm::vec3& max);
    BoundingBox getAABB(glm::mat4 transform);
};

struct TextureSampler{
    vk::Filter magFilter;
    vk::Filter minFilter;
    vk::SamplerAddressMode addressModeU;
    vk::SamplerAddressMode addressModeV;
    vk::SamplerAddressMode addressModeW;
};

struct GLTFTexture{
    Sego::VmaImageViewSampler image_view_sampler_;
    vk::ImageLayout imageLayout_;
    uint32_t width_,height_;
    uint32_t mipLevels_ = 1;
    uint32_t layerCount_ = 1;
    vk::Format format_;
    inline Sego::VmaImageViewSampler& GetImageViewSampler() {
        return image_view_sampler_;
    }
    void fromglTfImage(tinygltf::Image& gltfimage, TextureSampler textureSampler);
    void destory(){
        image_view_sampler_.destroy();
    }
};

struct PBRMaterial{
    enum AlphaMode{ ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
    AlphaMode alphaMode = ALPHAMODE_OPAQUE;
    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    glm::vec4 emissiveFactor = glm::vec4(0.0f);
    GLTFTexture* baseColorTexture;
    Sego::VmaImageViewSampler& GetColorImageViewSampler();
    GLTFTexture* metallicRoughnessTexture; //Roughness
    Sego::VmaImageViewSampler& GetMetallicRoughnessImageViewSampler();
    GLTFTexture* normalTexture; //normal 通用
    Sego::VmaImageViewSampler& GetNormalImageViewSampler();
    GLTFTexture* occlusionTexture; //AO 通用
    Sego::VmaImageViewSampler& GetOcclusionImageViewSampler();
    GLTFTexture* emissiveTexture;
    Sego::VmaImageViewSampler& GetEmissiveImageViewSampler();
   
    bool doubleSided = false;
    struct TexCoordSets{
        uint8_t baseColor = 0;
        uint8_t metallicRoughness = 0;
        uint8_t specularGlossiness = 0;
        uint8_t normal = 0;
        uint8_t occlusion = 0;
        uint8_t emissive = 0;
    } texCoordSets;
    struct Extensions{
        GLTFTexture* specularGlossinessTexture; // KHR_materials_pbrSpecularGlossiness
        GLTFTexture* diffuseTexture; // KHR_materials_pbrSpecularGlossiness
        glm::vec4 diffuseFactor = glm::vec4(1.0f);
        glm::vec3 specularFactor = glm::vec3(1.0f);
    } extension;

    struct PbrWorkflows{
        bool metallicRoughness = true;
        bool specularGlossiness = false;
    } pbrWorkflows;
    int index = 0;
    bool unlit = false; //KHR_materials_unlit
    float emissiveStrength  = 1.0f;
};

struct Primitive{
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t vertexCount;
    PBRMaterial& material;
    bool hasIndices;
    BoundingBox bb;
    Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, PBRMaterial& material);
    void setBoundingBox(const glm::vec3& min, const glm::vec3& max);
};

struct Mesh{
    std::vector<Primitive*> primitives;
    BoundingBox bb;
    BoundingBox aabb;
    struct UniformBuffer{
        Sego::VmaBuffer buffer;
    }uniformBuffer;
    struct UniformBlock{
        glm::mat4 matrix;
        glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
        float jointcount = 0;
    }uniformBlock;
    Mesh(glm::mat4 matrix);
    ~Mesh();
    void setBoundingBox(const glm::vec3& min, const glm::vec3& max);
};

//Skin
struct Skin{
    std::string name;
    Node* skeletonRoot  = nullptr;
    std::vector<glm::mat4> inverseBindMatrices;
    std::vector<Node*> joints;
};

//Node
struct Node {
    Node *parent;
    uint32_t index;
    std::vector<Node*> children;
    glm::mat4 matrix;
    std::string name;
    Mesh *mesh;
    Skin *skin;
    int32_t skinIndex = -1;
    glm::vec3 translation{};
    glm::vec3 scale{ 1.0f };
    glm::quat rotation{};
    BoundingBox bvh;
    BoundingBox aabb;
    bool useCachedMatrix{ false };
    glm::mat4 cachedLocalMatrix{ glm::mat4(1.0f) };
    glm::mat4 cachedMatrix{ glm::mat4(1.0f) };
    glm::mat4 localMatrix();
    glm::mat4 getMatrix();
    void update();
    ~Node();
};
//动画通道
struct AnimationChannel {
    enum PathType { TRANSLATION, ROTATION, SCALE }; 
    PathType path;          //影响的节点的属性
    Node *node;             //表示影响的节点
    uint32_t samplerIndex;  //采样器索引
};
//动画采样器
struct AnimationSampler {
    enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
    InterpolationType interpolation;    //插值方式
    std::vector<float> inputs;          //输入采样器的时间数组
    std::vector<glm::vec4> outputsVec4; //表示采样器的输出值数组
};
//动画
struct Animation {
    std::string name;   //动画名称
    std::vector<AnimationSampler> samplers; //采样器数组
    std::vector<AnimationChannel> channels; //通道数组
    float start = std::numeric_limits<float>::max();//动画开始时间
    float end = std::numeric_limits<float>::min();//动画结束时间
};

// Model
struct Model{
    Sego::VmaBuffer vertexBuffer_;
    Sego::VmaBuffer indexBuffer_;

    glm::mat4 aabb;

    std::vector<Node*> nodes;
    std::vector<Node*> linearNodes;

    std::vector<Skin*> skins;

    std::vector<GLTFTexture> textures;
    std::vector<TextureSampler> textureSamplers;
    std::vector<PBRMaterial> materials;
    std::vector<Animation> animations;
    std::vector<std::string> extensions;

    struct Dimensions{
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
    }dimensions;

    struct LoaderInfo{
        uint32_t* indexBuffer;
        MeshAndSkeletonVertex* vertexBuffer;
        size_t indexPos = 0;
        size_t vertexPos = 0;
    };

    void destory();
    void loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, LoaderInfo& loaderInfo, float globalscale);
    void getNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount);
    void loadSkins(tinygltf::Model& gltfModel);
    void loadTextures(tinygltf::Model& gltfModel);
    vk::SamplerAddressMode getVkWrapMode(int32_t wrapMode);
    vk::Filter getVkFilterMode(int32_t filterMode);
    void loadTextureSamplers(tinygltf::Model& gltfModel);
    void loadMaterials(tinygltf::Model& gltfModel);
    void loadAnimations(tinygltf::Model& gltfModel);
    void loadFromFile(std::string filename,float scale = 1.0f);
    void drawNode(Node* node, vk::CommandBuffer commandBuffer);
    void draw(vk::CommandBuffer commandBuffer);
    void calculateBoundingBox(Node* node, Node* parent);
    void getSceneDimensions();
    void updateAnimation(uint32_t index, float time);
    Node* findNode(Node* parent, uint32_t index);
    Node* nodeFromIndex(uint32_t index);
};


}