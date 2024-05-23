#pragma once
#include "render_pass.hpp"
#include "framework/Render/Render_data.hpp"
enum class EFilterType
{
    Irradiance, Prefilter
};

namespace Sego{
    class FilterCubePass :public RenderPass{
    public:
        FilterCubePass(std::shared_ptr<class TextureCube>& skybox_texture);
        virtual void Init() override;
        virtual void Render() override;
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        
        std::shared_ptr<class StaticMeshRenderData> getCubeMesh(){ return skybox_;}
        VmaImageViewSampler getirradianceTexture() {return m_cube_image_view_samplers[0];}
        VmaImageViewSampler getPrefilterTexture() {return m_cube_image_view_samplers[1];}
        uint32_t getPrefilterMipLevels() { return m_mip_levels[1]; }
    private:
    void drawNode(vk::CommandBuffer cmd,vk::PipelineLayout pipelineLayout, Node* node); 
    private:
        vk::Format m_format[2];
        uint32_t m_sizes[2];
        uint32_t m_mip_levels[2];

        vk::RenderPass m_render_passes[2];
        vk::Framebuffer m_framebuffers[2];
        VmaImageView m_color_image_views[2];
        VmaImageViewSampler m_cube_image_view_samplers[2];

        std::shared_ptr<class StaticMeshRenderData> skybox_;
        std::shared_ptr<class TextureCube> skytexture_;
    };



} 

