#pragma once
#include "render_pass.hpp"

namespace Sego{
    class PointShadowPass :public RenderPass{
    public:
        PointShadowPass();
        virtual void Init() override;
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        void updateCubes(const std::vector<ShadowCubeCreateInfo>& shadow_cube_cis);
        const std::vector<VmaImageViewSampler>& getShadowImageViewSamplers();

    private:
        void renderNode(vk::CommandBuffer cmdBuffer,GltfModel::Node *node,uint32_t index);
    private:
        void createDynamicBuffers(size_t size);
        std::vector<vk::Format> m_format;
        uint32_t depthSize_;
        float m_cascade_split_lambda;

        VmaImageViewSampler m_depth_image_view_sampler;

		std::vector<VmaImageViewSampler> m_shadow_image_view_samplers;
		std::vector<vk::Framebuffer> m_framebuffers;
		std::vector<std::vector<VmaBuffer>> m_shadow_cube_ubss;

		std::vector<glm::vec3> m_light_poss;
    };



    
}