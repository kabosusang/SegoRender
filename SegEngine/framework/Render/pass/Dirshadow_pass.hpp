#pragma once
#include "render_pass.hpp"

namespace Sego{
    class DirShadowPass :public RenderPass{
    public:
        DirShadowPass();
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        
  private:
        void drawNode(vk::CommandBuffer cmd,vk::PipelineLayout pipelineLayout, Node* node,std::shared_ptr<StaticMeshRenderData>& Rendata);
        void render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<StaticMeshRenderData>& Rendata);
    private:

     
    private:
        vk::Format m_format;
        uint32_t depthSize_;
        //pick Entity Pass
        VmaImageViewSampler ShadowMap_;
        

    };



    
}