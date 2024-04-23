#pragma once
#include "pch.h"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Material.hpp"

#include "resource/asset/Texture2D.hpp"
#include "resource/asset/CubeTexture.hpp"

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
        Sprite,Base,StaticMesh,Skybox
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

        //push Constant
        glm::mat4 Meshmvp_;

    };

    struct StaticMeshRenderData : public MeshRenderData{
        StaticMeshRenderData() {type = RenderDataType::StaticMesh;}
        
        std::vector<imageIndex> textureindex_;
        std::vector<Node*> nodes_; //nodes
        std::vector<Material> materials_;
        std::vector<Texture2D> textures_;
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

}