#pragma once
#include "render_pass.hpp"

namespace Sego{
    class MainPass : public RenderPass
    {
    public:
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;
        virtual void recreateframbuffer();

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        void temporarilyInit();
        void addDescriptorSet();

        //temporary
        void createUniformBuffers();
        void updateUniformBuffer(uint32_t currentImage);
    private:

        //temporary Data
        VmaBuffer vertexBuffer_;
        VmaBuffer indexBuffer_;
        std::vector<VmaBuffer> uniformBuffers_;

        //Color Texture
        VmaImageViewSampler textureIVs_;
        //Depth Texture
        VmaImageViewSampler depthIVs_;
    };




}