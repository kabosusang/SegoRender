#include "toy2d/context.hpp"
#include <vector>
#include "SDL.h"

namespace toy2d {
//创建CreateDebugUtilsMessengerEXT
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
        if(messageSeverity >=VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
            SDL_Log("validation layer: %s",pCallbackData->pMessage);
        }
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}


std::unique_ptr<Context> Context::instance_ = nullptr;

void Context::Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func) {
    instance_.reset(new Context(extensions, func));
}

void Context::Quit() {
    instance_.reset();
}

Context::Context(const std::vector<const char*>& extensions, CreateSurfaceFunc func) {
    createInstance(extensions);
    pickupPhyiscalDevice();
    surface = func(instance);
    queryQueueFamilyIndices();
    createDevice();
    getQueues();
    renderProcess.reset(new RenderProcess);
}

Context::~Context() {
    instance.destroySurfaceKHR(surface);
    device.destroy();
    instance.destroy(); 
}

void Context::createInstance(const std::vector<const char*>& extensions) {
    vk::InstanceCreateInfo createInfo;
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_API_VERSION_1_3);

    createInfo.setPApplicationInfo(&appInfo);
    instance = vk::createInstance(createInfo);

    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
    

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    populateDebugMessengerCreateInfo(debugCreateInfo);//为了让验证层涵盖到创建instance

    RemoveNosupportedElems<const char*, vk::LayerProperties>(layers, vk::enumerateInstanceLayerProperties(),
                           [](const char* e1, const vk::LayerProperties& e2) {
                                return std::strcmp(e1, e2.layerName) == 0;
                           });
    createInfo.setPEnabledLayerNames(layers)
              .setPEnabledExtensionNames(extensions)
              .setPNext((VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo);



    instance = vk::createInstance(createInfo);
}

void Context::setdebug()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }

}
void Context::pickupPhyiscalDevice() {
    auto devices = instance.enumeratePhysicalDevices();
    phyDevice = devices[0];

    std::cout << phyDevice.getProperties().deviceName << std::endl;
}

void Context::createDevice() {
    std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vk::DeviceCreateInfo createInfo;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float priorities = 1.0;
    if (queueFamilyIndices.presentQueue.value() == queueFamilyIndices.graphicsQueue.value()) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPQueuePriorities(&priorities)
                       .setQueueCount(1)
                       .setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value());
        queueCreateInfos.push_back(std::move(queueCreateInfo));
    } else {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPQueuePriorities(&priorities)
                       .setQueueCount(1)
                       .setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value());
        queueCreateInfos.push_back(queueCreateInfo);
        queueCreateInfo.setPQueuePriorities(&priorities)
                       .setQueueCount(1)
                       .setQueueFamilyIndex(queueFamilyIndices.presentQueue.value());
        queueCreateInfos.push_back(queueCreateInfo);
    }
    createInfo.setQueueCreateInfos(queueCreateInfos)
              .setPEnabledExtensionNames(extensions);

    device = phyDevice.createDevice(createInfo);
}

void Context::queryQueueFamilyIndices() {
    auto properties = phyDevice.getQueueFamilyProperties();
    for (int i = 0; i < properties.size(); i++) {
        const auto& property = properties[i];
        if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsQueue = i;
        }
        if (phyDevice.getSurfaceSupportKHR(i, surface)) {
            queueFamilyIndices.presentQueue = i;
        }

        if (queueFamilyIndices) {
            break;
        }
    }
}

void Context::getQueues() {
    graphcisQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
    presentQueue = device.getQueue(queueFamilyIndices.presentQueue.value(), 0);
}

}