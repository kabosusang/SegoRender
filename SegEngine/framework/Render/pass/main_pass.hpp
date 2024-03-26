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
        virtual void recreateframbuffer(uint32_t width,uint32_t height);

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        void temporarilyInit();
        void addDescriptorSet();

        VmaImageViewSampler getColorTexture() { return colorIVs_;}

        //temporary
        void createUniformBuffers();
        void updateUniformBuffer(uint32_t currentImage);
    private:
        std::vector<vk::Format> m_formats;

        //temporary Data
        VmaBuffer vertexBuffer_;
        VmaBuffer indexBuffer_;
        std::vector<VmaBuffer> uniformBuffers_;
        VmaImageViewSampler textureIVs_;

        //Color Texture
        VmaImageViewSampler colorIVs_;
        //Depth Texture
        VmaImageViewSampler depthIVs_;
    };




}