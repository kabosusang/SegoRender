#pragma once
#include "render_pass.hpp"


namespace Sego{
    class MainPass : public RenderPass
    {
    public:
        MainPass();
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();

        VmaImageViewSampler getColorTexture() { return colorIVs_;}
        VmaImageViewSampler getDepthTexture() { return depthIVs_;}
        
        //Output Function
        void recreateframbuffer(uint32_t width,uint32_t height);
    private:
        void drawNode(vk::CommandBuffer cmd,vk::PipelineLayout pipelineLayout, Node* node);
        void render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<MeshRenderData> Rendata);
        void render_sprite(vk::CommandBuffer cmdBuffer,std::shared_ptr<SpriteRenderData> sprite);
    
    public:
       
    private:
        std::vector<vk::Format> m_formats;
        //Color Texture
        VmaImageViewSampler colorIVs_;
        //Depth Texture
        VmaImageViewSampler depthIVs_;

       
    };




}