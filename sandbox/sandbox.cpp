#include "SDL.h"
#include "SDL_vulkan.h"
#include <iostream>
#include <vector>

#include "toy2d/toy2d.hpp"
constexpr int WIDTH = 1024;
constexpr int HEIGHT = 720;


int main(int argc,char** argv){

   
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("sandbox",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT,
                                          SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN);
    if (!window) {
        SDL_Log("create window failed");
        exit(2);
    }
    bool shouldClose = false;
    SDL_Event event;
    
    unsigned int extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window,&extensionCount,nullptr);
    std::vector<const char*> extensions(extensionCount);
    SDL_Vulkan_GetInstanceExtensions(window,&extensionCount,extensions.data());
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//add debug extension

    toy2d::Init(extensions,[&](vk::Instance instance){
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window,instance,&surface)) {
            throw std::runtime_error("create surface failed");
        }
        return surface;
    },WIDTH,HEIGHT);
    
    while (!shouldClose) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
        }
    }

    toy2d::Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}






