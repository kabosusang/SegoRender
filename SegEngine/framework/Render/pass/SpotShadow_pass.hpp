#pragma once
#include "render_pass.hpp"

namespace Sego{
    class SpotShadowPass :public RenderPass{
    public:
        SpotShadowPass();
        virtual void Init() override;
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        void updateFrustums(const std::vector<ShadowFrustumCreateInfo>& shadow_frustum_cis);
        const std::vector<VmaImageViewSampler>& getShadowImageViewSamplers();

        std::vector<glm::mat4> m_light_view_projs;
    private:
        void renderNode(vk::CommandBuffer cmdBuffer,GltfModel::Node *node,uint32_t index);
    private:
        void createDynamicBuffers(size_t size);
        vk::Format m_format;
        uint32_t depthSize_;

        VmaImageViewSampler m_depth_image_view_sampler;

		std::vector<VmaImageViewSampler> m_shadow_image_view_samplers;
		std::vector<vk::Framebuffer> m_framebuffers;
    };



    
}