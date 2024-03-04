#include "toy2d/context.hpp"
#include <iostream>
#include "SDL.h"

namespace toy2d{

std::unique_ptr<Context> Context::instance_ = nullptr; //instance Context

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        SDL_Log("validation layer - ERROR: %s", pCallbackData->pMessage);
    }

    return VK_FALSE;
}
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}


void Context::Init(const std::vector<const char*>& extensions,CreateSurfaceFunc func){
    instance_.reset(new Context(extensions,func));
}
void Context::Quit(){
    instance_.reset();
}

Context::Context(const std::vector<const char*>& extensions,CreateSurfaceFunc func){
    createInstance(extensions);
    pickupPhysicalDevice();
    surface = func(instance);
    queryQueueFamilyIndices();
    createdevice();
    getQueues();
}

Context::~Context(){  
    instance.destroySurfaceKHR(surface);
    device.destroy();
    instance.destroy();
}  

void Context::createInstance(const std::vector<const char*>& extensions){

    vk::InstanceCreateInfo createInfo{};
    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    
    populateDebugMessengerCreateInfo(debugCreateInfo);

    vk::ApplicationInfo appInfo{};
    appInfo.setApiVersion(VK_API_VERSION_1_3)
           .setPEngineName("SDL2Engine");
           
    createInfo.setPApplicationInfo(&appInfo)
              .setEnabledLayerCount(layers.size())
              .setPpEnabledLayerNames(layers.data())
              .setEnabledExtensionCount(extensions.size())
              .setPpEnabledExtensionNames(extensions.data())
              .setPNext(&debugCreateInfo);

    instance = vk::createInstance(createInfo);
}

void Context::setupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        VkResult result = func(instance, &createInfo, nullptr, &debugMessenger);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    } else {
        throw std::runtime_error("vkCreateDebugUtilsMessengerEXT not available!");
    }

}

void Context::pickupPhysicalDevice(){
    auto devices = instance.enumeratePhysicalDevices();
    for(auto& device : devices){
            auto feature = device.getFeatures();
            if(feature.geometryShader){
                physicalDevice = device;
                SDL_Log(physicalDevice.getProperties().deviceName);
                break;
            }
        }
}

void Context::createdevice(){
    std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceCreateInfo createInfo{};
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    if(queueFamilyIndices.graphicsQueue.value() == queueFamilyIndices.presentQueue.value()){
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value())
                       .setQueueCount(1)
                       .setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(std::move(queueCreateInfo));
    }else{
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value())
                       .setQueueCount(1)
                       .setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
        queueCreateInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value())
                       .setQueueCount(1)
                       .setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    createInfo.setQueueCreateInfos(queueCreateInfos)
                .setPEnabledFeatures(nullptr)
                .setEnabledExtensionCount(extensions.size())
                .setPpEnabledExtensionNames(extensions.data());

    device = physicalDevice.createDevice(createInfo);
}

void Context::queryQueueFamilyIndices(){
    auto properties = physicalDevice.getQueueFamilyProperties();
    for(int i = 0; i < properties.size(); i++){
        const auto& property = properties[i];
        if(property.queueFlags & vk::QueueFlagBits::eGraphics){
            queueFamilyIndices.graphicsQueue = i;
        }
        if(physicalDevice.getSurfaceSupportKHR(i,surface)){
        queueFamilyIndices.presentQueue = i;
        }
        if(queueFamilyIndices){
            break;
        }
    }
}

void Context::getQueues(){
    graphicsQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(),0);
    presentQueue = device.getQueue(queueFamilyIndices.presentQueue.value(),0);
}



}
