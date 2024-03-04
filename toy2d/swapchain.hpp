#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d{

class SwapChain final{
public:
    vk::SwapchainKHR swapChain;

    SwapChain(int w,int h);
    ~SwapChain();

    struct SwapchainInfo{
        vk::Extent2D imageExtent;
        uint32_t imageCount;
        vk::SurfaceFormatKHR format;
        vk::SurfaceTransformFlagsKHR transform; //image transform
        vk::PresentModeKHR present;

    };
    SwapchainInfo info{};
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;

    void queryInfo(int w,int h);
    void getImages();
    void createImageViews();

};
}