#pragma once
#include "render_pass.hpp"
#include "resource/asset/Import/gltf_import.hpp"

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
        void drawNode(vk::PipelineLayout pipelineLayout, Node* node);
        void temporarilyInit();

        VmaImageViewSampler getColorTexture() { return colorIVs_;}

        //temporary
        void createUniformBuffers();
        void updateUniformBuffer(uint32_t currentImage);
    private:
        std::vector<vk::Format> m_formats;

        //temporary Data
        std::shared_ptr<MeshRenderData> Rendata;

        std::vector<VmaBuffer> uniformBuffers_;
        //Color Texture
        VmaImageViewSampler colorIVs_;
        //Depth Texture
        VmaImageViewSampler depthIVs_;

        //DescriptorSetLayout
        std::vector<vk::WriteDescriptorSet> desc_writes;
    };




}