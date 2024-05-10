#pragma once
#include "tiny_gltf.h"
#include "framework/Render/Render_data.hpp"

namespace Sego{
    class GlTFImporter{
    public:
        static void LoadNodes(const tinygltf::Node& inputNode, 
        const tinygltf::Model& input, Node* parent, 
        std::vector<uint32_t>& indexBuffer, std::vector<StaticVertex>& vertexBuffer
        ,std::shared_ptr<StaticMeshRenderData>& meshRenderData);

        static std::shared_ptr<StaticMeshRenderData> LoadglTFFile(const std::string& path);
    private:

    };
   

    struct BoundingBox{
        glm::vec3 min;
        glm::vec3 max;
        bool valid = false;
        BoundingBox();
        BoundingBox(const glm::vec3& min, const glm::vec3& max);
        BoundingBox getAABB(glm::mat4 transform);
    };

    struct PBRMaterial{
        enum AlphaMode{ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND};
        float alphaCutoff = 1.0f;
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        glm::vec4 emissiveFactor = glm::vec4(0.0f);
        std::shared_ptr<Texture2D> m_base_color_texure;
		std::shared_ptr<Texture2D> m_metallic_roughness_texure;
		std::shared_ptr<Texture2D> m_normal_texure;
        std::shared_ptr<Texture2D> m_occlusion_texure;
		std::shared_ptr<Texture2D> m_emissive_texure;
        bool doubleSided = false;
        struct TexCoordSets{
            uint8_t baseColor = 0;
            uint8_t metallicRoughness = 0;
            uint8_t normal = 0;
            uint8_t occlusion = 0;
            uint8_t emissive = 0;
        } texCoordSets;
        struct Extensions{
            std::shared_ptr<Texture2D> m_specularGlossiness_texure; // KHR_materials_pbrSpecularGlossiness
            std::shared_ptr<Texture2D> m_diffuse_texure; // KHR_materials_pbrSpecularGlossiness
            glm::vec4 diffuseFactor = glm::vec4(1.0f);
            glm::vec3 specularFactor = glm::vec3(1.0f);
        } extension;

        struct PbrWorkflows{
            bool metallicRoughness = true;
            bool specularGlossiness = false;
        } pbrWorkflows;
        int index = 0;
        bool unlit = false;
        float emissiveIntensity = 1.0f;
    };



}