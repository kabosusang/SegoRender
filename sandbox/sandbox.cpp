#include "SegEngine.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>
#include "../editor/Base/base.hpp"

const int WIDTH = 1024;
const int HEIGHT = 720;

int main(int argc, char** argv) {
    Sego::SGwindow::Init(WIDTH,HEIGHT);
    auto window = Sego::SGwindow::Instance()->getWindow();
 
    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
    
    
    Sego::Init(extensions,
        [&](VkInstance instance){
            VkSurfaceKHR surface;
            SDL_Vulkan_CreateSurface(window, instance, &surface);
            return surface;
        }, WIDTH, HEIGHT);
        
    auto renderer = Sego::GetRenderer();

    bool shouldClose = false;
    SDL_Event event;

    float x = 100, y = 100;
    bool stop_rendering = false;

    Sego::Texture* texture1 = Sego::LoadTexture("resources/role.png");
    Sego::Texture* texture2 = Sego::LoadTexture("resources/texture.jpg");

    renderer->SetDrawColor(Sego::Color{1, 1, 1});
    while (!shouldClose) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
            if(event.type == SDL_WINDOW_RESIZABLE){
                renderer->SetFrameBufferSize(event.window.data1, event.window.data2);
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                    stop_rendering = true;
                }
                if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
                    stop_rendering = false;
                }
            }
            //send SDL event to imgui for handling
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        //start imgui new frame
        Sego::ImGuiBase::ImGuiRenderBase();
        Sego::ImGuiBase::Render();
        Sego::ImGuiBase::ImGuiRenderEndBase();

        //render the scene
        renderer->StartRender();
        renderer->DrawTexture(Sego::Rect{Sego::Vec{x, y}, Sego::Size{200, 300}}, *texture1);
        renderer->DrawTexture(Sego::Rect{Sego::Vec{500, 100}, Sego::Size{200, 300}}, *texture2);
        renderer->EndRender();

    }

    Sego::DestroyTexture(texture1);
    Sego::DestroyTexture(texture2);
    Sego::Quit();

    return 0;
}

