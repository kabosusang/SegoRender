#define VMA_IMPLEMENTATION
#include "../include/context.hpp"

namespace Sego {
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

    createVmaAllocator(); //create Allocator

    


}

vk::Instance Context::createInstance(std::vector<const char*>& extensions) {
    vk::InstanceCreateInfo info; 

    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_API_VERSION_1_3);
    info.setPApplicationInfo(&appInfo)
        .setEnabledExtensionCount(extensions.size())
        .setPpEnabledExtensionNames(extensions.data());
    
    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
    info.setPEnabledLayerNames(layers);

    return vk::createInstance(info);
}

void Context::createVmaAllocator(){

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.instance = instance;
    allocatorInfo.physicalDevice = phyDevice;
    allocatorInfo.device = device;
    allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

	VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator_);
}

bool Context::isDeviceSuitable(vk::PhysicalDevice device){

    return true;
}


vk::PhysicalDevice Context::pickupPhysicalDevice() {
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.size() == 0) {
        std::cout << "you don't have suitable device to support vulkan" << std::endl;
        exit(1);
    }

    for(auto device : devices){
        if(isDeviceSuitable(device)){
            device.getProperties(&physical_deviceproperties_);
        }
    }

    return devices[0];
}

vk::Device Context::createDevice(vk::SurfaceKHR surface) {

    vk::DeviceCreateInfo deviceCreateInfo;
    queryQueueInfo(surface);
    std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};
    deviceCreateInfo.setPEnabledExtensionNames(extensions);

    vk::PhysicalDeviceFeatures features;
    features.setSamplerAnisotropy(true);

    deviceCreateInfo.setPEnabledFeatures(&features);
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
    shaderManager = std::make_unique<ShaderManager>();
}


Context::~Context() {
    swapchain.reset();
    device.destroy();
    instance.destroy();
}


}
