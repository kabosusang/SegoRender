#include "toy2d.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"
#include <SDL_video.h>

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("sandbox",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1024, 720,
                                          SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN);
    if (!window) {
        SDL_Log("create window failed");
        exit(2);
    }

    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
    extensions.push_back("VK_EXT_debug_utils");
    
    Sego::Init(extensions,
        [&](VkInstance instance){
            VkSurfaceKHR surface;
            SDL_Vulkan_CreateSurface(window, instance, &surface);
            return surface;
        }, 1024, 720);
    auto renderer = Sego::GetRenderer();

    bool shouldClose = false;
    SDL_Event event;

    float x = 100, y = 100;

    Sego::Texture* texture1 = Sego::LoadTexture("resources/role.png");
    Sego::Texture* texture2 = Sego::LoadTexture("resources/texture.jpg");

    renderer->SetDrawColor(Sego::Color{1, 1, 1});
    while (!shouldClose) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_a) {
                    x -= 10;
                }
                if (event.key.keysym.sym == SDLK_d) {
                    x += 10;
                }
                if (event.key.keysym.sym == SDLK_w) {
                    y -= 10;
                }
                if (event.key.keysym.sym == SDLK_s) {
                    y += 10;
                }
            }
        }

        renderer->StartRender();
        renderer->DrawTexture(Sego::Rect{Sego::Vec{x, y}, Sego::Size{200, 300}}, *texture1);
        renderer->DrawTexture(Sego::Rect{Sego::Vec{500, 100}, Sego::Size{200, 300}}, *texture2);
        renderer->EndRender();
    
    }

    Sego::DestroyTexture(texture1);
    Sego::DestroyTexture(texture2);

    Sego::Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
