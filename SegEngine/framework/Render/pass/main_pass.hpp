#pragma once
#include "render_pass.hpp"
#include "resource/asset/CubeTexture.hpp"

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
        void drawNode(vk::CommandBuffer cmd,vk::PipelineLayout pipelineLayout, Node* node,std::shared_ptr<StaticMeshRenderData>& Rendata);
        void render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<StaticMeshRenderData>& Rendata);
        void render_sprite(vk::CommandBuffer cmdBuffer,std::shared_ptr<SpriteRenderData> sprite);
    private:
        std::vector<vk::Format> m_formats;
        //Color Texture
        VmaImageViewSampler colorIVs_;
        //Depth Texture
        VmaImageViewSampler depthIVs_;


        //
        std::vector<vk::PushConstantRange> mesh_push_constant_ranges_;


        //Skybox
        std::shared_ptr<TextureCube> skyboxs_;
        
    };




}