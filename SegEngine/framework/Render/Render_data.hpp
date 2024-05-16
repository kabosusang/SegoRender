#pragma once
#include "pch.h"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Material.hpp"

#include "resource/asset/Texture2D.hpp"
#include "resource/asset/CubeTexture.hpp"
#include "resource/asset/base/Light.h"
#include "resource/asset/GLTFModel.hpp"

#define LUTPATH "resources/assets/engine/texture/brdflut.tex"

namespace Sego{
    struct Node{
        Node* parent;
		std::vector<Node*> children;
		Mesh mesh;
		glm::mat4 matrix;
		~Node() {
			for (auto& child : children) {
				delete child;
			}
		}
    };

    enum class RenderDataType{
        Sprite,Base,StaticMesh,Skybox,Lighting,SkyLight
    };

    struct RenderData{
        RenderDataType type ;
        RenderData() :type(RenderDataType::Base){}
    };

    struct SpriteRenderData : public RenderData{
       SpriteRenderData(){
              type = RenderDataType::Sprite;
         }
        //Texture2D texture;
        VmaBuffer vertexBuffer_;
        VmaBuffer indexBuffer_;
        uint32_t indexCount_;
        //push constant
        glm::mat4 Spritemvp_;
        int32_t UseTex;
        //texture
        std::shared_ptr<Texture2D> Spritetexture;

        // Editor-only
        uint32_t EntityID = -1;

        void destory(){
            vertexBuffer_.destroy();
            indexBuffer_.destroy();
        }

    };

    struct MeshRenderData : public RenderData{ 
        VmaBuffer vertexBuffer_;
        VmaBuffer indexBuffer_;

        glm::mat4 model_;
        //push Constant
        glm::mat4 Meshmvp_;
    };


    struct StaticMeshRenderData : public MeshRenderData{
        StaticMeshRenderData() {type = RenderDataType::StaticMesh;}
        
        std::vector<imageIndex> textureindex_;
        std::vector<Node*> nodes_; //nodes
        std::vector<Material> materials_;
        std::vector<Texture2D> textures_;
        
        void destory(){
            vertexBuffer_.destroy();
            indexBuffer_.destroy();
        }
        uint32_t EntityID = -1;
    };


    struct SkyboxRenderData : public StaticMeshRenderData{
        SkyboxRenderData(){
              type = RenderDataType::Skybox;
        }
        
         void destory(){
            vertexBuffer_.destroy();
            indexBuffer_.destroy();
         }
    };

    struct PbrMeshRenderData : public MeshRenderData{
        PbrMeshRenderData() { type = RenderDataType::StaticMesh; }

        std::shared_ptr<GltfModel::Model> model = nullptr;
        
        void destory() {
           model->destory();
        }
        uint32_t EntityID = -1;
    };



    struct LightingRenderData : public RenderData
	{
		LightingRenderData() { type = RenderDataType::Lighting; }

        //LightObj
		std::vector<VmaBuffer> lighting_ubs;
        
		VmaImageViewSampler directional_light_shadow_texture;
	};


    struct SkyLightRenderData : public RenderData{
        SkyLightRenderData() { type = RenderDataType::SkyLight;m_prefilter_mip_levels = 0; }


        VmaImageViewSampler lutBrdfIVs_;
        VmaImageViewSampler irradianceIVs_;
        VmaImageViewSampler prefilteredIVs_;
        uint32_t m_prefilter_mip_levels;
        std::shared_ptr<StaticMeshRenderData> cube_mesh = nullptr;
        std::shared_ptr<TextureCube> textureCube = nullptr;

        void CreateIBLTexture();

    };


    struct ViewProjs{
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct SceneRenderSettings{
        float exposure = 4.5f; //曝光
        float gamma = 2.2f;//伽马
        float debugViewInputs = 0;
        float debugViewEquation = 0;
    };

    

    extern SceneRenderSettings SceneRenderData;
}