#pragma once
#include "pch.h"
#include "resource/asset/base/Vertex.hpp"
#include "resource/asset/base/Mesh.hpp"
#include "resource/asset/Texture2D.hpp"
#include "resource/asset/Material.hpp"


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
        Base
    };

    struct RenderData{
        RenderDataType type = RenderDataType::Base;
    };

    struct MeshRenderData : public RenderData{
       
        VmaBuffer vertexBuffer_;
        VmaBuffer indexBuffer_;
        std::vector<imageIndex> textureindex_;
        std::vector<Texture2D> textures_;
        std::vector<Material> materials_;
        std::vector<Node*> nodes_;
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

    

}