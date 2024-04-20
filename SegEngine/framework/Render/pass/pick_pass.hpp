#pragma once
#include "render_pass.hpp"

namespace Sego{
    class PickPass :public RenderPass{
    public:
        PickPass();
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();

        //Output Function
        void recreateframbuffer(uint32_t width,uint32_t height);
        uint32_t ReadPixelInt(uint32_t x, uint32_t y);
      
  private:
        void drawNode(vk::CommandBuffer cmd,vk::PipelineLayout pipelineLayout, Node* node,std::shared_ptr<StaticMeshRenderData>& Rendata);
        void render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<StaticMeshRenderData>& Rendata);
        void render_sprite(vk::CommandBuffer cmdBuffer,std::shared_ptr<SpriteRenderData>& sprite);
    
    private:
        float scale_ratio_ = 0;
        uint32_t m_mouse_x = 0;
        uint32_t m_mouse_y = 0;

        uint32_t SelectEntity;
        uint32_t decodeEntityID(const uint8_t* color);
        //glm::vec4 encodeEntityID(uint32_t id);
     
    private:
        std::vector<vk::Format> m_formats;
        //pick Entity Pass
        VmaImageView EntityIV_;
        VmaImageView DepthIV_;
        

    };



    
}