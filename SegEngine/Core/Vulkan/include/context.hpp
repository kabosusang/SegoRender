#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>
#include "swapchain.hpp"
#include "resource/shader/shader_manager.hpp"

namespace Sego {

class Context {
public:
    using GetSurfaceCallback = std::function<VkSurfaceKHR(VkInstance)>;
    friend void Init(std::vector<const char*>&, GetSurfaceCallback, int, int);

    static void Init(std::vector<const char*>& extensions, GetSurfaceCallback);
    static void Quit();
    static Context& Instance();

    struct QueueInfo {
        std::optional<std::uint32_t> graphicsIndex;
        std::optional<std::uint32_t> presentIndex;
    } queueInfo;

    vk::Instance instance;
    vk::PhysicalDevice phyDevice;
    vk::Device device;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<ShaderManager> shaderManager;

private:
    static Context* instance_;
    vk::SurfaceKHR surface_;

    GetSurfaceCallback getSurfaceCb_ = nullptr;

    Context(std::vector<const char*>& extensions, GetSurfaceCallback);
    ~Context();
public:
    void initSwapchain(int windowWidth, int windowHeight);

private:
    vk::Instance createInstance(std::vector<const char*>& extensions);
    vk::PhysicalDevice pickupPhysicalDevice();
    vk::Device createDevice(vk::SurfaceKHR);

    void queryQueueInfo(vk::SurfaceKHR);
};

}
