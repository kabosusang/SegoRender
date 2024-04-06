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

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();

        VmaImageViewSampler getColorTexture() { return colorIVs_;}

        //Output Function
        virtual void recreateframbuffer(uint32_t width,uint32_t height);
        void setProjection(const glm::mat4& projection){projection_ = projection;}
        void setView(const glm::mat4& view){CameraView_ = view;}
    private:
        void drawNode(vk::CommandBuffer cmd,vk::PipelineLayout pipelineLayout, Node* node);
        void temporarilyInit();

        void render_mesh(vk::CommandBuffer cmdBuffer,std::shared_ptr<MeshRenderData> Rendata);
        void render_sprite(vk::CommandBuffer cmdBuffer,std::shared_ptr<SpriteRenderData> sprite);
    
    private:
        glm::mat4 projection_ = glm::mat4(1.0f);
        glm::mat4 CameraView_ = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    public:
        //temporary
        void createUniformBuffers();
        void updateUniformBuffer(uint32_t currentImage);
    
    private:
        std::vector<vk::Format> m_formats;

        std::vector<VmaBuffer> uniformBuffers_; // View Projection Uniform
        //Color Texture
        VmaImageViewSampler colorIVs_;
        //Depth Texture
        VmaImageViewSampler depthIVs_;

        //DescriptorSetLayout
        std::vector<vk::WriteDescriptorSet> desc_writes;
    };




}