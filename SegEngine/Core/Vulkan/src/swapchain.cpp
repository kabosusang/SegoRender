#include "../include/swapchain.hpp"
#include "../include/context.hpp"
#include <limits>

namespace Sego {

Swapchain::Swapchain(vk::SurfaceKHR surface, int windowWidth, int windowHeight): surface(surface) {
    querySurfaceInfo(windowWidth, windowHeight);
    swapchain = createSwapchain();
    createImageAndViews();
}

Swapchain::~Swapchain() {
    auto& ctx = Context::Instance();
    for (auto& img : SwapchainImagesAview_) {
        ctx.device.destroyImageView(img.view);
    }

    ctx.device.destroySwapchainKHR(swapchain);
    ctx.instance.destroySurfaceKHR(surface);
}


void Swapchain::querySurfaceInfo(int windowWidth, int windowHeight) {
    surfaceInfo_.format = querySurfaceeFormat();
    auto capability = Context::Instance().phyDevice.getSurfaceCapabilitiesKHR(surface);
    surfaceInfo_.count = std::clamp(capability.minImageCount + 1,
                                    capability.minImageCount, capability.maxImageCount);
    surfaceInfo_.transform = capability.currentTransform;
    surfaceInfo_.extent = querySurfaceExtent(capability, windowWidth, windowHeight);
    surfaceInfo_.presentMode = chooseSwapPresentMode();
}
vk::PresentModeKHR Swapchain::chooseSwapPresentMode() {
    auto presentModes = Context::Instance().phyDevice.getSurfacePresentModesKHR(surface);
    for (auto& mode : presentModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR Swapchain::querySurfaceeFormat() {
    auto formats = Context::Instance().phyDevice.getSurfaceFormatsKHR(surface);
    for (auto& format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

vk::Extent2D Swapchain::querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windowHeight) {
    if (capability.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capability.currentExtent;
    } else {
        auto extent = vk::Extent2D{
            static_cast<uint32_t>(windowWidth),
            static_cast<uint32_t>(windowHeight)
        };

        extent.width = std::clamp(extent.width, capability.minImageExtent.width, capability.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capability.minImageExtent.height, capability.maxImageExtent.height);
        return extent;
    }
}

vk::SwapchainKHR Swapchain::createSwapchain() {
    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setClipped(true)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setImageExtent(surfaceInfo_.extent)
              .setImageColorSpace(surfaceInfo_.format.colorSpace)
              .setImageFormat(surfaceInfo_.format.format)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setMinImageCount(surfaceInfo_.count)
              .setImageArrayLayers(1)
              .setPresentMode(surfaceInfo_.presentMode)
              .setPreTransform(surfaceInfo_.transform)
              .setSurface(surface);

    auto& ctx = Context::Instance();
    if (ctx.queueInfo.graphicsIndex.value() == ctx.queueInfo.presentIndex.value()) {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        std::array queueIndices = {ctx.queueInfo.graphicsIndex.value(), ctx.queueInfo.presentIndex.value()};
        createInfo.setQueueFamilyIndices(queueIndices);
    }

    return ctx.device.createSwapchainKHR(createInfo);
}

void Swapchain::createImageAndViews() {
    auto& ctx = Context::Instance();
    auto images = ctx.device.getSwapchainImagesKHR(swapchain);
    for (auto& image : images) {
        Image img;
        img.image = image;
        vk::ImageViewCreateInfo viewCreateInfo;
        vk::ImageSubresourceRange range;
        range.setBaseArrayLayer(0)
             .setBaseMipLevel(0)
             .setLayerCount(1)
             .setLevelCount(1)
             .setAspectMask(vk::ImageAspectFlagBits::eColor);
        viewCreateInfo.setImage(image)
                      .setFormat(surfaceInfo_.format.format)
                      .setViewType(vk::ImageViewType::e2D)
                      .setSubresourceRange(range)
                      .setComponents(vk::ComponentMapping{});
        img.view = ctx.device.createImageView(viewCreateInfo);
        this->SwapchainImagesAview_.push_back(img);
    }
}

void Swapchain::cleanupSwapChain() {
    auto& ctx = Context::Instance();

    for (auto& img : SwapchainImagesAview_) {
        ctx.device.destroyImageView(img.view,nullptr);
    }
    SwapchainImagesAview_.clear();
    ctx.device.destroySwapchainKHR(swapchain,nullptr);
}

void Swapchain::recreateSwapChain(uint32_t width, uint32_t height){
    auto& ctx = Context::Instance();
    ctx.device.waitIdle();
    cleanupSwapChain();
    querySurfaceInfo(width, height);
    swapchain = createSwapchain();
    createImageAndViews();
}


}
