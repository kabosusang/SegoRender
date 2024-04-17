#pragma once
#include "pch.h"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Texture2D.hpp"
#include "resource/asset/Material.hpp"


#include "resource/host_device.h"

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
        Sprite,Base,Mesh,Skybox
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
        std::vector<imageIndex> textureindex_;
        std::vector<Texture2D> textures_;
        std::vector<Material> materials_;
        std::vector<Node*> nodes_;

        glm::mat4 Meshmodel_;

        void destory(){
            for(auto& node : nodes_){
                delete node;
            }
            vertexBuffer_.destroy();
            indexBuffer_.destroy();
            for(auto& texture : textures_){
                texture.destory();
            }
            
        }
    };


    struct SkyboxRenderData : public RenderData{
         SkyboxRenderData(){
              type = RenderDataType::Skybox;
         }

        VmaBuffer vertexBuffer_;
		VmaBuffer indexBuffer_;
		uint32_t index_count;
        glm::mat4 skybox_mvp;
		VmaImageViewSampler skybox_texture;

         void destory(){
            vertexBuffer_.destroy();
            indexBuffer_.destroy();
            skybox_texture.destroy();
         }
    };

}