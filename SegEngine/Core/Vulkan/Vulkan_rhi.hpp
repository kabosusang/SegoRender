#pragma once
#include "pch.h"
#include "context.hpp"
#include "framework/Render/pass/main_pass.hpp"
#include "framework/Render/pass/ui_pass.hpp"
#include "framework/Render/pass/pick_pass.hpp"
#include "framework/Render/pass/Dirshadow_pass.hpp"

//asste
#include "resource/asset/base/Light.h"

namespace Sego{

    class VulkanRhi{
    public:
        static void Init(std::vector<const char*>& extensions,
        Context::GetSurfaceCallback cb, int windowWidth, int windowHeight);
        void IsResized() { framebufferResized = true; }

        void render();
        void destory();
        void recreateSwapchain();
        void resizeframbuffer(uint32_t w,uint32_t h);
        //Render Frame
		void waitFrame();
        void updtaUniform();
		void recordFrame();
		void submitFrame();
		void presentFrame();
     
        static VulkanRhi& Instance();

        //Output Function
        inline vk::CommandBuffer getCommandBuffer() { return cmdBuffers_[currentFrame_]; }
        inline vk::CommandPool getSingleCommandPool() { return singlePool_; }
        inline uint32_t getMaxFlightCount() { return maxFlightCount_; }
        inline uint32_t getFlightCount() { return currentFrame_; }
        inline uint32_t getImageIndex() { return currentImageIndex_; }
        PFN_vkCmdPushDescriptorSetKHR getCmdPushDescriptorSet() { return vkCmdPushDescriptorSet_; }
        vk::ImageView getColorImageView();
        VmaImageViewSampler getDirShadowMap();

        uint32_t ReadPixel(uint32_t x, uint32_t y) { return pickPass_->ReadPixelInt(x, y);}
        
        inline std::vector<VmaBuffer>& getUniformBuffer() { return uniformBuffers_;}
        inline VmaBuffer getCurrentUniformBuffer() { return uniformBuffers_[currentFrame_];}
        //Render Output Function
        void setClearColor(const glm::vec4& color);
        void setProjection(const glm::mat4& projection);
        void setView(const glm::mat4& view);

        void SetDepthBias(float Constant,float Slope){
            dirPass_->SetBias(Constant,Slope);
        }
        void SetRenderDatas(std::vector<std::shared_ptr<RenderData>>& render_Datas){
            dirPass_->setRenderDatas(render_Datas);
            pickPass_->setRenderDatas(render_Datas);
            mainPass_->setRenderDatas(render_Datas);
        }

        void SetSkyboxRenderData(std::shared_ptr<SkyboxRenderData>& skybox){
            mainPass_->setSkyboxRenderData(skybox);
        }

        void SetLightRenderData(std::shared_ptr<LightingRenderData>& light){
            mainPass_->setLightRenderData(light);
        }

        void updateShadowConstans(shadowConstans& shadowubos){
            dirPass_->updateShadowConstans(shadowubos);
        }
     
    public:
        std::shared_ptr<Texture2D> defaultTexture;
        std::shared_ptr<TextureCube> defaultSkybox;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionMatrix;
        
    private:
        static VulkanRhi* instance_;
        
        VulkanRhi(uint32_t windowWidth, uint32_t windowHeight);
        ~VulkanRhi() = default;
        uint32_t maxFlightCount_ = 2;
        vk::CommandPool cmdPool_;
        vk::CommandPool singlePool_;
    
        uint32_t currentFrame_ = 0;
        uint32_t currentImageIndex_ = 0;

        std::vector<vk::CommandBuffer> cmdBuffers_;
        std::vector<vk::Semaphore> imageAvailableSemaphores_;
        std::vector<vk::Semaphore> renderFinishedSemaphores_;
        std::vector<vk::Fence> inFlightFences_;

        // additional device extension functions
        PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet_;

        //Function 
        void createCmdPool();
        void createCmdBuffers();
        void createSemaphoresAndFence();
        void loadExtensionFuncs();
        void createUniformBuffers();
        //Test RenderPass
        std::unique_ptr<class UiPass> uiPass_;
        std::unique_ptr<class MainPass> mainPass_;
        std::unique_ptr<class PickPass> pickPass_;
        std::unique_ptr<class DirShadowPass> dirPass_;

        //All Render mvp UniformBuffer
        std::vector<VmaBuffer> uniformBuffers_; //uniform buffer view projection lightMatrix
        bool framebufferResized = false;
    };




}