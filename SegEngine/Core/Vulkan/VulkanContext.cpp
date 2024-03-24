#include "pch.h"
#include "VulkanContext.hpp"
#include "Core/Base/Input.hpp"
#include "Vulkan_rhi.hpp"

//Imgui Context
#include "imgui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>
#include "SDL_vulkan.h"
#include "SDL.h"

namespace Sego{
VulkanContext* VulkanContext::instance_ = nullptr;

VulkanContext& VulkanContext::Instance() {
    return *instance_;
}

VulkanContext::VulkanContext(SDL_Window* windowHandle){
    windowHandle_ = windowHandle;
    instance_ = this;
}

void VulkanContext::Init(){
    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(windowHandle_, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(windowHandle_, &count, extensions.data());
    auto[w,h] = Input::GetWindowSize();
    width_ = w;
    height_ = h;
    
    VulkanRhi::Init(extensions,
    [&](VkInstance instance){
        VkSurfaceKHR surface;
        SDL_Vulkan_CreateSurface(windowHandle_, instance, &surface);
        return surface;
    }, width_, height_);
}


void VulkanContext::SwapBuffers(){
   
}

VulkanContext::~VulkanContext(){
    VulkanRhi::Instance().destory();
}

bool VulkanContext::RebuildSwapChain(){
    VulkanRhi::Instance().IsResized();
    return true;
}


}
