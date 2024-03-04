#include "toy2d/swapchain.hpp"
#include "toy2d/context.hpp"

#include <array>

namespace toy2d{
SwapChain::SwapChain(int w,int h){
    queryInfo(w,h);//query swaochain info

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.setClipped(true)
              .setImageArrayLayers(1)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setSurface(Context::GetInstance().surface)
              .setImageColorSpace(info.format.colorSpace)
              .setImageFormat(info.format.format)
              .setImageExtent(info.imageExtent)
              .setMinImageCount(info.imageCount)
              .setPresentMode(info.present);
    auto& queueIndicescs = Context::GetInstance().queueFamilyIndices;
    if(queueIndicescs.graphicsQueue.value() == queueIndicescs.presentQueue.value()){
        createInfo.setQueueFamilyIndices(queueIndicescs.graphicsQueue.value())
                  .setImageSharingMode(vk::SharingMode::eExclusive);
    }else {
        std::array indices = {queueIndicescs.graphicsQueue.value(),queueIndicescs.presentQueue.value()};
        createInfo.setQueueFamilyIndices(indices)
                  .setImageSharingMode(vk::SharingMode::eConcurrent); //并行
    }

    swapChain = Context::GetInstance().device.createSwapchainKHR(createInfo);
}

SwapChain::~SwapChain(){
    for(auto imageView : imageViews){
        Context::GetInstance().device.destroyImageView(imageView);
    }
    Context::GetInstance().device.destroySwapchainKHR(swapChain);
}

void SwapChain::queryInfo(int w,int h){
    auto& phyDevice = Context::GetInstance().physicalDevice;
    auto& surface = Context::GetInstance().surface;
    auto formats = phyDevice.getSurfaceFormatsKHR(surface);
    info.format = formats[0];
    for(const auto& format : formats){
        if(format.format == vk::Format::eR8G8B8A8Srgb && 
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear){
            info.format = format;
            break;
        }
    }
    auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
    info.imageCount = std::clamp<uint32_t>(2,capabilities.minImageCount,capabilities.maxImageCount ); //double buffering

    info.imageExtent.width = std::clamp<uint32_t>(w,capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    info.imageExtent.height = std::clamp<uint32_t>(h,capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    info.transform = capabilities.currentTransform;

    auto presentModes = phyDevice.getSurfacePresentModesKHR(surface);
    info.present = vk::PresentModeKHR::eFifo;
    for(const auto& present : presentModes){
        if(present == vk::PresentModeKHR::eMailbox){
            info.present = present;
            break;
        }
    }

}
void SwapChain::getImages(){
    images = Context::GetInstance().device.getSwapchainImagesKHR(swapChain);
}

void SwapChain::createImageViews(){
    imageViews.resize(images.size());
    for(int i = 0; i < images.size();i++){
        vk::ImageViewCreateInfo createInfo{};
        vk::ComponentMapping mapping;
        //mapping.setA(vk::ComponentSwizzle::eIdentity) // color mapping
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0)
             .setLevelCount(1)
             .setBaseArrayLayer(0)
             .setLayerCount(1)
             .setAspectMask(vk::ImageAspectFlagBits::eColor);


        createInfo.setImage(images[i])
                  .setViewType(vk::ImageViewType::e2D)
                  .setComponents(mapping)
                  .setFormat(info.format.format)
                  .setSubresourceRange(range);

        imageViews[i] = Context::GetInstance().device.createImageView(createInfo);
    }
}


}
