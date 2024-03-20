#include "pch.h"
#include "VulkanContext.hpp"
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


    VulkanRhi::Init(extensions,
    [&](VkInstance instance){
        VkSurfaceKHR surface;
        SDL_Vulkan_CreateSurface(windowHandle_, instance, &surface);
        return surface;
    }, width_, height_);
    
    
}


void VulkanContext::SwapBuffers(){
    VulkanRhi::Instance().render();
}

VulkanContext::~VulkanContext(){
    VulkanRhi::Instance().destory();
}



}
