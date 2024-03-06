#include "toy2d/context.hpp"
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


Context* Context::instance_ = nullptr;

void Context::Init(std::vector<const char*>& extensions, GetSurfaceCallback cb) {
    instance_ = new Context(extensions, cb);
}

void Context::Quit() {
    delete instance_;
}

Context& Context::Instance() {
    return *instance_;
}

Context::Context(std::vector<const char*>& extensions, GetSurfaceCallback cb) {
    getSurfaceCb_ = cb;

    instance = createInstance(extensions);
    if (!instance) {
        std::cout << "instance create failed" << std::endl;
        exit(1);
    }

    phyDevice = pickupPhysicalDevice();
    if (!phyDevice) {
        std::cout << "pickup physical device failed" << std::endl;
        exit(1);
    }

    surface_ = getSurfaceCb_(instance);
    if (!surface_) {
        std::cout << "create surface failed" << std::endl;
        exit(1);
    }

    device = createDevice(surface_);
    if (!device) {
        std::cout << "create device failed" << std::endl;
        exit(1);
    }

    graphicsQueue = device.getQueue(queueInfo.graphicsIndex.value(), 0);
    presentQueue = device.getQueue(queueInfo.presentIndex.value(), 0);
}

vk::Instance Context::createInstance(std::vector<const char*>& extensions) {
    vk::InstanceCreateInfo info; 
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_API_VERSION_1_3);
    
 VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
 populateDebugMessengerCreateInfo(debugCreateInfo);//为了让验证层涵盖到创建instance


    info.setPApplicationInfo(&appInfo)
        .setPEnabledExtensionNames(extensions)
        .setPNext((VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo);

    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
    info.setPEnabledLayerNames(layers);

    return vk::createInstance(info);
}

vk::PhysicalDevice Context::pickupPhysicalDevice() {
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.size() == 0) {
        std::cout << "you don't have suitable device to support vulkan" << std::endl;
        exit(1);
    }
    return devices[0];
}

vk::Device Context::createDevice(vk::SurfaceKHR surface) {
    vk::DeviceCreateInfo deviceCreateInfo;
    queryQueueInfo(surface);
    std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    deviceCreateInfo.setPEnabledExtensionNames(extensions);

    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    float priority = 1;
    if (queueInfo.graphicsIndex.value() == queueInfo.presentIndex.value()) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPQueuePriorities(&priority);
        queueCreateInfo.setQueueCount(1);
        queueCreateInfo.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
        queueInfos.push_back(queueCreateInfo);
    } else {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPQueuePriorities(&priority);
        queueCreateInfo.setQueueCount(1);
        queueCreateInfo.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
        queueInfos.push_back(queueCreateInfo);

        queueCreateInfo.setQueueFamilyIndex(queueInfo.presentIndex.value());
        queueInfos.push_back(queueCreateInfo);
    }
    deviceCreateInfo.setQueueCreateInfos(queueInfos);

    return phyDevice.createDevice(deviceCreateInfo);
}

void Context::queryQueueInfo(vk::SurfaceKHR surface) {
    auto queueProps = phyDevice.getQueueFamilyProperties();
    for (int i = 0; i < queueProps.size(); i++) {
        if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            queueInfo.graphicsIndex = i;
        }

        if (phyDevice.getSurfaceSupportKHR(i, surface)) {
            queueInfo.presentIndex = i;
        }

        if (queueInfo.graphicsIndex.has_value() &&
            queueInfo.presentIndex.has_value()) {
            break;
        }
    }
}

void Context::initSwapchain(int windowWidth, int windowHeight) {
    swapchain = std::make_unique<Swapchain>(surface_, windowWidth, windowHeight);
}

void Context::initRenderProcess() {
    renderProcess = std::make_unique<RenderProcess>();
}

void Context::initGraphicsPipeline() {
    auto vertexSource = ReadWholeFile("./shader/vert.spv");
    auto fragSource = ReadWholeFile("./shader/frag.spv");
    renderProcess->RecreateGraphicsPipeline(vertexSource, fragSource);
}

void Context::initCommandPool() {
    commandManager = std::make_unique<CommandManager>();
}

Context::~Context() {
    commandManager.reset();
    renderProcess.reset();
    swapchain.reset();
    device.destroy();
    instance.destroy();
}

}
