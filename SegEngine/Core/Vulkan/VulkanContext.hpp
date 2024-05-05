#pragma once
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/Render.hpp"

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

        auto& GetRenderer() { return render_; }

    private:
        static VulkanContext* instance_;
        //Render
        std::shared_ptr<class Renderer> render_;

        SDL_Window* windowHandle_;
        uint32_t width_, height_;
        
    };
}
