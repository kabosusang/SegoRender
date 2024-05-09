#pragma once
#include "render_pass.hpp"
#include "resource/asset/CubeTexture.hpp"

namespace Sego{
    class MainPass : public RenderPass
    {
    public:
        MainPass();
        virtual void Render() override;
        virtual void Init() override;
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();

        VmaImageViewSampler getColorTexture() { return colorIVs_;}
        //Output Function
        void recreateframbuffer(uint32_t width,uint32_t height);
        void setSkyboxRenderData(std::shared_ptr<SkyboxRenderData>& skybox){
            skybox_ = skybox;
        }
        void setLightRenderData(std::shared_ptr<LightingRenderData>& light){
            lightdata_ = light;
        }
        //Renderer2D
        void render_sprite(vk::CommandBuffer cmdBuffer,std::shared_ptr<SpriteRenderData> sprite);

    private:
        //Gbuffer
        void BuildDeferCommandBuffer();
        void CreateDeferFramebuffer();
        void CreateDeferObject();
        
        void DeferRenderSkybox(vk::CommandBuffer cmdBuffer);
        void DeferdrawNodeSkybox(vk::CommandBuffer cmdBuffer , vk::PipelineLayout pipelineLayout, Node* node);
        
        void DeferRendererMesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<StaticMeshRenderData>& Rendata);
        void DeferdrawNode(vk::CommandBuffer cmdBuffer , vk::PipelineLayout pipelineLayout, Node* node,std::shared_ptr<StaticMeshRenderData>& Rendata);
        std::vector<vk::Format> m_gbufferformats;
        vk::Framebuffer DeferredFrameBuffer_;
        vk::RenderPass DeferredRenderPass_;
         //Deferred RenderPass -- GBuffer
        VmaImageViewSampler albedoIVs_;
        VmaImageViewSampler positionIVs_;
        VmaImageViewSampler normalIVs_;
        VmaImageView deferdepthIVs_;
        //pipeline 
        //skybox
        vk::DescriptorSetLayout deferskyboxSetLayout_;
        vk::PipelineLayout deferskyboxpipelineLayout_;
        vk::Pipeline deferskyboxpipeline_;
        //static mesh
        vk::DescriptorSetLayout deferdescriptorSetLayout_;
        vk::PipelineLayout deferpipelineLayout_;
        vk::Pipeline deferpipeline_;
        vk::Viewport  viewport_defer{};
        vk::Rect2D scissor_defer{};
        vk::PipelineViewportStateCreateInfo viewport_state_defer{};
    private:
        std::vector<vk::Format> m_formats;
        vk::SampleCountFlagBits m_msaaSamples;

        //ForWard RenderPass
        //Color Texture
        VmaImageViewSampler colorIVs_;
        //Depth Texture
        VmaImageViewSampler depthIVs_;
        //MSAA Texture
        VmaImageView msaaIVs_;

    
        std::vector<vk::PushConstantRange> mesh_push_constant_ranges_; //mesh
        std::vector<vk::PushConstantRange> cubmap_push_constant_ranges_; //cubemap
         std::vector<vk::PushConstantRange> forward_push_constant_ranges_;//Forward
        //skybox
        std::shared_ptr<SkyboxRenderData> skybox_;
        //LightData
        std::shared_ptr<LightingRenderData> lightdata_;
        
    };




}