#pragma once
#include "render_pass.hpp"

namespace Sego{
    class DirShadowPass :public RenderPass{
    public:
        DirShadowPass();
        virtual void Init() override;
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        void updateCascades(ShadowCascadeCreateInfo& shadowubos);
        VmaImageViewSampler getShadowImageViewSampler() { return ShadowMap_; }
        ShadowCascadeUBO m_shadow_cascade_ubo;
        float m_cascade_splits[SHADOW_CASCADE_NUM];
    
    private:
        void renderNode(vk::CommandBuffer cmdBuffer,GltfModel::Node *node);
    private:
        vk::Format m_format;
        uint32_t depthSize_;
        float m_cascade_split_lambda;

        glm::mat4 lightvp_;
        //pick Entity Pass
        VmaImageViewSampler ShadowMap_;
        std::vector<VmaBuffer> shadowconstans_;

    };



    
}