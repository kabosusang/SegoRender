#pragma once 

#include "vulkan/vulkan.hpp"
#include "toy2d/swapchain.hpp"
#include <memory>
#include <optional>
#include <cassert>
#include <functional>

namespace toy2d{
using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

class Context final{
public:
    static void Init(const std::vector<const char*>& extensions,CreateSurfaceFunc func);
    static void Quit();
    static Context& GetInstance(){
        assert(instance_);
        return *instance_;
    }

    ~Context();

    struct QueueFamliyIndices final{
        std::optional<uint32_t> graphicsQueue;
        std::optional<uint32_t> presentQueue;

        operator bool() const{
            return graphicsQueue.has_value() && presentQueue.has_value();
        }
    };

    vk::Instance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Queue graphicsQueue; //graphics queue
    vk::Queue presentQueue; //present queue

    vk::SurfaceKHR surface;
    std::unique_ptr<SwapChain> swapChain;

    QueueFamliyIndices queueFamilyIndices;

    void InitSwapChain(int w,int h){
        swapChain.reset(new SwapChain(w,h));
    }
    void DestroySwapChain(){
        swapChain.reset();
    }

private:
    Context(const std::vector<const char*>& extensions,CreateSurfaceFunc func);

    static std::unique_ptr<Context> instance_;

    void createInstance(const std::vector<const char*>& extensions);
    void setupDebugMessenger();
    void pickupPhysicalDevice();
    void createdevice();
    void getQueues();

    void queryQueueFamilyIndices();
    

    

};

}
