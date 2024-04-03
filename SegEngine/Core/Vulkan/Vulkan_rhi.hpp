#pragma once
#include "pch.h"
#include "context.hpp"
#include "framework/Render/pass/main_pass.hpp"
#include "framework/Render/pass/ui_pass.hpp"

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

        //Render Output Function
        void setClearColor(const glm::vec4& color);
        void setProjection(const glm::mat4& projection);
        void setView(const glm::mat4& view);

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


        //Test RenderPass
        std::unique_ptr<class MainPass> mainPass_;
        std::unique_ptr<class RenderPass> uiPass_;


        bool framebufferResized = false;
    };




}