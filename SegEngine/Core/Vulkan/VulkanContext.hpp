#pragma once

#include "Renderer/GraphicsContext.hpp"
class SDL_Window;

namespace Sego{
    class VulkanContext : public GraphicsContext{
    public:
        static VulkanContext& Instance();

        VulkanContext(SDL_Window* windowHandle);
        ~VulkanContext();
        virtual void Init() override;
        virtual void SwapBuffers() override;
        virtual bool RebuildSwapChain() override;
        SDL_Window* GetWindowHandle() { return windowHandle_; }

    private:
        static VulkanContext* instance_;
        
        SDL_Window* windowHandle_;
        uint32_t width_, height_;
        
    };
}
