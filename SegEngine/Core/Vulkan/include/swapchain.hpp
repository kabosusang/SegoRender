#pragma once
#include "vulkan/vulkan.hpp"
#include "../Vulkantool.hpp"

namespace Sego {

class Swapchain final {
public:
    struct Image {
        vk::Image image;
        vk::ImageView view;
    };
    
    vk::SurfaceKHR surface = nullptr;
    vk::SwapchainKHR swapchain = nullptr;
    std::vector<Image> SwapchainImagesAview_;
    
    
    const auto& GetExtent() const { return surfaceInfo_.extent; }
    const auto& GetFormat() const { return surfaceInfo_.format; }
    const auto& GetDepthFormat() const { return depthFormat_; }

    Swapchain(vk::SurfaceKHR, int windowWidth, int windowHeight);
    ~Swapchain();

    void recreateSwapChain(uint32_t width, uint32_t height);

private:
    struct SurfaceInfo {
        vk::SurfaceFormatKHR format;
        vk::Extent2D extent;
        std::uint32_t count;
        vk::SurfaceTransformFlagBitsKHR transform;
        vk::PresentModeKHR presentMode;
    } surfaceInfo_;
    //Depth Format
    vk::Format depthFormat_;
    vk::Format getProperImageFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);


    vk::SwapchainKHR createSwapchain();
    void cleanupSwapChain();
    void querySurfaceInfo(int windowWidth, int windowHeight);
    vk::SurfaceFormatKHR querySurfaceeFormat();
    vk::Extent2D querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windowHeight);
    vk::PresentModeKHR chooseSwapPresentMode();
    
    //Resource
    void createImageAndViews();
    void RecreateImageview();
};

}
