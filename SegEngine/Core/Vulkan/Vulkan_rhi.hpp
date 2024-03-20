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

        void render();
        void destory();
      

        void recreateSwapchain();
        //Render Frame
		void waitFrame();
		void recordFrame();
		void submitFrame();
		void presentFrame();
     
        static VulkanRhi& Instance();

        std::vector<vk::CommandBuffer> cmdBuffers_;
        uint32_t currentFrame_ = 0;
        uint32_t currentImageIndex_ = 0;


        //Output Function
        vk::CommandBuffer getCommandBuffer() { return cmdBuffers_[currentFrame_]; }
        vk::CommandPool getSingleCommandPool() { return singlePool_; }
        uint32_t getImageIndex() { return currentImageIndex_; }
        uint32_t getFlightCount() { return currentFrame_; }
       

    private:
        static VulkanRhi* instance_;
        
        VulkanRhi(uint32_t windowWidth, uint32_t windowHeight);
        ~VulkanRhi() = default;
        uint32_t maxFlightCount_ = 2;
        vk::CommandPool cmdPool_;
        vk::CommandPool singlePool_;
        

        std::vector<vk::Semaphore> imageAvailableSemaphores_;
        std::vector<vk::Semaphore> renderFinishedSemaphores_;
        std::vector<vk::Fence> inFlightFences_;


        //Function 
        void createCmdPool();
        void createCmdBuffers();
        void createSemaphoresAndFence();

        //Test RenderPass
        std::unique_ptr<MainPass> mainPass_;
        std::unique_ptr<UiPass> uiPass_;

    };




}